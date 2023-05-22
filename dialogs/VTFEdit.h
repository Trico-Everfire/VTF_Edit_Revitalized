#pragma once
#include "../libs/VTFLib/VTFLib/VTFLib.h"
#include "../src/ImageSettingsWidget.h"
#include "../src/ImageViewWidget.h"
#include "../src/InfoWidget.h"
#include "../src/ResourceWidget.h"
#include "../widgets/VMTQSyntaxHighlighter.h"
#include "../widgets/VTFQMenuBar.h"

#include <QDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMenuBar>
#include <QScrollArea>

namespace ui
{
	class CVTFEdit : public QDialog
	{
		QString pPath = "./";
		VTFQAction *addVTFTabEntry( VTFLib::CVTFFile *vVTF, QFileInfo vTabNameInfo );
#ifdef VMT_EDITOR
		VMTQAction *addVMTTabEntry( VTFLib::CVMTFile *vVMT, QString fileName, QString filePath );
#endif
		void SetFileSystemEnabled( bool enabled );

	public:
		CVTFEdit( QWidget *pParent );
		bool setVTFFromFile( const char *path );
		ImageViewWidget *pImageViewWidget = new ImageViewWidget( nullptr );
		VFileScrollArea *pVFileWidgetScrollArea = new VFileScrollArea( this );
		ImageSettingsWidget *pImageSettingsWidget = new ImageSettingsWidget( pImageViewWidget, this );
		InfoWidget *pInfoWidget = new InfoWidget( this );
		ResourceWidget *pResourceWidget = new ResourceWidget( this );
		VTFQMenuBar *instanceBar = new VTFQMenuBar( this );
		QMenuBar *Menubar = new QMenuBar( this );
#ifdef VMT_EDITOR
		VMTTextEdit *pVMTTextEditor = new VMTTextEdit( this );
#endif
		QGridLayout *pDialogLayout = new QGridLayout( this );
		QList<QAction *> *pDisabledActionHolder = new QList<QAction *>();
		void generateVTFFromImage( QString qString );
		QAction *createCheckableAction( QString name, QObject *parent );
		void generateVTFFromImages( QStringList filePaths );
	};
} // namespace ui