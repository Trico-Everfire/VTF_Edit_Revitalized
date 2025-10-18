#include "MainWindow.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"
#include "ApplicationOptionsDialog.h"
#include "EntryTree.h"
#include "ProcessVTF.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QComboBox>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QShortcut>
#include <QStyle>
#include <QTextEdit>
#include <vcryptpp/vcryptpp.h>

using namespace ui;

CMainWindow::CMainWindow() :
	QMainWindow()
{
	this->options = ApplicationOptions::getInstance();

	auto themeOptions = this->options->get( OPT_THEME_SETTINGS, ApplicationOptions::themeSettingsDefault ).toObject();
	auto selectedPalette = ApplicationOptions::getTheme( themeOptions.value( "theme" ).toVariant().value<ApplicationOptions::ApplicationPaletteOptions>() );
	QApplication::setPalette( selectedPalette );
	QApplication::setStyle( themeOptions.value( "style" ).toString() );

	setAcceptDrops( true );

	auto centralWidget = new QWidget( this );

	auto pMainLayout = new QGridLayout( centralWidget );

	this->setCentralWidget( centralWidget );

	pImageTabWidget = new VTFTabBar( this );

	pImageTabWidget->setTabsClosable( true );
	pImageTabWidget->setMovable( true );

	pMainLayout->addWidget( pImageTabWidget, 0, 1, Qt::AlignTop );

	m_pScrollWidget = new QWidget( this );
	m_pScrollWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	auto scrollLayout = new QGridLayout( m_pScrollWidget );

	pImageViewWidget = new ImageViewWidget( m_pScrollWidget );

	scrollLayout->addWidget( pImageViewWidget, 0, 0, 2, 1 );

	m_pHorizontalScrollBar = new QScrollBar( Qt::Horizontal, m_pScrollWidget );
	m_pHorizontalScrollBar->setMinimum( 0 );
	m_pHorizontalScrollBar->setMaximum( 4096 );
	m_pHorizontalScrollBar->setValue( 4096 / 2 );
	//	m_pHorizontalScrollBar->setSingleStep( 100 );
	m_pHorizontalScrollBar->setPageStep( 4096 * 4096 );
	m_pHorizontalScrollBar->setMinimumWidth( 512 );
	m_pHorizontalScrollBar->setMinimumHeight( 16 );
	m_pHorizontalScrollBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_pHorizontalScrollBar->setDisabled( true );

	pImageViewWidget->setXOffset( 4096 / 2 );
	pImageViewWidget->setYOffset( 4096 / 2 );

	scrollLayout->addWidget( m_pHorizontalScrollBar, 1, 0, Qt::AlignBottom );

	m_pVerticalScrollBar = new QScrollBar( Qt::Vertical, m_pScrollWidget );
	m_pVerticalScrollBar->setMinimum( 0 );
	m_pVerticalScrollBar->setMaximum( 4096 );
	m_pVerticalScrollBar->setValue( 4096 / 2 );
	m_pVerticalScrollBar->setPageStep( 4096 * 4096 );
	m_pVerticalScrollBar->setMinimumHeight( 512 );
	m_pVerticalScrollBar->setMinimumWidth( 16 );
	m_pVerticalScrollBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_pVerticalScrollBar->setDisabled( true );
	scrollLayout->addWidget( m_pVerticalScrollBar, 0, 1, Qt::AlignRight );

	pMainLayout->addWidget( m_pScrollWidget, 1, 1 );

	auto pSettingsFileSystemTab = new QTabWidget( this );

	pImageSettingsWidget = new ImageSettingsWidget( pImageViewWidget, this );

	pSettingsFileSystemTab->addTab( pImageSettingsWidget, "Settings" );

	pFileSystemTree = new EntryTree( pSettingsFileSystemTab );

	pSettingsFileSystemTab->addTab( pFileSystemTree, "File System" );

	pMainLayout->addWidget( pSettingsFileSystemTab, 0, 0, 2, 1, Qt::AlignLeft );

	auto pInfoResourceTabWidget = new QTabWidget( this );

	pImageInfo = new InfoWidget( this );

	pInfoResourceTabWidget->addTab( pImageInfo, "Info" );

	pResourceWidget = new ResourceWidget( this );

	pInfoResourceTabWidget->addTab( pResourceWidget, "Resources" );

	pAdditionalInformationWidget = new AdditionalInformationWidget( this );

	pInfoResourceTabWidget->addTab( pAdditionalInformationWidget, "Additional Information" );

	pMainLayout->addWidget( pInfoResourceTabWidget, 0, 2, 2, 1, Qt::AlignRight );

	m_pMainMenuBar = this->menuBar();
	m_pMainMenuBar->setNativeMenuBar( false );

	connect( pFileSystemTree, &EntryTree::doubleClicked, pFileSystemTree, [this]( const QModelIndex &parent )
			 {
				 if ( TreeItem *item = reinterpret_cast<TreeItem *>( parent.internalPointer() ) )
				 {
					 if ( item->getItemType() == TreeItem::VPK_INTERNAL )
					 {
						 TreeItem *mainParent = item;
						 while ( mainParent->getItemType() != TreeItem::VPK_FILE )
						 {
							 mainParent = mainParent->parentItem();
						 }

						 if ( item->getEntry().ends_with( "vtf" ) )
						 {
							 //							 							 auto data = mainParent->pakFile()->readEntry( mainParent->pakFile()->findEntry( item->getEntry() ).value() );
							 //							 //
							 //							 							 vtfpp::VTF *file = new vtfpp::VTF( data.value().data(), data.value().size() );
							 //							 							 addVTFToTab( file, item->getEntry().data() );
						 }
					 }
					 //					 model->fillItem( item );
					 //					 QTreeView::rowsInserted( parent, 0, model->rowCount( parent ) );
				 }
			 } );

	connect( m_pHorizontalScrollBar, &QScrollBar::valueChanged, pImageViewWidget, [&]( int val )
			 {
				 pImageViewWidget->setXOffset( val );
				 //				 qInfo() << m_pHorizontalScrollBar->maximum();
				 //				 if ( m_pHorizontalScrollBar->maximum() < 2 )
				 //				 {
				 //					 pImageViewWidget->setXOffset( 4096 / 2 );
				 //					 return;
				 //				 }
				 //				 pImageViewWidget->setXOffset( remap( val, m_pHorizontalScrollBar->minimum(), m_pHorizontalScrollBar->maximum(), 0, 4096 ) );
			 } );

	connect( m_pVerticalScrollBar, &QScrollBar::valueChanged, pImageViewWidget, [&]( int val )
			 {
				 //				 qInfo() << val;
				 pImageViewWidget->setYOffset( val );
				 // qInfo() << m_pVerticalScrollBar->maximum();
				 //				 if ( m_pVerticalScrollBar->maximum() < 2 )
				 //				 {
				 //					 pImageViewWidget->setYOffset( 4096 / 2 );
				 //					 return;
				 //				 }
				 //
				 //				 pImageViewWidget->setYOffset( remap( val, m_pVerticalScrollBar->minimum(), m_pVerticalScrollBar->maximum(), 0, 4096 ) );
			 } );

	connect( pImageTabWidget, &QTabBar::tabCloseRequested, this, &CMainWindow::removeVTFTab );

	connect( pImageTabWidget, &QTabBar::currentChanged, this, &CMainWindow::tabChanged );

	connect( pImageTabWidget, &VTFTabBar::onRightMouseClicked, this, &CMainWindow::openTabContextMenu );

	connect( pImageViewWidget, &ImageViewWidget::animated, pImageSettingsWidget, &ImageSettingsWidget::set_frame );

	connect( pImageSettingsWidget, &ImageSettingsWidget::fileModified, this, [&]
			 {
				 const auto key = pImageTabWidget->tabData( pImageTabWidget->currentIndex() ).value<intptr_t>();
				 if ( !this->vtfWidgetList.contains( key ) )
					 return;

				 this->vtfWidgetList[key].hasSaved = false;

				 pImageTabWidget->hasFilesChanged();
			 } );

	connect( pImageViewWidget, &ImageViewWidget::zoomChanged, this, [&]( float zoom )
			 {
				 //				 qInfo() << zoom;
				 //				 qInfo() << ( 4096 * 10 ) / zoom;
				 m_pHorizontalScrollBar->setPageStep( ( 4096 * 4 ) / zoom );
				 m_pVerticalScrollBar->setPageStep( ( 4096 * 4 ) / zoom );
				 //				 float value remap( 1, zoom, 0, 0, 100 );
				 //				 if ( zoom < 1 )
				 //					 value = 0;
				 //				 qInfo() << zoom;
				 //				 qInfo() << value;
				 //				 m_pHorizontalScrollBar->setPageStep( value * 100 );
				 //				 m_pHorizontalScrollBar->setMaximum( value );
				 //				 m_pHorizontalScrollBar->setValue( value / 2 );
				 //
				 //				 m_pVerticalScrollBar->setPageStep( value * 100 );
				 //				 m_pVerticalScrollBar->setMaximum( value );
				 //				 m_pVerticalScrollBar->setValue( value / 2 );
			 } );

	connect( pImageViewWidget, &ImageViewWidget::onRightClick, this, [this]()
			 {
				 auto current_index = pImageTabWidget->currentIndex();
				 auto ind = pImageTabWidget->tabData( current_index ).value<intptr_t>();
				 auto vtf = vtfWidgetList.value( ind );

				 if ( !vtf )
					 return;

				 QMenu *menu = new QMenu( this );
				 auto save = menu->addAction( "Save" );
				 auto saveAs = menu->addAction( "Save As" );
				 auto copy = menu->addAction( "Copy" );
				 auto edit = menu->addAction( "Edit" );
				 auto remove = menu->addAction( "Remove" );
				 connect( save, &QAction::triggered, this, &CMainWindow::saveCurrentVTFToFile );
				 connect( saveAs, &QAction::triggered, this, &CMainWindow::saveCurrentVTFToFileAs );
				 connect( copy, &QAction::triggered, this, [this, vtf]
						  {
							  int fr, fa, p, s;
							  pImageSettingsWidget->aquireFFPS( fr, fa, p, s );
							  auto data = vtf->getImageDataAsRGBA8888( p, fr, fa - 1, s );

							  auto img = QImage( (const unsigned char *)data.data(), (int)vtf->getWidth( p ), (int)vtf->getHeight( p ), QImage::Format_RGBA8888 );

							  if ( img.isNull() )
								  return;

							  QApplication::clipboard()->setPixmap( QPixmap::fromImage( img ) );
						  } );
				 connect( edit, &QAction::triggered, this, [&, ind]
						  {
							  if ( pImageTabWidget->count() < 1 )
								  return;

							  //							  const auto key = pImageTabWidget->tabData( ind ).value<intptr_t>();
							  if ( !this->vtfWidgetList.contains( ind ) )
								  return;
							  //
							  auto pVTF = &this->vtfWidgetList[ind];
							  auto editVTFDialog = std::make_unique<CVTFCreationDialog>( this, pVTF->vtf );

							  if ( editVTFDialog->exec() == QDialog::Rejected )
								  return;
							  pVTF->hasSaved = false;
							  pImageTabWidget->hasFilesChanged();
							  emit pImageTabWidget->currentChanged( pImageTabWidget->currentIndex() );
						  } );
				 connect( remove, &QAction::triggered, this, [&, current_index]
						  {
							  this->removeVTFTab( current_index );
						  } );
				 menu->popup( QCursor::pos() );
			 } );

	connect( pImageInfo->getSlider(), &QSlider::valueChanged, pImageViewWidget, &ImageViewWidget::setHDRGamma );
	connect( pImageInfo, &InfoWidget::spriteSheetInfoUpdated, pImageViewWidget, &ImageViewWidget::setSpriteSheet );

	connect( this, &CMainWindow::onDropped, this, &CMainWindow::processDroppedItems );
	//	connect( scrollWidget, &ZoomScrollArea::onScrollUp, this, [&, areaSize]
	//			 {
	//				 pImageViewWidget->zoom( 0.05 );
	//				 scrollWidget->verticalScrollBar()->setPageStep( ( areaSize.height() - ( areaSize.height() * pImageViewWidget->getZoom() ) ) );
	//				 scrollWidget->horizontalScrollBar()->setPageStep( ( areaSize.width() - ( areaSize.width() * pImageViewWidget->getZoom() ) ) );
	//			 } );
	//
	//	connect( scrollWidget, &ZoomScrollArea::onScrollDown, this, [&, areaSize]
	//			 {
	//				 pImageViewWidget->zoom( -0.05 );
	//				 scrollWidget->verticalScrollBar()->setPageStep( ( ( areaSize.height() * pImageViewWidget->getZoom() ) - ( areaSize.height() ) ) );
	//				 scrollWidget->horizontalScrollBar()->setPageStep( ( ( areaSize.width() * pImageViewWidget->getZoom() ) - ( areaSize.width() ) ) );
	//			 } );

	setupMenuBar();
	setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	adjustSize();

	new QShortcut( QKeySequence( Qt::CTRL | Qt::Key_Q ), this, SLOT( close() ) );
	new QShortcut( QKeySequence( Qt::CTRL | Qt::Key_S ), this, SLOT( saveCurrentVTFToFile() ) );
	new QShortcut( QKeySequence( QKeyCombination( Qt::CTRL | Qt::SHIFT | Qt::Key_S ) ), this, SLOT( saveCurrentVTFToFileAs() ) );
	new QShortcut( QKeyCombination( Qt::CTRL | Qt::ALT | Qt::SHIFT, Qt::Key_S ), this, SLOT( saveAllVTFsToFiles() ) );
	new QShortcut( QKeySequence( Qt::CTRL | Qt::Key_V ), this, SLOT( onPaste() ) );
	new QShortcut( QKeyCombination( Qt::CTRL | Qt::ALT, Qt::Key_O ), this, SLOT( openOptionsWindow() ) );
}

