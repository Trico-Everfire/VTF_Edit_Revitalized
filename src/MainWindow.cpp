#include "MainWindow.h"

#include "VTFEImport.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>

using namespace ui;

CMainWindow::CMainWindow( QWidget *pParent ) :
	QDialog( pParent )
{
	this->setWindowTitle( "VTF Edit Revitalized" );

	auto pMainLayout = new QGridLayout( this );

	pImageTabWidget = new QTabBar( this );

	pImageTabWidget->setTabsClosable( true );
	pImageTabWidget->setMovable( true );

	pMainLayout->addWidget( pImageTabWidget, 0, 1, Qt::AlignTop );

	auto scrollWidget = new ZoomScrollArea( this );

	pImageViewWidget = new ImageViewWidget( this );

	scrollWidget->setWidget( pImageViewWidget );
	scrollWidget->setMinimumSize( 512, 512 );

	pMainLayout->addWidget( scrollWidget, 1, 1, Qt::AlignTop );

	pImageSettingsWidget = new ImageSettingsWidget( pImageViewWidget, this );
	pMainLayout->addWidget( pImageSettingsWidget, 0, 0, 2, 1, Qt::AlignLeft );

	auto pInfoResourceTabWidget = new QTabWidget( this );

	pImageInfo = new InfoWidget( this );

	pInfoResourceTabWidget->addTab( pImageInfo, "Info" );

	pResourceWidget = new ResourceWidget( this );

	pInfoResourceTabWidget->addTab( pResourceWidget, "Resources" );

	pMainLayout->addWidget( pInfoResourceTabWidget, 0, 2, 2, 1, Qt::AlignRight );

	m_pMainMenuBar = new QMenuBar( this );
	m_pMainMenuBar->setNativeMenuBar( false );
	pMainLayout->setMenuBar( m_pMainMenuBar );

	connect( pImageTabWidget, &QTabBar::tabCloseRequested, this, &CMainWindow::removeVTFTab );

	connect( pImageTabWidget, &QTabBar::currentChanged, this, &CMainWindow::tabChanged );

	connect( scrollWidget, &ZoomScrollArea::onScrollUp, this, [&]
			 {
				 pImageViewWidget->zoom( 0.1 );
			 } );

	connect( scrollWidget, &ZoomScrollArea::onScrollDown, this, [&]
			 {
				 pImageViewWidget->zoom( -0.1 );
			 } );

	setupMenuBar();
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
	pResourceWidget->set_vtf( pVTF );
	pImageSettingsWidget->set_vtf( pVTF );
	pImageSettingsWidget->set_vtf( pVTF );
	pImageInfo->update_info( pVTF );
}

void CMainWindow::setupMenuBar()
{
	auto pFileMenuTab = m_pMainMenuBar->addMenu( "File" );
	pFileMenuTab->addAction( "Open", this, &CMainWindow::openVTF );
	pFileMenuTab->addAction( "Save", this, &CMainWindow::saveVTFToFile );
	pFileMenuTab->addAction( "Export", this, &CMainWindow::exportVTFToFile );
	pFileMenuTab->addAction( "Import...", this, &CMainWindow::importFromFile );

	auto pToolMenuTab = m_pMainMenuBar->addMenu( "Tools" );
	pToolMenuTab->addAction( "VTF Version Editor", this, &CMainWindow::compressVTFFile );
	pToolMenuTab->addAction( "Folder to VTF", this, &CMainWindow::ImageToVTF );

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
	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open VTF", QDir::currentPath(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePaths.isEmpty() )
		return;

	auto pCompressionDialog = new QDialog( this );
	pCompressionDialog->setWindowTitle( "VTF Version Editor" );
	auto vBLayout = new QGridLayout( pCompressionDialog );

	auto label1 = new QLabel( "VTF Version:", this );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );

	auto pVtfVersionBox = new QComboBox( this );
