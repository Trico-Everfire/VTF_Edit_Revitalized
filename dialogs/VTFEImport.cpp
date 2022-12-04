#include "VTFEImport.h"

#include "../common/flagsandformats.hpp"
#include "../widgets/ImageSettingsWidget.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCommandLineParser>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <cmath>

VTFEImport::VTFEImport( QWidget *pParent, const QString &filePath ) :
	QDialog( pParent )
{
	images_ = new VTFEImageFormat *[1];
	addImage( filePath );
	this->setWindowTitle( tr( "VTF Options" ) );

	// filling default information.
	options.ImageFormat = VTFImageFormat::IMAGE_FORMAT_DXT5;
	options.uiVersion[0] = 7;
	options.uiVersion[1] = 5;
	options.uiStartFrame = 0;
	options.bResize = true;
	options.bMipmaps = true;
	options.ResizeMethod = VTFResizeMethod::RESIZE_NEAREST_POWER2;
	options.bResizeClamp = true;
	options.uiResizeClampWidth = 2048;
	options.uiResizeClampHeight = 2048;

	InitializeWidgets();
}

VTFEImport::VTFEImport( QWidget *pParent, const QStringList &filePaths ) :
	QDialog( pParent )
{
	images_ = new VTFEImageFormat *[filePaths.count()];
	for ( int i = 0; i < filePaths.count(); i++ )
		addImage( filePaths[i] );

	this->setWindowTitle( tr( "VTF Options" ) );

	// filling default information.
	options.ImageFormat = VTFImageFormat::IMAGE_FORMAT_DXT5;
	options.uiVersion[0] = 7;
	options.uiVersion[1] = 5;
	options.uiStartFrame = 0;
	options.bResize = true;
	options.bMipmaps = true;
	options.ResizeMethod = VTFResizeMethod::RESIZE_NEAREST_POWER2;
	options.bResizeClamp = true;
	options.uiResizeClampWidth = 2048;
	options.uiResizeClampHeight = 2048;

	InitializeWidgets();
}

void VTFEImport::addImage( const QString &filePath )
{
	auto tempImage = QImage( filePath );
	auto tempImage1 = tempImage.convertToFormat( QImage::Format_RGBA8888 );
	images_[imageAmount_] = new VTFEImageFormat(
		const_cast<vlByte *>( tempImage1.constBits() ), tempImage1.width(), tempImage1.height(), 0, IMAGE_FORMAT_RGBA8888 );
	imageAmount_++;
}

