#include "MainWindow.h"

#include "../libs/stb/stb_image.h"
#include "EntryTree.h"
#include "Options.h"
#include "VTFEImport.h"

#include <QApplication>
#include <QBuffer>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QStyle>

using namespace ui;

#define remap( value, low1, high1, low2, high2 ) ( low2 + ( value - low1 ) * ( high2 - low2 ) / ( high1 - low1 ) )

CMainWindow::CMainWindow() :
	QMainWindow()
{
	this->setWindowTitle( "VTF Edit Revitalized" );

	setAcceptDrops( true );

	QWidget *centralWidget = new QWidget( this );

	auto pMainLayout = new QGridLayout( centralWidget );

	this->setCentralWidget( centralWidget );

	pImageTabWidget = new QTabBar( this );

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

	scrollLayout->addWidget( m_pVerticalScrollBar, 0, 1, Qt::AlignRight );

	pMainLayout->addWidget( m_pScrollWidget, 1, 1 );
	//	scrollWidget->setWidget( pImageViewWidget );
	//	scrollWidget->setMinimumSize( 512, 512 );
	//
	//	scrollWidget->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	//	scrollWidget->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	//
	//	scrollWidget->horizontalScrollBar()->setRange( 0, 4096 );
	//	scrollWidget->verticalScrollBar()->setRange( 0, 4096 );
	//	QPoint topLeft = scrollWidget->viewport()->rect().topLeft();
	//	scrollWidget->viewport()->resize( 4096, 4096 );

	//	QSize areaSize = scrollWidget->viewport()->size();
	//	qInfo() << this->size();

	//	scrollWidget->viewport()->move( 255, 255 );
	//	qInfo() << scrollWidget->rect().center();
	//	scrollWidget->verticalScrollBar()->setPageStep( areaSize.height() / 0.01 );
	//	scrollWidget->horizontalScrollBar()->setPageStep( areaSize.width() / 0.01 );
	//	scrollWidget->horizontalScrollBar()->setValue( 4096 / 2 );
	//	scrollWidget->verticalScrollBar()->setValue( 4096 / 2 );
	//	scrollWidget->horizontalScrollBar()->setMinimum( 0 );
	//	scrollWidget->horizontalScrollBar()->setMaximum( 512 );
	//	scrollWidget->verticalScrollBar()->setMinimum( 0 );
	//	scrollWidget->verticalScrollBar()->setMaximum( 512 );
	//	scrollWidget->ensureVisible( 0, 0, 0, 0 );

	//	pMainLayout->addWidget( scrollWidget, 1, 1 );

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

	pMainLayout->addWidget( pInfoResourceTabWidget, 0, 2, 2, 1, Qt::AlignRight );

	m_pMainMenuBar = this->menuBar(); // new QMenuBar( this );
	m_pMainMenuBar->setNativeMenuBar( false );
	//	pMainLayout->setMenuBar( m_pMainMenuBar );

	//	connect( this, & )
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
							 auto data = mainParent->pakFile()->readEntry( mainParent->pakFile()->findEntry( item->getEntry() ).value() );
							 VTFLib::CVTFFile *file = new VTFLib::CVTFFile {};
							 file->Load( data.value().data(), data.value().size(), false );
							 addVTFToTab( file, item->getEntry().data() );
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

	connect( pImageViewWidget, &ImageViewWidget::animated, pImageSettingsWidget, &ImageSettingsWidget::set_frame );

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
}

VTFLib::CVTFFile *CMainWindow::getVTFFromVTFFile( const char *path )
{
	auto vVTF = new VTFLib::CVTFFile();
	if ( !vVTF->Load( path, false ) )
		return nullptr;
	return vVTF;
}

void CMainWindow::addVTFFromPathToTab( const QString &path )
{
	QFileInfo fileInfo( path );

	auto pVTF = getVTFFromVTFFile( fileInfo.filePath().toUtf8().constData() );

	addVTFToTab( pVTF, fileInfo.fileName() );
}