void CMainWindow::openOptionsWindow()
{
	auto optionsMenu = new ApplicationOptionsDialog( this );
	//	optionsMenu->setMinimumSize( 500, 400 );
	optionsMenu->exec();
}

bool CMainWindow::separateSpriteSheetVTF()
{
	const auto key = pImageTabWidget->tabData( pImageTabWidget->currentIndex() ).value<intptr_t>();
	auto pVTF = this->vtfWidgetList.value( key );

	if ( !pVTF )
		return false;

	if ( pVTF->getResources().empty() )
		return false;

	if ( !pVTF->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
		return false;

	auto saveLocation = QFileDialog::getExistingDirectory( this, "Save To:" );
	if ( saveLocation.isEmpty() )
		return false;

	auto spriteSheetData = pVTF->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA )->getDataAsParticleSheet();

	return true;
};

void CMainWindow::onPaste()
{
	auto imageBoard = QApplication::clipboard();

	if ( imageBoard->image().isNull() )
		return;

	auto clipboardVTF = new vtfpp::VTF();

	auto processVTF = std::make_unique<CVTFCreationDialog>( this, clipboardVTF );

	processVTF->addImage( imageBoard->image() );

	if ( processVTF->exec() )
		addVTFToTab( clipboardVTF, "Clipboard" );
}

vtfpp::VTF *CMainWindow::getVTFFromVTFFile( const char *path )
{
	return new vtfpp::VTF( path );
}

void CMainWindow::addVTFFromPathToTab( const QString &path )
{
	QFileInfo fileInfo( path );

	auto pVTF = getVTFFromVTFFile( fileInfo.filePath().toUtf8().constData() );

	addVTFToTab( pVTF, fileInfo.fileName(), fileInfo.filePath(), true );
}

void CMainWindow::addVTFToTab( vtfpp::VTF *pVTF, const QString &name, const QString &path, bool fromFile )
{
	if ( pVTF )
	{
		this->vtfWidgetList.insert( reinterpret_cast<intptr_t>( pVTF ), { pVTF, path, name, fromFile, fromFile } );

		int index = pImageTabWidget->addTab( name );

		pImageTabWidget->setTabData( index, QVariant::fromValue( reinterpret_cast<intptr_t>( pVTF ) ) );

		pImageTabWidget->setCurrentIndex( index );

		pImageTabWidget->hasFilesChanged();

		// When creating the first tab, we need to call currentChanged, because setCurrentIndex
		// fires currentChanged when the first tab is created before we store VTF data.
		// So to apply the VTF changes we need to call it ourselves.
		if ( pImageTabWidget->count() == 1 )
			pImageTabWidget->currentChanged( index );
	}
}

void CMainWindow::removeVTFTab( int index )
{
	// we own the VTF, so we dispose of it too.
	const auto key = pImageTabWidget->tabData( index ).value<intptr_t>();
	auto vtf = this->vtfWidgetList.value( key );

	if ( !vtf.hasSaved )
	{
		auto dialogResult = QMessageBox::question( this, "Save VTF?", "Unsaved changes. Do you want to save the VTF?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
		if ( dialogResult == QMessageBox::Yes )
			saveVTFToFile( key );
		if ( dialogResult == QMessageBox::Cancel )
			return;
	}

	this->vtfWidgetList.remove( key );

	if ( pImageTabWidget->currentIndex() == index )
	{
		pImageViewWidget->set_vtf( nullptr );
		pResourceWidget->set_vtf( nullptr );
		pAdditionalInformationWidget->set_vtf( nullptr );
		pImageSettingsWidget->set_vtf( { nullptr } );
		pImageInfo->update_info( nullptr );
	}

	pImageTabWidget->removeTab( index );

	delete vtf.vtf;
}

void CMainWindow::tabChanged( int index )
{
	const auto key = pImageTabWidget->tabData( index ).value<intptr_t>();

	auto pVTF = this->vtfWidgetList.value( key );

	pImageViewWidget->set_vtf( pVTF.vtf );
	pImageViewWidget->set_rgba( redBox->isChecked(), greenBox->isChecked(), blueBox->isChecked(), alphaBox->isChecked() );
	pImageViewWidget->stopAnimating();
	pResourceWidget->set_vtf( pVTF.vtf );
	pAdditionalInformationWidget->set_vtf( pVTF.vtf );
	//	pImageSettingsWidget->set_vtf( { nullptr } );
	pImageSettingsWidget->set_vtf( pVTF );
	pImageInfo->update_info( pVTF.vtf );

	//	if ( !pVTF )
	//	{
	//		//setSliderPosition( pVTF->GetHeight() / 2 );
	//		scrollWidget->horizontalScrollBar()->//setSliderPosition( pVTF->GetWidth() / 2 );
	//	}
	m_pHorizontalScrollBar->setEnabled( pVTF );
	m_pHorizontalScrollBar->setValue( 4096 / 2 );
	m_pVerticalScrollBar->setEnabled( pVTF );
	m_pVerticalScrollBar->setValue( 4096 / 2 );
}

void CMainWindow::setupMenuBar()
{
	auto pFileMenuTab = m_pMainMenuBar->addMenu( "File" );
	pFileMenuTab->addAction( "Open", this, &CMainWindow::openVTF );
	pFileMenuTab->addAction( "Save", this, &CMainWindow::saveCurrentVTFToFile );
	pFileMenuTab->addAction( "Save As", this, &CMainWindow::saveCurrentVTFToFileAs );
	pFileMenuTab->addAction( "Save All", this, &CMainWindow::saveAllVTFsToFiles );
	pFileMenuTab->addAction( "Export", this, &CMainWindow::exportVTFToFile );
	pFileMenuTab->addAction( "Import...", this, &CMainWindow::importFromFile );

	auto pToolMenuTab = m_pMainMenuBar->addMenu( "Tools" );
	pToolMenuTab->addAction( "VTF Version Editor (Individual)", this, &CMainWindow::compressVTFFile )->setDisabled( true );
	pToolMenuTab->addAction( "VTF Version Editor (Batch)", this, &CMainWindow::compressVTFFolder )->setDisabled( true );
	pToolMenuTab->addAction( "Batch Convert", this, &CMainWindow::batchConvert )->setDisabled( true );
	pToolMenuTab->addAction( "FontToVTF", this, &CMainWindow::fontToVTF );
	pToolMenuTab->addAction( "Combine RGBA / Create MRAO", this, &CMainWindow::importMRAO )->setDisabled( true );

	auto pViewMenu = m_pMainMenuBar->addMenu( "View" );
	redBox = createCheckableAction( "Red", pViewMenu );
	greenBox = createCheckableAction( "Green", pViewMenu );
	blueBox = createCheckableAction( "Blue", pViewMenu );
	alphaBox = createCheckableAction( "Alpha", pViewMenu );
	connect( redBox, &QAction::triggered, [this]( bool checked )
			 {
				 pImageViewWidget->set_red( checked );
			 } );
	connect( greenBox, &QAction::triggered, [this]( bool checked )
			 {
				 pImageViewWidget->set_green( checked );
			 } );
	connect( blueBox, &QAction::triggered, [this]( bool checked )
			 {
				 pImageViewWidget->set_blue( checked );
			 } );
	connect( alphaBox, &QAction::triggered, [this]( bool checked )
			 {
				 pImageViewWidget->set_alpha( checked );
			 } );
	pViewMenu->addAction( redBox );
	pViewMenu->addAction( greenBox );
	pViewMenu->addAction( blueBox );
	pViewMenu->addAction( alphaBox );

	auto pHelpMenu = m_pMainMenuBar->addMenu( "Help" );
	pHelpMenu->addAction( "Options", this, SLOT( openOptionsWindow() ) );

	auto pAbout = m_pMainMenuBar->addMenu( "About" );
	pAbout->addAction( "About QT", qApp, &QApplication::aboutQt );
	pAbout->addAction( "About VTF Forge", this, &CMainWindow::About );
}

QAction *CMainWindow::createCheckableAction( const QString &name, QObject *parent )
{
	auto maskBox = new QAction( name, parent );
	maskBox->setCheckable( true );
	maskBox->setChecked( true );
	return maskBox;
}

void CMainWindow::compressVTFFile()
{
	auto configurationMenu = new CVTFCreationDialog( this, nullptr );

	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open VTF", recentPaths.last(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePaths.isEmpty() )
		return;

	if ( recentPaths.contains( filePaths[0] ) )
		recentPaths.removeAt( recentPaths.indexOf( filePaths[0] ) );
	recentPaths.push_back( filePaths[0] );

	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	configurationMenu->exec();

	for ( const auto &path : recentPaths )
	{
		auto vtf = std::make_unique<vtfpp::VTF>( path.toStdString() );
		if ( !vtf )
			return;

		configurationMenu->setVTF( vtf.get() );
		configurationMenu->applyChanges();
	}
	//	configurationMenu->applyChanges()

	return;
#ifdef FALSE
	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open VTF", recentPaths.last(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePaths.isEmpty() )
		return;

	if ( recentPaths.contains( filePaths[0] ) )
		recentPaths.removeAt( recentPaths.indexOf( filePaths[0] ) );
	recentPaths.push_back( filePaths[0] );

	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	auto pCompressionDialog = new QDialog( this );
	pCompressionDialog->setWindowTitle( "VTF Version Editor" );
	auto vBLayout = new QGridLayout( pCompressionDialog );

	auto label1 = new QLabel( "VTF Version:", this );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );

	auto pVtfVersionBox = new QComboBox( this );
	int setbackIndex = 1;
#ifdef CHAOS_INITIATIVE
	setbackIndex = 2;
	for ( int i = 0; i <= 6; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( 7 ) + "." + QString::number( i ), i );
	}
	pVtfVersionBox->setCurrentIndex( pVtfVersionBox->count() - setbackIndex );
	vBLayout->addWidget( pVtfVersionBox, 0, 1, Qt::AlignRight );

