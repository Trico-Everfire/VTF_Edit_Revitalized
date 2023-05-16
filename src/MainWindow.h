#pragma once

#include "ImageSettingsWidget.h"
#include "InfoWidget.h"
#include "ResourceWidget.h"

#include <QDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QScrollArea>
#include <QWheelEvent>

namespace ui
{

	class CMainWindow : public QDialog
	{
		Q_OBJECT

		QHash<intptr_t, VTFLib::CVTFFile *> vtfWidgetList;

	public:
		CMainWindow( QWidget *pParent = nullptr );
		~CMainWindow()
		{
			foreach( auto vtf, vtfWidgetList )
				delete vtf;
		}

		ImageViewWidget *pImageViewWidget;
		ImageSettingsWidget *pImageSettingsWidget;
		ResourceWidget *pResourceWidget;
		InfoWidget *pImageInfo;
		QTabBar *pImageTabWidget;
		QMenuBar *m_pMainMenuBar;
		static VTFLib::CVTFFile *getVTFFromVTFFile( const char *path );
		void addVTFFromPathToTab( const QString &path );
		void removeVTFTab( int index );
		void setupMenuBar();
		void openVTF();
		void importFromFile();
		void generateVTFFromImage( const QString &filePath );
		void generateVTFFromImages( QStringList filePaths );
		void addVTFToTab( VTFLib::CVTFFile *pVTF, const QString &name );
		void NewVTFFromVTF( const QString &filePath );
		void tabChanged( int index );
		void exportVTFToFile();
		void saveVTFToFile();
		void compressVTFFile();
		void processCLIArguments( const int &argCount, char **pString );
		static QAction *createCheckableAction( const QString &name, QObject *parent );
		QAction *redBox;
		QAction *greenBox;
		QAction *blueBox;
		QAction *alphaBox;
		void ImageToVTF();
	};

	class ZoomScrollArea : public QScrollArea
	{
		Q_OBJECT
		void wheelEvent( QWheelEvent *event ) override;
		bool event( QEvent * ) override;
		bool m_isCTRLHeld = false;

	public:
		ZoomScrollArea( QWidget *pParent );

	signals:
		void onScrollUp();
		void onScrollDown();
	};
} // namespace ui