#include "ProcessVTF.h"

#include "../src/MainWindow.h"
#include "../src/VTFEImageContainer.h"
#include "../src/flagsandformats.hpp"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QComboBox>
#include <QDropEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QStandardItemModel>
#include <QTabWidget>

ProcessVTF::ProcessVTF( QWidget *parent, vtfpp::VTF *vtf ) :
	QDialog( parent ), processVtf( vtf ), imageListList {}, fileName( "untitled" ), imageID { 0 }
{
	for ( uint16_t i = 0; i < this->processVtf->getFrameCount(); i++ )
		for ( uint16_t j = 0; j < this->processVtf->getFaceCount(); j++ )
			for ( uint16_t k = 0; k < this->processVtf->getDepth(); k++ )
				for ( uint16_t l = 0; l < this->processVtf->getMipCount(); l++ )
				{
					this->imageList[imageID++] = ( new VTFEImageContainer( this->processVtf->getImageDataRaw( l, i, j, k ).data(), this->processVtf->getWidth( l ), this->processVtf->getHeight( l ), this->processVtf->getFormat() ) );
				}

	this->setupUI();
	this->setAcceptDrops( true );
	this->pasteShortcut = new QShortcut( QKeySequence( Qt::CTRL | Qt::Key_Q ), this, SLOT( close() ) );
	this->exitShortcut = new QShortcut( QKeySequence( Qt::CTRL | Qt::Key_V ), this, SLOT( onPaste() ) );
}

bool ProcessVTF::addImage( const QString &imagePath )
{
	if ( !QFileInfo::exists( imagePath ) )
		return false;

	if ( !QFileInfo( imagePath ).permission( QFile::ReadUser ) )
		return false;

	if ( imagePath.endsWith( ".gif" ) )
	{
		vtfpp::ImageFormat inputFormat;
		int inputWidth, inputHeight, inputFrameCount;
		auto imageData_ = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( imagePath.toStdString() ), inputFormat, inputWidth, inputHeight, inputFrameCount );

		if ( inputFormat == vtfpp::ImageFormat::EMPTY || !inputWidth || !inputHeight || !inputFrameCount )
			return false;

		const auto frameSize = vtfpp::ImageFormatDetails::getDataLength( inputFormat, inputWidth, inputHeight );
		for ( int i = 0; i < inputFrameCount; i++ )
		{
			this->imageList[imageID++] = ( new VTFEImageContainer( imageData_.data() + frameSize * i, inputWidth, inputHeight, inputFormat ) );
		}

		return true;
	}

	this->imageList[imageID++] = new VTFEImageContainer( imagePath );
	return true;
}

bool ProcessVTF::addImage( const QStringList &imagePaths )
{
	return std::ranges::all_of( imagePaths.begin(), imagePaths.end(), [this]( const QString &str )
								{
									return addImage( str );
								} );
}

bool ProcessVTF::addImage( const std::byte *data, size_t size, uint16_t width, uint16_t height, vtfpp::ImageFormat format )
{
	if ( size == 0 )
		return false;
	uint16_t id = imageID++;
	auto container = this->imageList[id] = new VTFEImageContainer { data, width, height, format };
	if ( container->getSize() < size )
	{
		delete container;
		this->imageList.remove( id );
		imageID--;
		return false;
	}

	return true;
}

bool ProcessVTF::addImage( const std::vector<std::byte> &data, uint16_t width, uint16_t height, vtfpp::ImageFormat format )
{
	return addImage( data.data(), data.size(), width, height, format );
}

bool ProcessVTF::addImage( const QImage &image )
{
	auto nim = image.convertToFormat( QImage::Format_RGBA8888 );
	this->imageList[imageID++] = ( new VTFEImageContainer( reinterpret_cast<const std::byte *>( nim.constBits() ), nim.width(), nim.height(), vtfpp::ImageFormat::RGBA8888 ) );
	return true;
}