#ifdef CHAOS_INITIATIVE
	auto pAuxCompressionBox = new QCheckBox( pCompressionDialog );
	pAuxCompressionBox->setText( tr( "AUX Compression" ) );
	pAuxCompressionBox->setDisabled( true );
	vBLayout->addWidget( pAuxCompressionBox, 1, 0, Qt::AlignLeft );

	auto label2 = new QLabel( "Aux Compression Level:", pCompressionDialog );
	label2->setDisabled( true );

	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	auto pAuxCompressionLevelBox = new QComboBox( pCompressionDialog );
	for ( int i = 0; i <= 9; i++ )
	{
		pAuxCompressionLevelBox->addItem( QString::number( i ), i );
	}
	pAuxCompressionLevelBox->setDisabled( true );
	pAuxCompressionLevelBox->setCurrentIndex( pAuxCompressionLevelBox->count() - 1 );
	vBLayout->addWidget( pAuxCompressionLevelBox, 2, 1, Qt::AlignRight );
#endif

	auto pCustomDestination = new QCheckBox( "Custom Destination", pCompressionDialog );
	vBLayout->addWidget( pCustomDestination, 3, 0 );

	auto pDestinationLocation = new QLineEdit( "In Place", pCompressionDialog );
	pDestinationLocation->setReadOnly( true );
	pDestinationLocation->setDisabled( true );

	vBLayout->addWidget( pDestinationLocation, 4, 0 );

	auto pSelectDestinationLocation = new QPushButton( pCompressionDialog );
	pSelectDestinationLocation->setDisabled( true );

	pSelectDestinationLocation->setIcon( qApp->style()->standardIcon( QStyle::SP_FileDialogContentsView ) );

	vBLayout->addWidget( pSelectDestinationLocation, 4, 1 );

	auto pRecomputeReflectivity = new QCheckBox( "Recompute Reflectivity", pCompressionDialog );

	vBLayout->addWidget( pRecomputeReflectivity, 5, 0, 1, 2 );

	auto pButtonLayout = new QHBoxLayout();

	auto pOkButton = new QPushButton( "Update Version", pCompressionDialog );
	pButtonLayout->addWidget( pOkButton, Qt::AlignCenter );

	auto pCancelButton = new QPushButton( "Cancel", pCompressionDialog );
	pButtonLayout->addWidget( pCancelButton, Qt::AlignCenter );

	vBLayout->addLayout( pButtonLayout, 6, 0, 1, 2 );

	bool compress = false;

#ifdef CHAOS_INITIATIVE
	connect( pVtfVersionBox, &QComboBox::currentTextChanged, pCompressionDialog, [&pVtfVersionBox, &pAuxCompressionBox]()
			 {
				 pAuxCompressionBox->setEnabled( pVtfVersionBox->currentData().toInt() >= 6 );
				 pAuxCompressionBox->toggled( pVtfVersionBox->currentData().toInt() >= 6 && pAuxCompressionBox->isChecked() );
			 } );

	connect( pAuxCompressionBox, &QCheckBox::toggled, pCompressionDialog, [&pAuxCompressionLevelBox, &label2]( bool checked )
			 {
				 pAuxCompressionLevelBox->setEnabled( checked );
				 label2->setEnabled( checked );
			 } );
#endif

	connect( pCustomDestination, &QCheckBox::toggled, pCompressionDialog, [&pDestinationLocation, &pSelectDestinationLocation]( bool checked )
			 {
				 pDestinationLocation->setEnabled( checked );
				 pDestinationLocation->setText( "" );
				 pSelectDestinationLocation->setEnabled( checked );
			 } );

	connect( pSelectDestinationLocation, &QPushButton::pressed, pCompressionDialog, [&pDestinationLocation, &recentPaths]
			 {
				 pDestinationLocation->setText( QFileDialog::getExistingDirectory( nullptr, "Save to:", recentPaths.last() ) );
			 } );

	// This is fine, ->exec() stalls the application until closed so compress never falls
	// out of scope.
	connect( pOkButton, &QPushButton::pressed, pCompressionDialog, [&pCompressionDialog, &compress]
			 {
				 compress = true;
				 pCompressionDialog->close();
			 } );

	connect( pCancelButton, &QPushButton::pressed, pCompressionDialog, &QDialog::close );

	pCompressionDialog->exec();

	if ( !compress )
		return;

	if ( recentPaths.contains( pDestinationLocation->text() ) )
		recentPaths.removeAt( recentPaths.indexOf( pDestinationLocation->text() ) );
	recentPaths.push_back( pDestinationLocation->text() );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	QString pathDirectory {};
	if ( pCustomDestination->isChecked() )
	{
		if ( !pDestinationLocation->text().isEmpty() )
			pathDirectory = pDestinationLocation->text();
		else
		{
			QMessageBox::warning( this, "EMPTY DESTINATION!", "The destination is left empty, cancelling.", QMessageBox::Ok );
			return;
		}
	}

	foreach( QString filePath, filePaths )
	{
		vtfpp::VTF *pVTF = getVTFFromVTFFile( filePath.toUtf8().constData() );

		if ( !pVTF )
		{
			QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
			return;
		}

#ifdef CHAOS_INITIATIVE
		if ( pVTF->getVersion() == pVtfVersionBox->currentData().toInt() && pVTF->getCompressionLevel() == pAuxCompressionLevelBox->currentData().toInt() )
			continue;
#else
		if ( pVTF->getVersion() == pVtfVersionBox->currentData().toInt() )
			continue;
#endif

		pVTF->setVersion( pVtfVersionBox->currentData().toInt() );

#ifdef CHAOS_INITIATIVE
		if ( pAuxCompressionBox->isChecked() )
		{
			pVTF->setCompressionLevel( pAuxCompressionLevelBox->currentData().toInt() );
		}
#endif

		if ( pRecomputeReflectivity->isChecked() )
		{
			pVTF->computeReflectivity();
		}

		if ( pathDirectory.isEmpty() )
			pVTF->bake( filePath.toUtf8().constData() );
		else
			pVTF->bake( ( pathDirectory + "/" + QFileInfo( filePath ).fileName() ).toUtf8().constData() );

		delete pVTF;
	}
#endif
}

