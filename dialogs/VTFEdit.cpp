//
// Created by trico on 16-7-22.
//

#include "VTFEdit.h"

#include "../src/VTFEImport.h"
#include "VTFEAbout.h"

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScroller>
#include <QTreeView>

using namespace ui;

CVTFEdit::CVTFEdit( QWidget *pParent ) :
	QDialog( pParent )
{
	this->resize( 540, 540 );
	this->setFixedSize( 540, 540 );
	this->setWindowTitle( "VTF Edit Revitalized" );
	pImageViewWidget->setDisabled( true );

	auto fileMenu = Menubar->addMenu( "File" );
#ifdef VMT_EDITOR
	fileMenu->addAction(
		"New",
		[&]()
		{
			auto vVMT = new VTFLib::CVMTFile();
			addVMTTabEntry( vVMT, "untitled", QDir::currentPath() );
		} );
#endif
	fileMenu->addAction(
		"Open",
		[&]()
		{
			QString filePath = QFileDialog::getOpenFileName(
				this, "Open VTF", pPath, "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

			if ( filePath.isEmpty() )
				return;
			if ( !setVTFFromFile( filePath.toStdString().c_str() ) )
				QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
		} );
	fileMenu->addSeparator();
	pDisabledActionHolder->append( fileMenu->addAction(
		"Save",
		[&]()
		{
			if ( this->instanceBar->count() < 1 )
				return;
			auto pVTFQAction = ( (VTFQAction *)this->instanceBar->currentWidget() );
			QString str;
			if ( !pVTFQAction->hasSavePath() )
			{
				str = QFileDialog::getSaveFileName(
					this, "Open VTF", pPath, "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );
			}
			else
			{
				str = pVTFQAction->getSavePath();
			}
			if ( !str.endsWith( ".vtf" ) )
				str.append( ".vtf" );
			pVTFQAction->getVTF()->Save( str.toStdString().c_str() );
			auto t = QFileInfo( str.toStdString().c_str() ).fileName();
			if ( !pVTFQAction->hasSavePath() )
			{
				this->instanceBar->setTabText( this->instanceBar->indexOf( pVTFQAction ), t );
				pVTFQAction->setSavePath( str );
			}
		} ) );
	pDisabledActionHolder->append( fileMenu->addAction(
		"Save As...",
		[&]()
		{
			if ( this->instanceBar->count() < 1 )
				return;
			auto pVTFQAction = ( (VTFQAction *)this->instanceBar->currentWidget() );
			QString str;
			if ( !pVTFQAction->hasSavePath() )
			{
				str = QFileDialog::getSaveFileName(
					this, "Open VTF", pPath, "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );
			}
			else
			{
				str = pVTFQAction->getSavePath();
			}
			if ( !str.endsWith( ".vtf" ) )
				str.append( ".vtf" );
			pVTFQAction->getVTF()->Save( str.toStdString().c_str() );
			auto t = QFileInfo( str.toStdString().c_str() ).fileName();
			if ( !pVTFQAction->hasSavePath() )
			{
				this->instanceBar->setTabText( this->instanceBar->indexOf( pVTFQAction ), t );
				pVTFQAction->setSavePath( str );
			}
		} ) );
	pDisabledActionHolder->append( fileMenu->addAction(
		"Save All",
		[&]()
		{
			for ( int i = 0; i < this->instanceBar->count(); i++ )
			{
				auto pVTFQAction = ( (VTFQAction *)this->instanceBar->widget( i ) );
				QString str;
				if ( !pVTFQAction->hasSavePath() )
				{
					str = QFileDialog::getSaveFileName(
						this, "Open VTF", pPath, "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );
				}
				else
				{
					str = pVTFQAction->getSavePath();
				}
				if ( !str.endsWith( ".vtf" ) )
					str.append( ".vtf" );
				pVTFQAction->getVTF()->Save( str.toStdString().c_str() );
				auto t = QFileInfo( str.toStdString().c_str() ).fileName();
				if ( !pVTFQAction->hasSavePath() )
				{
					this->instanceBar->setTabText( this->instanceBar->indexOf( pVTFQAction ), t );
					pVTFQAction->setSavePath( str );
				}
			}
		} ) );
	fileMenu->addSeparator();
	fileMenu->addAction(
		"Import",
		[&]()
		{
			QStringList filePaths = QFileDialog::getOpenFileNames(
				this, "Open", "./", "*.bmp *.gif *.jpg *.jpeg *.png *.tga", nullptr,
				QFileDialog::Option::DontUseNativeDialog );
			if ( filePaths.count() == 1 )
				generateVTFFromImage( filePaths[0] );
			else
				generateVTFFromImages( filePaths );
		} );
	fileMenu->addAction(
		"Import from VTF",
		[&]()
		{
			QString filePath = QFileDialog::getOpenFileName(
				this, "Open VTF", pPath, "*.vtf", nullptr, QFileDialog::Option::DontUseNativeDialog );

			auto vVTF = new VTFLib::CVTFFile();
			if ( !vVTF->Load( filePath.toStdString().c_str(), false ) )
				return QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
			auto vtfWindow = VTFEImport::fromVTF( this, vVTF );
			if ( !vtfWindow )
				return QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
			vtfWindow->exec();
			if ( vtfWindow->isCancelled() )
			{
				vtfWindow->generateVTF();
				auto vNewVTF = vtfWindow->getVTF();
				if ( !vNewVTF )
					return QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
				addVTFTabEntry( vNewVTF, QFileInfo( filePath ).fileName() );
			}
			delete vVTF;
		} );
	pDisabledActionHolder->append( fileMenu->addAction(
		"Export",
		[&]()
		{
			auto wget = (VTFQAction *)this->instanceBar->currentWidget();
			auto text = this->instanceBar->tabText( this->instanceBar->currentIndex() );
			VTFLib::CVTFFile *pFile = ( wget )->getVTF();

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

			QString filePath = QFileDialog::getSaveFileName(
				this, fImageAmount > 1 ? "Export to *" : "Export to _x*",
				"./" + ( text.mid( 0, text.lastIndexOf( "." ) ) ) + ".tga", "*.bmp *.gif *.jpg *.jpeg *.png *.tga", nullptr,
				QFileDialog::Option::DontUseNativeDialog );
			qInfo() << filePath;
			if ( filePath.isEmpty() )
				return;

			for ( int i = 0; i < fImageAmount; i++ )
			{
				vlUInt frames = type == 0 ? i + 1 : 1;
				vlUInt faces = type == 1 ? i + 1 : 1;
				vlUInt slices = type == 2 ? i + 1 : 1;

				auto size =
					VTFLib::CVTFFile::ComputeImageSize( pFile->GetWidth(), pFile->GetHeight(), 1, IMAGE_FORMAT_RGBA8888 );
				auto pDest = static_cast<vlByte *>( malloc( size ) );
				qInfo() << VTFLib::CVTFFile::ConvertToRGBA8888(
					pFile->GetData( frames, faces, slices, 0 ), pDest, pFile->GetWidth(), pFile->GetHeight(),
					pFile->GetFormat() );
				auto img = QImage( pDest, pFile->GetWidth(), pFile->GetHeight(), QImage::Format_RGBA8888 );
				if ( fImageAmount > 1 )
				{
					QString nummedPath =
						filePath.mid( 0, filePath.count() - 4 ) + "_" + QString::number( i ) +
						filePath.mid( filePath.count() - 4, filePath.count() );
					qInfo() << nummedPath;
					qInfo() << img.save( nummedPath );
				}
				else
					img.save( filePath );

				free( pDest );
			}
		} ) );
	pDisabledActionHolder->append( fileMenu->addAction(
		"Export All",
		[&]()
		{
			this->close();
		} ) );

	fileMenu->addSeparator();
	fileMenu->addMenu( "Recent Files" );
	fileMenu->addSeparator();
	fileMenu->addAction(
		"Exit",
		[&]()
		{
			this->close();
		} );

	//	auto editMenu = Menubar->addMenu("Edit");
	//	editMenu
	//		->addAction(
	//			"Copy",
	//			[&]()
	//			{
	//				this->close();
	//			})
	//		->setDisabled(true);
	//	editMenu
	//		->addAction(
	//			"Paste",
	//			[&]()
	//			{
	//				this->close();
	//			})
	//		->setDisabled(true);

	auto viewMenu = Menubar->addMenu( "View" );
	//	auto redBox = createCheckableAction("Red",viewMenu);
	//	auto greenBox = createCheckableAction("Green",viewMenu);
	//	auto blueBox = createCheckableAction("Blue",viewMenu);
	//	auto alphaBox = createCheckableAction("Alpha",viewMenu);
	//	connect(redBox, &QAction::triggered,[this](bool checked){pImageViewWidget->set_red(checked);});
	//	connect(greenBox, &QAction::triggered,[this](bool checked){pImageViewWidget->set_green(checked);});
	//	connect(blueBox, &QAction::triggered,[this](bool checked){pImageViewWidget->set_blue(checked);});
	//	connect(alphaBox, &QAction::triggered,[this](bool checked){pImageViewWidget->set_alpha(checked);});
	//	viewMenu->addAction(redBox);
	//	viewMenu->addAction(greenBox);
	//	viewMenu->addAction(blueBox);
	//	viewMenu->addAction(alphaBox);
	viewMenu->addAction( "Tile" );

	auto toolsMenu = Menubar->addMenu( "Tools" );
#ifdef VMT_EDITOR
	toolsMenu->addAction( "Create VMT File" );
#endif
	toolsMenu->addAction( "Convert Folder" );
	toolsMenu->addAction( "Create WAD File" );
	auto optionsMenu = Menubar->addMenu( "Options" );
	optionsMenu->addAction( "Auto Create VMT File" );
	auto checkable1 = new QAction( "File Mapping", optionsMenu );
	checkable1->setCheckable( true );
	checkable1->setChecked( true );
	optionsMenu->addAction( checkable1 );
	auto checkable2 = new QAction( "Volatile Access", optionsMenu );
	checkable2->setCheckable( true );
	checkable2->setChecked( true );
	optionsMenu->addAction( checkable2 );
	optionsMenu->addAction( "Keybinds" );
	auto helpMenu = Menubar->addMenu( "Help" );
	helpMenu->addAction(
		"About",
		[&]()
		{
			auto aboutWindow = new VTFEAbout( this );
			aboutWindow->exec();
		} );

	for ( int i = 0; i < pDisabledActionHolder->count(); i++ )
		pDisabledActionHolder->value( i )->setDisabled( true );

	pDialogLayout->addWidget( Menubar, 0, 0, Qt::AlignTop );
	pDialogLayout->addWidget( instanceBar, 1, 1, Qt::AlignTop );

	auto pTabWidget = new QTabWidget( this );
	auto model = new QFileSystemModel;
	model->setRootPath( QDir::homePath() );
	model->setNameFilterDisables( false );
	QStringList strlst;
	strlst.append( "*.vtf" );
	strlst.append( "*.bmp" );
	strlst.append( "*.gif" );
	strlst.append( "*.jpg" );
	strlst.append( "*.jpeg" );
	strlst.append( "*.png" );
	strlst.append( "*.tga" );
	model->setNameFilters( strlst );
	auto tree = new QTreeView();

	tree->setFixedSize( this->width() / 2, this->height() );
	tree->setModel( model );
	tree->setRootIndex( model->index( QDir::homePath() ) );
	connect(
		tree, &QTreeView::doubleClicked,
		[this, tree]( const QModelIndex &index )
		{
			QString path = ( (QFileSystemModel *)index.model() )->filePath( index );
			if ( path.endsWith( "/" ) )
				tree->setExpanded( index, !tree->isExpanded( index ) );
			if ( path.endsWith( ".bmp" ) || path.endsWith( ".gif" ) || path.endsWith( ".jpg" ) || path.endsWith( ".jpeg" ) ||
				 path.endsWith( ".png" ) || path.endsWith( ".tga" ) )
			{
				generateVTFFromImage( path );
			}
			if ( path.endsWith( ".vtf" ) )
			{
				auto msg = new QMessageBox();
				msg->addButton( "Open", QMessageBox::NoRole );
				msg->addButton( "Import as VTF", QMessageBox::NoRole );
				msg->setStandardButtons( QMessageBox::Cancel );
				connect(
					msg, &QMessageBox::buttonClicked,
					[this, path]( QAbstractButton *btn )
					{
						if ( btn->text() == "Open" )
						{
							if ( !setVTFFromFile( path.toStdString().c_str() ) )
								QMessageBox::warning( this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
						}
						else
						{
							auto vVTF = new VTFLib::CVTFFile();
							if ( !vVTF->Load( path.toStdString().c_str(), false ) )
								return QMessageBox::warning(
									this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
							auto vtfWindow = VTFEImport::fromVTF( this, vVTF );
							if ( !vtfWindow )
								return QMessageBox::warning(
									this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
							vtfWindow->exec();
							if ( !vtfWindow->isCancelled() )
							{
								vtfWindow->generateVTF();
								auto vNewVTF = vtfWindow->getVTF();
								if ( !vNewVTF )
									return QMessageBox::warning(
										this, "INVALID VTF", "The VTF is invalid.", QMessageBox::Ok );
								addVTFTabEntry( vNewVTF, QFileInfo( path ).fileName() );
							}
							delete vVTF;
						}
					} );
				msg->exec();

				delete msg;
			}
		} );
	tree->header()->resizeSection( 0, tree->width() );
	pTabWidget->addTab( tree, tr( "FileSystem" ) );

	pImageSettingsWidget->setFixedSize( this->width() / 2, this->height() );
	pTabWidget->addTab( pImageSettingsWidget, tr( "Image" ) );
	pDialogLayout->addWidget( pTabWidget, 2, 0, Qt::AlignLeft );
#ifdef VMT_EDITOR
	new VMTQSyntaxHighlighter( pVMTTextEditor->document() );
	pVMTTextEditor->setFixedSize( this->width(), this->height() );
	pVMTTextEditor->setHidden( true );
	pVMTTextEditor->setDisabled( true );
#endif

	pVFileWidgetScrollArea->setFixedSize( this->width(), this->height() );
	pDialogLayout->addWidget( pVFileWidgetScrollArea, 2, 1, Qt::AlignCenter );

	auto pTabWidget2 = new QTabWidget( this );
	pTabWidget2->addTab( pInfoWidget, tr( "Info" ) );
	pTabWidget2->addTab( pResourceWidget, tr( "Resource" ) );
	pDialogLayout->addWidget( pTabWidget2, 2, 2 );

	pDialogLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );
	pDialogLayout->setAlignment( Qt::AlignTop );
}

QAction *CVTFEdit::createCheckableAction( QString name, QObject *parent )
{
	auto maskBox = new QAction( name, parent );
	maskBox->setCheckable( true );
	maskBox->setChecked( true );
	return maskBox;
}

// Fetches and parses the VTF from a file and sets it as the main Image.
bool CVTFEdit::setVTFFromFile( const char *path )
{
	auto vVTF = new VTFLib::CVTFFile();
	if ( !vVTF->Load( path, false ) )
		return false;
	QFileInfo fl( path );
	auto pVTFWidget = addVTFTabEntry( vVTF, fl );
	if ( fl.fileName().endsWith( ".vtf" ) )
		pVTFWidget->setSavePath( fl.filePath() );
	return true;
}

VTFQAction *CVTFEdit::addVTFTabEntry( VTFLib::CVTFFile *vVTF, QFileInfo vTabName )
{
	this->setWindowTitle( "VTF Edit Revitalized - " + vTabName.fileName() );
	pPath = ( vTabName.filePath() );
	auto pVTFWidget = instanceBar->addVTFAction(
		vVTF, vTabName.fileName(),
		[this]( VTFLib::CVTFFile *vtf )
		{
			pImageViewWidget->setDisabled( false );
#ifdef VMT_EDITOR
			pVMTTextEditor->setHidden( true );
			pVMTTextEditor->setDisabled( true );
#endif
			pImageSettingsWidget->set_vtf( vtf );
			pImageSettingsWidget->set_vtf( vtf ); // needs to be set twice due to bug related to
			pVFileWidgetScrollArea->setWidget( pImageViewWidget );
			pImageViewWidget->set_vtf( vtf );
			pImageViewWidget->repaint();
			pResourceWidget->set_vtf( vtf );
			pInfoWidget->update_info( vtf );
		} );
	connect(
		pVTFWidget, &VTFQAction::setFinalsDefault,
		[this]()
		{
			this->setWindowTitle( "VTF Edit Revitalized" );
			pImageSettingsWidget->set_vtf( nullptr );
			pImageViewWidget->set_vtf( nullptr );
			pImageViewWidget->repaint();
			//			pVFileWidgetScrollArea->takeWidget();
			pResourceWidget->set_vtf( nullptr );
			pInfoWidget->update_info( nullptr );
			for ( int i = 0; i < pDisabledActionHolder->count(); i++ )
				pDisabledActionHolder->value( i )->setDisabled( true );
		} );
	for ( int i = 0; i < pDisabledActionHolder->count(); i++ )
		pDisabledActionHolder->value( i )->setDisabled( false );
	return pVTFWidget;
}
#ifdef VMT_EDITOR
VMTQAction *CVTFEdit::addVMTTabEntry( VTFLib::CVMTFile *vVMT, QString fileName, QString filePath )
{
	this->setWindowTitle( "VTF Edit Revitalized - " + fileName );
	pPath = ( filePath );
	auto pVMTWidget = instanceBar->addVMTAction(
		vVMT, fileName,
		[this]( VTFLib::CVMTFile *vmt )
		{
			pImageSettingsWidget->set_vtf( nullptr );
			pImageViewWidget->set_vtf( nullptr );
			pImageViewWidget->repaint();
			pImageViewWidget->setDisabled( true );
			pVMTTextEditor->setDisabled( false );
			pVMTTextEditor->setHidden( false );
			pVFileWidgetScrollArea->setWidget( pVMTTextEditor );
			pResourceWidget->set_vtf( nullptr );
			pInfoWidget->update_info( nullptr );
		} );
	return pVMTWidget;
}
#endif

void CVTFEdit::SetFileSystemEnabled( bool enabled )
{
}

void CVTFEdit::generateVTFFromImage( QString filePath )
{
	if ( filePath.isEmpty() )
		return;
	auto newWindow = new VTFEImport( this, filePath );
	newWindow->exec();
	if ( newWindow->isCancelled() )
		return;
	VTFErrorType result = newWindow->generateVTF();
	switch ( result )
	{
		case VTFErrorType::INVALIDIMAGE:
			QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
			return;
	}
	auto vVTF = newWindow->getVTF();
	QFileInfo fl( filePath );
	addVTFTabEntry( vVTF, fl );
}

void CVTFEdit::generateVTFFromImages( QStringList filePaths )
{
	if ( filePaths.isEmpty() )
		return;
	auto newWindow = new VTFEImport( this, filePaths );
	newWindow->exec();
	if ( newWindow->isCancelled() )
		return;
	VTFErrorType result = newWindow->generateVTF();
	switch ( result )
	{
		case VTFErrorType::NODATA:
		case VTFErrorType::INVALIDIMAGE:
			QMessageBox::critical( this, "INVALID IMAGE", "The Image is invalid.", QMessageBox::Ok );
			return;
	}
	auto vVTF = newWindow->getVTF();
	QFileInfo fl( filePaths[0] );
	addVTFTabEntry( vVTF, fl );
}