void ProcessVTF::setupUI()
{
	auto mainLayout = new QGridLayout( this );

	auto mainListWidget = new QTabWidget( this );
	auto imageProcessorWidget = new QWidget( mainListWidget );

	auto imageProcessorLayout = new QGridLayout( imageProcessorWidget );

	this->imageListList = new SharedTabWidget( imageProcessorWidget );
	this->imageListList->setDragDropMode( QAbstractItemView::DragOnly );
	this->imageListList->setModel( new QStandardItemModel( this->imageListList ) );

	this->imageListList->setDeleteOnDrag( false );

	imageProcessorLayout->addWidget( this->imageListList, 0, 0, 4, 1, Qt::AlignRight );

	typeComboBox = new QComboBox( imageProcessorWidget );
	typeComboBox->addItem( "Single Frame Texture", static_cast<int>( TextureType::SINGLE_IMAGE ) );
	typeComboBox->addItem( "Animated Texture", static_cast<int>( TextureType::ANIMATED_TEXTURE ) );
	typeComboBox->addItem( "Cubemap/Envmap", static_cast<int>( TextureType::CUBEMAP ) );
	typeComboBox->addItem( "Animated Cubemap/Envmap", static_cast<int>( TextureType::ANIMATED_CUBEMAP ) );
	typeComboBox->addItem( "Volumetric Texture", static_cast<int>( TextureType::VOLUMETRIC_TEXTURES ) );
	imageProcessorLayout->addWidget( typeComboBox, 0, 1, 1, 5 );
	// TODO MIPMAP BUTTON.
	mipmapCheckBox = new QCheckBox( "Generate Mipmaps", mainListWidget );
	imageProcessorLayout->addWidget( mipmapCheckBox, 1, 1, 1, 5 );

	//////////////////////////////////////////////////////////// MIPMAPS

	mipList = new SharedTabWidget( imageProcessorWidget );
	//	mipList->setItemDelegate( new RemovableItemDelegate( mipList ) );
	mipList->setModel( new QStandardItemModel( mipList ) );
	mipList->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	mipList->setDragDropMode( QAbstractItemView::DragDrop );
	imageProcessorLayout->addWidget( mipList, 2, 3, 1, 3, Qt::AlignRight );
	mipList->setDeleteOnDrag( false );

	////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////// Single Frame.

	singleFrameContainerButton = new DropButton( this, "Drag Image Here.", this );
	singleFrameContainerButton->setMinimumSize( 512, 256 );
	imageProcessorLayout->addWidget( singleFrameContainerButton, 2, 1, 1, 2, Qt::AlignLeft );

	connect( singleFrameContainerButton, &DropButton::onImageInserted, imageProcessorWidget, [&]
			 {
				 auto type = typeComboBox->currentData().value<TextureType>();
				 if ( type != TextureType::SINGLE_IMAGE )
					 return;

				 applButton->setEnabled( true );
			 } );

	connect( singleFrameContainerButton, &DropButton::pressed, imageProcessorWidget, [&]
			 {
				 auto type = typeComboBox->currentData().value<TextureType>();
				 if ( type != TextureType::SINGLE_IMAGE )
					 return;

				 applButton->setEnabled( false );
			 } );

	////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////// Multi Frame.
	frameList = new SharedTabWidget( this );
	frameList->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	//	frameList->setItemDelegate( new RemovableItemDelegate( frameList ) );
	frameList->setModel( new QStandardItemModel( frameList ) );
	frameList->setDragDropMode( QAbstractItemView::DragDrop );
	frameList->hide();
	frameList->setMinimumSize( 512, 256 );
	frameList->setDeleteOnDrag( false );
	// imageProcessorLayout->setContentsMargins( 0, 0, 0, 0 );
	imageProcessorLayout->addWidget( frameList, 2, 1, 1, 2, Qt::AlignRight );
	////////////////////////////////////////////////////////////

	connect( mipmapCheckBox, &QCheckBox::clicked, this, [&, imageProcessorLayout]( bool checked )
			 {
				 if ( checked )
				 {
					 imageProcessorLayout->addWidget( singleFrameContainerButton, 2, 1, 1, 5, Qt::AlignCenter );
					 imageProcessorLayout->addWidget( frameList, 2, 1, 1, 5, Qt::AlignCenter );
					 mipList->hide();
					 return;
				 }

				 mipList->show();

				 auto type = typeComboBox->currentData().value<TextureType>();
				 switch ( type )
				 {
					 case TextureType::SINGLE_IMAGE:
					 case TextureType::ANIMATED_TEXTURE:
						 imageProcessorLayout->addWidget( mipList, 2, 3, 1, 3 );
						 break;
					 case TextureType::CUBEMAP:
						 break;
					 case TextureType::ANIMATED_CUBEMAP:
						 break;
					 case TextureType::VOLUMETRIC_TEXTURES:
						 break;
				 }
				 imageProcessorLayout->addWidget( singleFrameContainerButton, 2, 1, 1, 2, Qt::AlignLeft );
				 imageProcessorLayout->addWidget( frameList, 2, 1, 1, 2, Qt::AlignLeft );
			 } );

	connect( typeComboBox, &QComboBox::currentIndexChanged, this, [&]
			 {
				 auto type = typeComboBox->currentData().value<TextureType>();
				 switch ( type )
				 {
					 case TextureType::SINGLE_IMAGE:
						 this->frameList->hide();
						 this->singleFrameContainerButton->show();
						 break;
					 case TextureType::ANIMATED_TEXTURE:
						 this->frameList->show();
						 this->singleFrameContainerButton->hide();
						 break;
					 case TextureType::CUBEMAP:
						 break;
					 case TextureType::ANIMATED_CUBEMAP:
						 break;
					 case TextureType::VOLUMETRIC_TEXTURES:
						 break;
				 }
			 } );

	/*
		auto frameLabel = new QLabel( "Frames:", this );
		imageProcessorLayout->addWidget( frameLabel, 2, 1 );

		auto faceLabel = new QLabel( "Faces:", this );
		imageProcessorLayout->addWidget( faceLabel, 2, 2 );

		auto slicesLabel = new QLabel( "slices:", this );
		imageProcessorLayout->addWidget( slicesLabel, 2, 3 );

		auto mipmapsLabel = new QLabel( "mipmaps:", this );
		imageProcessorLayout->addWidget( mipmapsLabel, 2, 4 );

		frameList = new SharedTabWidget( imageProcessorWidget );
		frameList->setItemDelegate( new RemovableItemDelegate( frameList ) );
		frameList->setModel( new QStandardItemModel( frameList ) );
		frameList->setDragDropMode( QAbstractItemView::DragDrop );
		imageProcessorLayout->addWidget( frameList, 3, 1 );

		auto faceButtons = new QDialogButtonBox();
		auto faceLayout = new QGridLayout();
		faceLayout->setSpacing( 4 );
		faceLayout->addItem( new QSpacerItem( 64, 64 ), 0, 0 );
		auto topMiddleButton = new DropButton( this );
		topMiddleButton->setFixedSize( 64, 64 );
		faceButtons->addButton( topMiddleButton, QDialogButtonBox::NoRole );
		faceLayout->addWidget( topMiddleButton, 0, 1 );
		faceLayout->addItem( new QSpacerItem( 64, 64 ), 0, 2 );

		auto middleTopButton = new DropButton( this );
		middleTopButton->setFixedSize( 64, 64 );
		faceButtons->addButton( middleTopButton, QDialogButtonBox::NoRole );
		faceLayout->addWidget( middleTopButton, 1, 0 );
		auto middleMiddleButton = new DropButton( this );
		middleMiddleButton->setFixedSize( 64, 64 );
		faceButtons->addButton( middleMiddleButton, QDialogButtonBox::NoRole );
		faceLayout->addWidget( middleMiddleButton, 1, 1 );
		auto middleBottomButton = new DropButton( this );
		middleBottomButton->setFixedSize( 64, 64 );
		faceButtons->addButton( middleBottomButton, QDialogButtonBox::NoRole );
		faceLayout->addWidget( middleBottomButton, 1, 2 );
		auto middleEvenMoreBottomButton = new DropButton( this );
		faceLayout->addWidget( middleEvenMoreBottomButton, 1, 3 );
		faceButtons->addButton( middleEvenMoreBottomButton, QDialogButtonBox::NoRole );
		middleEvenMoreBottomButton->setFixedSize( 64, 64 );

		faceLayout->addItem( new QSpacerItem( 64, 64 ), 2, 0 );
		auto bottomMiddleButton = new DropButton( this );
		bottomMiddleButton->setFixedSize( 64, 64 );
		faceButtons->addButton( bottomMiddleButton, QDialogButtonBox::NoRole );
		faceLayout->addWidget( bottomMiddleButton, 2, 1 );
		faceLayout->addItem( new QSpacerItem( 64, 64 ), 2, 2 );
		imageProcessorLayout->addLayout( faceLayout, 3, 2 );

		sliceList = new SharedTabWidget( imageProcessorWidget );
		sliceList->setItemDelegate( new RemovableItemDelegate( sliceList ) );
		sliceList->setModel( new QStandardItemModel( sliceList ) );
		sliceList->setDragDropMode( QAbstractItemView::DragDrop );
		imageProcessorLayout->addWidget( sliceList, 3, 3 );

		mipList = new SharedTabWidget( imageProcessorWidget );
		mipList->setItemDelegate( new RemovableItemDelegate( mipList ) );
		mipList->setModel( new QStandardItemModel( mipList ) );
		mipList->setDragDropMode( QAbstractItemView::DragDrop );
		imageProcessorLayout->addWidget( mipList, 3, 4 );

		mainListWidget->addTab( imageProcessorWidget, "Image Layout" );

		auto vtfOptionsWidget = new QWidget( this );
		auto vtfOptionsLayout = new QGridLayout( vtfOptionsWidget );

		pFormatCombo = new QComboBox( this );
		pFormatCombo->setDisabled( true );
		for ( auto &fmt : IMAGE_FORMATS )
		{
			if ( vtfpp::ImageFormat::P8 != fmt.format )
				pFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
		}

		vtfOptionsLayout->addWidget( pFormatCombo, 0, 3, 1, 2 );

		mainListWidget->addTab( vtfOptionsWidget, "VTF Options" );

		connect( typeComboBox, &QComboBox::currentIndexChanged, this, [&, faceButtons, frameLabel, faceLabel, slicesLabel]
				 {
					 auto currentData = static_cast<TextureType>( typeComboBox->currentData().toInt() );
					 switch ( currentData )
					 {
						 case TextureType::SINGLE_IMAGE:
							 frameList->setDisabled( false );
							 frameList->setMaximum( 1 );
							 frameLabel->setDisabled( false );
							 slicesLabel->setDisabled( true );
							 sliceList->setDisabled( true );
							 faceLabel->setDisabled( true );
							 faceButtons->setDisabled( true );
							 break;
						 case TextureType::ANIMATED_TEXTURE:
							 frameLabel->setDisabled( false );
							 frameList->setDisabled( false );
							 frameList->setMaximum( 65535 );
							 sliceList->setDisabled( true );
							 slicesLabel->setDisabled( true );
							 faceLabel->setDisabled( true );
							 faceButtons->setDisabled( true );
							 break;
						 case TextureType::CUBEMAP:
							 frameList->setDisabled( true );
							 frameLabel->setDisabled( true );
							 slicesLabel->setDisabled( true );
							 sliceList->setDisabled( true );
							 faceLabel->setDisabled( false );
							 faceButtons->setDisabled( false );
							 break;
						 case TextureType::ANIMATED_CUBEMAP:
							 frameLabel->setDisabled( false );
							 frameList->setDisabled( false );
							 frameList->setMaximum( 65535 );
							 sliceList->setDisabled( true );
							 slicesLabel->setDisabled( true );
							 faceLabel->setDisabled( false );
							 faceButtons->setDisabled( false );
							 break;
						 case TextureType::VOLUMETRIC_TEXTURES:
							 frameLabel->setDisabled( true );
							 frameList->setDisabled( true );
							 frameList->setMaximum( 1 );
							 faceLabel->setDisabled( true );
							 faceButtons->setDisabled( true );
							 sliceList->setDisabled( false );
							 slicesLabel->setDisabled( false );
							 break;
					 }
				 } );
		typeComboBox->currentIndexChanged( 0 );

		connect( mipmapCheckBox, &QCheckBox::clicked, this, [&, mipmapsLabel]( bool genMips )
				 {
					 this->mipList->setDisabled( genMips );
					 mipmapsLabel->setDisabled( genMips );
				 } );

		connect( applButton, &QPushButton::pressed, this, &ProcessVTF::accept );

		connect( closeButton, &QPushButton::pressed, this, &ProcessVTF::reject );

		//	auto model = dynamic_cast<QStandardItemModel *>( baseImageList->model() );
		//	connect( model, &QStandardItemModel::rowsInserted, this, [&, applButton]()
		//			 {
		//				 pFormatCombo->setEnabled( true );
		//				 applButton->setEnabled( true );
		//			 } );
		//	connect( model, &QStandardItemModel::rowsRemoved, this, [&, applButton]()
		//			 {
		//				 pFormatCombo->setDisabled( true );
		//				 applButton->setDisabled( true );
		//			 } );
	*/
	mainListWidget->addTab( imageProcessorWidget, "Image Layout" );
	auto buttons = new QDialogButtonBox( this );
	applButton = buttons->addButton( QDialogButtonBox::Apply );
	applButton->setDisabled( true );
	auto closeButton = buttons->addButton( QDialogButtonBox::Close );
	mainLayout->addWidget( buttons, 1, 0 );

	connect( applButton, &QPushButton::pressed, this, &ProcessVTF::accept );

	connect( closeButton, &QPushButton::pressed, this, &ProcessVTF::reject );

	mainLayout->addWidget( mainListWidget, 0, 0 );
}