#ifdef CHAOS_INITIATIVE
	for ( int i = 0; i <= VTF_MINOR_VERSION; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( VTF_MAJOR_VERSION ) + "." + QString::number( i ), i );
	}
	pVtfVersionBox->setCurrentIndex( pVtfVersionBox->count() - 2 );
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

	auto pButtonLayout = new QHBoxLayout();

	auto pOkButton = new QPushButton( "Update Version", pCompressionDialog );
	pButtonLayout->addWidget( pOkButton, Qt::AlignCenter );

	auto pCancelButton = new QPushButton( "Cancel", pCompressionDialog );
	pButtonLayout->addWidget( pCancelButton, Qt::AlignCenter );

	vBLayout->addLayout( pButtonLayout, 5, 0, 1, 2 );

	bool compress = false;

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

	connect( pCustomDestination, &QCheckBox::toggled, pCompressionDialog, [&pDestinationLocation, &pSelectDestinationLocation]( bool checked )
			 {
				 pDestinationLocation->setEnabled( checked );
				 pDestinationLocation->setText( "" );
				 pSelectDestinationLocation->setEnabled( checked );
			 } );

	connect( pSelectDestinationLocation, &QPushButton::pressed, pCompressionDialog, [&pDestinationLocation]
			 {
				 pDestinationLocation->setText( QFileDialog::getExistingDirectory( nullptr, "Save to:", QDir::currentPath() ) );
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

		if ( pVTF->GetMinorVersion() == pVtfVersionBox->currentData().toInt() && pVTF->GetAuxCompressionLevel() == pAuxCompressionLevelBox->currentData().toInt() )
			continue;

		pVTF->SetVersion( 7, pVtfVersionBox->currentData().toInt() );

		if ( pAuxCompressionBox->isChecked() )
		{
			pVTF->SetAuxCompressionLevel( pAuxCompressionLevelBox->currentData().toInt() );
		}

		if ( pathDirectory.isEmpty() )
			pVTF->Save( filePath.toUtf8().constData() );
		else
			pVTF->Save( ( pathDirectory + "/" + QFileInfo( filePath ).fileName() ).toUtf8().constData() );

		delete pVTF;
	}
}

void CMainWindow::ImageToVTF()
{
}

void CMainWindow::importFromFile()
{
	QStringList filePaths = QFileDialog::getOpenFileNames(
		this, "Open", "./", "*.bmp *.gif *.jpg *.jpeg *.png *.tga *.hdr *.vtf", nullptr,
		QFileDialog::Option::DontUseNativeDialog );

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
	QString filePath = QFileDialog::getOpenFileName(
		this, "Open VTF", QDir::currentPath(), "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

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

	QString filePath = QFileDialog::getSaveFileName(
		this, fImageAmount > 1 ? "Export to *" : "Export to _x*",
		QDir::currentPath(), "*.bmp *.gif *.jpg *.jpeg *.png *.tga", nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	for ( int i = 0; i < fImageAmount; i++ )
	{
		vlUInt frames = type == 0 ? i + 1 : 1;
		vlUInt faces = type == 1 ? i + 1 : 1;
		vlUInt slices = type == 2 ? i + 1 : 1;

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
				filePath.mid( 0, filePath.count() - 4 ) + "_" + QString::number( i ) +
				filePath.mid( filePath.count() - 4, filePath.count() );

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

	QString filePath = QFileDialog::getSaveFileName(
		this, "Save VTF",
		QDir::currentPath(), "*.vtf", nullptr,
		QFileDialog::Option::DontUseNativeDialog );

	if ( filePath.isEmpty() )
		return;

	if ( !filePath.endsWith( ".vtf" ) )
		filePath.append( ".vtf" );

	pVTF->Save( filePath.toUtf8().constData() );
}

void CMainWindow::processCLIArguments( const int &argCount, char **pString )
{
	for ( int i = 0; i < argCount; i++ )
	{
		QString filePath = QString( pString[i] );
		if ( filePath.endsWith( ".vtf" ) )
			addVTFFromPathToTab( filePath );
	}
}

ZoomScrollArea::ZoomScrollArea( QWidget *pParent ) :
	QScrollArea( pParent )
{
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

	return QScrollArea::event( event );
}