VTFErrorType VTFEImport::generateVTF()
{
	if ( !images_ )
		return VTFErrorType::NODATA;

	auto vFile = new VTFLib::CVTFFile;

	options.ImageFormat = static_cast<tagVTFImageFormat>( pGeneralTab->formatCombo_->currentData().toInt() );
	options.uiVersion[0] = 7;
	options.uiVersion[1] = pAdvancedTab->vtfVersionBox_->currentData().toInt();
	options.bResize = ( pGeneralTab->resizeMethodCombo_->isEnabled() && pGeneralTab->resizeCheckbox_->isChecked() );
	options.bMipmaps =
		( pGeneralTab->generateMipmapsCheckbox_->isEnabled() && pGeneralTab->generateMipmapsCheckbox_->isChecked() );
	options.MipmapFilter = static_cast<VTFMipmapFilter>( pGeneralTab->mipmapFilterCombo_->currentData().toInt() );
	options.ResizeMethod = static_cast<VTFResizeMethod>( pGeneralTab->resizeMethodCombo_->currentData().toInt() );
	options.bResizeClamp = ( pGeneralTab->clampCheckbox_->isEnabled() && pGeneralTab->clampCheckbox_->isChecked() );
	;
	options.uiResizeClampWidth = pGeneralTab->clampWidthCombo_->currentData().toInt();
	options.uiResizeClampHeight = pGeneralTab->clampHeightCombo_->currentData().toInt();
	options.bReflectivity =
		( pAdvancedTab->computeReflectivityCheckBox_->isEnabled() &&
		  pAdvancedTab->computeReflectivityCheckBox_->isChecked() );
	options.sReflectivity[0] = pAdvancedTab->luminanceWeightRedBox_->value();
	options.sReflectivity[1] = pAdvancedTab->luminanceWeightGreenBox_->value();
	options.sReflectivity[2] = pAdvancedTab->luminanceWeightBlueBox_->value();
	options.bThumbnail =
		( pAdvancedTab->generateThumbnailCheckBox_->isEnabled() &&
		  pAdvancedTab->generateThumbnailCheckBox_->isChecked() );
	options.bGammaCorrection =
		( pAdvancedTab->gammaCorrectionCheckBox_->isEnabled() && pAdvancedTab->gammaCorrectionCheckBox_->isChecked() );
	options.sGammaCorrection = pAdvancedTab->gammaCorrectionBox_->value();
	options.bSphereMap =
		( pAdvancedTab->generateSphereMapCheckBox_->isEnabled() &&
		  pAdvancedTab->generateSphereMapCheckBox_->isChecked() );
	if ( flags != 0 )
	{
		options.uiFlags = flags;
	}
	pFFSArray = new vlByte *[imageAmount_];

	// auto CMYK = pAdvancedTab->colorCorrectionDialog_->color().toCmyk();
#ifdef COLOR_CORRECTION
	for ( int i = 0; i < imageAmount_; i++ )
	{
		vlByte *imgData = new vlByte[images_[i]->getSize()];
		memcpy( imgData, images_[i]->getData(), images_[i]->getSize() );
		//		//int bpp = VTFLib::CVTFFile::GetImageFormatInfo(images_[i]->getFormat()).uiBytesPerPixel;
		for ( int s = 0; s < images_[i]->getSize(); s += 4 )
		{
			//			int rgb1[3] = {0, 0, 0};
			//			int rgb2[3] = {0, 0, 0};
			//			int rgb3[3] = {0, 0, 0};
			//			auto currentColor = new QColor(imgData[s],imgData[s + 1],imgData[s + 2]);
			//			auto currentCMYK = currentColor->toCmyk();
			//			float r = pAdvancedTab->colorCorrectionDialog_->color().saturationF();

			//			currentColor->setCmyk(((currentCMYK.cyan()*(1 - r)) + (CMYK.cyan() * r)), ((currentCMYK.magenta()*(1 - r)) + (CMYK.magenta() * r)),((currentCMYK.yellow()*(1 - r)) - (CMYK.yellow() * r)), ((currentCMYK.black()*(1 - r)) - (CMYK.black() * r)) );
			//			auto currentRGB = currentCMYK.toRgb();
			//			auto RGB = CMYK.toRgb();

			//			Advanced::HSVtoRGB((HSV.hueF() ) * 360,HSV.saturationF() * 100,HSV.valueF() * 100, rgb1);
			//			Advanced::HSVtoRGB(HSV.hueF() * 360,HSV.saturationF() * 100,HSV.valueF() * 100, rgb2);
			//			Advanced::HSVtoRGB(HSV.hueF() * 360,HSV.saturationF() * 100,HSV.valueF() * 100, rgb3);

			//			float hue = currentColor->hueF() + pAdvancedTab->colorCorrectionDialog_->color().hueF();
			//			hue /= (hue / 2);
			//			float saturation = currentColor->saturationF() + pAdvancedTab->colorCorrectionDialog_->color().saturationF();
			//			saturation /= (saturation / 2);
			//			float value = currentColor->valueF() + pAdvancedTab->colorCorrectionDialog_->color().valueF();
			//			value /= (value / 2);
			//			currentColor->setHsvF(hue, saturation, value);

			//			currentColor->setHsvF()

			//			imgData[s] = currentColor->red();
			//			imgData[s+1] = currentColor->green();
			//			imgData[s+2] = currentColor->blue();
			// imgData[s+3] = (imgData[s + 3] - HSV.alpha()) * 2;
		}
		pFFSArray[i] = const_cast<vlByte *>( imgData );
	}
#endif
	int frames = pGeneralTab->typeCombo_->currentIndex() == 0 ? imageAmount_ : 1;
	int faces = pGeneralTab->typeCombo_->currentIndex() == 1 ? imageAmount_ : 1;
	int slices = pGeneralTab->typeCombo_->currentIndex() == 2 ? imageAmount_ : 1;

	if ( !vFile->Create( images_[0]->getWidth(), images_[0]->getHeight(), frames, faces, slices, pFFSArray, options ) )
		return VTFErrorType::INVALIDIMAGE;
	if ( !vFile->IsLoaded() )
		return VTFErrorType::INVALIDIMAGE;

	if ( vFile->GetSupportsResources() )
	{
	}

#ifdef NORMAL_GENERATION
	if ( pGeneralTab->generateNormalMapCheckbox_->isEnabled() && pGeneralTab->generateNormalMapCheckbox_->isChecked() )
	{
		vFile->SetFlag( TEXTUREFLAGS_NORMAL, true );

		if ( !vFile->IsLoaded() )
			return VTFErrorType::INVALIDIMAGE;

		if ( vFile->GetFlags() & TEXTUREFLAGS_ENVMAP )
		{
			VTFLib::LastError.Set( "Image is an enviroment map." );
			return VTFErrorType::INVALIDIMAGE;
		}

		if ( !vFile->GetHasImage() )
		{
			VTFLib::LastError.Set( "No image data to generate normal map from." );
			return VTFErrorType::INVALIDIMAGE;
		}

		vlByte *lpData = vFile->GetData( 0, 0, 0, 0 );

		// Will hold frame's converted image data.
		vlByte *lpSource =
			new vlByte[vFile->ComputeImageSize( vFile->GetWidth(), vFile->GetHeight(), 1, IMAGE_FORMAT_RGBA32323232F )];

		// Get the frame's image data.
		if ( !vFile->Convert(
				 lpData, lpSource, vFile->GetWidth(), vFile->GetHeight(), vFile->GetFormat(),
				 IMAGE_FORMAT_RGBA32323232F ) )
		{
			//			delete []lpSource;
			return VTFErrorType::INVALIDIMAGE;
		}

		// Will hold normal image data.
		vlByte *lpDest =
			new vlByte[vFile->ComputeImageSize( vFile->GetWidth(), vFile->GetHeight(), 1, vFile->GetFormat() )];

		// toGreyScale(lpSource, vFile->ComputeImageSize(vFile->GetWidth(), vFile->GetHeight(), 1,
		// IMAGE_FORMAT_RGBA8888) ,255, 255, 255, 255); lpDest = TEScO::generateFormattedHeightmap(lpSource,
		// vFile->GetWidth(), vFile->GetHeight());

		// Set the frame's image data.
		if ( !vFile->Convert(
				 lpSource /*lpDest*/, lpDest, vFile->GetWidth(), vFile->GetHeight(), IMAGE_FORMAT_RGBA32323232F,
				 vFile->GetFormat() ) )
		{
			//			delete []lpSource;	// Moved from above.
			//			delete []lpDest;
			return VTFErrorType::INVALIDIMAGE;
		}
		vFile->SetData( 0, 0, 0, 0, lpDest );
		//		delete []lpSource;	// Moved from above.
		//		delete []lpDest;

		// qInfo() <<
		// vFile->GenerateNormalMap(static_cast<VTFKernelFilter>(pGeneralTab->kernelFilterCombo_->currentData().toInt()),
		// static_cast<VTFHeightConversionMethod>(pGeneralTab->heightConversionCombo_->currentData().toInt()),static_cast<VTFNormalAlphaResult>(pGeneralTab->normalAlphaResultCombo_->currentData().toInt()));
	}
#endif
#ifdef CHAOS_INITIATIVE
	if ( pAdvancedTab->auxCompressionBox_->isEnabled() && pAdvancedTab->auxCompressionBox_->isChecked() )
		vFile->SetAuxCompressionLevel( pAdvancedTab->auxCompressionLevelBox_->currentData().toInt() );
#endif
	this->VTF = vFile;
	delete[] pFFSArray;
	return VTFErrorType::SUCCESSS;
}