void CMainWindow::compressVTFFolder()
{
#ifdef COMPRESSVTF

	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();
	QString dirPath = QFileDialog::getExistingDirectory(
		this, "Open VTF", recentPaths.last(), QFileDialog::Option::DontUseNativeDialog );

	if ( dirPath.isEmpty() )
		return;

	if ( recentPaths.contains( dirPath ) )
		recentPaths.removeAt( recentPaths.indexOf( dirPath ) );
	recentPaths.push_back( dirPath );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	auto pCompressionDialog = new QDialog( this );
	pCompressionDialog->setWindowTitle( "VTF Version Editor" );
	auto vBLayout = new QGridLayout( pCompressionDialog );

	auto label1 = new QLabel( "VTF Version:", this );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );

	auto pVtfVersionBox = new QComboBox( this );
	int setbackIndex = 1;
#ifdef CHAOS_INITIATIVE
	setbackIndex = 2;
	for ( int i = 0; i <= 6; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( 7 ) + "." + QString::number( i ), i );
	}
	pVtfVersionBox->setCurrentIndex( pVtfVersionBox->count() - setbackIndex );
	vBLayout->addWidget( pVtfVersionBox, 0, 1, Qt::AlignRight );

#ifdef CHAOS_INITIATIVE
	auto pAuxCompressionBox = new QCheckBox( pCompressionDialog );
	pAuxCompressionBox->setText( tr( "AUX Compression" ) );
	pAuxCompressionBox->setDisabled( true );
	vBLayout->addWidget( pAuxCompressionBox, 1, 0, Qt::AlignLeft );

	auto label2 = new QLabel( "Aux Compression Level:", pCompressionDialog );
	label2->setDisabled( true );

	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	auto pAuxCompressionLevelBox = new QComboBox( pCompressionDialog );
	for ( int i = 0; i <= 9; i++ )
	{
		pAuxCompressionLevelBox->addItem( QString::number( i ), i );
	}
	pAuxCompressionLevelBox->setDisabled( true );
	pAuxCompressionLevelBox->setCurrentIndex( pAuxCompressionLevelBox->count() - 1 );
	vBLayout->addWidget( pAuxCompressionLevelBox, 2, 1, Qt::AlignRight );
#endif

	auto pCustomDestination = new QCheckBox( "Custom Destination", pCompressionDialog );
	vBLayout->addWidget( pCustomDestination, 3, 0 );

	auto pDestinationLocation = new QLineEdit( "In Place", pCompressionDialog );
	pDestinationLocation->setReadOnly( true );
	pDestinationLocation->setDisabled( true );

	vBLayout->addWidget( pDestinationLocation, 4, 0 );

	auto pSelectDestinationLocation = new QPushButton( pCompressionDialog );
	pSelectDestinationLocation->setDisabled( true );

	pSelectDestinationLocation->setIcon( qApp->style()->standardIcon( QStyle::SP_FileDialogContentsView ) );

	vBLayout->addWidget( pSelectDestinationLocation, 4, 1 );

	auto pRecomputeReflectivity = new QCheckBox( "Recompute Reflectivity", pCompressionDialog );

	vBLayout->addWidget( pRecomputeReflectivity, 5, 0, 1, 2 );

	auto pButtonLayout = new QHBoxLayout();

	auto pOkButton = new QPushButton( "Update Version", pCompressionDialog );
	pButtonLayout->addWidget( pOkButton, Qt::AlignCenter );

	auto pCancelButton = new QPushButton( "Cancel", pCompressionDialog );
	pButtonLayout->addWidget( pCancelButton, Qt::AlignCenter );

	vBLayout->addLayout( pButtonLayout, 6, 0, 1, 2 );

	bool compress = false;

#ifdef CHAOS_INITIATIVE
	connect( pVtfVersionBox, &QComboBox::currentTextChanged, pCompressionDialog, [pVtfVersionBox, pAuxCompressionBox, pAuxCompressionLevelBox, label2]()
			 {
				 pAuxCompressionBox->setEnabled( pVtfVersionBox->currentData().toInt() >= 6 );
				 pAuxCompressionBox->toggled( pVtfVersionBox->currentData().toInt() >= 6 && pAuxCompressionBox->isChecked() );
			 } );

	connect( pAuxCompressionBox, &QCheckBox::toggled, pCompressionDialog, [pAuxCompressionLevelBox, label2]( bool checked )
			 {
				 pAuxCompressionLevelBox->setEnabled( checked );
				 label2->setEnabled( checked );
			 } );
#endif

	connect( pCustomDestination, &QCheckBox::toggled, pCompressionDialog, [pDestinationLocation, pSelectDestinationLocation]( bool checked )
			 {
				 pDestinationLocation->setEnabled( checked );
				 pDestinationLocation->setText( "" );
				 pSelectDestinationLocation->setEnabled( checked );
			 } );

	connect( pSelectDestinationLocation, &QPushButton::pressed, pCompressionDialog, [pDestinationLocation, &recentPaths]
			 {
				 pDestinationLocation->setText( QFileDialog::getExistingDirectory( nullptr, "Save to:", recentPaths.last() ) );
			 } );

	// This is fine, ->exec() stalls the application until closed so compress never falls
	// out of scope.
	connect( pOkButton, &QPushButton::pressed, pCompressionDialog, [pCompressionDialog, &compress]
			 {
				 compress = true;
				 pCompressionDialog->close();
			 } );

	connect( pCancelButton, &QPushButton::pressed, pCompressionDialog, &QDialog::close );

	pCompressionDialog->exec();

	if ( !compress )
		return;

	if ( recentPaths.contains( pDestinationLocation->text() ) )
		recentPaths.removeAt( recentPaths.indexOf( pDestinationLocation->text() ) );
	recentPaths.push_back( pDestinationLocation->text() );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	QString pathDirectory {};
	if ( pCustomDestination->isChecked() )
	{
		if ( !pDestinationLocation->text().isEmpty() )
			pathDirectory = pDestinationLocation->text();
		else
		{
			QMessageBox::warning( this, "EMPTY DESTINATION!", "The destination is left empty, cancelling.", QMessageBox::Ok );
			return;
		}
	}

	//	QString pathless;

	QStringList VTFPaths;
	QDirIterator it( dirPath, QStringList() << "*.vtf", QDir::Files, QDirIterator::Subdirectories );
	while ( it.hasNext() )
	{
		QString path = it.next();

		QStringList temp = path.split( dirPath );
		temp.pop_front();
		QStringList temp2 = temp.join( "" ).split( "/" );
		if ( temp2.size() >= 2 )
		{
			temp2.pop_front();
			temp2.pop_back();
		}

		if ( !pathDirectory.isEmpty() )
		{
			QString dirCreator = pathDirectory;
			for ( const auto &tPath : temp2 )
			{
				dirCreator += "/" + tPath;
				if ( !QDir().exists( dirCreator ) )
					QDir().mkdir( dirCreator );
			}
		}

		std::unique_ptr<vtfpp::VTF> pVTF( getVTFFromVTFFile( path.toStdString().c_str() ) );

		if ( !pVTF || !*pVTF )
		{
			QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.\n" + dirPath, QMessageBox::Ok );
			continue;
		}
#ifdef CHAOS_INITIATIVE
		if ( pVTF->getVersion() == pVtfVersionBox->currentData().toInt() && pVTF->getCompressionLevel() == pAuxCompressionLevelBox->currentData().toInt() && pathDirectory.isEmpty() )
			continue;
#else
		if ( pVTF->getVersion() == pVtfVersionBox->currentData().toInt() && pathDirectory.isEmpty() )
			continue;
#endif
		pVTF->setVersion( pVtfVersionBox->currentData().toInt() );

#ifdef CHAOS_INITIATIVE
		if ( pAuxCompressionBox->isChecked() )
		{
			pVTF->setCompressionLevel( pAuxCompressionLevelBox->currentData().toInt() );
		}
#endif

		if ( pRecomputeReflectivity->isChecked() )
		{
			pVTF->computeReflectivity();
		}

		if ( pathDirectory.isEmpty() )
		{
			pVTF->bake( path.toUtf8().constData() );
			if ( !true )
			{
				QMessageBox::warning( this, "Unable to save VTF", "The VTF cannot be saved.\n" + dirPath, QMessageBox::Ok );
			}
		}
		else
		{
			pVTF->bake( ( pathDirectory + "/" + temp.join( "" ) ).toUtf8().constData() );
			if ( false )
			{
				QMessageBox::warning( this, "Unable to save VTF", "The VTF cannot be saved.\n" + dirPath, QMessageBox::Ok );
			}
		}
	}
#endif
}

struct VTFFolder
{
	bool isAnimation = false;
	QStringList paths {};
};