void CMainWindow::addVTFToTab( VTFLib::CVTFFile *pVTF, const QString &name )
{
	if ( pVTF )
	{
		int index = pImageTabWidget->addTab( name );

		this->vtfWidgetList.insert( reinterpret_cast<intptr_t>( pVTF ), pVTF );
		pImageTabWidget->setTabData( index, QVariant::fromValue( reinterpret_cast<intptr_t>( pVTF ) ) );

		pImageTabWidget->setCurrentIndex( index );

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

	this->vtfWidgetList.remove( key );

	if ( pImageTabWidget->currentIndex() == index )
	{
		pImageViewWidget->set_vtf( nullptr );
		pResourceWidget->set_vtf( nullptr );
		pImageSettingsWidget->set_vtf( nullptr );
		pImageInfo->update_info( nullptr );
	}

	pImageTabWidget->removeTab( index );

	delete vtf;
}

void CMainWindow::tabChanged( int index )
{
	const auto key = pImageTabWidget->tabData( index ).value<intptr_t>();

	auto pVTF = this->vtfWidgetList.value( key );

	pImageViewWidget->set_vtf( pVTF );
	pImageViewWidget->set_rgba( redBox->isChecked(), greenBox->isChecked(), blueBox->isChecked(), alphaBox->isChecked() );
	pImageViewWidget->stopAnimating();
	pResourceWidget->set_vtf( pVTF );
	pImageSettingsWidget->set_vtf( pVTF );
	pImageSettingsWidget->set_vtf( pVTF );
	pImageInfo->update_info( pVTF );

	//		if ( pVTF )
	//		{
	//			scrollWidget->verticalScrollBar()->setSliderPosition( pVTF->GetHeight() / 2 );
	//			scrollWidget->horizontalScrollBar()->setSliderPosition( pVTF->GetWidth() / 2 );
	//		}

	m_pHorizontalScrollBar->setValue( 4096 / 2 );
	m_pVerticalScrollBar->setValue( 4096 / 2 );
}

void CMainWindow::setupMenuBar()
{
	auto pFileMenuTab = m_pMainMenuBar->addMenu( "File" );
	pFileMenuTab->addAction( "Open", this, &CMainWindow::openVTF );
	pFileMenuTab->addAction( "Save", this, &CMainWindow::saveVTFToFile );
	pFileMenuTab->addAction( "Export", this, &CMainWindow::exportVTFToFile );
	pFileMenuTab->addAction( "Import...", this, &CMainWindow::importFromFile );

	auto pToolMenuTab = m_pMainMenuBar->addMenu( "Tools" );
	pToolMenuTab->addAction( "VTF Version Editor (Individual)", this, &CMainWindow::compressVTFFile );
	pToolMenuTab->addAction( "VTF Version Editor (Batch)", this, &CMainWindow::compressVTFFolder );
	pToolMenuTab->addAction( "Folders to VTF", this, &CMainWindow::foldersToVTF );
	pToolMenuTab->addAction( "FontToVTF", this, &CMainWindow::fontToVTF );

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
#ifdef COMPRESSVTF
	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open VTF", recentPaths.last(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePaths.isEmpty() )
		return;

	if ( recentPaths.contains( filePaths[0] ) )
		recentPaths.removeAt( recentPaths.indexOf( filePaths[0] ) );
	recentPaths.push_back( filePaths[0] );
	Options::set( STR_OPEN_RECENT, recentPaths );

	auto pCompressionDialog = new QDialog( this );
	pCompressionDialog->setWindowTitle( "VTF Version Editor" );
	auto vBLayout = new QGridLayout( pCompressionDialog );

	auto label1 = new QLabel( "VTF Version:", this );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );

	auto pVtfVersionBox = new QComboBox( this );
	int setbackIndex = 1;
#ifdef CHAOS_INITIATIVE
	setbackIndex = 2;
	for ( int i = 0; i <= VTF_MINOR_VERSION; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( VTF_MAJOR_VERSION ) + "." + QString::number( i ), i );
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
	Options::set( STR_OPEN_RECENT, recentPaths );

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
		VTFLib::CVTFFile *pVTF = getVTFFromVTFFile( filePath.toUtf8().constData() );

		if ( !pVTF )
		{
			QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
			return;
		}

#ifdef CHAOS_INITIATIVE
		if ( pVTF->GetMinorVersion() == pVtfVersionBox->currentData().toInt() && pVTF->GetAuxCompressionLevel() == pAuxCompressionLevelBox->currentData().toInt() )
			continue;
#else
		if ( pVTF->GetMinorVersion() == pVtfVersionBox->currentData().toInt() )
			continue;
#endif

		pVTF->SetVersion( 7, pVtfVersionBox->currentData().toInt() );

#ifdef CHAOS_INITIATIVE
		if ( pAuxCompressionBox->isChecked() )
		{
			pVTF->SetAuxCompressionLevel( pAuxCompressionLevelBox->currentData().toInt() );
		}
#endif

		if ( pRecomputeReflectivity->isChecked() )
		{
			pVTF->ComputeReflectivity();
		}

		if ( pathDirectory.isEmpty() )
			pVTF->Save( filePath.toUtf8().constData() );
		else
			pVTF->Save( ( pathDirectory + "/" + QFileInfo( filePath ).fileName() ).toUtf8().constData() );

		delete pVTF;
	}
