#include "VTFEImport.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "../libs/stb/stb_image.h"
#include "../src/ImageSettingsWidget.h"
#include "../src/MainWindow.h"
// #include "Options.h"
#include "../src/flagsandformats.hpp"
#include "../src/supported_formats/TiffSupport.h"
#include "kvpp/kvpp.h"

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFuture>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressDialog>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <cmath>

VTFEImport::VTFEImport( QWidget *pParent, const QString &filePath, bool &hasData ) :
	QDialog( pParent )
{
	hasData = true;

	addImage( filePath );
	while ( !importThreads.isEmpty() )
		continue;

	if ( imageList.empty() )
	{
		hasData = false;
		return;
	}

	SetDefaults();

	InitializeWidgets();

	pGeneralTab->pFormatCombo->setCurrentIndex( pGeneralTab->pFormatCombo->findData( static_cast<uint32_t>( grabFirst()->getFormat() ) ) );
}

VTFEImport::VTFEImport( QWidget *pParent, const QStringList &filePaths, bool &hasData ) :
	QDialog( pParent )
{
	hasData = true;

	QProgressDialog *progress = new QProgressDialog( "", "Cancel", 0, filePaths.count() );
	progress->setMinimumDuration( 0 );
	progress->setWindowTitle( "Opening Image Files" );
	progress->setLabelText( "Processing..." );
	progress->setWindowModality( Qt::WindowModal );

	for ( int i = 0; i < filePaths.count(); i++ )
	{
		addImage( filePaths[i] );
		progress->setValue( i + 1 );
	}

	while ( !importThreads.isEmpty() )
		continue;

	if ( imageList.empty() )
	{
		hasData = false;
		return;
	}

	SetDefaults();

	InitializeWidgets();

	pGeneralTab->pFormatCombo->setCurrentIndex( pGeneralTab->pFormatCombo->findData( static_cast<uint32_t>( grabFirst()->getFormat() ) ) );
	pGeneralTab->pAlphaDetectedFormatCombo->setCurrentIndex( pGeneralTab->pAlphaDetectedFormatCombo->findData( static_cast<uint32_t>( grabFirst()->getFormat() ) ) );
}

void VTFEImport::SetDefaults()
{
	this->setWindowTitle( tr( "VTF Options" ) );
}

std::unique_ptr<vtfpp::VTF> VTFEImport::GenerateVTF( VTFErrorType &err )
{
	if ( imageList.empty() )
	{
		err = VTFErrorType::NO_DATA;
		return nullptr;
	}

	auto destinationFormat = static_cast<vtfpp::ImageFormat>( vtfpp::ImageFormatDetails::alpha( grabFirst()->getFormat() ) == 0 ? pGeneralTab->pFormatCombo->currentData().toInt() : pGeneralTab->pAlphaDetectedFormatCombo->currentData().toInt() );
	vtfpp::VTF::CreationOptions options;
	options.outputFormat = destinationFormat;
	options.version = pAdvancedTab->pVtfVersionBox->currentData().toInt();
	options.widthResizeMethod = static_cast<vtfpp::ImageConversion::ResizeMethod>( pGeneralTab->pResizeMethodCombo->currentData().toInt() );
	options.heightResizeMethod = static_cast<vtfpp::ImageConversion::ResizeMethod>( pGeneralTab->pResizeMethodCombo->currentData().toInt() );
	options.flags = vtfImageFlags;
	options.flags |= pGeneralTab->pSRGBCheckbox->isChecked() ? vtfpp::VTF::FLAG_V5_SRGB : 0;
	options.computeReflectivity = pAdvancedTab->pComputeReflectivityCheckBox->isChecked();
	options.computeThumbnail = ( pAdvancedTab->pGenerateThumbnailCheckBox->isEnabled() && pAdvancedTab->pGenerateThumbnailCheckBox->isChecked() );
	options.filter = static_cast<vtfpp::ImageConversion::ResizeFilter>( pGeneralTab->pResizeFilterCombo->currentData().toInt() );
	options.computeMips = false;
#ifdef CHAOS_INITIATIVE
	if ( pAdvancedTab->pAuxCompressionBox->isEnabled() && pAdvancedTab->pAuxCompressionBox->isChecked() )
		options.compressionLevel = ( pAdvancedTab->pAuxCompressionLevelBox->currentData().toInt() );
#endif

	int frames = imageList.size(); // pGeneralTab->pTypeCombo->currentIndex() == 0 ? imageList.size() - 1 : 0;
	int faces = 0;				   // pGeneralTab->pTypeCombo->currentIndex() == 1 ? imageList.size() - 1 : 0;
	int slices = 0;				   // pGeneralTab->pTypeCombo->currentIndex() == 2 ? imageList.size() - 1 : 0;

	options.initialFrameCount = frames;
	auto vFile = vtfpp::VTF::create( imageList[0]->getFormat(), imageList[0]->getWidth(), imageList[0]->getHeight(), options );

	err = VTFErrorType::SUCCESS;
	auto testThreads = imageList;

	QProgressDialog *progress = new QProgressDialog( "", "Cancel", 0, imageList.size() );
	progress->setMinimumDuration( 0 );
	progress->setWindowTitle( "Inserting Image Data" );
	progress->setLabelText( "Processing Iamges" );
	progress->setWindowModality( Qt::WindowModal );

	QMap<int, QThread *> threads {};
	int processed = imageList.size();
	for ( int i = 0; i < imageList.size(); i++ )
	{
		auto lamb = [this, i, &vFile, &processed, &threads, options]() mutable
		{
			bool imageDataSet = vFile.setImage( imageList[i]->getData(), imageList[i]->getFormat(), imageList[i]->getWidth(), imageList[i]->getHeight(), options.filter, 0, i, 0, 0 );
			threads.remove( i );
			processed--;
		};

		threads[i] = QThread::create( lamb );
	}

	for ( const auto &thrd : threads )
	{
		thrd->start();
		QApplication::processEvents();
	}

	for ( ; !threads.empty(); )
	{
		progress->setValue( progress->maximum() - processed );
		QApplication::processEvents();
	}

	progress->setValue( imageList.size() );

	delete progress;
	if ( this->pGeneralTab->pGenerateMipmapsCheckbox->isChecked() )
	{
		progress = new QProgressDialog();
		progress->setWindowTitle( "Generating Mipmaps" );
		progress->setLabelText( "Mipmap generation in process" );
		progress->setWindowModality( Qt::WindowModal );
		progress->setMaximum( 1 );
		progress->setMinimum( 0 );
		progress->open();

		progress->setValue( 0 );
		vFile.computeMips( options.filter );
		progress->setValue( 1 );
		delete progress;
	}

	if ( !vFile.hasImageData() )
	{
		err = VTFErrorType::INVALID_IMAGE;
		return nullptr;
	}

	if ( vFile.getVersion() > 2 )
	{
		if ( pResourceTab->pLodControlResourceCheckBox->isChecked() )
		{
			vFile.setLODResource( pResourceTab->pControlResourceCrampUBox->value(), pResourceTab->pControlResourceCrampVBox->value() );
		}

		if ( pResourceTab->pCreateInformationResourceCheckBox->isChecked() )
		{
			auto pVMTFile = kvpp::KV1Writer {};
			auto root = pVMTFile.addChild( "Information" );

			if ( pResourceTab->pInformationResourceAuthor->text().length() > 0 )
			{
				root.addChild( "Author", pResourceTab->pInformationResourceAuthor->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceContact->text().length() > 0 )
			{
				root.addChild( "Contact", pResourceTab->pInformationResouceContact->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceVersion->text().length() > 0 )
			{
				root.addChild( "Version", pResourceTab->pInformationResouceVersion->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceModification->text().length() > 0 )
			{
				root.addChild( "Modification", pResourceTab->pInformationResouceModification->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceDescription->text().length() > 0 )
			{
				root.addChild( "Description", pResourceTab->pInformationResouceDescription->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceComments->text().length() > 0 )
			{
				root.addChild( "Comments", pResourceTab->pInformationResouceComments->text().toUtf8().constData() );
			}
			auto kvd = pVMTFile.bake();
			vFile.setKeyValuesDataResource( kvd );
		}
	}

	err = VTFErrorType::SUCCESS;
	return std::make_unique<vtfpp::VTF>( vFile.bake() );
}

void VTFEImport::addImage( const QString &qString )
{
	int currentSize = threadsImported;
	while ( importThreads.size() > 12 )
		continue;
	if ( qString.endsWith( ".gif" ) )
	{
		vtfpp::ImageFormat inputFormat;
		int inputWidth, inputHeight, inputFrameCount;
		auto imageData_ = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( qString.toStdString() ), inputFormat, inputWidth, inputHeight, inputFrameCount );

		if ( inputFormat == vtfpp::ImageFormat::EMPTY || !inputWidth || !inputHeight || !inputFrameCount )
			return;

		const auto frameSize = vtfpp::ImageFormatDetails::getDataLength( inputFormat, inputWidth, inputHeight );
		for ( int i = 0; i < inputFrameCount; i++ )
		{
			auto lamb = [currentSize, i, frameSize, imageData_, inputWidth, inputHeight, inputFormat, this]() mutable
			{
				this->imageList[currentSize + i] = ( new VTFEImageContainer( imageData_.data() + frameSize * i, inputWidth, inputHeight, inputFormat ) );
				importThreads.remove( currentSize + i );
			};

			auto thread = importThreads[currentSize + i] = QThread::create( lamb );
			thread->start();
		}
		threadsImported += inputFrameCount;

		return;
	}

	auto lamb = [this, currentSize, qString]() mutable
	{
		this->imageList[currentSize] = ( new VTFEImageContainer( qString ) );
		//		importThreads[currentSize]->deleteLater();
		importThreads.remove( currentSize );
	};

	auto thread = importThreads[currentSize] = QThread::create( lamb );
	thread->start();

	threadsImported++;
}

void VTFEImport::addImage( const QImage &iamge )
{
	int currentSize = threadsImported;
	auto lamb = [this, currentSize, iamge]() mutable
	{
		QBuffer imageBuffer;
		QByteArray imageData;
		auto nim = iamge.convertToFormat( QImage::Format_RGBA8888 );

		this->imageList[currentSize] = ( new VTFEImageContainer( reinterpret_cast<const std::byte *>( nim.constBits() ), nim.width(), nim.height(), vtfpp::ImageFormat::RGBA8888 ) );
		//		importThreads[currentSize]->deleteLater();
		importThreads.remove( currentSize );
	};

	auto thread = importThreads[currentSize] = QThread::create( lamb );
	thread->start();

	threadsImported++;
}

VTFEImport::VTFEImport( QWidget *pParent ) :
	QDialog( pParent )
{
	this->setWindowTitle( tr( "VTF Options" ) );
}

void VTFEImport::InitializeWidgets()
{
	auto vBLayout = new QGridLayout( this );
	auto widget = new QTabWidget( this );

	pImageProcessor = new ImageProcessor( this );
	pGeneralTab = new GeneralTab( this );
	pAdvancedTab = new AdvancedTab( this );
	pResourceTab = new ResourceTab( this );

	widget->addTab( pImageProcessor, tr( "Images" ) );
	widget->addTab( pGeneralTab, tr( "General" ) );
	widget->addTab( pAdvancedTab, tr( "Advanced" ) );
	widget->addTab( pResourceTab, tr( "Resource" ) );
	widget->setCurrentIndex( 1 ); // We wanna start on General.
	vBLayout->addWidget( widget, 0, 0, 1, 2 );
	auto pPreviewButton = new QPushButton( this );
	pPreviewButton->setText( tr( "Preview" ) );
	connect(
		pPreviewButton, &QPushButton::pressed,
		[&]()
		{
			auto dialog = new QDialog();
			auto vRLayout = new QGridLayout( dialog );

			auto scrollArea = new ui::ZoomScrollArea( dialog );
			auto vIVW = new ImageViewWidget();
			vIVW->setMinimumSize( 512, 512 );
			//			scrollArea->setWidget( vIVW );
			auto vISW = new ImageSettingsWidget( vIVW, dialog );
			vRLayout->addWidget( vISW, 0, 0 );
			vRLayout->addWidget( vIVW, 0, 1, Qt::AlignCenter );

			connect( scrollArea, &ui::ZoomScrollArea::onScrollUp, dialog, [vIVW]
					 {
						 vIVW->zoom( 0.1 );
					 } );

			connect( scrollArea, &ui::ZoomScrollArea::onScrollDown, dialog, [vIVW]
					 {
						 vIVW->zoom( -0.1 );
					 } );

			auto pMainMenuBar = new QMenuBar( dialog );

			auto pViewMenu = pMainMenuBar->addMenu( "View" );
			auto redBox = ui::CMainWindow::createCheckableAction( "Red", pViewMenu );
			auto greenBox = ui::CMainWindow::createCheckableAction( "Green", pViewMenu );
			auto blueBox = ui::CMainWindow::createCheckableAction( "Blue", pViewMenu );
			auto alphaBox = ui::CMainWindow::createCheckableAction( "Alpha", pViewMenu );

			connect( redBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_red( checked );
					 } );
			connect( greenBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_green( checked );
					 } );
			connect( blueBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_blue( checked );
					 } );
			connect( alphaBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_alpha( checked );
					 } );

			pViewMenu->addAction( redBox );
			pViewMenu->addAction( greenBox );
			pViewMenu->addAction( blueBox );
			pViewMenu->addAction( alphaBox );

			vRLayout->setMenuBar( pMainMenuBar );

			VTFErrorType err;
			auto vtfFile = GenerateVTF( err );
			if ( err == SUCCESS )
			{
				vIVW->set_vtf( vtfFile.get() );
				vISW->set_vtf( { vtfFile.get() } );
			}
			scrollArea->resize( dialog->width() + vISW->width(), dialog->height() );
			dialog->setAttribute( Qt::WA_DeleteOnClose );
			dialog->exec();
		} );
	vBLayout->addWidget( pPreviewButton, 1, 0, Qt::AlignLeft );

	auto blayoutBox = new QDialogButtonBox( this );
	this->acceptButton = blayoutBox->addButton( "Accept", QDialogButtonBox::AcceptRole );
	auto cancelled = blayoutBox->addButton( "Cancel", QDialogButtonBox::RejectRole );
	connect(
		acceptButton, &QPushButton::pressed,
		[this]
		{
			isCancelled = false;
			close();
		} );
	connect(
		cancelled, &QPushButton::pressed,
		[this]
		{
			close();
		} );
	vBLayout->addWidget( blayoutBox, 1, 1, Qt::AlignRight );
}

