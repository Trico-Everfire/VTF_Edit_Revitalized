#pragma once

#include "ImageSettingsWidget.h"
#include "InfoWidget.h"
#include "ResourceWidget.h"

#include <QDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMenuBar>
#include <QScrollArea>
#include <QWheelEvent>

class EntryTree;

namespace ui
{

	class ZoomScrollArea;

	class CMainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		const inline static QStringList supportedWildcardImageList = { "*.bmp", "*.gif", "*.tga", "*.png", "*.jpg", "*.jpeg", "*.tif", "*.tiff", "*.hdr" };
		const inline static QStringList supportedImageList = { "bmp", "gif", "tga", "png", "jpg", "jpeg", "tif", "tiff", "hdr" };

	private:
		QHash<intptr_t, vtfpp::VTF *> vtfWidgetList;

	public:
		CMainWindow();
		~CMainWindow()
		{
			foreach( auto vtf, vtfWidgetList )
				delete vtf;
		}

		ImageViewWidget *pImageViewWidget;
		ImageSettingsWidget *pImageSettingsWidget;
		EntryTree *pFileSystemTree;
		ResourceWidget *pResourceWidget;
		InfoWidget *pImageInfo;
		QTabBar *pImageTabWidget;
		QMenuBar *m_pMainMenuBar;
		QWidget *m_pScrollWidget;
		QScrollBar *m_pHorizontalScrollBar;
		QScrollBar *m_pVerticalScrollBar;
		static vtfpp::VTF *getVTFFromVTFFile( const char *path );
		void addVTFFromPathToTab( const QString &path );
		void removeVTFTab( int index );
		void setupMenuBar();
		void openVTF();
		void importFromFile();
		void generateVTFFromImage( const QString &filePath );
		void generateVTFFromImages( QStringList filePaths );
		void addVTFToTab( vtfpp::VTF *pVTF, const QString &name );
		void NewVTFFromVTF( const QString &filePath );
		void tabChanged( int index );
		void exportVTFToFile();
		void saveVTFToFile();
		void compressVTFFile();
		static QAction *createCheckableAction( const QString &name, QObject *parent );
		QAction *redBox;
		QAction *greenBox;
		QAction *blueBox;
		QAction *alphaBox;
		void batchConvert();
		void compressVTFFolder();
		void generateVTFFromFont( const QString &filepath );
		void fontToVTF();

		void resizeEvent( QResizeEvent * ) override;
		void dragEnterEvent( QDragEnterEvent *event ) override;
		void dropEvent( QDropEvent *event ) override;
		void addFile( QString filePath );
		void consoleParameters( int argc, char **argv );
		bool separateSpriteSheetVTF();
	};

	class ZoomScrollArea : public QAbstractScrollArea
	{
		Q_OBJECT

		QWidget *pWidget;

		void wheelEvent( QWheelEvent *event ) override;
		bool event( QEvent * ) override;

		bool m_isCTRLHeld = false;

	public:
		ZoomScrollArea( QWidget *pParent );

		QWidget *widget() const;
		void setWidget( QWidget *widget );
		QWidget *takeWidget();

	signals:
		void onScrollUp();
		void onScrollDown();
	};
} // namespace ui