#endif
}

void CMainWindow::compressVTFFolder()
{
#ifdef COMPRESSVTF

	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );
	QString dirPath = QFileDialog::getExistingDirectory(
		this, "Open VTF", recentPaths.last(), QFileDialog::Option::DontUseNativeDialog );

	if ( dirPath.isEmpty() )
		return;

	if ( recentPaths.contains( dirPath ) )
		recentPaths.removeAt( recentPaths.indexOf( dirPath ) );
	recentPaths.push_back( dirPath );
	Options::set( STR_OPEN_RECENT, recentPaths );

	auto pCompressionDialog = new QDialog( this );
	pCompressionDialog->setWindowTitle( "VTF Version Editor" );
	auto vBLayout = new QGridLayout( pCompressionDialog );

	auto label1 = new QLabel( "VTF Version:", this );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );

	auto pVtfVersionBox = new QComboBox( this );
	int setbackIndex = 1;
#ifdef CHAOS_INITIATIVE
	setbackIndex = 2;
	for ( int i = 0; i <= VTF_MINOR_VERSION; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( VTF_MAJOR_VERSION ) + "." + QString::number( i ), i );
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
	Options::set( STR_OPEN_RECENT, recentPaths );

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

		std::unique_ptr<VTFLib::CVTFFile> pVTF( getVTFFromVTFFile( path.toStdString().c_str() ) );

		if ( !pVTF || !pVTF->IsLoaded() )
		{
			QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.\n" + dirPath, QMessageBox::Ok );
			continue;
		}
#ifdef CHAOS_INITIATIVE
		if ( pVTF->GetMinorVersion() == pVtfVersionBox->currentData().toInt() && pVTF->GetAuxCompressionLevel() == pAuxCompressionLevelBox->currentData().toInt() && pathDirectory.isEmpty() )
			continue;
#else
		if ( pVTF->GetMinorVersion() == pVtfVersionBox->currentData().toInt() && pathDirectory.isEmpty() )
			continue;
#endif
		pVTF->SetVersion( 7, pVtfVersionBox->currentData().toInt() );

#ifdef CHAOS_INITIATIVE
		if ( pAuxCompressionBox->isChecked() )
		{
			pVTF->SetAuxCompressionLevel( pAuxCompressionLevelBox->currentData().toInt() );
		}