bool VTFEImport::editVTF( vtfpp::VTF *pFile )
{
	this->editableVTF = pFile;
}

VTFEImport *VTFEImport::FromVTF( QWidget *pParent, const vtfpp::VTF *pFile )
{
	//	auto oldVTF = pFile.bake();
	//
	//	auto newVTF = new vtfpp::VTF( oldVTF );
	auto newVTF = new vtfpp::VTF( *pFile );
	auto vVTFImport = new VTFEImport( pParent );
	vVTFImport->editVTF( newVTF );
	vVTFImport->InitializeWidgets();
	vVTFImport->pAdvancedTab->pVtfVersionBox->setCurrentIndex( pFile->getVersion() );
	emit vVTFImport->pAdvancedTab->pVtfVersionBox->currentTextChanged( "7." + QString::number( pFile->getVersion() ) );
#ifdef CHAOS_INITIATIVE
	vVTFImport->pAdvancedTab->pAuxCompressionBox->setChecked( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION )->getDataAsAuxCompressionLevel() > 0 );
	emit vVTFImport->pAdvancedTab->pAuxCompressionBox->clicked( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION )->getDataAsAuxCompressionLevel() > 0 );
	if ( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION ) )
		vVTFImport->pAdvancedTab->pAuxCompressionLevelBox->setCurrentIndex( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION )->getDataAsAuxCompressionLevel() );
