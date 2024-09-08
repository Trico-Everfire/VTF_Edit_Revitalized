#include "dialogs/VTFEdit.h"
#include "src/MainWindow.h"
#include "src/Options.h"

#include <QApplication>
#include <QCommonStyle>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QNativeIpcKey>
#include <QSharedMemory>
#include <QStyleFactory>
#include <csignal>

using namespace ui;

QLocalServer *server = nullptr;
auto basetrm = std::get_terminate();

__sighandler_t oldAbrtHandler;
__sighandler_t oldSegfHandler;

void exceptionHandler()
{
	qInfo() << "Wew";
	if ( server )
		server->close();
	delete server;
	server = nullptr;
	basetrm();
};

extern "C" void c_exceptionHandler( int signal_number )
{
	exceptionHandler();
}

int closeApplication()
{
	int res = QApplication::exec();
	delete server;
	server = nullptr;
	return res;
}

#define tr

void displayError( QLocalSocket::LocalSocketError socketError )
{
	switch ( socketError )
	{
		case QLocalSocket::ServerNotFoundError:
			QMessageBox::information( nullptr, tr( "Local Fortune Client" ),
									  tr( "The host was not found. Please make sure "
										  "that the server is running and that the "
										  "server name is correct." ) );
			break;
		case QLocalSocket::ConnectionRefusedError:
			QMessageBox::information( nullptr, tr( "Local Fortune Client" ),
									  tr( "The connection was refused by the peer. "
										  "Make sure the fortune server is running, "
										  "and check that the server name "
										  "is correct." ) );
			break;
		case QLocalSocket::PeerClosedError:
			break;
		default:
			QMessageBox::information( nullptr, tr( "Local Fortune Client" ),
									  tr( "The following error occurred: %1." ) );
			//										  .arg(socket->errorString()));
	}
}

int main( int argc, char **argv )
{
	QApplication app( argc, argv );

	const QString appKey = "QTVTFER_LOCAL_P";

	QLocalSocket *socket = new QLocalSocket();
	socket->connectToServer( appKey );

	if ( socket->isOpen() )
	{
		QByteArray data;

		QDataStream out( socket );
		out.setVersion( QDataStream::Qt_6_7 );

		for ( int i = 0; i < argc; i++ )
		{
			data.push_back( argv[i] );
			data.push_back( '\n' );
		}
		out << data;
		qInfo() << data;
		qInfo() << "IsOpen";
		if ( !socket->waitForBytesWritten( -1 ) )
		{
			qDebug() << "writen Bytes error " << socket->errorString();
			return 1;
		}
		socket->flush();
		//		if ( !socket->waitForBytesWritten() )
		//			return 1;
		socket->waitForDisconnected( 30000 );
		return 0;
	}
	delete socket;

	QCommonStyle *style = (QCommonStyle *)QStyleFactory::create( "fusion" );
	QApplication::setStyle( style );

	QPalette palette;
	palette.setColor( QPalette::Window, QColor( 49, 54, 59 ) );
	palette.setColor( QPalette::WindowText, Qt::white );
	palette.setColor( QPalette::Base, QColor( 27, 30, 32 ) );
	palette.setColor( QPalette::AlternateBase, QColor( 49, 54, 59 ) );
	palette.setColor( QPalette::ToolTipBase, Qt::black );
	palette.setColor( QPalette::ToolTipText, Qt::white );
	palette.setColor( QPalette::Text, Qt::white );
	palette.setColor( QPalette::Button, QColor( 49, 54, 59 ) );
	palette.setColor( QPalette::ButtonText, Qt::white );
	palette.setColor( QPalette::BrightText, Qt::red );
	palette.setColor( QPalette::Link, QColor( 42, 130, 218 ) );
	palette.setColor( QPalette::Highlight, QColor( 42, 130, 218 ) );
	palette.setColor( QPalette::HighlightedText, Qt::black );
	palette.setColor( QPalette::Active, QPalette::Button, QColor( 49, 54, 59 ) );
	palette.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::darkGray );
	palette.setColor( QPalette::Disabled, QPalette::WindowText, Qt::darkGray );
	palette.setColor( QPalette::Disabled, QPalette::Text, Qt::darkGray );
	palette.setColor( QPalette::Disabled, QPalette::Light, QColor( 49, 54, 59 ) );

	QApplication::setPalette( palette );

	std::unique_ptr<QSettings> options;
	if ( Options::isStandalone() )
	{
		auto configPath = QApplication::applicationDirPath() + "/config.ini";
		options = std::make_unique<QSettings>( configPath, QSettings::Format::IniFormat );
	}
	else
	{
		options = std::make_unique<QSettings>();
	}

	if ( options->value( STR_OPEN_RECENT ).value<QStringList>().isEmpty() )
		options->setValue( STR_OPEN_RECENT, QStringList() << QDir::currentPath() );

	Options::setupOptions( *options );

	auto pVTFEdit = new ui::CMainWindow();
	pVTFEdit->setAttribute( Qt::WA_DeleteOnClose );

	if ( !Options::get<bool>( OPT_START_MAXIMIZED ) )
	{
		pVTFEdit->show();
	}
	else
	{
		pVTFEdit->showMaximized();
	}

	server = new QLocalServer();
	std::set_terminate( exceptionHandler );
	signal( SIGTERM, &c_exceptionHandler );

	QObject::connect( server, &QLocalServer::newConnection, [&]
					  {
						  qInfo() << "New connected";
						  auto socket = server->nextPendingConnection();

						  qInfo() << socket->waitForReadyRead( 3000 );

						  QObject::connect( socket, &QLocalSocket::errorOccurred, &displayError );

						  QDataStream in;
						  in.setDevice( socket );
						  in.setVersion( QDataStream::Qt_6_7 );

						  in.startTransaction();
						  QByteArray nextFortune;
						  in >> nextFortune;
						  if ( !in.commitTransaction() )
							  return;
						  QStringList list = QString( nextFortune ).split( '\n' );
						  socket->disconnectFromServer();

						  char **aquiredArgs = new char *[list.size()];
						  for ( int i = 0; i < list.size(); i++ )
						  {
							  int sz = list[i].size() + 1;
							  char *ptr = aquiredArgs[i] = new char[sz];
							  memcpy( ptr, list[i].toStdString().c_str(), sz );
						  }

						  pVTFEdit->consoleParameters( list.size(), aquiredArgs );

						  for ( int i = 0; i < list.size(); i++ )
							  delete aquiredArgs[i];

						  delete[] aquiredArgs;
					  } );

	server->listen( appKey );

	pVTFEdit->consoleParameters( argc, argv );

	QApplication::setWindowIcon( QIcon( "vtf_edit_revitalised2.png" ).pixmap( 1080, 1080 ) );
	return closeApplication();
}