void CMainWindow::batchConvert()
{
	auto batchConvertQDialog = new QDialog( this );
	batchConvertQDialog->setWindowTitle( "Batch Convert" );
	auto batchLayout = new QGridLayout( batchConvertQDialog );
	auto batchOptionsGroup = new QGroupBox( "Options:", batchConvertQDialog );
	batchOptionsGroup->setMinimumWidth( 320 );
	auto batchOptionsLayout = new QGridLayout( batchOptionsGroup );

	auto optionsfolderInputLabel = new QLabel( "Input Folder:", batchOptionsGroup );
	batchOptionsLayout->addWidget( optionsfolderInputLabel, 0, 0 );
	auto optionsFolderInputLineEdit = new QLineEdit( batchOptionsGroup );
	batchOptionsLayout->addWidget( optionsFolderInputLineEdit, 0, 1, 1, 3 );
	auto optionsFolderInputButton = new QPushButton( "...", batchOptionsGroup );
	optionsFolderInputButton->setFixedSize( 20, 20 );
	batchOptionsLayout->addWidget( optionsFolderInputButton, 0, 4 );

	auto optionsfolderOutputLabel = new QLabel( "Output Folder:", batchOptionsGroup );
	batchOptionsLayout->addWidget( optionsfolderOutputLabel, 1, 0 );
	auto optionsFolderOutputLineEdit = new QLineEdit( batchOptionsGroup );
	batchOptionsLayout->addWidget( optionsFolderOutputLineEdit, 1, 1, 1, 3 );
	auto optionsFolderOutputButton = new QPushButton( "...", batchOptionsGroup );
	optionsFolderOutputButton->setFixedSize( 20, 20 );
	batchOptionsLayout->addWidget( optionsFolderOutputButton, 1, 4 );

	QString toVTFString = "*.tga";
	QString toImageString = "*.vtf";

	auto toOrFrom = new QComboBox( batchOptionsGroup );
	toOrFrom->addItem( "To VTF", true );
	toOrFrom->addItem( "To Image", false );
	batchOptionsLayout->addWidget( toOrFrom, 2, 0 );

	auto optionsToSelectedFormat = new QComboBox( batchOptionsGroup );
	optionsToSelectedFormat->addItem( ".tga" );
	optionsToSelectedFormat->hide();
	batchOptionsLayout->addWidget( optionsToSelectedFormat, 2, 1 );

	auto optionsToVTFLineEdit = new QLineEdit( "*.tga", batchOptionsGroup );
	batchOptionsLayout->addWidget( optionsToVTFLineEdit, 2, 1, 1, 3 );

	auto OptionsVTFConversionOptionDisplay = new QPushButton( "?", batchOptionsGroup );
	OptionsVTFConversionOptionDisplay->setFixedSize( 20, 20 );
	batchOptionsLayout->addWidget( OptionsVTFConversionOptionDisplay, 2, 4 );

	auto optionsRecursiveCheckBox = new QCheckBox( "Recursive", batchOptionsGroup );
	optionsRecursiveCheckBox->setChecked( true );
	batchOptionsLayout->addWidget( optionsRecursiveCheckBox, 4, 0 );

	auto optionsExportFFSCheckBox = new QCheckBox( "Export frames/faces/slices", batchOptionsGroup );
	optionsExportFFSCheckBox->hide();
	batchOptionsLayout->addWidget( optionsExportFFSCheckBox, 4, 1, 1, 2 );

	auto optionsExportFFSDisplayButton = new QPushButton( "?", batchOptionsGroup );
	optionsExportFFSDisplayButton->setFixedSize( 20, 20 );
	optionsExportFFSDisplayButton->hide();
	batchOptionsLayout->addWidget( optionsExportFFSDisplayButton, 4, 4 );

	batchLayout->addWidget( batchOptionsGroup, 0, 0 );

	auto progressGroup = new QGroupBox( "Progress:", batchConvertQDialog );
	auto progressLayout = QHBoxLayout( progressGroup );
	QProgressBar *frogressBar = new QProgressBar( this );
	frogressBar->setMinimum( 0 );
	frogressBar->setTextVisible( true );
	frogressBar->setMinimumSize( 128, 20 );
	progressLayout.addWidget( frogressBar );

	batchLayout->addWidget( progressGroup, 1, 0 );

	auto logGroupBox = new QGroupBox( "Log:", batchConvertQDialog );
	auto logLayout = new QVBoxLayout( logGroupBox );
	auto logTextBlock = new QTextEdit( logGroupBox );
	logTextBlock->setReadOnly( true );
	logTextBlock->setMinimumSize( 128, 100 );
	logLayout->addWidget( logTextBlock );

	batchLayout->addWidget( logGroupBox, 2, 0 );

	auto buttonsLayout = new QGridLayout();
	auto optionsButton = new QPushButton( "Options", batchConvertQDialog );
	buttonsLayout->addWidget( optionsButton, 0, 0, Qt::AlignLeft );

	auto convertCloseButtonBox = new QDialogButtonBox( batchConvertQDialog );
	convertCloseButtonBox->addButton( "Convert", QDialogButtonBox::ApplyRole );
	convertCloseButtonBox->addButton( "Close", QDialogButtonBox::RejectRole );
	buttonsLayout->addWidget( convertCloseButtonBox, 0, 1, Qt::AlignRight );

	batchLayout->addLayout( buttonsLayout, 3, 0 );

	batchConvertQDialog->resize( 0, 0 ); // Make the window the smallest it can be.

	// auto pVTFImportWindow = CVTFCreationDialog::Standalone( this );
	auto vtf = new vtfpp::VTF();
	auto processVTF = std::make_unique<CVTFCreationDialog>( this, vtf );
	connect( OptionsVTFConversionOptionDisplay, &QPushButton::pressed, this, [batchConvertQDialog]
			 {
				 auto displayDialog = new QDialog( batchConvertQDialog );
				 auto displayLayout = new QVBoxLayout( displayDialog );
				 auto displayLabel = new QLabel( "having a file named `.animation.txt` inside a folder\nwill treat the entire folder's image contents\nas animation frames rather than individual VTFs.\nThis may result in the `.txt` file being invisible,\nmake sure you allow viewing hidden\nfiles in your file explorer settings.", displayDialog );
				 auto baseFont = displayLabel->font();
				 baseFont.setPointSize( 12 );
				 displayLabel->setFont( baseFont );
				 displayLayout->addWidget( displayLabel );
				 auto displayCloseButton = new QPushButton( "Close", displayDialog );
				 connect( displayCloseButton, &QPushButton::pressed, displayDialog, &QDialog::close );
				 displayLayout->addWidget( displayCloseButton );
				 displayDialog->show();
			 } );
	connect( optionsExportFFSDisplayButton, &QPushButton::pressed, this, [batchConvertQDialog]
			 {
				 auto displayDialog = new QDialog( batchConvertQDialog );
				 auto displayLayout = new QVBoxLayout( displayDialog );
				 auto displayLabel = new QLabel( "Exporting frames/faces/slices will suffix the exported image with _frame(X), _face(X) and/or _slice(X)\nWhere (X) will be replaced with the corresponding frame/face/slice.", displayDialog );
				 auto baseFont = displayLabel->font();
				 baseFont.setPointSize( 12 );
				 displayLabel->setFont( baseFont );
				 displayLayout->addWidget( displayLabel );
				 auto displayCloseButton = new QPushButton( "Close", displayDialog );
				 connect( displayCloseButton, &QPushButton::pressed, displayDialog, &QDialog::close );
				 displayLayout->addWidget( displayCloseButton );
				 displayDialog->show();
			 } );
	connect( optionsButton, &QPushButton::pressed, processVTF.get(), [&, vtf]
			 {
				 if ( !processVTF->exec() )
					 delete vtf;
			 } );
	connect( toOrFrom, &QComboBox::currentIndexChanged, batchOptionsGroup, [toOrFrom, optionsToSelectedFormat, batchOptionsLayout, optionsToVTFLineEdit, OptionsVTFConversionOptionDisplay, optionsExportFFSDisplayButton, optionsExportFFSCheckBox, &toVTFString, &toImageString]
			 {
				 int curr = toOrFrom->currentData().toBool();
				 if ( curr )
				 {
					 optionsToVTFLineEdit->setText( toVTFString );
					 optionsToSelectedFormat->hide();
					 optionsExportFFSDisplayButton->hide();
					 optionsExportFFSCheckBox->hide();
					 batchOptionsLayout->addWidget( optionsToVTFLineEdit, 2, 1, 1, 3 );
					 OptionsVTFConversionOptionDisplay->show();
				 }
				 else
				 {
					 optionsToVTFLineEdit->setText( toImageString );
					 optionsToSelectedFormat->show();
					 optionsExportFFSDisplayButton->show();
					 optionsExportFFSCheckBox->show();
					 batchOptionsLayout->addWidget( optionsToVTFLineEdit, 2, 2, 1, 2 );
					 OptionsVTFConversionOptionDisplay->hide();
				 }
			 } );
	connect( optionsToVTFLineEdit, &QLineEdit::textChanged, batchOptionsGroup, [toOrFrom, &toVTFString, &toImageString]( const QString &str )
			 {
				 int curr = toOrFrom->currentData().toBool();
				 if ( curr )
					 toVTFString = str;
				 else
					 toImageString = str;
			 } );

	batchConvertQDialog->exec();

	//	auto recentPaths = options->get(STR_OPEN_RECENT).toVariant().toStringList();
	//
	//	QString importFrom = QFileDialog::getExistingDirectory(
	//		this, "Import From", recentPaths.last(),
	//		QFileDialog::Option::DontUseNativeDialog );
	//
	//	QString exportTo = QFileDialog::getExistingDirectory(
	//		this, "Export To", recentPaths.last(),
	//		QFileDialog::Option::DontUseNativeDialog );
	//
	//	if ( importFrom.isEmpty() )
	//		return;
	//
	//	if ( exportTo.isEmpty() )
	//		return;
	//
	//	if ( recentPaths.contains( importFrom ) )
	//		recentPaths.removeAt( recentPaths.indexOf( importFrom ) );
	//	recentPaths.push_back( importFrom );
	//
	//	if ( recentPaths.contains( exportTo ) )
	//		recentPaths.removeAt( recentPaths.indexOf( exportTo ) );
	//	recentPaths.push_back( exportTo );
	//	options->set( STR_OPEN_RECENT, QVariant(recentPaths.toList()).toJsonValue() );
	//
	//	auto pVTFImportWindow = CVTFCreationDialog::Standalone( this );
	//
	//	pVTFImportWindow->exec();
	//
	//	if ( pVTFImportWindow->IsCancelled() )
	//		return;
	//
	//	//	QStringList VTFPaths;
	//	std::map<QString, VTFFolder> folders;
	//	VTFFolder *current;
	//	QStringList list;
	//	list << supportedWildcardImageList
	//		 << ".animation.txt";
	//
	//	QDirIterator it( importFrom, list, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories );
	//	while ( it.hasNext() )
	//	{
	//		QString path = it.next();
	//
	//		QStringList temp = path.split( importFrom );
	//		temp.pop_front();
	//		QStringList temp2 = temp.join( "" ).split( "/" );
	//		temp2.pop_front();
	//		temp2.pop_back();
	//		QString joined = temp2.join( "/" );
	//
	//		if ( !folders.contains( joined ) )
	//		{
	//			folders[joined] = {};
	//		}
	//
	//		current = &folders[joined];
	//
	//		if ( path.endsWith( ".animation.txt" ) )
	//		{
	//			current->isAnimation = true;
	//			continue;
	//		}
	//
	//		if ( importFrom != exportTo )
	//		{
	//			QString dirCreator = exportTo;
	//			for ( const auto &tPath : temp2 )
	//			{
	//				dirCreator += "/" + tPath;
	//				if ( !QDir().exists( dirCreator ) )
	//					QDir().mkdir( dirCreator );
	//			}
	//		}
	//
	//		current->paths.push_back( path );
	//	}
	//
	//	QProgressBar frogressBar( this );
	//	frogressBar.setMinimum( 0 );
	//	frogressBar.setTextVisible( true );
	//	frogressBar.setMinimumSize( 512, 64 );
	//	frogressBar.move( ( this->width() / 2 ) - 256, ( this->height() / 2 ) - 32 );
	//
	//	for ( auto &[first, second] : folders )
	//	{
	//		frogressBar.show();
	//		second.paths.sort();
	//		pVTFImportWindow->clearImageList();
	//		QString fullpath = exportTo;
	//		if ( !first.isEmpty() )
	//			fullpath.push_back( "/" + first + "/" );
	//
	//		if ( second.isAnimation )
	//		{
	//			frogressBar.setMaximum( 1 );
	//			QString vtfFileName = ( fullpath + "/" + QFileInfo( second.paths[0] ).baseName() + ".vtf" );
	//			frogressBar.setFormat( "Creating Animated VTF: " + vtfFileName );
	//			frogressBar.setValue( 0 );
	//			for ( const auto &file : second.paths )
	//			{
	//				pVTFImportWindow->AddImage( file );
	//			}
	//
	//			VTFErrorType err;
	//			auto vtf = pVTFImportWindow->GenerateVTF( err );
	//			if ( err != SUCCESS )
	//			{
	//				QMessageBox::warning( this, "VTF Failed to generate.", QString( "The VTF failed to generate, reason: " ) + ( ( err == INVALID_IMAGE ) ? "Invalid Image" : "No Image Data" ) );
	//				frogressBar.setValue( 1 );
	//				frogressBar.close();
	//				continue;
	//			}
	//			bool saved = vtf->Save( vtfFileName.toStdString().c_str() );
	//			if ( !saved )
	//				QMessageBox::warning( this, "VTF Failed to save.", QString( "The VTF failed to save, file: " ) + vtfFileName );
	//
	//			frogressBar.setValue( 1 );
	//			frogressBar.close();
	//			continue;
	//		}
	//
	//		frogressBar.setMaximum( second.paths.size() );
	//		frogressBar.setValue( 0 );
	//		for ( const auto &file : second.paths )
	//		{
	//			QString vtfFileName = ( fullpath + "/" + QFileInfo( file ).baseName() + ".vtf" );
	//			frogressBar.setFormat( "Creating VTF: " + vtfFileName );
	//			frogressBar.setValue( frogressBar.value() + 1 );
	//			pVTFImportWindow->AddImage( file );
	//			VTFErrorType err;
	//			auto vtf = pVTFImportWindow->GenerateVTF( err );
	//			if ( err != SUCCESS )
	//			{
	//				QMessageBox::warning( this, "VTF Failed to generate.", QString( "The VTF failed to generate, reason: " ) + ( ( err == INVALID_IMAGE ) ? "Invalid Image" : "No Image Data" ) );
	//				continue;
	//			}
	//			bool saved = vtf->Save( vtfFileName.toStdString().c_str() );
	//			if ( !saved )
	//				QMessageBox::warning( this, "VTF Failed to save.", QString( "The VTF failed to save, file: " ) + vtfFileName.toStdString().c_str() );
	//			pVTFImportWindow->clearImageList();
	//		}
	//		frogressBar.close();
	//	}
}

