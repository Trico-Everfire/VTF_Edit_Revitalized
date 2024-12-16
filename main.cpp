#include "src/ApplicationOptionsWidget.h"
#include "src/MainWindow.h"

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>

using namespace ui;

// class changeableApplication : public QApplication
//{
//	style
// };
//	bool event( QEvent *event ) override
//	{
//		if ( event->type() == QEvent::ApplicationPaletteChange )
//		{
//			updateColorMode();
//			Q_EMIT paletteChanged();
//		}
//		return QApplication::event( event );
//	}
//
//	void updateColorMode()
//	{
//		// Detect if we use a dark theme
//		const QPalette palette;
//		const bool isDarkMode = false; //= palette.base().color().lightness()
//									   //< palette.windowText().color().lightness();
//		setProperty( "DARK_MODE", isDarkMode );
//	}
//   };

int main( int argc, char **argv )
{
	//	QGuiApplication::setDesktopSettingsAware( false );
	QApplication app( argc, argv );

	QApplication::setWindowIcon( QIcon( ":/VTF_Forge_Icon.ico" ) );

	const QString appKey = "QTVTFER_LOCAL_P";

	auto socket = std::make_unique<QLocalSocket>();
	socket->connectToServer( appKey );

	bool multiInstance = QCoreApplication::arguments().contains( "--multi-instance" ) || QCoreApplication::arguments().contains( "-mi" );

	if ( socket->isOpen() && !multiInstance )
	{
		QByteArray data;

		QDataStream out( socket.get() );
		out.setVersion( QDataStream::Qt_6_7 );

		for ( int i = 0; i < argc; i++ )
		{
			data.push_back( argv[i] );
			data.push_back( '\n' );
		}
		out << data;
		qInfo() << data;
		if ( !socket->waitForBytesWritten( -1 ) )
		{
			qDebug() << "writen Bytes error " << socket->errorString();
			return 1;
		}
		socket->flush();

		socket->waitForDisconnected( 30000 );
		return 0;
	}

	if ( socket->error() == QLocalSocket::ConnectionRefusedError )
		QLocalServer::removeServer( appKey );

	//	QApplication::setPalette( palette );

	//	std::unique_ptr<QSettings> options;
	//	if ( Options::isStandalone() )
	//	{
	//		auto configPath = QApplication::applicationDirPath() + "/config.ini";
	//		options = std::make_unique<QSettings>( configPath, QSettings::Format::IniFormat );
	//	}
	//	else
	//	{
	//		options = std::make_unique<QSettings>();
	//	}
	//
	//	if ( options->value( STR_OPEN_RECENT ).value<QStringList>().isEmpty() )
	//		options->setValue( STR_OPEN_RECENT, QStringList() << QDir::currentPath() );
	//
	//	Options::setupOptions( *options );

	auto pVTFEdit = new ui::CMainWindow();
	pVTFEdit->setWindowIcon( QIcon( ":/VTF_Forge_Icon.ico" ) );
	pVTFEdit->setAttribute( Qt::WA_DeleteOnClose );

	if ( !pVTFEdit->options->get<bool>( OPT_START_MAXIMIZED, false ) )
	{
		pVTFEdit->show();
	}
	else
	{
		pVTFEdit->showMaximized();
	}

	auto server = std::make_unique<QLocalServer>();
	if ( !multiInstance )
	{
		QObject::connect( server.get(), &QLocalServer::newConnection, [&]
						  {
							  auto socket = std::unique_ptr<QLocalSocket>( server->nextPendingConnection() );

							  socket->waitForReadyRead( 3000 );

							  QDataStream in;
							  in.setDevice( socket.get() );
							  in.setVersion( QDataStream::Qt_6_7 );

							  in.startTransaction();
							  QByteArray argData;
							  in >> argData;
							  if ( !in.commitTransaction() )
								  return;
							  QStringList argList = QString( argData ).split( '\n' );
							  socket->disconnectFromServer();
							  pVTFEdit->consoleParameters( argList );
							  pVTFEdit->activateWindow();
						  } );

		server->listen( appKey );
	}

	pVTFEdit->consoleParameters( argc, argv );
	return QApplication::exec();
}
