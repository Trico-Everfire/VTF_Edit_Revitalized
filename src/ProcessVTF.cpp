#include "ProcessVTF.h"

#include "MainWindow.h"
#include "VTFEImageContainer.h"
#include "flagsandformats.hpp"

#include <QBuffer>
#include <QComboBox>
#include <QDropEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTabWidget>

ProcessVTF::ProcessVTF( QWidget *parent, vtfpp::VTF *vtf ) :
	QDialog( parent ), processVtf( vtf ), imageListList {}, fileName( "untitled" )
{
	this->setupUI();
	this->setAcceptDrops( true );
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

void ProcessVTF::setupUI()
{
	auto mainLayout = new QGridLayout( this );

	auto mainListWidget = new QTabWidget( this );
	auto imageProcessorWidget = new QWidget( mainListWidget );

	auto imageProcessorLayout = new QGridLayout( imageProcessorWidget );

	this->imageListList = new SharedTabWidget( imageProcessorWidget );
	this->imageListList->setDragDropMode( QAbstractItemView::DragOnly );
	this->imageListList->setModel( new QStandardItemModel( this->imageListList ) );
	this->imageListList->setItemDelegate( new RemovableItemDelegate( this->imageListList ) );

	this->imageListList->setDeleteOnDrag( false );

	imageProcessorLayout->addWidget( this->imageListList, 0, 0, 4, 1 );

	typeComboBox = new QComboBox( imageProcessorWidget );
	typeComboBox->addItem( "Single Frame Texture" );
	typeComboBox->addItem( "Animated Texture" );
	typeComboBox->addItem( "Cubemap/Envmap" );
	typeComboBox->addItem( "Animated Cubemap/Envmap" );
	typeComboBox->addItem( "Volumetric Texture" );
	imageProcessorLayout->addWidget( typeComboBox, 0, 1, 1, 2 );

	pFormatCombo = new QComboBox( this );
	pFormatCombo->setDisabled( true );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( vtfpp::ImageFormat::P8 != fmt.format )
			pFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}

	imageProcessorLayout->addWidget( pFormatCombo, 0, 3, 1, 2 );

	baseImageList = new SharedTabWidget( imageProcessorWidget );
	baseImageList->setMaximum( 1 );
	baseImageList->setItemDelegate( new RemovableItemDelegate( baseImageList ) );
	baseImageList->setModel( new QStandardItemModel( baseImageList ) );
	baseImageList->setDragDropMode( QAbstractItemView::DragDrop );
	baseImageList->setFixedHeight( 72 );
	imageProcessorLayout->addWidget( baseImageList, 1, 1, 1, 4 );

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

	auto buttons = new QDialogButtonBox( this );
	auto applButton = buttons->addButton( QDialogButtonBox::Apply );
	applButton->setDisabled( true );
	auto closeButton = buttons->addButton( QDialogButtonBox::Close );

	connect( applButton, &QPushButton::pressed, this, &ProcessVTF::accept );
	connect( closeButton, &QPushButton::pressed, this, &ProcessVTF::reject );
	auto model = dynamic_cast<QStandardItemModel *>( baseImageList->model() );
	connect( model, &QStandardItemModel::rowsInserted, this, [&, applButton]()
			 {
				 pFormatCombo->setEnabled( true );
				 applButton->setEnabled( true );
			 } );
	connect( model, &QStandardItemModel::rowsRemoved, this, [&, applButton]()
			 {
				 pFormatCombo->setDisabled( true );
				 applButton->setDisabled( true );
			 } );

	mainLayout->addWidget( buttons, 1, 0 );

	mainLayout->addWidget( mainListWidget, 0, 0 );
}