#endif

		if ( pRecomputeReflectivity->isChecked() )
		{
			pVTF->ComputeReflectivity();
		}

		if ( pathDirectory.isEmpty() )
		{
			if ( !pVTF->Save( path.toUtf8().constData() ) )
			{
				QMessageBox::warning( this, "Unable to save VTF", "The VTF cannot be saved.\n" + dirPath, QMessageBox::Ok );
			}
		}
		else
		{
			if ( !pVTF->Save( ( pathDirectory + "/" + temp.join( "" ) ).toUtf8().constData() ) )
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

void CMainWindow::foldersToVTF()
{
	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QString importFrom = QFileDialog::getExistingDirectory(
		this, "Import From", recentPaths.last(),
		QFileDialog::Option::DontUseNativeDialog );

	QString exportTo = QFileDialog::getExistingDirectory(
		this, "Export To", recentPaths.last(),
		QFileDialog::Option::DontUseNativeDialog );

	if ( importFrom.isEmpty() )
		return;

	if ( exportTo.isEmpty() )
		return;

	if ( recentPaths.contains( importFrom ) )
		recentPaths.removeAt( recentPaths.indexOf( importFrom ) );
	recentPaths.push_back( importFrom );

	if ( recentPaths.contains( exportTo ) )
		recentPaths.removeAt( recentPaths.indexOf( exportTo ) );
	recentPaths.push_back( exportTo );
	Options::set( STR_OPEN_RECENT, recentPaths );

	auto pVTFImportWindow = VTFEImport::Standalone( this );

	pVTFImportWindow->exec();

	if ( pVTFImportWindow->IsCancelled() )
		return;

	//	QStringList VTFPaths;
	std::map<QString, VTFFolder> folders;
	VTFFolder *current;
	QStringList list;
	list << supportedWildcardImageList
		 << ".animation.txt";

	QDirIterator it( importFrom, list, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories );
	while ( it.hasNext() )
	{
		QString path = it.next();

		QStringList temp = path.split( importFrom );
		temp.pop_front();
		QStringList temp2 = temp.join( "" ).split( "/" );
		temp2.pop_front();
		temp2.pop_back();
		QString joined = temp2.join( "/" );

		if ( !folders.contains( joined ) )
		{
			folders[joined] = {};
		}

		current = &folders[joined];

		if ( path.endsWith( ".animation.txt" ) )
		{
			current->isAnimation = true;
			continue;
		}

		if ( importFrom != exportTo )
		{
			QString dirCreator = exportTo;
			for ( const auto &tPath : temp2 )
			{
				dirCreator += "/" + tPath;
				if ( !QDir().exists( dirCreator ) )
					QDir().mkdir( dirCreator );
			}
		}

		current->paths.push_back( path );
	}

	QProgressBar frogressBar( this );
	frogressBar.setMinimum( 0 );
	frogressBar.setTextVisible( true );
	frogressBar.setMinimumSize( 512, 64 );
	frogressBar.move( ( this->width() / 2 ) - 256, ( this->height() / 2 ) - 32 );

	for ( auto &[first, second] : folders )
	{
		frogressBar.show();
		second.paths.sort();
		pVTFImportWindow->clearImageList();
		QString fullpath = exportTo;
		if ( !first.isEmpty() )
			fullpath.push_back( "/" + first + "/" );

		if ( second.isAnimation )
		{
			frogressBar.setMaximum( 1 );
			QString vtfFileName = ( fullpath + "/" + QFileInfo( second.paths[0] ).baseName() + ".vtf" );
			frogressBar.setFormat( "Creating Animated VTF: " + vtfFileName );
			frogressBar.setValue( 0 );
			for ( const auto &file : second.paths )
			{
				pVTFImportWindow->AddImage( file );
			}

			VTFErrorType err;
			auto vtf = pVTFImportWindow->GenerateVTF( err );
			if ( err != SUCCESS )
			{
				QMessageBox::warning( this, "VTF Failed to generate.", QString( "The VTF failed to generate, reason: " ) + ( ( err == INVALID_IMAGE ) ? "Invalid Image" : "No Image Data" ) );
				frogressBar.setValue( 1 );
				frogressBar.close();
				continue;
			}
			bool saved = vtf->Save( vtfFileName.toStdString().c_str() );
			if ( !saved )
				QMessageBox::warning( this, "VTF Failed to save.", QString( "The VTF failed to save, file: " ) + vtfFileName );

			frogressBar.setValue( 1 );
			frogressBar.close();
			continue;
		}

		frogressBar.setMaximum( second.paths.size() );
		frogressBar.setValue( 0 );
		for ( const auto &file : second.paths )
		{
			QString vtfFileName = ( fullpath + "/" + QFileInfo( file ).baseName() + ".vtf" );
			frogressBar.setFormat( "Creating VTF: " + vtfFileName );
			frogressBar.setValue( frogressBar.value() + 1 );
			pVTFImportWindow->AddImage( file );
			VTFErrorType err;
			auto vtf = pVTFImportWindow->GenerateVTF( err );
			if ( err != SUCCESS )
			{
				QMessageBox::warning( this, "VTF Failed to generate.", QString( "The VTF failed to generate, reason: " ) + ( ( err == INVALID_IMAGE ) ? "Invalid Image" : "No Image Data" ) );
				continue;
			}
			bool saved = vtf->Save( vtfFileName.toStdString().c_str() );
			if ( !saved )
				QMessageBox::warning( this, "VTF Failed to save.", QString( "The VTF failed to save, file: " ) + vtfFileName.toStdString().c_str() );
			pVTFImportWindow->clearImageList();
		}
		frogressBar.close();
	}
}

void CMainWindow::importFromFile()
{
	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open", recentPaths.last(), supportedWildcardImageList.join( " " ) + " *.vtf", nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePaths.isEmpty() )
		return;

	if ( recentPaths.contains( filePaths[0] ) )
		recentPaths.removeAt( recentPaths.indexOf( filePaths[0] ) );
	recentPaths.push_back( filePaths[0] );
	Options::set( STR_OPEN_RECENT, recentPaths );

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
	auto pVTF = getVTFFromVTFFile( filePath.toUtf8().constData() );

	if ( !pVTF )
	{
		QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
		return;
	}

	auto pVTFImportWindow = VTFEImport::FromVTF( this, pVTF );

	delete pVTF;

	pVTFImportWindow->exec();

	if ( pVTFImportWindow->IsCancelled() )
		return;

	VTFErrorType err;
	pVTF = pVTFImportWindow->GenerateVTF( err );

	if ( err != SUCCESS )
	{
		QMessageBox::warning( this, "INVALID VTF", "Unable to process VTF.", QMessageBox::Ok );
		return;
	}

	addVTFToTab( pVTF, QFileInfo( filePath ).fileName() );
}

void CMainWindow::openVTF()
{
	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QString filePath = QFileDialog::getOpenFileName(
		this, "Open VTF", recentPaths.last(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	Options::set( STR_OPEN_RECENT, recentPaths );

	if ( !QFileInfo( filePath ).isReadable() )
		return;

	addVTFFromPathToTab( filePath );
}

void CMainWindow::generateVTFFromImage( const QString &filePath )
{
	if ( filePath.isEmpty() )
		return;

	bool canRun;
	auto newWindow = new VTFEImport( this, filePath, canRun );

	if ( !canRun )
		return;

	newWindow->exec();
	if ( newWindow->IsCancelled() )
		return;

	VTFErrorType err;
	auto pVTF = newWindow->GenerateVTF( err );
	if ( err != SUCCESS )
	{
		QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
		return;
	}

	addVTFToTab( pVTF, QFileInfo( filePath ).fileName() );
}

void CMainWindow::generateVTFFromImages( QStringList filePaths )
{
	if ( filePaths.isEmpty() )
		return;
	bool canRun;
	auto newWindow = new VTFEImport( this, filePaths, canRun );

	if ( !canRun )
		return;

	newWindow->exec();

	if ( newWindow->IsCancelled() )
		return;

	VTFErrorType err;
	auto pVTF = newWindow->GenerateVTF( err );
	if ( err != SUCCESS )
	{
		QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
		return;
	}

	QFileInfo fl( filePaths[0] );
	addVTFToTab( pVTF, fl.fileName() );
}

void CMainWindow::fontToVTF()
{
	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QString filePath = QFileDialog::getOpenFileName(
		this, "Open TTF/OTF", recentPaths.last(), "*.ttf *.otf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	Options::set( STR_OPEN_RECENT, recentPaths );

	generateVTFFromFont( filePath );
}

void CMainWindow::generateVTFFromFont( const QString &filepath )
{
	int id = QFontDatabase::addApplicationFont( filepath );
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

	QByteArray im;
	QBuffer bufferrgb( &im );
	bufferrgb.open( QIODevice::WriteOnly );

	image.save( &bufferrgb, "PNG" );

	int x, y, n;

	stbi_uc *data = stbi_load_from_memory( reinterpret_cast<const stbi_uc *>( bufferrgb.data().constData() ), bufferrgb.size(), &x, &y, &n, 4 );
	QFontDatabase::removeApplicationFont( id );

	auto newWindow = VTFEImport::FromFont( this, data, 1024, 1024 );

	newWindow->exec();

	if ( newWindow->IsCancelled() )
		return;

	VTFErrorType err;
	auto pVTF = newWindow->GenerateVTF( err );
	if ( err != SUCCESS )
	{
		QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
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
	int fImageAmount = pVTF->GetFrameCount();
	if ( pVTF->GetFaceCount() > fImageAmount )
	{
		fImageAmount = pVTF->GetFaceCount();
		type = 1;
	}
	if ( pVTF->GetDepth() > fImageAmount )
	{
		fImageAmount = pVTF->GetDepth();
		type = 2;
	}

	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QString filePath = QFileDialog::getSaveFileName(
		this, fImageAmount > 1 ? "Export to *" : "Export to _x*",
		recentPaths.last(), supportedWildcardImageList.join( " " ), nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	Options::set( STR_OPEN_RECENT, recentPaths );

	for ( int i = 0; i < fImageAmount; i++ )
	{
		vlUInt frames = type == 0 ? i : 0;
		vlUInt faces = type == 1 ? i : 0;
		vlUInt slices = type == 2 ? i : 0;

		auto size =
			VTFLib::CVTFFile::ComputeImageSize( pVTF->GetWidth(), pVTF->GetHeight(), 1, IMAGE_FORMAT_RGBA8888 );
		auto pDest = static_cast<vlByte *>( malloc( size ) );
		VTFLib::CVTFFile::ConvertToRGBA8888(
			pVTF->GetData( frames, faces, slices, 0 ), pDest, pVTF->GetWidth(), pVTF->GetHeight(),
			pVTF->GetFormat() );
		auto img = QImage( pDest, pVTF->GetWidth(), pVTF->GetHeight(), QImage::Format_RGBA8888 );
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

		free( pDest );
	}
}

void CMainWindow::saveVTFToFile()
{
	const auto key = pImageTabWidget->tabData( pImageTabWidget->currentIndex() ).value<intptr_t>();

	auto pVTF = this->vtfWidgetList.value( key );

	if ( !pVTF )
		return;

	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	QString filePath = QFileDialog::getSaveFileName(
		this, "Save VTF",
		QFileInfo( recentPaths.last() ).completeBaseName(), "*.vtf", nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	Options::set( STR_OPEN_RECENT, recentPaths );

	if ( !filePath.endsWith( ".vtf" ) )
		filePath.append( ".vtf" );

	pVTF->Save( filePath.toUtf8().constData() );
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

void CMainWindow::dropEvent( QDropEvent *event )
{
	foreach( const QUrl &url, event->mimeData()->urls() )
	{
		this->addFile( url.toLocalFile() );
	}
}

void CMainWindow::consoleParameters( int argc, char **argv )
{
	QStringList extendedSupportedImageList = supportedImageList;
	extendedSupportedImageList << "ttf"
							   << "otf"
							   << "vtf";

	for ( int i = 1; i < argc; i++ )
	{
		if ( !extendedSupportedImageList.contains( QFileInfo( argv[i] ).suffix() ) )
			continue;

		this->addFile( argv[i] );
	}
}

void CMainWindow::addFile( QString filePath )
{
	QString suffix = QFileInfo( filePath ).suffix();

	auto recentPaths = Options::get<QStringList>( STR_OPEN_RECENT );

	if ( recentPaths.contains( filePath ) )
		recentPaths.removeAt( recentPaths.indexOf( filePath ) );
	recentPaths.push_back( filePath );
	Options::set( STR_OPEN_RECENT, recentPaths );

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

	if ( suffix == "ttf" || suffix == "otf" )
	{
		generateVTFFromFont( filePath );
		return;
	}
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