vlBool VTFEImport::IsPowerOfTwo( vlUInt uiSize )
{
	return uiSize > 0 && ( uiSize & ( uiSize - 1 ) ) == 0;
}

VTFLib::CVTFFile *VTFEImport::getVTF()
{
	return this->VTF;
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

	pGeneralTab = new GeneralTab( this );
	pAdvancedTab = new AdvancedTab( this );
	pResourceTab = new ResourceTab( this );

	widget->addTab( pGeneralTab, tr( "General" ) );
	widget->addTab( pAdvancedTab, tr( "Advanced" ) );
	widget->addTab( pResourceTab, tr( "Resource" ) );
	vBLayout->addWidget( widget, 0, 0, 1, 2 );
	QPushButton *preview = new QPushButton( this );
	preview->setText( tr( "Preview" ) );
	connect(
		preview, &QPushButton::pressed,
		[&]()
		{
			auto dialog = new QDialog();
			auto vRLayout = new QGridLayout( dialog );
			auto scrollArea = new QScrollArea( dialog );
			auto vIVW = new ImageViewWidget( scrollArea );
			scrollArea->setWidget( vIVW );
			auto vISW = new ImageSettingsWidget( vIVW, dialog );
			vRLayout->addWidget( vISW, 0, 0 );
			vRLayout->addWidget( scrollArea, 0, 1, Qt::AlignCenter );

			auto vtfFile = generateVTF();
			if ( vtfFile == SUCCESSS )
			{
				vIVW->set_vtf( VTF );
				vISW->set_vtf( VTF );
			}
			scrollArea->resize( dialog->width() + vISW->width(), dialog->height() );
			dialog->exec();
			delete VTF;
		} );
	vBLayout->addWidget( preview, 1, 0, Qt::AlignLeft );

	QDialogButtonBox *blayoutBox = new QDialogButtonBox( this );
	auto accept = blayoutBox->addButton( "Accept", QDialogButtonBox::AcceptRole );
	auto cancelled = blayoutBox->addButton( "Cancel", QDialogButtonBox::RejectRole );
	connect(
		accept, &QPushButton::pressed,
		[this]
		{
			cancelled_ = false;
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

VTFEImport *VTFEImport::fromVTF( QWidget *pParent, VTFLib::CVTFFile *pFile )
{
	auto vVTFImport = new VTFEImport( pParent );

	int type = 0;
	int fImageAmount = pFile->GetFrameCount();
	if ( pFile->GetFaceCount() > fImageAmount )
	{
		fImageAmount = pFile->GetFaceCount();
		type = 1;
	}
	if ( pFile->GetDepth() > fImageAmount )
	{
		fImageAmount = pFile->GetDepth();
		type = 2;
	}

	const bool hasAlpha = VTFLib::CVTFFile::GetImageFormatInfo( pFile->GetFormat() ).uiAlphaBitsPerPixel > 0;
	vVTFImport->images_ = new VTFEImageFormat *[fImageAmount];

	for ( int i = 0; i < fImageAmount; i++ )
	{
		vlUInt frames = type == 0 ? i + 1 : 1;
		vlUInt faces = type == 1 ? i + 1 : 1;
		vlUInt slices = type == 2 ? i + 1 : 1;

		auto size = VTFLib::CVTFFile::ComputeImageSize( pFile->GetWidth(), pFile->GetHeight(), 1, IMAGE_FORMAT_RGBA8888 );
		auto pDest = static_cast<vlByte *>( malloc( size ) );
		qInfo() << VTFLib::CVTFFile::ConvertToRGBA8888(
			pFile->GetData( frames, faces, slices, 0 ), pDest, pFile->GetWidth(), pFile->GetHeight(), pFile->GetFormat() );
		vVTFImport->images_[vVTFImport->imageAmount_] =
			new VTFEImageFormat( pDest, pFile->GetWidth(), pFile->GetHeight(), pFile->GetDepth(), IMAGE_FORMAT_RGBA8888 );
		vVTFImport->imageAmount_++;
		free( pDest );
	}

	vVTFImport->InitializeWidgets();

	vVTFImport->pAdvancedTab->vtfVersionBox_->setCurrentIndex( pFile->GetMinorVersion() );
	emit vVTFImport->pAdvancedTab->vtfVersionBox_->currentTextChanged( "7." + QString::number( pFile->GetMinorVersion() ) );
#ifdef CHAOS_INITIATIVE
	vVTFImport->pAdvancedTab->auxCompressionBox_->setChecked( pFile->GetAuxCompressionLevel() > 0 );
	emit vVTFImport->pAdvancedTab->auxCompressionBox_->clicked( pFile->GetAuxCompressionLevel() > 0 );
	if ( pFile->GetAuxCompressionLevel() > 0 )
		vVTFImport->pAdvancedTab->auxCompressionLevelBox_->setCurrentIndex( pFile->GetAuxCompressionLevel() );
#endif
	vVTFImport->pGeneralTab->generateMipmapsCheckbox_->setChecked( pFile->GetMipmapCount() > 0 );
	emit vVTFImport->pGeneralTab->generateMipmapsCheckbox_->clicked( pFile->GetMipmapCount() > 0 );
	vVTFImport->pGeneralTab->typeCombo_->setCurrentIndex( type );
	vVTFImport->pGeneralTab->typeCombo_->currentTextChanged( QString::number( type ) );
	vVTFImport->pGeneralTab->formatCombo_->setCurrentIndex(
		vVTFImport->pGeneralTab->formatCombo_->findData( pFile->GetFormat() ) );
	vlSingle r;
	vlSingle g;
	vlSingle b;
	pFile->GetReflectivity( r, g, b );
	vVTFImport->pAdvancedTab->luminanceWeightRedBox_->setValue( r );
	vVTFImport->pAdvancedTab->luminanceWeightGreenBox_->setValue( g );
	vVTFImport->pAdvancedTab->luminanceWeightBlueBox_->setValue( b );

	vVTFImport->flags = pFile->GetFlags();

	return vVTFImport;
}

GeneralTab::GeneralTab( VTFEImport *parent ) :
	QDialog( parent )
{
	vMainLayout = new QGridLayout( this );
	vMainLayout->setAlignment( Qt::AlignTop );
	GeneralOptions();
	GeneralResize();
	GeneralMipMaps();
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
	formatCombo_ = new QComboBox( this );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( VTFLib::CVTFFile::GetImageFormatInfo( fmt.format ).bIsSupported )
			formatCombo_->addItem( tr( fmt.name ), (int)fmt.format );
	}
	vBLayout->addWidget( formatCombo_, 0, 1, Qt::AlignRight );
	auto label2 = new QLabel();
	label2->setText( tr( "Texture Type:" ) );
	typeCombo_ = new QComboBox( this );
	typeCombo_->addItem( tr( "Animated Texture" ) );
	typeCombo_->addItem( tr( "Environment Map" ) );
	typeCombo_->addItem( tr( "Volume Texture" ) );

	auto vParent = static_cast<VTFEImport *>( this->parent() );
	connect(
		typeCombo_, &QComboBox::currentTextChanged, this->parent(),
		[this, vParent]()
		{
			bool shouldCheck = typeCombo_->currentIndex() == 0;
#ifdef NORMAL_GENERATION
			generateNormalMapCheckbox_->setDisabled( !shouldCheck );
			emit generateNormalMapCheckbox_->clicked( shouldCheck && generateNormalMapCheckbox_->isChecked() );
#endif
			if ( vParent )
			{
				vParent->pAdvancedTab->generateSphereMapCheckBox_->setDisabled( shouldCheck );
			}
		} );

	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	vBLayout->addWidget( typeCombo_, 1, 1, Qt::AlignRight );
	vMainLayout->addWidget( vBoxGeneralOptions, 0, 0 );
}

void GeneralTab::GeneralResize()
{
	auto vBoxResize = new QGroupBox( tr( "Resize" ), this );
	auto vBLayout = new QGridLayout( vBoxResize );
	resizeCheckbox_ = new QCheckBox( this );
	resizeCheckbox_->setText( tr( "Resize" ) );
	auto parent = static_cast<VTFEImport *>( this->parent() );
	vlBool b1 = parent->IsPowerOfTwo( parent->images_[0]->getWidth() );
	vlBool b2 = parent->IsPowerOfTwo( parent->images_[0]->getHeight() );
	resizeCheckbox_->setChecked( !( b1 && b2 ) );
	resizeCheckbox_->setDisabled( !( b1 && b2 ) );
	if ( !( b1 && b2 ) )
		resizeCheckbox_->setToolTip( tr( "Image is not in power of 2 and therefore needs resizing." ) );
	vBLayout->addWidget( resizeCheckbox_, 0, 0, Qt::AlignLeft );
	auto label1 = new QLabel();
	label1->setText( tr( "Resize Method:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	resizeMethodCombo_ = new QComboBox( this );
	resizeMethodCombo_->addItem( tr( "Nearest Power Of 2" ), (int)RESIZE_NEAREST_POWER2 );
	resizeMethodCombo_->addItem( tr( "Biggest Power Of 2" ), (int)RESIZE_BIGGEST_POWER2 );
	resizeMethodCombo_->addItem( tr( "Smallest Power Of 2" ), (int)RESIZE_SMALLEST_POWER2 );
	resizeMethodCombo_->setCurrentIndex( 1 );
	vBLayout->addWidget( resizeMethodCombo_, 1, 1, Qt::AlignRight );
	auto label2 = new QLabel();
	label2->setText( tr( "Resize Filter:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	resizeFilterCombo_ = new QComboBox( this );
	resizeFilterCombo_->addItem( tr( "Box" ), (int)MIPMAP_FILTER_BOX );
	resizeFilterCombo_->addItem( tr( "Triangle" ), (int)MIPMAP_FILTER_TRIANGLE );
	resizeFilterCombo_->addItem( tr( "Quadratic" ), (int)MIPMAP_FILTER_QUADRATIC );
	resizeFilterCombo_->addItem( tr( "Cubic" ), (int)MIPMAP_FILTER_CUBIC );
	resizeFilterCombo_->addItem( tr( "Catrom" ), (int)MIPMAP_FILTER_CATROM );
	resizeFilterCombo_->addItem( tr( "Mitchell" ), (int)MIPMAP_FILTER_MITCHELL );
	resizeFilterCombo_->addItem( tr( "Gaussian" ), (int)MIPMAP_FILTER_GAUSSIAN );
	resizeFilterCombo_->addItem( tr( "Sine Cardinal" ), (int)MIPMAP_FILTER_SINC );
	resizeFilterCombo_->addItem( tr( "Bessel" ), (int)MIPMAP_FILTER_BESSEL );
	resizeFilterCombo_->addItem( tr( "Hanning" ), (int)MIPMAP_FILTER_HANNING );
	resizeFilterCombo_->addItem( tr( "Hamming" ), (int)MIPMAP_FILTER_HAMMING );
	resizeFilterCombo_->addItem( tr( "Blackman" ), (int)MIPMAP_FILTER_BLACKMAN );
	resizeFilterCombo_->addItem( tr( "Kaiser" ), (int)MIPMAP_FILTER_KAISER );
	vBLayout->addWidget( resizeFilterCombo_, 2, 1, Qt::AlignRight );
	clampCheckbox_ = new QCheckBox( this );
	clampCheckbox_->setText( tr( "Clamp" ) );

	vBLayout->addWidget( clampCheckbox_, 3, 0, Qt::AlignLeft );
	auto label3 = new QLabel();
	label3->setText( tr( "Maximum Width:" ) );
	clampWidthCombo_ = new QComboBox( this );
	vBLayout->addWidget( label3, 4, 0, Qt::AlignLeft );
	vBLayout->addWidget( clampWidthCombo_, 4, 1, Qt::AlignRight );

	auto label4 = new QLabel();
	label4->setText( tr( "Maximum Height:" ) );
	clampHeightCombo_ = new QComboBox( this );
	for ( int i = 1; i <= 4096; i *= 2 )
	{
		clampHeightCombo_->addItem( QString::number( i ), i );
		clampWidthCombo_->addItem( QString::number( i ), i );
	}

	clampHeightCombo_->setCurrentIndex( clampHeightCombo_->count() - 2 );
	clampWidthCombo_->setCurrentIndex( clampWidthCombo_->count() - 2 );

	label3->setDisabled( true );
	label4->setDisabled( true );
	clampHeightCombo_->setDisabled( true );
	clampWidthCombo_->setDisabled( true );

	connect(
		clampCheckbox_, &QCheckBox::clicked, this->parent(),
		[label3, label4, this]( bool checked )
		{
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
			clampHeightCombo_->setDisabled( !checked );
			clampWidthCombo_->setDisabled( !checked );
		} );

	if ( !resizeCheckbox_->isChecked() )
	{
		label1->setDisabled( true );
		label2->setDisabled( true );
		resizeMethodCombo_->setDisabled( true );
		resizeFilterCombo_->setDisabled( true );
		clampCheckbox_->setDisabled( true );
	}
	connect(
		resizeCheckbox_, &QCheckBox::clicked, this->parent(),
		[label1, label2, label3, label4, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			resizeMethodCombo_->setDisabled( !checked );
			resizeFilterCombo_->setDisabled( !checked );
			clampCheckbox_->setDisabled( !checked );
			bool secondChecked = clampCheckbox_->isChecked();
			label3->setDisabled( !checked || !secondChecked );
			label4->setDisabled( !checked || !secondChecked );
			clampHeightCombo_->setDisabled( !checked || !secondChecked );
			clampWidthCombo_->setDisabled( !checked || !secondChecked );
		} );

	vBLayout->addWidget( label4, 5, 0, Qt::AlignLeft );
	vBLayout->addWidget( clampHeightCombo_, 5, 1, Qt::AlignRight );
	vMainLayout->addWidget( vBoxResize, 1, 0 );
}

void GeneralTab::GeneralMipMaps()
{
	auto vBoxMipMaps = new QGroupBox( tr( "Mipmaps" ), this );
	auto vBLayout = new QGridLayout( vBoxMipMaps );
	generateMipmapsCheckbox_ = new QCheckBox( this );
	generateMipmapsCheckbox_->setText( tr( "Generate Mipmaps" ) );
	vBLayout->addWidget( generateMipmapsCheckbox_, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel();
	label1->setText( tr( "Mipmap Filter:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );

	mipmapFilterCombo_ = new QComboBox( this );
	mipmapFilterCombo_->addItem( tr( "Box" ), (int)MIPMAP_FILTER_BOX );
	mipmapFilterCombo_->addItem( tr( "Triangle" ), (int)MIPMAP_FILTER_TRIANGLE );
	mipmapFilterCombo_->addItem( tr( "Quadratic" ), (int)MIPMAP_FILTER_QUADRATIC );
	mipmapFilterCombo_->addItem( tr( "Cubic" ), (int)MIPMAP_FILTER_CUBIC );
	mipmapFilterCombo_->addItem( tr( "Catrom" ), (int)MIPMAP_FILTER_CATROM );
	mipmapFilterCombo_->addItem( tr( "Mitchell" ), (int)MIPMAP_FILTER_MITCHELL );
	mipmapFilterCombo_->addItem( tr( "Gaussian" ), (int)MIPMAP_FILTER_GAUSSIAN );
	mipmapFilterCombo_->addItem( tr( "Sine Cardinal" ), (int)MIPMAP_FILTER_SINC );
	mipmapFilterCombo_->addItem( tr( "Bessel" ), (int)MIPMAP_FILTER_BESSEL );
	mipmapFilterCombo_->addItem( tr( "Hanning" ), (int)MIPMAP_FILTER_HANNING );
	mipmapFilterCombo_->addItem( tr( "Hamming" ), (int)MIPMAP_FILTER_HAMMING );
	mipmapFilterCombo_->addItem( tr( "Blackman" ), (int)MIPMAP_FILTER_BLACKMAN );
	mipmapFilterCombo_->addItem( tr( "Kaiser" ), (int)MIPMAP_FILTER_KAISER );
	vBLayout->addWidget( mipmapFilterCombo_, 1, 1, Qt::AlignRight );

	label1->setDisabled( true );
	mipmapFilterCombo_->setDisabled( true );

	connect(
		generateMipmapsCheckbox_, &QCheckBox::clicked, this->parent(),
		[this, label1]( bool checked )
		{
			label1->setDisabled( !checked );
			mipmapFilterCombo_->setDisabled( !checked );
		} );

	vMainLayout->addWidget( vBoxMipMaps, 0, 1 );
}
#ifdef NORMAL_GENERATION
void GeneralTab::GeneralNormalMap()
{
	auto vBoxMipMaps = new QGroupBox( tr( "Normal Map" ), this );
	auto vBLayout = new QGridLayout( vBoxMipMaps );
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

	vMainLayout->addWidget( vBoxMipMaps, 1, 1 );
}
#endif
AdvancedTab::AdvancedTab( VTFEImport *parent ) :
	QDialog( parent )
{
	vMainLayout = new QGridLayout( this );
	vMainLayout->setAlignment( Qt::AlignTop );
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
	vtfVersionBox_ = new QComboBox( this );
#ifdef CHAOS_INITIATIVE
	for ( int i = 0; i <= VTF_MINOR_VERSION; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		vtfVersionBox_->addItem( QString::number( VTF_MAJOR_VERSION ) + "." + QString::number( i ), i );
	}
	vtfVersionBox_->setCurrentIndex( vtfVersionBox_->count() - 2 );
	vBLayout->addWidget( vtfVersionBox_, 0, 1, Qt::AlignRight );
#ifdef CHAOS_INITIATIVE
	auxCompressionBox_ = new QCheckBox( this );
	auxCompressionBox_->setText( tr( "AUX Compression" ) );
	vBLayout->addWidget( auxCompressionBox_, 1, 0, Qt::AlignLeft );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Aux Compression Level:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	auxCompressionLevelBox_ = new QComboBox( this );
	for ( int i = 0; i <= 9; i++ )
	{
		auxCompressionLevelBox_->addItem( QString::number( i ), i );
	}
	auxCompressionLevelBox_->setCurrentIndex( auxCompressionLevelBox_->count() - 1 );
	vBLayout->addWidget( auxCompressionLevelBox_, 2, 1, Qt::AlignRight );

	auxCompressionBox_->setDisabled( true );
	auxCompressionLevelBox_->setDisabled( true );
	label2->setDisabled( true );

	connect(
		vtfVersionBox_, &QComboBox::currentTextChanged, this->parent(),
		[this, label2]( QString text )
		{
			bool iscompressCompatible = ( QString( text.at( 2 ).toLatin1() ).toInt() >= 6 );
			auxCompressionBox_->setDisabled( !iscompressCompatible );
			bool isChecked = auxCompressionBox_->isChecked();
			auxCompressionLevelBox_->setDisabled( !iscompressCompatible || !isChecked );
			label2->setDisabled( !iscompressCompatible || !isChecked );
		} );
	connect(
		auxCompressionBox_, &QCheckBox::clicked, this->parent(),
		[this, label2]( bool checked )
		{
			auxCompressionLevelBox_->setDisabled( !checked );
			label2->setDisabled( !checked );
		} );
#endif
	vMainLayout->addWidget( vBoxVersion, 0, 0 );
}

void AdvancedTab::GammaCorrectionMenu()
{
	auto vBoxGammaCorrection = new QGroupBox( tr( "Gamma Correction" ), this );
	auto vBLayout = new QGridLayout( vBoxGammaCorrection );

	gammaCorrectionCheckBox_ = new QCheckBox( this );
	gammaCorrectionCheckBox_->setText( tr( "Gamma Correction" ) );
	vBLayout->addWidget( gammaCorrectionCheckBox_, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Correction:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	gammaCorrectionBox_ = new QDoubleSpinBox( this );

	gammaCorrectionBox_->setValue( 2.30 );
	vBLayout->addWidget( gammaCorrectionBox_, 1, 1, Qt::AlignRight );

	gammaCorrectionBox_->setDisabled( true );
	label1->setDisabled( true );

	connect(
		gammaCorrectionCheckBox_, &QCheckBox::clicked, this->parent(),
		[label1, this]( bool clicked )
		{
			gammaCorrectionBox_->setDisabled( !clicked );
			label1->setDisabled( !clicked );
		} );

	vMainLayout->addWidget( vBoxGammaCorrection, 1, 0 );
}

void AdvancedTab::Miscellaneous()
{
	auto vBoxMiscellaneous = new QGroupBox( tr( "Miscellaneous" ), this );
	auto vBLayout = new QGridLayout( vBoxMiscellaneous );
	computeReflectivityCheckBox_ = new QCheckBox( this );
	computeReflectivityCheckBox_->setText( tr( "Compute Reflectivity" ) );
	vBLayout->addWidget( computeReflectivityCheckBox_, 0, 0, Qt::AlignLeft );
	generateThumbnailCheckBox_ = new QCheckBox( this );
	generateThumbnailCheckBox_->setText( tr( "Generate Thumbnail" ) );
	vBLayout->addWidget( generateThumbnailCheckBox_, 1, 0, Qt::AlignLeft );
	generateSphereMapCheckBox_ = new QCheckBox( this );
	generateSphereMapCheckBox_->setText( tr( "Generate Sphere Map" ) );
	generateSphereMapCheckBox_->setDisabled( true );
	vBLayout->addWidget( generateSphereMapCheckBox_, 2, 0, Qt::AlignLeft );
	vMainLayout->addWidget( vBoxMiscellaneous, 2, 0 );
}

void AdvancedTab::DTXCompression()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "DTX Compression" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Quality:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	dtxCompressionQuality_ = new QComboBox( this );
	dtxCompressionQuality_->addItem( tr( "low" ) );
	dtxCompressionQuality_->addItem( tr( "medium" ) );
	dtxCompressionQuality_->addItem( tr( "high" ) );
	dtxCompressionQuality_->setCurrentIndex( dtxCompressionQuality_->count() - 1 );
	vBLayout->addWidget( dtxCompressionQuality_, 0, 1, Qt::AlignRight );

	vMainLayout->addWidget( vBoxDTXCompression, 3, 0 );
}

void AdvancedTab::LuminanceWeights()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "Luminance Weights" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Red:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	luminanceWeightRedBox_ = new QDoubleSpinBox( this );
	luminanceWeightRedBox_->setDecimals( 3 );
	luminanceWeightRedBox_->setValue( 0.299 );
	vBLayout->addWidget( luminanceWeightRedBox_, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Green:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	luminanceWeightGreenBox_ = new QDoubleSpinBox( this );
	luminanceWeightGreenBox_->setDecimals( 3 );
	luminanceWeightGreenBox_->setValue( 0.587 );
	vBLayout->addWidget( luminanceWeightGreenBox_, 1, 1, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Blue:" ) );
	vBLayout->addWidget( label3, 2, 0, Qt::AlignLeft );
	luminanceWeightBlueBox_ = new QDoubleSpinBox( this );
	luminanceWeightBlueBox_->setDecimals( 3 );
	luminanceWeightBlueBox_->setValue( 0.114 );
	vBLayout->addWidget( luminanceWeightBlueBox_, 2, 1, Qt::AlignRight );

	vMainLayout->addWidget( vBoxDTXCompression, 0, 1 );
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
	unsharpenMaskRadiusBox_ = new QDoubleSpinBox( this );
	unsharpenMaskRadiusBox_->setValue( 2 );
	vBLayout->addWidget( unsharpenMaskRadiusBox_, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Amount:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	unsharpenMaskAmountBox_ = new QDoubleSpinBox( this );
	unsharpenMaskAmountBox_->setValue( 0.5 );
	vBLayout->addWidget( unsharpenMaskAmountBox_, 1, 1, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label3, 2, 0, Qt::AlignLeft );
	unsharpenMaskThresholdBox_ = new QDoubleSpinBox( this );
	unsharpenMaskThresholdBox_->setValue( 0 );
	vBLayout->addWidget( unsharpenMaskThresholdBox_, 2, 1, Qt::AlignRight );

	vMainLayout->addWidget( vBoxDTXCompression, 1, 1 );
}

void AdvancedTab::XSharpenOptions()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "X Sharpen Options" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Strength:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	xSharpenOptionsStrengthBox_ = new QDoubleSpinBox( this );
	xSharpenOptionsStrengthBox_->setValue( 2 );
	vBLayout->addWidget( xSharpenOptionsStrengthBox_, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	xSharpenOptionsThresholdBox_ = new QDoubleSpinBox( this );
	xSharpenOptionsThresholdBox_->setValue( 0.5 );
	vBLayout->addWidget( xSharpenOptionsThresholdBox_, 1, 1, Qt::AlignRight );

	vMainLayout->addWidget( vBoxDTXCompression, 2, 1 );
}

ResourceTab::ResourceTab( VTFEImport *parent ) :
	QDialog( parent )
{
	vMainLayout = new QGridLayout( this );
	vMainLayout->setAlignment( Qt::AlignTop );
	LODControlResource();
	InformationResource();
}

void ResourceTab::LODControlResource()
{
	auto vBoxLODControlResource = new QGroupBox( tr( "LOD Control Resource" ), this );
	auto vBLayout = new QGridLayout( vBoxLODControlResource );

	lodControlResourceCheckBox_ = new QCheckBox( this );
	lodControlResourceCheckBox_->setText( tr( "Create LOD Control Resource" ) );
	vBLayout->addWidget( lodControlResourceCheckBox_, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Strength:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	controlResourceCrampUBox_ = new QDoubleSpinBox( this );
	controlResourceCrampUBox_->setValue( 2 );
	vBLayout->addWidget( controlResourceCrampUBox_, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	controlResourceCrampVBox_ = new QDoubleSpinBox( this );
	controlResourceCrampVBox_->setValue( 0.5 );
	vBLayout->addWidget( controlResourceCrampVBox_, 2, 1, Qt::AlignRight );

	label1->setDisabled( true );
	label2->setDisabled( true );
	controlResourceCrampUBox_->setDisabled( true );
	controlResourceCrampVBox_->setDisabled( true );

	connect(
		lodControlResourceCheckBox_, &QCheckBox::clicked,
		[label1, label2, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			controlResourceCrampUBox_->setDisabled( !checked );
			controlResourceCrampVBox_->setDisabled( !checked );
		} );

	vMainLayout->addWidget( vBoxLODControlResource, 0, 0, Qt::AlignLeft );
}

void ResourceTab::InformationResource()
{
	auto vBoxInformationResource = new QGroupBox( tr( "Information Resource" ), this );
	auto vBLayout = new QGridLayout( vBoxInformationResource );
	createInformationResourceCheckBox_ = new QCheckBox( this );
	createInformationResourceCheckBox_->setText( tr( "Create LOD Control Resource" ) );
	vBLayout->addWidget( createInformationResourceCheckBox_, 0, 0, Qt::AlignLeft );

	QLabel *label1 = new QLabel( this );
	label1->setText( tr( "Author:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	informationResouceAuthor_ = new QLineEdit( this );
	vBLayout->addWidget( informationResouceAuthor_, 1, 1, Qt::AlignLeft );

	QLabel *label2 = new QLabel( this );
	label2->setText( tr( "Contact:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	informationResouceContact_ = new QLineEdit( this );
	vBLayout->addWidget( informationResouceContact_, 2, 1, Qt::AlignLeft );

	QLabel *label3 = new QLabel( this );
	label3->setText( tr( "Version:" ) );
	vBLayout->addWidget( label3, 3, 0, Qt::AlignLeft );
	informationResouceVersion_ = new QLineEdit( this );
	vBLayout->addWidget( informationResouceVersion_, 3, 1, Qt::AlignLeft );

	QLabel *label4 = new QLabel( this );
	label4->setText( tr( "Modification:" ) );
	vBLayout->addWidget( label4, 4, 0, Qt::AlignLeft );
	informationResouceModification_ = new QLineEdit( this );
	vBLayout->addWidget( informationResouceModification_, 4, 1, Qt::AlignLeft );

	QLabel *label5 = new QLabel( this );
	label5->setText( tr( "Description:" ) );
	vBLayout->addWidget( label5, 5, 0, Qt::AlignLeft );
	informationResouceDescription_ = new QLineEdit( this );
	vBLayout->addWidget( informationResouceDescription_, 5, 1, Qt::AlignLeft );

	QLabel *label6 = new QLabel( this );
	label6->setText( tr( "Comments:" ) );
	vBLayout->addWidget( label6, 6, 0, Qt::AlignLeft );
	informationResouceComments_ = new QLineEdit( this );
	vBLayout->addWidget( informationResouceComments_, 6, 1, Qt::AlignLeft );

	label1->setDisabled( true );
	label2->setDisabled( true );
	label3->setDisabled( true );
	label4->setDisabled( true );
	label5->setDisabled( true );
	label6->setDisabled( true );
	informationResouceAuthor_->setDisabled( true );
	informationResouceVersion_->setDisabled( true );
	informationResouceContact_->setDisabled( true );
	informationResouceModification_->setDisabled( true );
	informationResouceDescription_->setDisabled( true );
	informationResouceComments_->setDisabled( true );

	connect(
		createInformationResourceCheckBox_, &QCheckBox::clicked,
		[label1, label2, label3, label4, label5, label6, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
			label5->setDisabled( !checked );
			label6->setDisabled( !checked );
			informationResouceAuthor_->setDisabled( !checked );
			informationResouceVersion_->setDisabled( !checked );
			informationResouceContact_->setDisabled( !checked );
			informationResouceModification_->setDisabled( !checked );
			informationResouceDescription_->setDisabled( !checked );
			informationResouceComments_->setDisabled( !checked );
		} );

	vMainLayout->addWidget( vBoxInformationResource, 1, 0, Qt::AlignLeft );
}