bool ProcessVTF::addImage( const QImage &image )
{
	auto nim = image.convertToFormat( QImage::Format_RGBA8888 );
	this->imageList[imageID++] = ( new VTFEImageContainer( reinterpret_cast<const std::byte *>( nim.constBits() ), nim.width(), nim.height(), vtfpp::ImageFormat::RGBA8888 ) );
	return true;
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
			auto img = QImage( reinterpret_cast<const unsigned char *>( vtf->getData().data() ), vtf->getWidth(), vtf->getHeight(), QImage::Format_RGBA8888 );
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

	return true;
}
void ProcessVTF::applyChanges()
{
	auto basemodel = dynamic_cast<QStandardItemModel *>( baseImageList->model() );

	auto rawDataNumber = basemodel->item( 0 )->data( Qt::UserRole ).toInt();

	auto imageContainer = imageList[rawDataNumber];
	if ( !imageContainer->getPath().isEmpty() )
		this->processVtf->setImage( imageContainer->getPath().toStdString() );
	else
		this->processVtf->setImage( imageContainer->getData(), imageContainer->getFormat(), imageContainer->getWidth(), imageContainer->getHeight() );

	this->processVtf->setFrameCount( frameList->model()->rowCount() + 1 );

	for ( int i = 0; i < frameList->model()->rowCount(); i++ )
	{
		auto frameModel = dynamic_cast<QStandardItemModel *>( frameList->model() );

		rawDataNumber = frameModel->item( i )->data( Qt::UserRole ).toInt();
		imageContainer = imageList[rawDataNumber];
		if ( !imageContainer->getPath().isEmpty() )
			processVtf->setImage( imageContainer->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::BILINEAR, 0, i + 1, 0, 0 );
		else
			processVtf->setImage( imageContainer->getData(), imageContainer->getFormat(), imageContainer->getWidth(), imageContainer->getHeight(), vtfpp::ImageConversion::ResizeFilter::BILINEAR, 0, i + 1, 0, 0 );
	}
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

void ProcessVTF::SharedTabWidget::dropEvent( QDropEvent *event )
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

void ProcessVTF::SharedTabWidget::dragMoveEvent( QDragMoveEvent *e )
{
	auto widget = dynamic_cast<SharedTabWidget *>( e->source() );
	if ( !widget )
		return e->ignore();

	if ( widget->selectedIndexes().count() > this->maximum )
		return e->ignore();

	e->accept();
	//	QListView::dragMoveEvent( e );
}
ProcessVTF::SharedTabWidget::SharedTabWidget( QWidget *parent ) :
	QListView( parent )
{
	//	auto model = new QStandardItemModel();
	//	this->setModel( model );
	setSelectionMode( MultiSelection );
}

QWidget *ProcessVTF::RemovableItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	auto clickableButtonItem = new QWidget( parent );

	auto clickableButtonLayout = new QHBoxLayout( clickableButtonItem );
	auto text = new QLabel( option.text, clickableButtonItem );
	clickableButtonLayout->addWidget( text );
	auto button = new QPushButton( "X", clickableButtonItem );
	button->setFixedSize( 24, 24 );
	clickableButtonLayout->addWidget( button );

	return clickableButtonItem; // QStyledItemDelegate::createEditor( parent, option, index );
}
ProcessVTF::RemovableItemDelegate::RemovableItemDelegate( QObject *parent ) :
	QStyledItemDelegate( parent )
{
}
void ProcessVTF::RemovableItemDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	QString value = index.model()->data( index, Qt::EditRole ).toString();

	//	QLabel *spinBox = dynamic_cast<QLabel *>( editor );
	//	spinBox->setText( value );
}
void ProcessVTF::RemovableItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	//	QLabel *spinBox = dynamic_cast<QLabel *>( editor );
	//
	//	QString value = spinBox->text();
	//
	//	model->setData( index, value, Qt::EditRole );
}
void ProcessVTF::RemovableItemDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	editor->setGeometry( option.rect );
}
void ProcessVTF::DropButton::dragEnterEvent( QDragEnterEvent *event )
{
	auto widget = dynamic_cast<SharedTabWidget *>( event->source() );
	if ( !widget )
		return event->ignore();
	event->accept();
}
void ProcessVTF::DropButton::dropEvent( QDropEvent *event )
{
	this->setText( "" );
	auto widget = dynamic_cast<SharedTabWidget *>( event->source() );
	auto model = dynamic_cast<QStandardItemModel *>( widget->model() );

	vtfpp::ImageFormat inputFormat;
	int inputWidth, inputHeight, inputFrameCount;
	auto data = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( model->itemFromIndex( widget->currentIndex() )->text().toStdString() ), inputFormat, inputWidth, inputHeight, inputFrameCount );
	auto newData = vtfpp::ImageConversion::convertImageDataToFormat( data, inputFormat, vtfpp::ImageFormat::RGBA8888, inputWidth, inputHeight );
	auto img = QImage( reinterpret_cast<unsigned char *>( newData.data() ), inputWidth, inputHeight, QImage::Format_RGBA8888 );
	this->setIcon( QIcon( QPixmap::fromImage( img ) ) );
	this->setIconSize( { 64, 64 } );
	event->acceptProposedAction();
}