int ProcessVTF::exec()
{
	if ( !processVtf )
		return QDialog::Rejected;

	if ( !this->readyUI() )
		return QDialog::Rejected;

	auto result = QDialog::exec();

	if ( this->imageList.isEmpty() )
		return QDialog::Rejected;

	if ( result == QDialog::Accepted )
		this->applyChanges();

	return result;
}

bool ProcessVTF::readyUI()
{
	if ( this->imageList.isEmpty() )
		return false;

	auto model = dynamic_cast<QStandardItemModel *>( this->imageListList->model() );
	model->clear();

	for ( const auto &[value, vtf] : this->imageList.asKeyValueRange() )
	{
		auto stdItem = new QStandardItem();
		if ( !vtf->getPath().isEmpty() )
		{
			stdItem->setText( vtf->getPath() );
			vtfpp::ImageFormat inputFormat;
			int inputWidth, inputHeight, inputFrameCount;
			auto data = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( vtf->getPath().toStdString() ), inputFormat, inputWidth, inputHeight, inputFrameCount );
			auto newData = vtfpp::ImageConversion::convertImageDataToFormat( data, inputFormat, vtfpp::ImageFormat::RGBA8888, inputWidth, inputHeight );
			//		QPixmap::loadFromData( newData, )
			auto img = QImage( reinterpret_cast<unsigned char *>( newData.data() ), inputWidth, inputHeight, QImage::Format_RGBA8888 );
			double aspect = (double)img.width() / img.height();
			stdItem->setData( QPixmap::fromImage( img.scaled( 64, 64 / aspect ) ), Qt::DecorationRole );
		}
		else
		{
			//			auto l = std::vector<std::byte*>(reinterpret_cast<const std::byte *>( vtf->getData().data() ), reinterpret_cast<const std::byte *>( vtf->getData().data() + vtf->getData().size() ));
			//
			auto img = QImage( reinterpret_cast<const unsigned char *>( vtfpp::ImageConversion::convertImageDataToFormat( vtf->getData(), vtf->getFormat(), vtfpp::ImageFormat::RGBA8888, vtf->getWidth(), vtf->getHeight() ).data() ), vtf->getWidth(), vtf->getHeight(), QImage::Format_RGBA8888 );
			double aspect = (double)img.width() / img.height();
			stdItem->setData( QPixmap::fromImage( img.scaled( 64, 64 / aspect ) ), Qt::DecorationRole );
		}
		stdItem->setData( value, Qt::UserRole );
		model->appendRow( stdItem );
		//			auto listWidgetItem = new QListWidgetItem( vtf->getPath().isEmpty() ? "Unnamed Image " + QString::number( this->imageListList->count() ) : QFileInfo( vtf->getPath() ).fileName() );
		//			listWidgetItem->setData( Qt::UserRole, value );
		//			this->imageListList->addItem( listWidgetItem );
		//
		//			auto base = new QWidget( this );
		//			auto baseLayout = QVBoxLayout( base );
	}

	//	for ( uint16_t i = 0; i < this->processVtf->getFrameCount(); i++ )
	//	{
	//		// this->processVtf->getImageDataAsRGBA8888(0,i,0,0);
	//		auto stdItem = new QStandardItem();
	//		auto img = QImage( reinterpret_cast<const unsigned char *>( this->processVtf->getImageDataAsRGBA8888(0,i,0,0).data() ), this->processVtf->getWidth(), this->processVtf->getHeight(), QImage::Format_RGBA8888 );
	//		double aspect = (double)img.width() / img.height();
	//		stdItem->setData( QPixmap::fromImage( img.scaled( 64, 64 / aspect ) ), Qt::DecorationRole );
	//		stdItem->setData( this->imageList.size(), Qt::UserRole );
	//		model->appendRow( stdItem );
	//
	//	}

	return true;
}