void CMainWindow::importFromFile()
{
	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open", recentPaths.last(), supportedWildcardImageList.join( " " ) + " *.vtf", nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePaths.isEmpty() )
		return;

	if ( recentPaths.contains( filePaths[0] ) )
		recentPaths.removeAt( recentPaths.indexOf( filePaths[0] ) );
	recentPaths.push_back( filePaths[0] );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	foreach( auto str, filePaths )
		if ( str.endsWith( ".vtf" ) )
		{
			NewVTFFromVTF( str );
			filePaths.removeAll( str );
		}

	if ( filePaths.count() < 1 )
		return;

	if ( filePaths.count() == 1 )
		generateVTFFromImage( filePaths[0] );
	else
		generateVTFFromImages( filePaths );
}

void CMainWindow::NewVTFFromVTF( const QString &filePath )
{
	auto pVTF = new vtfpp::VTF( filePath.toStdString() ); // getVTFFromVTFFile( filePath.toUtf8().constData() );

	if ( !pVTF )
	{
		QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
		return;
	}

	auto processVTF = std::make_unique<CVTFCreationDialog>( this, pVTF );

	if ( processVTF->exec() == QDialog::Rejected )
	{
		delete pVTF;
		return;
	}
	//	auto pVTFImportWindow = CVTFCreationDialog::FromVTF( this, pVTF );
	//
	//	pVTFImportWindow->exec();

	//	if ( pVTFImportWindow->IsCancelled() )
	//		return;
	//
	//	VTFErrorType err;
	//	pVTF = pVTFImportWindow->GenerateVTF( err ).release();
	//
	//	if ( err != SUCCESS )
	//	{
	//		QMessageBox::warning( this, "INVALID VTF", "Unable to process VTF.", QMessageBox::Ok );
	//		return;
	//	}

	addVTFToTab( pVTF, QFileInfo( filePath ).fileName(), filePath );
}