#endif
	vVTFImport->pGeneralTab->pGenerateMipmapsCheckbox->setChecked( pFile->getMipCount() > 0 );
	emit vVTFImport->pGeneralTab->pGenerateMipmapsCheckbox->clicked( pFile->getMipCount() > 0 );
	// vVTFImport->pGeneralTab->pTypeCombo->setCurrentIndex( type );
	//		vVTFImport->pGeneralTab->pTypeCombo->currentTextChanged( QString::number( type ) );
	vVTFImport->pGeneralTab->pFormatCombo->setCurrentIndex(
		vVTFImport->pGeneralTab->pFormatCombo->findData( static_cast<uint32_t>( pFile->getFormat() ) ) );
	vVTFImport->pGeneralTab->pAlphaDetectedFormatCombo->setCurrentIndex(
		vVTFImport->pGeneralTab->pAlphaDetectedFormatCombo->findData( static_cast<uint32_t>( pFile->getFormat() ) ) );

	auto v = pFile->getReflectivity();
	vVTFImport->pAdvancedTab->pLuminanceWeightRedBox->setValue( v[0] );
	vVTFImport->pAdvancedTab->pLuminanceWeightGreenBox->setValue( v[1] );
	vVTFImport->pAdvancedTab->pLuminanceWeightBlueBox->setValue( v[2] );

	vVTFImport->pGeneralTab->pSRGBCheckbox->setChecked( pFile->getFlags() & vtfpp::VTF::FLAG_V5_SRGB );

	vVTFImport->vtfImageFlags = static_cast<vtfpp::VTF::Flags>( pFile->getFlags() );
	//	return this;
	//
	//	auto vVTFImport = new VTFEImport( pParent );
	//
	//	int type = 0;
	//	uint32_t fImageAmount = pFile->getFrameCount();
	//	if ( pFile->getFaceCount() > fImageAmount )
	//	{
	//		fImageAmount = pFile->getFaceCount();
	//		type = 1;
	//	}
	//	if ( pFile->getSliceCount() > fImageAmount )
	//	{
	//		fImageAmount = pFile->getSliceCount();
	//		type = 2;
	//	}
	//
	//	for ( int i = 0; i < fImageAmount; i++ )
	//	{
	//		uint16_t frames = type == 0 ? i : 0;
	//		uint8_t faces = type == 1 ? i : 0;
	//		uint16_t slices = type == 2 ? i : 0;
	//
	//		auto rawSpan = pFile->getImageDataRaw( 0, frames, faces, slices );
	//
	//		vVTFImport->imageList[{ frames, faces, slices, 0 }] =
	//			new VTFEImageContainer( { rawSpan.begin(), rawSpan.end() }, pFile->getWidth(), pFile->getHeight(), pFile->getFormat() );
	//	}
	//
	//	vVTFImport->InitializeWidgets();
	//
	//	vVTFImport->pAdvancedTab->pVtfVersionBox->setCurrentIndex( pFile->getMinorVersion() );
	//	emit vVTFImport->pAdvancedTab->pVtfVersionBox->currentTextChanged( "7." + QString::number( pFile->getMinorVersion() ) );
	// #ifdef CHAOS_INITIATIVE
	//	vVTFImport->pAdvancedTab->pAuxCompressionBox->setChecked( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION )->getDataAsAuxCompressionLevel() > 0 );
	//	emit vVTFImport->pAdvancedTab->pAuxCompressionBox->clicked( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION )->getDataAsAuxCompressionLevel() > 0 );
	//	if ( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION ) )
	//		vVTFImport->pAdvancedTab->pAuxCompressionLevelBox->setCurrentIndex( pFile->getResource( vtfpp::Resource::TYPE_AUX_COMPRESSION )->getDataAsAuxCompressionLevel() );
	// #endif
	//	vVTFImport->pGeneralTab->pGenerateMipmapsCheckbox->setChecked( pFile->getMipCount() > 0 );
	//	emit vVTFImport->pGeneralTab->pGenerateMipmapsCheckbox->clicked( pFile->getMipCount() > 0 );
	//	vVTFImport->pGeneralTab->pTypeCombo->setCurrentIndex( type );
	//	vVTFImport->pGeneralTab->pTypeCombo->currentTextChanged( QString::number( type ) );
	//	vVTFImport->pGeneralTab->pFormatCombo->setCurrentIndex(
	//		vVTFImport->pGeneralTab->pFormatCombo->findData( static_cast<uint32_t>( pFile->getFormat() ) ) );
	//	vVTFImport->pGeneralTab->pAlphaDetectedFormatCombo->setCurrentIndex(
	//		vVTFImport->pGeneralTab->pAlphaDetectedFormatCombo->findData( static_cast<uint32_t>( pFile->getFormat() ) ) );
	//
	//	auto v = pFile->getReflectivity();
	//	vVTFImport->pAdvancedTab->pLuminanceWeightRedBox->setValue( v[0] );
	//	vVTFImport->pAdvancedTab->pLuminanceWeightGreenBox->setValue( v[1] );
	//	vVTFImport->pAdvancedTab->pLuminanceWeightBlueBox->setValue( v[2] );
	//
	//	vVTFImport->pGeneralTab->pSRGBCheckbox->setChecked( pFile->getFlags() & vtfpp::VTF::FLAG_SRGB );
	//
	//	vVTFImport->vtfImageFlags = pFile->getFlags();
	//
	//	return vVTFImport;
}

VTFEImport *VTFEImport::Standalone( QWidget *pParent )
{
	auto vVTFImport = new VTFEImport( pParent );

	vVTFImport->InitializeWidgets();

	vVTFImport->SetDefaults();

	return vVTFImport;
}

VTFEImport *VTFEImport::FromFont( QWidget *pParent, std::byte *buff, int width, int height )
{
	auto vVTFImport = new VTFEImport( pParent );
	vVTFImport->imageList[0] = new VTFEImageContainer(
		buff, width, height, vtfpp::ImageFormat::RGBA8888 );

	vVTFImport->InitializeWidgets();

	vVTFImport->SetDefaults();

	vVTFImport->pGeneralTab->vBoxResize->setDisabled( true );

	return vVTFImport;
}
void VTFEImport::clearImageList()
{
	for ( auto image : imageList )
	{
		delete image;
	}

	imageList.clear();
}

GeneralTab::GeneralTab( VTFEImport *parent ) :
	QDialog( parent )
{
	pMainLayout = new QGridLayout( this );
	pMainLayout->setAlignment( Qt::AlignTop );
	GeneralOptions();
	GeneralResize();
	GeneralMipMaps();
	GeneralCustomMipmaps();
#ifdef NORMAL_GENERATION
	GeneralNormalMap();
#endif
}