void ProcessVTF::applyChanges()
{
	auto type = typeComboBox->currentData().value<TextureType>();
	switch ( type )
	{
		case TextureType::SINGLE_IMAGE:
		{
			if ( !imageList.contains( this->singleFrameContainerButton->imageIndex ) )
				return;
			auto image = imageList.value( this->singleFrameContainerButton->imageIndex );

			if ( image->hasData() )
				this->processVtf->setImage( image->getData(), image->getFormat(), image->getWidth(), image->getHeight() );
			else
				this->processVtf->setImage( image->getPath().toStdString() );
		}
		break;
		case TextureType::ANIMATED_TEXTURE:
			break;
		case TextureType::CUBEMAP:
			break;
		case TextureType::ANIMATED_CUBEMAP:
			break;
		case TextureType::VOLUMETRIC_TEXTURES:
			break;
	}

	//	auto basemodel = dynamic_cast<QStandardItemModel *>( baseImageList->model() );
	//
	//	auto rawDataNumber = basemodel->item( 0 )->data( Qt::UserRole ).toInt();
	//
	//	auto imageContainer = imageList[rawDataNumber];
	//	if ( !imageContainer->getPath().isEmpty() )
	//		this->processVtf->setImage( imageContainer->getPath().toStdString() );
	//	else
	//		this->processVtf->setImage( imageContainer->getData(), imageContainer->getFormat(), imageContainer->getWidth(), imageContainer->getHeight() );
	//
	//	this->processVtf->setFrameCount( frameList->model()->rowCount() + 1 );
	//
	//	for ( int i = 0; i < frameList->model()->rowCount(); i++ )
	//	{
	//		auto frameModel = dynamic_cast<QStandardItemModel *>( frameList->model() );
	//
	//		rawDataNumber = frameModel->item( i )->data( Qt::UserRole ).toInt();
	//		imageContainer = imageList[rawDataNumber];
	//		if ( !imageContainer->getPath().isEmpty() )
	//			processVtf->setImage( imageContainer->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::BILINEAR, 0, i + 1, 0, 0 );
	//		else
	//			processVtf->setImage( imageContainer->getData(), imageContainer->getFormat(), imageContainer->getWidth(), imageContainer->getHeight(), vtfpp::ImageConversion::ResizeFilter::BILINEAR, 0, i + 1, 0, 0 );
	//	}
}