void CMainWindow::openVTF()
{
	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	QString filePath = QFileDialog::getOpenFileName(
		this, "Open VTF", recentPaths.last(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	if ( !QFileInfo( filePath ).isReadable() )
		return;

	addVTFFromPathToTab( filePath );
}

void CMainWindow::generateVTFFromImage( const QString &filePath )
{
	if ( filePath.isEmpty() )
		return;

	auto pVTF = new vtfpp::VTF();
	auto processVTF = std::make_unique<CVTFCreationDialog>( this, pVTF );

	processVTF->addImage( filePath );

	if ( processVTF->exec() == QDialog::Rejected )
	{
		delete pVTF;
		return;
	}
	//	bool canRun;
	//	auto newWindow = new CVTFCreationDialog( this, filePath, canRun );
	//
	//	if ( !canRun )
	//		return;
	//
	//	newWindow->exec();
	//	if ( newWindow->IsCancelled() )
	//		return;
	//
	//	VTFErrorType err;
	//	auto pVTF = newWindow->GenerateVTF( err );
	//	if ( err != SUCCESS )
	//	{
	//		QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
	//		return;
	//	}

	addVTFToTab( pVTF, QFileInfo( filePath ).fileName() );
}

void CMainWindow::generateVTFFromImages( QStringList filePaths )
{
	if ( filePaths.isEmpty() )
		return;

	auto pVTF = new vtfpp::VTF();
	auto processVTF = std::make_unique<CVTFCreationDialog>( this, pVTF );

	processVTF->addImage( filePaths );

	if ( processVTF->exec() == QDialog::Rejected )
	{
		delete pVTF;
		return;
	}

	//	bool canRun;
	//	auto newWindow = new CVTFCreationDialog( this, filePaths, canRun );
	//
	//	if ( !canRun )
	//		return;
	//
	//	newWindow->exec();
	//
	//	if ( newWindow->IsCancelled() )
	//		return;
	//
	//	VTFErrorType err;
	//	auto pVTF = newWindow->GenerateVTF( err );
	//	if ( err != SUCCESS )
	//	{
	//		QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
	//		return;
	//	}

	QFileInfo fl( filePaths[0] );
	addVTFToTab( pVTF, fl.fileName() );
}

void CMainWindow::fontToVTF()
{
	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	QString filePath = QFileDialog::getOpenFileName(
		this, "Open TTF/OTF", recentPaths.last(), "*.ttf *.otf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	generateVTFFromFont( filePath );
}

void CMainWindow::generateVTFFromFont( const QString &filepath )
{
	int id = -1; // QFontDatabase::addApplicationFont( purepath );
	if ( filepath.endsWith( "vfont" ) )
	{
		auto vContents = vcryptpp::VFONT::decrypt( sourcepp::fs::readFileBuffer( filepath.toStdString() ) );
		QByteArray barray = QByteArray { reinterpret_cast<const char *>( vContents.data() ), static_cast<qsizetype>( vContents.size() ) };
		id = QFontDatabase::addApplicationFontFromData( barray );
	}
	else
	{
		id = QFontDatabase::addApplicationFont( filepath );
	}

	if ( id == -1 )
		return;

	QString family = QFontDatabase::applicationFontFamilies( id ).at( 0 );
	QString charList = R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|})";

	QTextOption opts;

	QBuffer buff;
	QImage image( QSize( 1024, 1024 ), QImage::Format_RGBA8888 );

	QPainter painter;
	painter.begin( &image );
	// This fixes a weird issue where background will become garbage data
	// if not cleared first.
	painter.setCompositionMode( QPainter::CompositionMode_Source );
	painter.fillRect( QRect( 0, 0, 1024, 1024 ), QColor( 0, 0, 0, 0 ) );
	painter.setCompositionMode( QPainter::CompositionMode_SourceOver );

	painter.setBrush( Qt::transparent );
	painter.setPen( QPen( Qt::white ) );
	auto font = QFont( family );
	auto offset = 32;
	font.setPointSize( offset );
	painter.setFont( font );

	//	painter.drawRect( 0, 0, 1024, 1024 );
	for ( int i = 0, j = 0, k = 0; k < charList.length(); i++, k++ )
	{
		if ( i > 15 )
		{
			j++;
			i = 0;
		}
		painter.drawText( ( i * 64 ) + offset / 2, ( j * 64 ) + offset * 1.5, charList[k] );
	}
	painter.end();
	QFontDatabase::removeApplicationFont( id );

	auto pVTF = new vtfpp::VTF();
	auto processVTF = std::make_unique<CVTFCreationDialog>( this, pVTF );
	processVTF->addImage( image );

	if ( processVTF->exec() == QDialog::Rejected )
	{
		delete pVTF;
		return;
	}

	QFileInfo fl( filepath );
	addVTFToTab( pVTF, fl.fileName() );
}

void CMainWindow::exportVTFToFile()
{
	const auto key = pImageTabWidget->tabData( pImageTabWidget->currentIndex() ).value<intptr_t>();

	auto pVTF = this->vtfWidgetList.value( key );

	if ( !pVTF )
		return;

	int type = 0;
	int fImageAmount = pVTF->getFrameCount();
	if ( pVTF->getFaceCount() > fImageAmount )
	{
		fImageAmount = pVTF->getFaceCount();
		type = 1;
	}
	if ( pVTF->getDepth() > fImageAmount )
	{
		fImageAmount = pVTF->getDepth();
		type = 2;
	}

	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	QString filePath = QFileDialog::getSaveFileName(
		this, fImageAmount > 1 ? "Export to *" : "Export to _x*",
		recentPaths.last(), supportedWildcardImageList.join( " " ), nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	for ( int i = 0; i < fImageAmount; i++ )
	{
		uint32_t frames = type == 0 ? i : 0;
		uint32_t faces = type == 1 ? i : 0;
		uint32_t slices = type == 2 ? i : 0;

		//		auto size =
		//			vtfpp::ImageFormatDetails::getDataLength( vtfpp::ImageFormat::RGBA8888, pVTF->getWidth(), pVTF->getHeight(), 1 );
		//		auto pDest = static_cast<vlByte *>( malloc( size ) );
		//
		//		VTFLib::CVTFFile::ConvertToRGBA8888( pVTF->GetData( frames, faces, slices, 0 ), pDest, pVTF->GetWidth(), pVTF->GetHeight(), pVTF->GetFormat() );
		auto data = pVTF->getImageDataAsRGBA8888( 0, faces, frames, slices );
		auto img = QImage( reinterpret_cast<const uchar *>( data.data() ), pVTF->getWidth(), pVTF->getHeight(), QImage::Format_RGBA8888 );
		if ( fImageAmount > 1 )
		{
			QString nummedPath =
				filePath.mid( 0, filePath.length() - 4 ) + "_" + QString::number( i ) +
				filePath.mid( filePath.length() - 4, filePath.length() );

			if ( !img.save( nummedPath ) )
				QMessageBox::warning( this, "Failed to save image", "Failed to save: " + nummedPath, QMessageBox::Ok );
			;
		}
		else if ( !img.save( filePath ) )
			QMessageBox::warning( this, "Failed to save image", "Failed to save: " + filePath, QMessageBox::Ok );
	}
}

void CMainWindow::saveCurrentVTFToFile()
{
	if ( pImageTabWidget->count() < 1 )
		return;

	const auto key = pImageTabWidget->tabData( pImageTabWidget->currentIndex() ).value<intptr_t>();
	CMainWindow::saveVTFToFile( key );
}

void CMainWindow::saveAllVTFsToFiles()
{
	for ( int i = 0; i < this->pImageTabWidget->count(); i++ )
		CMainWindow::saveVTFToFile( this->pImageTabWidget->tabData( i ).value<intptr_t>() );
}

void CMainWindow::saveVTFToFile( intptr_t key, bool saveAs )
{
	if ( !this->vtfWidgetList.contains( key ) )
		return;

	auto pVTF = &this->vtfWidgetList[key];

	QString filePath {};

	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();
	if ( pVTF->path.isEmpty() || saveAs )
	{
		filePath = QFileDialog::getSaveFileName(
			this, "Save VTF",
			QFileInfo( recentPaths.last() ).completeBaseName(), "*.vtf", nullptr );
	}
	else
	{
		filePath = pVTF->path;
	}

	if ( filePath.isEmpty() )
		return;

	pVTF->path = filePath;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );

	recentPaths.push_back( filePath );

	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	if ( !filePath.endsWith( ".vtf" ) )
		filePath.append( ".vtf" );

	pVTF->vtf->bake( filePath.toUtf8().constData() );

	pVTF->hasSaved = true;

	pImageTabWidget->hasFilesChanged();
}

void CMainWindow::resizeEvent( QResizeEvent *r )
{
	//	auto pos = scrollWidget->pos();
	//	pImageViewWidget->move( -r->size().width() - pos.x(), -r->size().height() - pos.y() );
	QMainWindow::resizeEvent( r );
}
void CMainWindow::dragEnterEvent( QDragEnterEvent *event )
{
	QStringList extendedSupportedImageList = supportedImageList;
	extendedSupportedImageList << "ttf"
							   << "otf"
							   << "vfont"
							   << "vtf";
	if ( event->mimeData()->hasUrls() )
	{
		for ( const auto &url : event->mimeData()->urls() )
		{
			qInfo() << QFileInfo( url.toLocalFile() ).suffix();
			if ( !extendedSupportedImageList.contains( QFileInfo( url.toLocalFile() ).suffix() ) )
				return;
		}
		event->acceptProposedAction();
	}
}
#include <QJsonDocument>
#include <QListView>
#include <QStandardItemModel>
#include <QThread>
void CMainWindow::dropEvent( QDropEvent *event )
{
	event->acceptProposedAction();
	QStringList dest;
	foreach( const QUrl &url, event->mimeData()->urls() )
	{
		dest.push_back( url.toLocalFile() );
	}

	QThread::create( [this, dest]
					 {
						 emit onDropped( dest );
					 } )
		->start();
}

void CMainWindow::consoleParameters( int argc, char **argv )
{
	QStringList extendedSupportedImageList = supportedImageList;
	extendedSupportedImageList << "ttf"
							   << "otf"
							   << "vfont"
							   << "vtf";

	for ( int i = 1; i < argc; i++ )
	{
		if ( !extendedSupportedImageList.contains( QFileInfo( argv[i] ).suffix() ) )
			continue;

		this->addFile( argv[i] );
	}
}

void CMainWindow::consoleParameters( const QStringList &params )
{
	QStringList extendedSupportedImageList = supportedImageList;
	extendedSupportedImageList << "ttf"
							   << "otf"
							   << "vfont"
							   << "vtf";

	for ( const auto &str : params )
	{
		if ( !extendedSupportedImageList.contains( QFileInfo( str ).suffix() ) )
			continue;

		this->addFile( str );
	}
}

void CMainWindow::addFile( const QString &filePath )
{
	QString suffix = QFileInfo( filePath ).suffix();

	auto recentPaths = options->get( STR_OPEN_RECENT, QJsonArray { QDir::currentPath() } ).toVariant().toStringList();

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	options->set( STR_OPEN_RECENT, QVariant( recentPaths.toList() ).toJsonValue() );

	if ( suffix == "vtf" )
	{
		addVTFFromPathToTab( filePath );
		return;
	}

	if ( supportedImageList.contains( suffix ) )
	{
		generateVTFFromImage( filePath );
		return;
	}

	if ( suffix == "ttf" || suffix == "otf" || "vfont" )
	{
		generateVTFFromFont( filePath );
		return;
	}
}
void CMainWindow::openTabContextMenu( int tab )
{
	//	auto ind = pImageTabWidget->tabData( tab ).value<intptr_t>();
	//	auto vtf = &vtfWidgetList.value( ind );
	QMenu *menu = new QMenu( this );
	auto save = menu->addAction( "Save" );
	auto saveAs = menu->addAction( "SaveAs" );
	auto edit = menu->addAction( "Edit" );
	auto remove = menu->addAction( "Remove" );
	connect( save, &QAction::triggered, this, &CMainWindow::saveCurrentVTFToFile );
	connect( saveAs, &QAction::triggered, this, &CMainWindow::saveCurrentVTFToFileAs );
	connect( edit, &QAction::triggered, this, [&, tab]
			 {
				 if ( pImageTabWidget->count() < 1 )
					 return;

				 const auto key = pImageTabWidget->tabData( tab ).value<intptr_t>();
				 if ( !this->vtfWidgetList.contains( key ) )
					 return;
				 //
				 auto pVTF = &this->vtfWidgetList[key];
				 auto editVTFDialog = std::make_unique<CVTFCreationDialog>( this, pVTF->vtf );

				 if ( editVTFDialog->exec() == QDialog::Rejected )
					 return;
				 pVTF->hasSaved = false;
				 pImageTabWidget->hasFilesChanged();
				 emit pImageTabWidget->currentChanged( tab );
			 } );
	connect( remove, &QAction::triggered, this, [&, tab]
			 {
				 this->removeVTFTab( tab );
			 } );
	menu->popup( QCursor::pos() );
}
void CMainWindow::processDroppedItems( const QStringList &paths )
{
	if ( paths.isEmpty() )
		return;

	bool sameType = true;
	QString firstPrefix = paths[0].split( "." ).last();
	for ( auto &item : paths )
	{
		if ( item.split( "." ).last() != firstPrefix )
			sameType = false;
	}

	if ( !sameType || paths.length() == 1 )
	{
		for ( auto &item : paths )
			addFile( item );
		return;
	}

	auto pVTF = new vtfpp::VTF();
	auto processVTF = std::make_unique<CVTFCreationDialog>( this, pVTF );

	processVTF->addImage( paths );

	if ( processVTF->exec() == QDialog::Rejected )
	{
		delete pVTF;
		return;
	}
	//	bool canRun;
	//	auto newWindow = new CVTFCreationDialog( this, filePath, canRun );
	//
	//	if ( !canRun )
	//		return;
	//
	//	newWindow->exec();
	//	if ( newWindow->IsCancelled() )
	//		return;
	//
	//	VTFErrorType err;
	//	auto pVTF = newWindow->GenerateVTF( err );
	//	if ( err != SUCCESS )
	//	{
	//		QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
	//		return;
	//	}

	addVTFToTab( pVTF, processVTF->getFileName() );
}

class donationList : public QListView
{
	using QListView::QListView;

	// We need to toss all of these events in the trash because it will create a phantom selection that cannot get rid of any other way.
	QModelIndex moveCursor( QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers ) override { return {}; };
	void mousePressEvent( QMouseEvent *event ) override { event->ignore(); };
	void mouseDoubleClickEvent( QMouseEvent *event ) override { event->ignore(); };
	void mouseMoveEvent( QMouseEvent *e ) override { e->ignore(); };
	void mouseReleaseEvent( QMouseEvent *e ) override { e->ignore(); };
};

void CMainWindow::About()
{
	auto aboutDialog = new QDialog( this );
	aboutDialog->setMinimumSize( 380, 550 );
	auto aboutLayout = new QGridLayout( aboutDialog );

	auto aboutText = QFile( ":/about.md" );
	aboutText.open( QFile::ReadOnly );
	//"VTF Forge V0.86\nDeveloped by Trico Everfire\nAdditional credits to Strata Source."
	auto infoLabel = new QLabel( QString( aboutText.readAll() ).arg( QSysInfo::kernelType(), QSysInfo::currentCpuArchitecture() ), aboutDialog );
	infoLabel->setTextFormat( Qt::MarkdownText );
	aboutLayout->addWidget( infoLabel, 0, 0, Qt::AlignTop );
	auto donationLabel = new QLabel( "### Support Me!:\n"
									 "<a href=\"https://ko-fi.com/trico_everfire\" target=\"_blank\" rel=\"noopener noreferrer\"><img src=\":/ko-fi-donation.svg\" alt=\"Ko-Fi\" /></a>",
									 aboutDialog );
	donationLabel->setTextFormat( Qt::MarkdownText );
	donationLabel->setOpenExternalLinks( true );
	aboutLayout->addWidget( donationLabel, 1, 0, Qt::AlignBottom );

	auto supportLayout = new QVBoxLayout();
	auto supportLabel = new QLabel( "Ko-fi supporters:", aboutDialog );
	supportLayout->addWidget( supportLabel );
	auto supportView = new donationList( aboutDialog );
	auto supporters = QFile( ":/supporters.json" );
	supporters.open( QFile::ReadOnly );
	QJsonDocument doc = QJsonDocument::fromJson( supporters.readAll() );
	auto model = new QStandardItemModel( 0, 0 );
	supportView->setModel( model );
	for ( auto supporter : doc.array() )
	{
		auto item = new QStandardItem( supporter.toString() );
		item->setSelectable( false );
		item->setEditable( false );
		item->setCheckable( false );
		item->setDragEnabled( false );
		item->setUserTristate( false );
		model->appendRow( item );
	}
	supportView->clearSelection();
	supportView->setSelectionMode( QAbstractItemView::NoSelection );
	supportView->setSelectionRectVisible( false );
	supportLayout->addWidget( supportView );

	aboutLayout->addItem( new QSpacerItem( 20, 1 ), 0, 1 );

	aboutLayout->addLayout( supportLayout, 0, 2, 2, 1, Qt::AlignLeft );
	aboutDialog->exec();
}

void CMainWindow::importMRAO()
{
	//	return;
	auto mraoDialog = new QDialog( this );
	auto mraoLayout = new QGridLayout( mraoDialog );

	auto selectMRAORGBA = new QComboBox( mraoDialog );
	selectMRAORGBA->addItem( "MRAO", true );
	selectMRAORGBA->addItem( "RGBA", false );
	mraoLayout->addWidget( selectMRAORGBA, 0, 0, 1, 2 );

	auto metalLabel = new QLabel( "Metal", mraoDialog );
	mraoLayout->addWidget( metalLabel, 1, 0 );

	auto metalImageButton = new QPushButton( mraoDialog );
	metalImageButton->setMinimumSize( 128, 128 );
	mraoLayout->addWidget( metalImageButton, 1, 1 );

	auto roughnessLabel = new QLabel( "Roughness", mraoDialog );
	mraoLayout->addWidget( roughnessLabel, 2, 0 );

	auto roughnessImageButton = new QPushButton( mraoDialog );
	roughnessImageButton->setMinimumSize( 128, 128 );
	mraoLayout->addWidget( roughnessImageButton, 2, 1 );

	auto aoLabel = new QLabel( "Ambient Occlusion", mraoDialog );
	mraoLayout->addWidget( aoLabel, 3, 0 );
	auto aoImageButton = new QPushButton( mraoDialog );
	aoImageButton->setMinimumSize( 128, 128 );
	mraoLayout->addWidget( aoImageButton, 3, 1 );

	auto alphaLabel = new QLabel( "Alpha", mraoDialog );
	mraoLayout->addWidget( alphaLabel, 4, 0 );
	alphaLabel->setHidden( true );
	auto alphaImageButton = new QPushButton();
	alphaImageButton->setMinimumSize( 128, 128 );
	mraoLayout->addWidget( alphaImageButton, 4, 1 );
	alphaImageButton->setHidden( true );

	auto display = new QLabel( mraoDialog );
	display->setMinimumSize( 512, 512 );

	auto background = QImage( ":/VTF_Forge_small_grayscale.png" );

	QPixmap transparent( background.size() );
	transparent.fill( Qt::transparent );
	QPainter p;
	p.begin( &transparent );
	p.setCompositionMode( QPainter::CompositionMode_Source );
	p.drawPixmap( 0, 0, QPixmap::fromImage( background ) );
	p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
	p.fillRect( transparent.rect(), QColor( 0, 0, 0, 40 ) );
	p.end();
	display->setPixmap( transparent.scaled( 512, 512 ) );
	mraoLayout->addWidget( display, 0, 2, 4, 1 );

	connect( selectMRAORGBA, &QComboBox::currentIndexChanged, mraoDialog, [mraoDialog, selectMRAORGBA, metalLabel, roughnessLabel, aoLabel, mraoLayout, alphaLabel, alphaImageButton, display]
			 {
				 auto data = selectMRAORGBA->currentData().toBool();
				 if ( data )
				 {
					 metalLabel->setText( "Metal" );
					 roughnessLabel->setText( "Roughness" );
					 aoLabel->setText( "Ambient Occlusion" );
					 mraoLayout->addWidget( display, 0, 2, 4, 1 );
					 alphaLabel->setHidden( true );
					 alphaImageButton->setHidden( true );
					 mraoDialog->resize( mraoDialog->minimumSize().width(), mraoDialog->minimumSize().height() );
					 return;
				 }

				 metalLabel->setText( "Red" );
				 roughnessLabel->setText( "Green" );
				 aoLabel->setText( "Blue" );

				 alphaLabel->setHidden( false );
				 alphaImageButton->setHidden( false );
				 mraoLayout->addWidget( display, 0, 2, 5, 1 );
				 mraoDialog->resize( mraoDialog->minimumSize().width(), mraoDialog->minimumSize().height() );
			 } );

	mraoDialog->exec();
}
void CMainWindow::closeEvent( QCloseEvent *event )
{
	for ( int i = 0; i < pImageTabWidget->count(); i++ )
	{
		auto key = pImageTabWidget->tabData( i ).value<intptr_t>();
		if ( !this->vtfWidgetList.contains( key ) )
			continue;

		auto pVTF = &this->vtfWidgetList[key];
		if ( pVTF->hasSaved )
			continue;

		auto dialogResult = QMessageBox::question( this, "Save VTF?", "Unsaved changes. Do you want to save the VTF?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
		if ( dialogResult == QMessageBox::Yes )
			this->saveVTFToFile( key );
		if ( dialogResult == QMessageBox::Cancel )
			return event->ignore();
	}
	event->accept();
}
void CMainWindow::saveCurrentVTFToFileAs()
{
	if ( pImageTabWidget->count() < 1 )
		return;

	const auto key = pImageTabWidget->tabData( pImageTabWidget->currentIndex() ).value<intptr_t>();
	CMainWindow::saveVTFToFile( key, true );
}

ZoomScrollArea::ZoomScrollArea( QWidget *pParent ) :
	QAbstractScrollArea( pParent )
{
	viewport()->setBackgroundRole( QPalette::NoRole );
}

void ZoomScrollArea::wheelEvent( QWheelEvent *event )
{
	if ( event->angleDelta().y() > 0 ) // up Wheel
	{
		if ( m_isCTRLHeld )
		{
			emit onScrollUp();
			event->ignore();
			return;
		}
	}
	else if ( event->angleDelta().y() < 0 ) // down Wheel
	{
		if ( m_isCTRLHeld )
		{
			emit onScrollDown();
			event->ignore();
			return;
		}
	}
	QAbstractScrollArea::wheelEvent( event );
}

bool ZoomScrollArea::event( QEvent *event )
{
	if ( event->type() == QEvent::KeyPress )
	{
		auto ke = static_cast<QKeyEvent *>( event );
		if ( ( ke->key() == Qt::Key_Control ) )
			m_isCTRLHeld = true;
	}

	if ( event->type() == QEvent::KeyRelease )
	{
		auto ke = static_cast<QKeyEvent *>( event );
		if ( ( ke->key() == Qt::Key_Control ) )
			m_isCTRLHeld = false;
	}

	// When we lose focus, we can no longer check for key events, so to prevent
	// weird behaviour upon defocus, we set m_isCTRLHeld to false.
	if ( event->type() == QEvent::FocusOut )
	{
		auto fe = static_cast<QFocusEvent *>( event );
		if ( fe->lostFocus() )
			m_isCTRLHeld = false;
	}

	return QAbstractScrollArea::event( event );
}

QWidget *ZoomScrollArea::takeWidget()
{
	QWidget *w = this->pWidget;
	pWidget = nullptr;
	if ( w )
		w->setParent( nullptr );
	return w;
}
QWidget *ZoomScrollArea::widget() const
{
	return pWidget;
}
void ZoomScrollArea::setWidget( QWidget *widget )
{
	if ( widget->parentWidget() != viewport() )
		widget->setParent( viewport() );
	pWidget = widget;
}
void VTFTabBar::mousePressEvent( QMouseEvent *event )
{
	if ( event->button() == Qt::RightButton )
	{
		emit onRightMouseClicked( this->tabAt( event->pos() ) );
	}

	QTabBar::mousePressEvent( event );
}
void VTFTabBar::hasFilesChanged()
{
	for ( int i = 0; i < count(); i++ )
	{
		auto tabContents = mainWindow->vtfWidgetList.value( tabData( i ).value<intptr_t>() );
		if ( !tabContents.hasSaved )
			setTabText( i, "*" + tabContents.name );
		else
			setTabText( i, tabContents.name );
	}
}
VTFTabBar::VTFTabBar( CMainWindow *parent ) :
	QTabBar( parent )
{
	this->mainWindow = parent;
}