void GeneralTab::GeneralOptions()
{
	auto vBoxGeneralOptions = new QGroupBox( tr( "General Options" ), this );
	auto vBLayout = new QGridLayout( vBoxGeneralOptions );
	auto label1 = new QLabel();
	label1->setText( tr( "Texture Format:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pFormatCombo = new QComboBox( this );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( vtfpp::ImageFormat::P8 != fmt.format )
			pFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}
	vBLayout->addWidget( pFormatCombo, 0, 1, Qt::AlignRight );

	auto alphaDetectedLabel = new QLabel();
	alphaDetectedLabel->setText( tr( "Alpha Texture Format:" ) );
	vBLayout->addWidget( alphaDetectedLabel, 1, 0, Qt::AlignLeft );
	pAlphaDetectedFormatCombo = new QComboBox( this );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( vtfpp::ImageFormat::P8 != fmt.format )
			pAlphaDetectedFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}

	vBLayout->addWidget( pAlphaDetectedFormatCombo, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel();
	label2->setText( tr( "Texture Type:" ) );
	pTypeCombo = new QComboBox( this );
	pTypeCombo->addItem( tr( "Animated Texture" ) );
	pTypeCombo->addItem( tr( "Environment Map" ) );
	pTypeCombo->addItem( tr( "Volume Texture" ) );

	auto vParent = static_cast<VTFEImport *>( this->parent() );
	connect(
		pTypeCombo, &QComboBox::currentTextChanged, this->parent(),
		[this, vParent]()
		{
			bool shouldCheck = pTypeCombo->currentIndex() == 0;
#ifdef NORMAL_GENERATION
			generateNormalMapCheckbox_->setDisabled( !shouldCheck );
			emit generateNormalMapCheckbox_->clicked( shouldCheck && generateNormalMapCheckbox_->isChecked() );
#endif
			if ( vParent )
			{
				vParent->pAdvancedTab->pGenerateSphereMapCheckBox->setDisabled( shouldCheck );
			}
		} );

	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	vBLayout->addWidget( pTypeCombo, 2, 1, Qt::AlignRight );
	pSRGBCheckbox = new QCheckBox( "sRGB Color Space", this );
	vBLayout->addWidget( pSRGBCheckbox, 3, 0, 1, 2, Qt::AlignLeft );
	pMainLayout->addWidget( vBoxGeneralOptions, 0, 0 );
}

void GeneralTab::GeneralResize()
{
	vBoxResize = new QGroupBox( tr( "Resize" ), this );
	auto vBLayout = new QGridLayout( vBoxResize );
	pResizeCheckbox = new QCheckBox( this );
	pResizeCheckbox->setText( tr( "Resize" ) );
	auto parent = static_cast<VTFEImport *>( this->parent() );
	if ( !parent->imageList.empty() )
	{
		bool b1 = sourcepp::math::isPowerOf2( parent->grabFirst()->getWidth() );
		bool b2 = sourcepp::math::isPowerOf2( parent->grabFirst()->getHeight() );
		pResizeCheckbox->setChecked( !( b1 && b2 ) );
		pResizeCheckbox->setDisabled( !( b1 && b2 ) );
		if ( !( b1 && b2 ) )
			pResizeCheckbox->setToolTip( tr( "Image is not in power of 2 and therefore needs resizing." ) );
	}
	else
	{
		pResizeCheckbox->setChecked( true );
		pResizeCheckbox->setDisabled( true );
		pResizeCheckbox->setToolTip( "Rescaling WILL be done to non power of two images This is a quirk of folder conversion." );
		auto pal = pResizeCheckbox->palette();
		pal.setColor( QPalette::WindowText, Qt::red );
		pResizeCheckbox->setPalette( pal );
	}
	vBLayout->addWidget( pResizeCheckbox, 0, 0, Qt::AlignLeft );
	auto label1 = new QLabel();
	label1->setText( tr( "Resize Method:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pResizeMethodCombo = new QComboBox( this );
	pResizeMethodCombo->addItem( tr( "Nearest Power Of 2" ), (int)vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_NEAREST );
	pResizeMethodCombo->addItem( tr( "Biggest Power Of 2" ), (int)vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER );
	pResizeMethodCombo->addItem( tr( "Smallest Power Of 2" ), (int)vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_SMALLER );
	pResizeMethodCombo->setCurrentIndex( 1 );
	vBLayout->addWidget( pResizeMethodCombo, 1, 1, Qt::AlignRight );
	auto label2 = new QLabel();
	label2->setText( tr( "Resize Filter:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pResizeFilterCombo = new QComboBox( this );
	pResizeFilterCombo->addItem( tr( "Default" ), (int)vtfpp::ImageConversion::ResizeFilter::DEFAULT );
	pResizeFilterCombo->addItem( tr( "Bi-linear" ), (int)vtfpp::ImageConversion::ResizeFilter::BILINEAR );
	pResizeFilterCombo->addItem( tr( "Catmullrom" ), (int)vtfpp::ImageConversion::ResizeFilter::CATMULL_ROM );
	pResizeFilterCombo->addItem( tr( "Cubic BSpline" ), (int)vtfpp::ImageConversion::ResizeFilter::CUBIC_BSPLINE );
	pResizeFilterCombo->addItem( tr( "Mitchell" ), (int)vtfpp::ImageConversion::ResizeFilter::MITCHELL );
	pResizeFilterCombo->addItem( tr( "Box" ), (int)vtfpp::ImageConversion::ResizeFilter::BOX );
	vBLayout->addWidget( pResizeFilterCombo, 2, 1, Qt::AlignRight );
	pClampCheckbox = new QCheckBox( this );
	pClampCheckbox->setText( tr( "Clamp" ) );

	vBLayout->addWidget( pClampCheckbox, 3, 0, Qt::AlignLeft );
	auto label3 = new QLabel();
	label3->setText( tr( "Maximum Width:" ) );
	pClampWidthCombo = new QComboBox( this );
	vBLayout->addWidget( label3, 4, 0, Qt::AlignLeft );
	vBLayout->addWidget( pClampWidthCombo, 4, 1, Qt::AlignRight );

	auto label4 = new QLabel();
	label4->setText( tr( "Maximum Height:" ) );
	pClampHeightCombo = new QComboBox( this );
	for ( int i = 1; i <= 4096; i *= 2 )
	{
		pClampHeightCombo->addItem( QString::number( i ), i );
		pClampWidthCombo->addItem( QString::number( i ), i );
	}

	pClampHeightCombo->setCurrentIndex( pClampHeightCombo->count() - 2 );
	pClampWidthCombo->setCurrentIndex( pClampWidthCombo->count() - 2 );

	label3->setDisabled( true );
	label4->setDisabled( true );
	pClampHeightCombo->setDisabled( true );
	pClampWidthCombo->setDisabled( true );

	connect(
		pClampCheckbox, &QCheckBox::clicked, this->parent(),
		[label3, label4, this]( bool checked )
		{
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
			pClampHeightCombo->setDisabled( !checked );
			pClampWidthCombo->setDisabled( !checked );
		} );

	if ( !pResizeCheckbox->isChecked() )
	{
		label1->setDisabled( true );
		label2->setDisabled( true );
		pResizeMethodCombo->setDisabled( true );
		pResizeFilterCombo->setDisabled( true );
		pClampCheckbox->setDisabled( true );
	}
	connect(
		pResizeCheckbox, &QCheckBox::clicked, this->parent(),
		[label1, label2, label3, label4, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			pResizeMethodCombo->setDisabled( !checked );
			pResizeFilterCombo->setDisabled( !checked );
			pClampCheckbox->setDisabled( !checked );
			bool secondChecked = pClampCheckbox->isChecked();
			label3->setDisabled( !checked || !secondChecked );
			label4->setDisabled( !checked || !secondChecked );
			pClampHeightCombo->setDisabled( !checked || !secondChecked );
			pClampWidthCombo->setDisabled( !checked || !secondChecked );
		} );

	vBLayout->addWidget( label4, 5, 0, Qt::AlignLeft );
	vBLayout->addWidget( pClampHeightCombo, 5, 1, Qt::AlignRight );
	pMainLayout->addWidget( vBoxResize, 1, 0 );
}

void GeneralTab::GeneralMipMaps()
{
	auto vBoxMipMaps = new QGroupBox( tr( "Mipmaps" ), this );
	auto vBLayout = new QGridLayout( vBoxMipMaps );
	pGenerateMipmapsCheckbox = new QCheckBox( this );
	pGenerateMipmapsCheckbox->setText( tr( "Generate Mipmaps" ) );
	vBLayout->addWidget( pGenerateMipmapsCheckbox, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel();
	label1->setText( tr( "Mipmap Filter:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );

	pMipmapFilterCombo = new QComboBox( this );
	pMipmapFilterCombo->addItem( tr( "Default" ), (int)vtfpp::ImageConversion::ResizeFilter::DEFAULT );
	pMipmapFilterCombo->addItem( tr( "Bi-linear" ), (int)vtfpp::ImageConversion::ResizeFilter::BILINEAR );
	pMipmapFilterCombo->addItem( tr( "Catmullrom" ), (int)vtfpp::ImageConversion::ResizeFilter::CATMULL_ROM );
	pMipmapFilterCombo->addItem( tr( "Cubic BSpline" ), (int)vtfpp::ImageConversion::ResizeFilter::CUBIC_BSPLINE );
	pMipmapFilterCombo->addItem( tr( "Mitchell" ), (int)vtfpp::ImageConversion::ResizeFilter::MITCHELL );
	pMipmapFilterCombo->addItem( tr( "Box" ), (int)vtfpp::ImageConversion::ResizeFilter::BOX );
	vBLayout->addWidget( pMipmapFilterCombo, 1, 1, Qt::AlignRight );

	label1->setDisabled( true );
	pMipmapFilterCombo->setDisabled( true );

	connect(
		pGenerateMipmapsCheckbox, &QCheckBox::clicked, this->parent(),
		[this, label1]( bool checked )
		{
			label1->setDisabled( !checked );
			pMipmapFilterCombo->setDisabled( !checked );
			vBoxCustomMipMaps->setDisabled( checked );
		} );

	pMainLayout->addWidget( vBoxMipMaps, 0, 1 );
}

void GeneralTab::GeneralCustomMipmaps()
{
	vBoxCustomMipMaps = new QGroupBox( tr( "Custom mipmaps" ), this );

	auto vBLayout = new QGridLayout( vBoxCustomMipMaps );

	auto pMipMapScrollArea = new QScrollArea( vBoxCustomMipMaps );
	pMipMapScrollArea->setWidgetResizable( true );

	auto pMipMapScrollAreaContent = new QWidget();

	auto pMipMapDialogLayout = new QVBoxLayout( pMipMapScrollAreaContent );

	auto parent = dynamic_cast<VTFEImport *>( this->parent() );

	auto pFrameBox = new QSpinBox( pMipMapScrollArea );
	pFrameBox->setPrefix( "Frame: " );
	vBLayout->addWidget( pFrameBox, 0, 0 );
	auto pFaceBox = new QSpinBox( pMipMapScrollArea );
	pFaceBox->setPrefix( "Face: " );
	vBLayout->addWidget( pFaceBox, 0, 1 );
	auto pSliceBox = new QSpinBox( pMipMapScrollArea );
	pSliceBox->setPrefix( "Slice: " );
	vBLayout->addWidget( pSliceBox, 0, 2 );

	pMipMapScrollArea->setWidget( pMipMapScrollAreaContent );

	if ( !parent->imageList.empty() )
	{
		uint32_t maxCubemaps = vtfpp::ImageDimensions::getRecommendedMipCountForDims( parent->grabFirst()->getFormat(), parent->grabFirst()->getWidth(), parent->grabFirst()->getHeight() );

		for ( int i = 1; i < maxCubemaps; i++ )
		{
			uint32_t uiMipWidth, uiMipHeight;
			uiMipWidth = vtfpp::ImageDimensions::getMipDim( i, parent->grabFirst()->getWidth() );
			uiMipHeight = vtfpp::ImageDimensions::getMipDim( i, parent->grabFirst()->getHeight() );
			auto mipMapButton = new QPushButton( QApplication::style()->standardIcon( QStyle::SP_FileIcon ), QString::number( uiMipWidth ) + " X " + QString::number( uiMipHeight ) );
			mipMapButton->setMinimumHeight( 24 );

			connect( mipMapButton, &QPushButton::clicked, this, [&, parent, pFrameBox, pFaceBox, pSliceBox, i, uiMipHeight, uiMipWidth]()
					 {
						 //						 auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

						 auto imagePath = QFileDialog::getOpenFileName( this, "Open Custom Mipmap", "/", ui::CMainWindow::supportedWildcardImageList.join( " " ) );
						 if ( imagePath.isEmpty() || !QFile( imagePath ).exists() )
							 return;

						 parent->addImage( imagePath );
						 // auto data = parent->imageList[{ static_cast<uint16_t>( pFrameBox->value() ), static_cast<uint8_t>( pFaceBox->value() ), static_cast<uint16_t>( pSliceBox->value() ), 0 }];

						 //						 auto newRawData = vtfpp::ImageConversion::resizeImageData( data->getData(), data->getFormat(), data->getWidth(), uiMipWidth, data->getHeight(), uiMipHeight, false, vtfpp::ImageConversion::ResizeFilter::DEFAULT );
						 //						 parent->imageList[{ static_cast<uint16_t>( pFrameBox->value() ), static_cast<uint8_t>( pFaceBox->value() ), static_cast<uint16_t>( pSliceBox->value() ), static_cast<uint8_t>( i ) }] = new VTFEImageContainer( newRawData.data(), uiMipWidth, uiMipHeight, data->getFormat() );
						 auto asda = 0;
					 } );

			pMipMapDialogLayout->addWidget( mipMapButton );
		}
	}
	else
	{
		vBoxCustomMipMaps->setDisabled( true );
	}

	vBLayout->addWidget( pMipMapScrollArea, 1, 0, 1, 3 );

	pMainLayout->addWidget( vBoxCustomMipMaps, 1, 1 );
}

#ifdef NORMAL_GENERATION
void GeneralTab::GeneralNormalMap()
{
	auto vBoxCustomMipMaps = new QGroupBox( tr( "Normal Map" ), this );
	auto vBLayout = new QGridLayout( vBoxCustomMipMaps );
	generateNormalMapCheckbox_ = new QCheckBox( this );
	generateNormalMapCheckbox_->setText( tr( "Generate Normal Map" ) );
	vBLayout->addWidget( generateNormalMapCheckbox_, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Kernel Filter:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	kernelFilterCombo_ = new QComboBox( this );
	kernelFilterCombo_->addItem( tr( "4x" ), KERNEL_FILTER_4X );
	kernelFilterCombo_->addItem( tr( "3x3" ), KERNEL_FILTER_3X3 );
	kernelFilterCombo_->addItem( tr( "5x5" ), KERNEL_FILTER_5X5 );
	kernelFilterCombo_->addItem( tr( "7x7" ), KERNEL_FILTER_7X7 );
	kernelFilterCombo_->addItem( tr( "9x9" ), KERNEL_FILTER_9X9 );
	kernelFilterCombo_->addItem( tr( "DUDV" ), KERNEL_FILTER_DUDV );
	vBLayout->addWidget( kernelFilterCombo_, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Height Source:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	heightConversionCombo_ = new QComboBox( this );
	heightConversionCombo_->addItem( tr( "Alpha Channel" ), (int)HEIGHT_CONVERSION_METHOD_ALPHA );
	heightConversionCombo_->addItem( tr( "Average RGB" ), (int)HEIGHT_CONVERSION_METHOD_AVERAGE_RGB );
	heightConversionCombo_->addItem( tr( "Biased RGB" ), (int)HEIGHT_CONVERSION_METHOD_BIASED_RGB );
	heightConversionCombo_->addItem( tr( "Red Channel" ), (int)HEIGHT_CONVERSION_METHOD_RED );
	heightConversionCombo_->addItem( tr( "Green Channel" ), (int)HEIGHT_CONVERSION_METHOD_GREEN );
	heightConversionCombo_->addItem( tr( "Blue Channel" ), (int)HEIGHT_CONVERSION_METHOD_BLUE );
	heightConversionCombo_->addItem( tr( "Max RGB" ), (int)HEIGHT_CONVERSION_METHOD_MAX_RGB );
	heightConversionCombo_->addItem( tr( "Colorspace" ), (int)HEIGHT_CONVERSION_METHOD_COLORSPACE );
	vBLayout->addWidget( heightConversionCombo_, 2, 1, Qt::AlignRight );
	auto label3 = new QLabel( this );
	label3->setText( tr( "Kernel Filter:" ) );
	vBLayout->addWidget( label3, 3, 0, Qt::AlignLeft );
	normalAlphaResultCombo_ = new QComboBox( this );
	normalAlphaResultCombo_->addItem( tr( "No Change" ), (int)NORMAL_ALPHA_RESULT_NOCHANGE );
	normalAlphaResultCombo_->addItem( tr( "Set To Height" ), (int)NORMAL_ALPHA_RESULT_HEIGHT );
	normalAlphaResultCombo_->addItem( tr( "Set To Black" ), (int)NORMAL_ALPHA_RESULT_BLACK );
	normalAlphaResultCombo_->addItem( tr( "Set To White" ), (int)NORMAL_ALPHA_RESULT_WHITE );
	vBLayout->addWidget( normalAlphaResultCombo_, 3, 1, Qt::AlignRight );

	auto label4 = new QLabel( this );
	label4->setText( tr( "Scale:" ) );
	vBLayout->addWidget( label4, 4, 0, Qt::AlignLeft );
	scaleSpinBox_ = new QDoubleSpinBox( this );
	scaleSpinBox_->setValue( 2.00 );
	scaleSpinBox_->setSingleStep( 0.05 );
	vBLayout->addWidget( scaleSpinBox_, 4, 1, Qt::AlignRight );

	checkbox5_ = new QCheckBox( this );
	checkbox5_->setText( tr( "Wrap Normal Map" ) );
	vBLayout->addWidget( checkbox5_, 5, 0, Qt::AlignLeft );

	kernelFilterCombo_->setDisabled( true );
	heightConversionCombo_->setDisabled( true );
	normalAlphaResultCombo_->setDisabled( true );
	scaleSpinBox_->setDisabled( true );
	checkbox5_->setDisabled( true );
	label1->setDisabled( true );
	label2->setDisabled( true );
	label3->setDisabled( true );
	label4->setDisabled( true );

	connect(
		generateNormalMapCheckbox_, &QCheckBox::clicked, this->parent(),
		[this, label1, label2, label3, label4]( bool checked )
		{
			kernelFilterCombo_->setDisabled( !checked );
			heightConversionCombo_->setDisabled( !checked );
			normalAlphaResultCombo_->setDisabled( !checked );
			scaleSpinBox_->setDisabled( !checked );
			checkbox5_->setDisabled( !checked );
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
		} );

	vMainLayout->addWidget( vBoxCustomMipMaps, 1, 1 );
}
#endif
AdvancedTab::AdvancedTab( VTFEImport *parent ) :
	QDialog( parent )
{
	pMainLayout = new QGridLayout( this );
	pMainLayout->setAlignment( Qt::AlignTop );
	VersionMenu();
	GammaCorrectionMenu();
	Miscellaneous();
	// DTXCompression(); //doesn't seem to be used in modern VTFEdit or VTFLib.
	LuminanceWeights();
#ifdef COLOR_CORRECTION
	ColorCorrectionMenu();
#endif
	//	UnsharpenMaskOptions();
	//	XSharpenOptions(); //According to ~smead on discord, these are used by the old DTX library to apply filters to a
	// filter... new DTX library doesn't have this
}

void AdvancedTab::VersionMenu()
{
	auto vBoxVersion = new QGroupBox( tr( "Version" ), this );
	auto vBLayout = new QGridLayout( vBoxVersion );
	auto label1 = new QLabel( this );
	label1->setText( tr( "VTF Version:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pVtfVersionBox = new QComboBox( this );
#ifdef CHAOS_INITIATIVE
	for ( int i = 0; i <= 6; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( 7 ) + "." + QString::number( i ), i );
	}
	pVtfVersionBox->setCurrentIndex( pVtfVersionBox->count() - 2 );
	vBLayout->addWidget( pVtfVersionBox, 0, 1, Qt::AlignRight );
#ifdef CHAOS_INITIATIVE
	pAuxCompressionBox = new QCheckBox( this );
	pAuxCompressionBox->setText( tr( "AUX Compression" ) );
	vBLayout->addWidget( pAuxCompressionBox, 1, 0, Qt::AlignLeft );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Aux Compression Level:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pAuxCompressionLevelBox = new QComboBox( this );
	for ( int i = 0; i <= 9; i++ )
	{
		pAuxCompressionLevelBox->addItem( QString::number( i ), i );
	}
	pAuxCompressionLevelBox->setCurrentIndex( pAuxCompressionLevelBox->count() - 1 );
	vBLayout->addWidget( pAuxCompressionLevelBox, 2, 1, Qt::AlignRight );

	pAuxCompressionBox->setDisabled( true );
	pAuxCompressionLevelBox->setDisabled( true );
	label2->setDisabled( true );

	connect(
		pVtfVersionBox, &QComboBox::currentTextChanged, this->parent(),
		[this, label2]( QString text )
		{
			bool iscompressCompatible = ( QString( text.at( 2 ).toLatin1() ).toInt() >= 6 );
			pAuxCompressionBox->setDisabled( !iscompressCompatible );
			bool isChecked = pAuxCompressionBox->isChecked();
			pAuxCompressionLevelBox->setDisabled( !iscompressCompatible || !isChecked );
			label2->setDisabled( !iscompressCompatible || !isChecked );
		} );
	connect(
		pAuxCompressionBox, &QCheckBox::clicked, this->parent(),
		[this, label2]( bool checked )
		{
			pAuxCompressionLevelBox->setDisabled( !checked );
			label2->setDisabled( !checked );
		} );
#endif
	pMainLayout->addWidget( vBoxVersion, 0, 0 );
}

void AdvancedTab::GammaCorrectionMenu()
{
	auto vBoxGammaCorrection = new QGroupBox( tr( "Gamma Correction" ), this );
	auto vBLayout = new QGridLayout( vBoxGammaCorrection );

	pGammaCorrectionCheckBox = new QCheckBox( this );
	pGammaCorrectionCheckBox->setText( tr( "Gamma Correction" ) );
	vBLayout->addWidget( pGammaCorrectionCheckBox, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Correction:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pGammaCorrectionBox = new QDoubleSpinBox( this );

	pGammaCorrectionBox->setValue( 2.30 );
	vBLayout->addWidget( pGammaCorrectionBox, 1, 1, Qt::AlignRight );

	pGammaCorrectionBox->setDisabled( true );
	label1->setDisabled( true );

	connect(
		pGammaCorrectionCheckBox, &QCheckBox::clicked, this->parent(),
		[label1, this]( bool clicked )
		{
			pGammaCorrectionBox->setDisabled( !clicked );
			label1->setDisabled( !clicked );
		} );

	pMainLayout->addWidget( vBoxGammaCorrection, 1, 0 );
}

void AdvancedTab::Miscellaneous()
{
	auto vBoxMiscellaneous = new QGroupBox( tr( "Miscellaneous" ), this );
	auto vBLayout = new QGridLayout( vBoxMiscellaneous );
	pComputeReflectivityCheckBox = new QCheckBox( this );
	pComputeReflectivityCheckBox->setText( tr( "Compute Reflectivity" ) );
	vBLayout->addWidget( pComputeReflectivityCheckBox, 0, 0, Qt::AlignLeft );
	pGenerateThumbnailCheckBox = new QCheckBox( this );
	pGenerateThumbnailCheckBox->setText( tr( "Generate Thumbnail" ) );
	vBLayout->addWidget( pGenerateThumbnailCheckBox, 1, 0, Qt::AlignLeft );
	pGenerateSphereMapCheckBox = new QCheckBox( this );
	pGenerateSphereMapCheckBox->setText( tr( "Generate Sphere Map" ) );
	pGenerateSphereMapCheckBox->setDisabled( true );
	vBLayout->addWidget( pGenerateSphereMapCheckBox, 2, 0, Qt::AlignLeft );
	pMainLayout->addWidget( vBoxMiscellaneous, 2, 0 );
}

void AdvancedTab::DTXCompression()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "DTX Compression" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Quality:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pDtxCompressionQuality = new QComboBox( this );
	pDtxCompressionQuality->addItem( tr( "low" ) );
	pDtxCompressionQuality->addItem( tr( "medium" ) );
	pDtxCompressionQuality->addItem( tr( "high" ) );
	pDtxCompressionQuality->setCurrentIndex( pDtxCompressionQuality->count() - 1 );
	vBLayout->addWidget( pDtxCompressionQuality, 0, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 3, 0 );
}

void AdvancedTab::LuminanceWeights()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "Luminance Weights" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Red:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pLuminanceWeightRedBox = new QDoubleSpinBox( this );
	pLuminanceWeightRedBox->setDecimals( 3 );
	pLuminanceWeightRedBox->setValue( 0.299 );
	vBLayout->addWidget( pLuminanceWeightRedBox, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Green:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	pLuminanceWeightGreenBox = new QDoubleSpinBox( this );
	pLuminanceWeightGreenBox->setDecimals( 3 );
	pLuminanceWeightGreenBox->setValue( 0.587 );
	vBLayout->addWidget( pLuminanceWeightGreenBox, 1, 1, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Blue:" ) );
	vBLayout->addWidget( label3, 2, 0, Qt::AlignLeft );
	pLuminanceWeightBlueBox = new QDoubleSpinBox( this );
	pLuminanceWeightBlueBox->setDecimals( 3 );
	pLuminanceWeightBlueBox->setValue( 0.114 );
	vBLayout->addWidget( pLuminanceWeightBlueBox, 2, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 0, 1 );
}
#ifdef COLOR_CORRECTION
void AdvancedTab::ColorCorrectionMenu()
{
	auto vBoxColorCorrection = new QGroupBox( tr( "Color Correction" ), this );
	auto vBLayout = new QGridLayout( vBoxColorCorrection );
	//	ColorCorrectionDialog_ = new QColorDialog(this);
	//	QColorDialog::ColorDialogOptions options;
	//	options.setFlag(QColorDialog::ShowAlphaChannel, true);
	//	options.setFlag(QColorDialog::NoButtons, true);
	//	//options.setFlag(QColorDialog::DontUseNativeDialog,true);
	//	ColorCorrectionDialog_->setOptions(options);
	//	ColorCorrectionDialog_->setFixedSize(20,20);
	colorCorrectionDialog_ = new QtColorTriangle( vBoxColorCorrection );
	QColor tmp;
	tmp.setHsv( 0, 0, 255 );
	colorCorrectionDialog_->setColor( tmp );
	colorCorrectionDialog_->setMinimumSize( 140, 140 );
	vBLayout->addWidget( colorCorrectionDialog_, 0, 0, 4, 1, Qt::AlignLeft );
	auto current = colorCorrectionDialog_->color();

	auto label1 = new QLabel( this );
	label1->setText( tr( "Hue:" ) );
	vBLayout->addWidget( label1, 0, 1, Qt::AlignLeft );
	colorCorrectionRedBox_ = new QDoubleSpinBox( this );
	colorCorrectionRedBox_->setDecimals( 3 );
	colorCorrectionRedBox_->setSingleStep( 0.01 );
	colorCorrectionRedBox_->setValue( current.redF() );
	colorCorrectionRedBox_->setRange( 0, 1 );
	vBLayout->addWidget( colorCorrectionRedBox_, 0, 2, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Saturation:" ) );
	vBLayout->addWidget( label2, 1, 1, Qt::AlignLeft );
	colorCorrectionGreenBox_ = new QDoubleSpinBox( this );
	colorCorrectionGreenBox_->setDecimals( 3 );
	colorCorrectionGreenBox_->setSingleStep( 0.01 );
	colorCorrectionGreenBox_->setValue( current.greenF() );
	colorCorrectionGreenBox_->setRange( 0, 1 );
	vBLayout->addWidget( colorCorrectionGreenBox_, 1, 2, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Value:" ) );
	vBLayout->addWidget( label3, 2, 1, Qt::AlignLeft );
	colorCorrectionBlueBox_ = new QDoubleSpinBox( this );
	colorCorrectionBlueBox_->setDecimals( 3 );
	colorCorrectionBlueBox_->setSingleStep( 0.01 );

	colorCorrectionBlueBox_->setValue( current.blueF() );
	colorCorrectionBlueBox_->setRange( 0, 1 );
	vBLayout->addWidget( colorCorrectionBlueBox_, 2, 2, Qt::AlignRight );

	auto label4 = new QLabel( this );
	label4->setText( tr( "Alpha:" ) );
	vBLayout->addWidget( label4, 3, 1, Qt::AlignLeft );
	colorCorrectionAlphaBox_ = new QDoubleSpinBox( this );
	colorCorrectionAlphaBox_->setDecimals( 3 );
	colorCorrectionAlphaBox_->setSingleStep( 0.01 );
	colorCorrectionAlphaBox_->setValue( current.alphaF() );
	colorCorrectionAlphaBox_->setRange( 0, 1 );
	colorCorrectionAlphaBox_->setStepType( QDoubleSpinBox::AdaptiveDecimalStepType );
	vBLayout->addWidget( colorCorrectionAlphaBox_, 3, 2, Qt::AlignRight );

	AdvancedTab::connect(
		colorCorrectionRedBox_, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), this,
		[this]( double value )
		{
			auto current = colorCorrectionDialog_->color().toHsv();
			current.setHsvF( value, current.saturationF(), current.valueF() );
			colorCorrectionDialog_->setColor( current );
		} );
	AdvancedTab::connect(
		colorCorrectionGreenBox_, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), this,
		[this]( double value )
		{
			auto current = colorCorrectionDialog_->color().toHsv();
			current.setHsvF( current.hueF(), value, current.valueF() );
			colorCorrectionDialog_->setColor( current );
		} );
	AdvancedTab::connect(
		colorCorrectionBlueBox_, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), this,
		[this]( double value )
		{
			auto current = colorCorrectionDialog_->color().toHsv();
			current.setHsvF( current.hueF(), current.saturationF(), value );
			colorCorrectionDialog_->setColor( current );
		} );

	connect(
		colorCorrectionDialog_, &QtColorTriangle::colorChanged,
		[this]( QColor c )
		{
			colorCorrectionRedBox_->setValue( c.hueF() );
			colorCorrectionGreenBox_->setValue( c.saturationF() );
			colorCorrectionBlueBox_->setValue( c.valueF() );
		} );

	vMainLayout->addWidget( vBoxColorCorrection, 1, 1 );
}
#endif
void AdvancedTab::HSVtoRGB( float H, float S, float V, int rgb[3] )
{
	if ( H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0 )
	{
		qInfo() << "The given HSV values are not in valid range" << Qt::endl;
		return;
	}
	float s = S / 100;
	float v = V / 100;
	float C = s * v;
	float X = C * ( 1 - abs( fmod( H / 60.0, 2 ) - 1 ) );
	float m = v - C;
	float r, g, b;
	if ( H >= 0 && H < 60 )
	{
		r = C, g = X, b = 0;
	}
	else if ( H >= 60 && H < 120 )
	{
		r = X, g = C, b = 0;
	}
	else if ( H >= 120 && H < 180 )
	{
		r = 0, g = C, b = X;
	}
	else if ( H >= 180 && H < 240 )
	{
		r = 0, g = X, b = C;
	}
	else if ( H >= 240 && H < 300 )
	{
		r = X, g = 0, b = C;
	}
	else
	{
		r = C, g = 0, b = X;
	}
	rgb[0] = ( r + m ) * 255;
	rgb[1] = ( g + m ) * 255;
	rgb[2] = ( b + m ) * 255;
}

void AdvancedTab::UnsharpenMaskOptions()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "Unsharpen Mask Options" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Radius:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pUnsharpenMaskRadiusBox = new QDoubleSpinBox( this );
	pUnsharpenMaskRadiusBox->setValue( 2 );
	vBLayout->addWidget( pUnsharpenMaskRadiusBox, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Amount:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	pUnsharpenMaskAmountBox = new QDoubleSpinBox( this );
	pUnsharpenMaskAmountBox->setValue( 0.5 );
	vBLayout->addWidget( pUnsharpenMaskAmountBox, 1, 1, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label3, 2, 0, Qt::AlignLeft );
	pUnsharpenMaskThresholdBox = new QDoubleSpinBox( this );
	pUnsharpenMaskThresholdBox->setValue( 0 );
	vBLayout->addWidget( pUnsharpenMaskThresholdBox, 2, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 1, 1 );
}

void AdvancedTab::XSharpenOptions()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "X Sharpen Options" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Strength:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pXSharpenOptionsStrengthBox = new QDoubleSpinBox( this );
	pXSharpenOptionsStrengthBox->setValue( 2 );
	vBLayout->addWidget( pXSharpenOptionsStrengthBox, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	pXSharpenOptionsThresholdBox = new QDoubleSpinBox( this );
	pXSharpenOptionsThresholdBox->setValue( 0.5 );
	vBLayout->addWidget( pXSharpenOptionsThresholdBox, 1, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 2, 1 );
}

ResourceTab::ResourceTab( VTFEImport *parent ) :
	QDialog( parent )
{
	pMainLayout = new QGridLayout( this );
	pMainLayout->setAlignment( Qt::AlignTop );
	LODControlResource();
	InformationResource();
}

void ResourceTab::LODControlResource()
{
	auto vBoxLODControlResource = new QGroupBox( tr( "LOD Control Resource" ), this );
	auto vBLayout = new QGridLayout( vBoxLODControlResource );

	pLodControlResourceCheckBox = new QCheckBox( this );
	pLodControlResourceCheckBox->setText( tr( "Create LOD Control Resource" ) );
	vBLayout->addWidget( pLodControlResourceCheckBox, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Strength:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pControlResourceCrampUBox = new QDoubleSpinBox( this );
	pControlResourceCrampUBox->setValue( 2 );
	vBLayout->addWidget( pControlResourceCrampUBox, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pControlResourceCrampVBox = new QDoubleSpinBox( this );
	pControlResourceCrampVBox->setValue( 0.5 );
	vBLayout->addWidget( pControlResourceCrampVBox, 2, 1, Qt::AlignRight );

	label1->setDisabled( true );
	label2->setDisabled( true );
	pControlResourceCrampUBox->setDisabled( true );
	pControlResourceCrampVBox->setDisabled( true );

	connect(
		pLodControlResourceCheckBox, &QCheckBox::clicked,
		[label1, label2, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			pControlResourceCrampUBox->setDisabled( !checked );
			pControlResourceCrampVBox->setDisabled( !checked );
		} );

	pMainLayout->addWidget( vBoxLODControlResource, 0, 0, Qt::AlignLeft );
}

void ResourceTab::InformationResource()
{
	auto vBoxInformationResource = new QGroupBox( tr( "Information Resource" ), this );
	auto vBLayout = new QGridLayout( vBoxInformationResource );
	pCreateInformationResourceCheckBox = new QCheckBox( this );
	pCreateInformationResourceCheckBox->setText( tr( "Create LOD Control Resource" ) );
	vBLayout->addWidget( pCreateInformationResourceCheckBox, 0, 0, Qt::AlignLeft );

	QLabel *label1 = new QLabel( this );
	label1->setText( tr( "Author:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pInformationResourceAuthor = new QLineEdit( this );
	vBLayout->addWidget( pInformationResourceAuthor, 1, 1, Qt::AlignLeft );

	QLabel *label2 = new QLabel( this );
	label2->setText( tr( "Contact:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pInformationResouceContact = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceContact, 2, 1, Qt::AlignLeft );

	QLabel *label3 = new QLabel( this );
	label3->setText( tr( "Version:" ) );
	vBLayout->addWidget( label3, 3, 0, Qt::AlignLeft );
	pInformationResouceVersion = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceVersion, 3, 1, Qt::AlignLeft );

	QLabel *label4 = new QLabel( this );
	label4->setText( tr( "Modification:" ) );
	vBLayout->addWidget( label4, 4, 0, Qt::AlignLeft );
	pInformationResouceModification = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceModification, 4, 1, Qt::AlignLeft );

	QLabel *label5 = new QLabel( this );
	label5->setText( tr( "Description:" ) );
	vBLayout->addWidget( label5, 5, 0, Qt::AlignLeft );
	pInformationResouceDescription = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceDescription, 5, 1, Qt::AlignLeft );

	QLabel *label6 = new QLabel( this );
	label6->setText( tr( "Comments:" ) );
	vBLayout->addWidget( label6, 6, 0, Qt::AlignLeft );
	pInformationResouceComments = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceComments, 6, 1, Qt::AlignLeft );

	label1->setDisabled( true );
	label2->setDisabled( true );
	label3->setDisabled( true );
	label4->setDisabled( true );
	label5->setDisabled( true );
	label6->setDisabled( true );
	pInformationResourceAuthor->setDisabled( true );
	pInformationResouceVersion->setDisabled( true );
	pInformationResouceContact->setDisabled( true );
	pInformationResouceModification->setDisabled( true );
	pInformationResouceDescription->setDisabled( true );
	pInformationResouceComments->setDisabled( true );

	connect(
		pCreateInformationResourceCheckBox, &QCheckBox::clicked,
		[label1, label2, label3, label4, label5, label6, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
			label5->setDisabled( !checked );
			label6->setDisabled( !checked );
			pInformationResourceAuthor->setDisabled( !checked );
			pInformationResouceVersion->setDisabled( !checked );
			pInformationResouceContact->setDisabled( !checked );
			pInformationResouceModification->setDisabled( !checked );
			pInformationResouceDescription->setDisabled( !checked );
			pInformationResouceComments->setDisabled( !checked );
		} );

	pMainLayout->addWidget( vBoxInformationResource, 1, 0, Qt::AlignLeft );
}

ImageProcessor::QMultiDragListWidget *groupBoxListWidget( QGroupBox *box )
{
	auto layout = new QVBoxLayout( box );
	auto list = new ImageProcessor::QMultiDragListWidget( box );
	list->setDragEnabled( true );
	list->setAcceptDrops( true );
	list->setSelectionMode( QAbstractItemView::MultiSelection );
	layout->addWidget( list );
	return list;
}

ImageProcessor::ImageProcessor( VTFEImport *parent ) :
	QDialog( parent )
{
	auto layout = new QGridLayout( this );

	auto zeroImageBox = new QGroupBox( "Base", this );
	auto zeroImageTab = groupBoxListWidget( zeroImageBox );
	zeroImageTab->setMaximumHeight( 24 );
	zeroImageTab->base = true;
	auto frameZero = parent->imageList.value( 0 );
	if ( frameZero )
	{
		auto item = new QListWidgetItem( frameZero->getPath() );
		item->setData( Qt::UserRole, 0 );
		zeroImageTab->addItem( item );
	}
	layout->addWidget( zeroImageBox, 0, 0, 1, 2 );
	auto frameBox = new QGroupBox( "Frames", this );
	auto frameTab = groupBoxListWidget( frameBox );

	for ( const auto &[key, value] : parent->imageList.asKeyValueRange() )
	{
		if ( key == 0 )
			continue;

		auto wItem = new QListWidgetItem( value->getPath() );
		wItem->setData( Qt::UserRole, key );
		frameTab->addItem( wItem );
	}

	layout->addWidget( frameBox, 1, 0 );
	auto faceBox = new QGroupBox( "Faces", this );
	auto faceTab = groupBoxListWidget( faceBox );
	layout->addWidget( faceBox, 1, 1 );
	auto sliceBox = new QGroupBox( "Slices", this );
	auto sliceTab = groupBoxListWidget( sliceBox );
	layout->addWidget( sliceBox, 2, 0 );
	//	layout->addWidget( ImageProcessorCustomMipmaps(), 2, 1 );

	connect( zeroImageTab, &QListWidget::itemChanged, this, [&, parent, zeroImageTab]()
			 {
				 parent->acceptButton->setEnabled( zeroImageTab->count() > 0 );
			 } );
}

// QGroupBox *ImageProcessor::ImageProcessorCustomMipmaps()
//{
//	vBoxCustomMipMaps = new QGroupBox( tr( "Custom mipmaps" ), this );
//
//	auto vBLayout = new QGridLayout( vBoxCustomMipMaps );
//
//	auto pMipMapScrollArea = new QScrollArea( vBoxCustomMipMaps );
//	pMipMapScrollArea->setWidgetResizable( true );
//
//	auto pMipMapScrollAreaContent = new QWidget();
//
//	auto pMipMapDialogLayout = new QVBoxLayout( pMipMapScrollAreaContent );
//
//	auto parent = dynamic_cast<VTFEImport *>( this->parent() );
//
//	auto pFrameBox = new QSpinBox( pMipMapScrollArea );
//	pFrameBox->setPrefix( "Frame: " );
//	vBLayout->addWidget( pFrameBox, 0, 0 );
//	auto pFaceBox = new QSpinBox( pMipMapScrollArea );
//	pFaceBox->setPrefix( "Face: " );
//	vBLayout->addWidget( pFaceBox, 0, 1 );
//	auto pSliceBox = new QSpinBox( pMipMapScrollArea );
//	pSliceBox->setPrefix( "Slice: " );
//	vBLayout->addWidget( pSliceBox, 0, 2 );
//
//	pMipMapScrollArea->setWidget( pMipMapScrollAreaContent );
//
//	if ( !parent->imageList.empty() )
//	{
//		uint32_t maxCubemaps = vtfpp::ImageDimensions::getRecommendedMipCountForDims( parent->grabFirst()->getFormat(), parent->grabFirst()->getWidth(), parent->grabFirst()->getHeight() );
//
//		for ( int i = 1; i < maxCubemaps; i++ )
//		{
//			uint32_t uiMipWidth, uiMipHeight;
//			uiMipWidth = vtfpp::ImageDimensions::getMipDim( i, parent->grabFirst()->getWidth() );
//			uiMipHeight = vtfpp::ImageDimensions::getMipDim( i, parent->grabFirst()->getHeight() );
//			auto mipMapButton = new QPushButton( QApplication::style()->standardIcon( QStyle::SP_FileIcon ), QString::number( uiMipWidth ) + " X " + QString::number( uiMipHeight ) );
//			mipMapButton->setMinimumHeight( 24 );
//
//			connect( mipMapButton, &QPushButton::clicked, this, [&, parent, pFrameBox, pFaceBox, pSliceBox, i, uiMipHeight, uiMipWidth]()
//					 {
//						 auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );
//
//						 auto imagePath = QFileDialog::getOpenFileName( this, "Open Custom Mipmap", recentPaths.last(), ui::CMainWindow::supportedWildcardImageList.join( " " ) );
//						 if ( imagePath.isEmpty() || !QFile( imagePath ).exists() )
//							 return;
//
//						 parent->addImage( imagePath, i );
//						 // auto data = parent->imageList[{ static_cast<uint16_t>( pFrameBox->value() ), static_cast<uint8_t>( pFaceBox->value() ), static_cast<uint16_t>( pSliceBox->value() ), 0 }];
//
//						 //						 auto newRawData = vtfpp::ImageConversion::resizeImageData( data->getData(), data->getFormat(), data->getWidth(), uiMipWidth, data->getHeight(), uiMipHeight, false, vtfpp::ImageConversion::ResizeFilter::DEFAULT );
//						 //						 parent->imageList[{ static_cast<uint16_t>( pFrameBox->value() ), static_cast<uint8_t>( pFaceBox->value() ), static_cast<uint16_t>( pSliceBox->value() ), static_cast<uint8_t>( i ) }] = new VTFEImageContainer( newRawData.data(), uiMipWidth, uiMipHeight, data->getFormat() );
//						 auto asda = 0;
//					 } );
//
//			pMipMapDialogLayout->addWidget( mipMapButton );
//		}
//	}
//	else
//	{
//		vBoxCustomMipMaps->setDisabled( true );
//	}
//
//	vBLayout->addWidget( pMipMapScrollArea, 1, 0, 1, 3 );
// }
void ImageProcessor::QMultiDragListWidget::dragMoveEvent( QDragMoveEvent *e )
{
	if ( this->base )
	{
		if ( this->count() >= 1 )
			return e->ignore();
		auto widget = reinterpret_cast<QMultiDragListWidget *>( e->source() );
		if ( widget->selectedItems().count() > 1 )
			return e->ignore();
	}
	e->accept();
}
#include <QMimeData>
void ImageProcessor::QMultiDragListWidget::dropEvent( QDropEvent *event )
{
	auto widget = reinterpret_cast<QMultiDragListWidget *>( event->source() );
	if ( this->base )
	{
		if ( this->count() >= 1 )
			return event->ignore();
	}

	if ( widget )
	{
		for ( auto item : widget->selectedItems() )
		{
			delete item;
			if ( this->base )
				break;
		}
		emit widget->itemChanged( nullptr );
	}

	QListWidget::dropEvent( event );
}
VTFEImport::VTFEImport( QWidget *pParent, vtfpp::VTF *vtf ) :
	editableVTF( vtf )
{
	for ( uint16_t i = 0; i < this->editableVTF->getFrameCount(); i++ )
		for ( uint16_t j = 0; j < this->editableVTF->getFaceCount(); j++ )
			for ( uint16_t k = 0; k < this->editableVTF->getDepth(); k++ )
				for ( uint16_t l = 0; l < this->editableVTF->getMipCount(); l++ )
				{
					this->imageList[this->imageList.count()] = ( new VTFEImageContainer( this->editableVTF->getImageDataRaw( l, i, j, k ).data(), this->editableVTF->getWidth( l ), this->editableVTF->getHeight( l ), this->editableVTF->getFormat() ) );
				}

	SetDefaults();

	InitializeWidgets();

	//	pGeneralTab->pFormatCombo->setCurrentIndex( pGeneralTab->pFormatCombo->findData( static_cast<uint32_t>( grabFirst()->getFormat() ) ) );
}
void VTFEImport::addImage( const QStringList &list )
{
	for ( const QString &str : list )
		addImage( str );
}
QString VTFEImport::getFileName()
{
	return "Unidentified.";
}