void ProcessVTF::dragEnterEvent( QDragEnterEvent *event )
{
	if ( event->mimeData()->hasUrls() )
	{
		for ( const auto &url : event->mimeData()->urls() )
		{
			qInfo() << QFileInfo( url.toLocalFile() ).suffix();
			if ( !ui::CMainWindow::supportedImageList.contains( QFileInfo( url.toLocalFile() ).suffix() ) )
				return;
		}
		event->acceptProposedAction();
	}
	else
		event->ignore();
}

void ProcessVTF::dropEvent( QDropEvent *event )
{
	foreach( const QUrl &url, event->mimeData()->urls() )
	{
		addImage( url.toLocalFile() );
	}
	readyUI();
	event->acceptProposedAction();
}

void ProcessVTF::onPaste()
{
	auto imageBoard = QApplication::clipboard();

	if ( imageBoard->image().isNull() )
		return;

	addImage( imageBoard->image() );
	readyUI();
}
ProcessVTF::~ProcessVTF()
{
	delete this->pasteShortcut;
	delete this->exitShortcut;
	//	this->imageList.clear();
	//	this->imageID = 0;
}
void ProcessVTF::canEnableApplyVTF()
{
	auto type = typeComboBox->currentData().value<TextureType>();
}

void SharedTabWidget::dropEvent( QDropEvent *event )
{
	auto widget = reinterpret_cast<SharedTabWidget *>( event->source() );
	auto model = dynamic_cast<QStandardItemModel *>( widget->model() );
	auto modelThis = dynamic_cast<QStandardItemModel *>( this->model() );

	if ( modelThis->rowCount() >= this->getMaximum() )
		return event->ignore();

	int i = 0;
	for ( auto item : widget->selectedIndexes() )
	{
		if ( i >= this->getMaximum() )
			break;

		if ( widget->shouldDeleteOnDrag() )
		{
			auto oldItem = model->takeItem( item.row() );
			modelThis->appendRow( oldItem );
			model->removeRow( item.row() );
		}
		else
		{
			auto tm = model->itemFromIndex( item );
			auto stdItem = new QStandardItem( tm->text() );
			stdItem->setData( tm->data( Qt::DecorationRole ), Qt::DecorationRole );
			stdItem->setData( tm->data( Qt::UserRole ), Qt::UserRole );
			modelThis->appendRow( stdItem );
		}

		i++;
	}
	event->accept();
	// QListView::dropEvent( event );
}

