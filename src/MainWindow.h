#pragma once

#include "ImageSettingsWidget.h"
#include "InfoWidget.h"
#include "ResourceWidget.h"
#include "VTFEImageContainer.h"

#include <QDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMenuBar>
#include <QScrollArea>
#include <QWheelEvent>

class EntryTree;

class ApplicationOptions;

namespace ui
{

	class ZoomScrollArea;
	class CMainWindow;
	class VTFTabBar : public QTabBar
	{
		//		using QTabBar::QTabBar;
		Q_OBJECT
		CMainWindow *mainWindow;

	public:
		VTFTabBar( CMainWindow *parent );

		void mousePressEvent( QMouseEvent * ) override;
		void hasFilesChanged();

	signals:
		void onRightMouseClicked( int tab );
	};

	class CMainWindow : public QMainWindow
	{
		Q_OBJECT
		friend VTFTabBar;

	public:
		const inline static QStringList supportedWildcardImageList = { "*.bmp", "*.gif", "*.tga", "*.png", "*.jpg", "*.jpeg", "*.tif", "*.exr", "*.tiff", "*.hdr" };
		const inline static QStringList supportedImageList = { "bmp", "gif", "tga", "png", "jpg", "jpeg", "tif", "tiff", "hdr", "exr" };

	private:
		QHash<intptr_t, VTFContainer> vtfWidgetList;

	public:
		CMainWindow();
		~CMainWindow()
		{
			foreach( auto container, vtfWidgetList )
				delete container.vtf;
		}

		ApplicationOptions *options;

		ImageViewWidget *pImageViewWidget;
		ImageSettingsWidget *pImageSettingsWidget;
		EntryTree *pFileSystemTree;
		ResourceWidget *pResourceWidget;
		InfoWidget *pImageInfo;
		VTFTabBar *pImageTabWidget;
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
		void addVTFToTab( vtfpp::VTF *pVTF, const QString &name, const QString &path = "", bool fromFile = false );
		void NewVTFFromVTF( const QString &filePath );
		void tabChanged( int index );
		void exportVTFToFile();
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
		void openTabContextMenu( int tab );
		void saveVTFToFile( intptr_t key );
		void processDroppedItems( const QStringList &paths );

	signals:
		void onDropped( QStringList paths );
	public slots:
		void saveAllVTFsToFiles();
		void saveCurrentVTFToFile();
		void onPaste();
		void consoleParameters( const QStringList &params );
		void About();
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