void SharedTabWidget::dragMoveEvent( QDragMoveEvent *e )
{
	auto widget = dynamic_cast<SharedTabWidget *>( e->source() );
	if ( !widget )
		return e->ignore();

	if ( widget->selectedIndexes().count() > this->maximum )
		return e->ignore();

	e->accept();
	//	QListView::dragMoveEvent( e );
}

SharedTabWidget::SharedTabWidget( QWidget *parent ) :
	QListView( parent )
{
	//	auto model = new QStandardItemModel();
	//	this->setModel( model );
	//	setSelectionMode( MultiSelection );
}

QWidget *RemovableItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	auto clickableButtonItem = new QWidget( parent );
	//	clickableButtonItem->setMouseTracking( true );
	//	clickableButtonItem->setAutoFillBackground( true );
	auto clickableButtonLayout = new QHBoxLayout( clickableButtonItem );
	auto text = new QLabel( option.text, clickableButtonItem );
	clickableButtonLayout->addWidget( text );
	auto button = new QPushButton( "X", clickableButtonItem );
	button->setFixedSize( 24, 24 );
	clickableButtonLayout->addWidget( button );

	return clickableButtonItem; // QStyledItemDelegate::createEditor( parent, option, index );
}
RemovableItemDelegate::RemovableItemDelegate( QObject *parent ) :
	QStyledItemDelegate( parent )
{
}

void RemovableItemDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	QString value = index.model()->data( index, Qt::EditRole ).toString();

	//	QLabel *spinBox = dynamic_cast<QLabel *>( editor );
	//	spinBox->setText( value );
}

void RemovableItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	//	QLabel *spinBox = dynamic_cast<QLabel *>( editor );
	//
	//	QString value = spinBox->text();
	//
	//	model->setData( index, value, Qt::EditRole );
}

void RemovableItemDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	editor->setGeometry( option.rect );
}
void RemovableItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QStyledItemDelegate::paint( painter, option, index );
}

void DropButton::dragEnterEvent( QDragEnterEvent *event )
{
	auto widget = dynamic_cast<SharedTabWidget *>( event->source() );
	if ( !widget )
		return event->ignore();
	event->accept();
}

void DropButton::dropEvent( QDropEvent *event )
{
	if ( !dropListProvider )
		return;

	this->setText( "" );
	auto widget = dynamic_cast<SharedTabWidget *>( event->source() );
	auto model = dynamic_cast<QStandardItemModel *>( widget->model() );

	auto item = model->itemFromIndex( widget->currentIndex() );
	this->imageIndex = item->data( Qt::UserRole ).toInt();
	if ( !item->text().isEmpty() )
	{
		vtfpp::ImageFormat inputFormat;
		int inputWidth, inputHeight, inputFrameCount;
		auto data = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( item->text().toStdString() ), inputFormat, inputWidth, inputHeight, inputFrameCount );
		auto newData = vtfpp::ImageConversion::convertImageDataToFormat( data, inputFormat, vtfpp::ImageFormat::RGBA8888, inputWidth, inputHeight );
		auto img = QImage( reinterpret_cast<unsigned char *>( newData.data() ), inputWidth, inputHeight, QImage::Format_RGBA8888 );
		this->setIcon( QIcon( QPixmap::fromImage( img ) ) );
	}
	else
	{
		auto imageContainer = dropListProvider->imageList[this->imageIndex];
		auto newData = vtfpp::ImageConversion::convertImageDataToFormat( imageContainer->getData(), imageContainer->getFormat(), vtfpp::ImageFormat::RGBA8888, imageContainer->getWidth(), imageContainer->getHeight() );
		auto img = QImage( reinterpret_cast<unsigned char *>( newData.data() ), imageContainer->getWidth(), imageContainer->getHeight(), QImage::Format_RGBA8888 );
		this->setIcon( QIcon( QPixmap::fromImage( img ) ) );
	}

	this->setIconSize( this->size() );
	hasImageInside = true;
	emit onImageInserted();
	event->acceptProposedAction();
}
