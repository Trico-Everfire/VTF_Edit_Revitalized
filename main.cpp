#include "dialogs/VTFEdit.h"
#include "libs/VPKTools/include/vpktool/VPK.h"
#include "src/MainWindow.h"

#include <QApplication>
#include <QCommonStyle>
#include <QDebug>
#include <QIcon>
#include <QStyleFactory>

using namespace ui;

int main( int argc, char **argv )
{
	QApplication app( argc, argv );

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

	auto pVTFEdit = new ui::CMainWindow( nullptr );
	pVTFEdit->processCLIArguments( argc, argv );
	pVTFEdit->setAttribute( Qt::WA_DeleteOnClose );
	pVTFEdit->show();

	QApplication::setWindowIcon( QIcon( "vtf_edit_revitalised2.png" ).pixmap( 1080, 1080 ) );
	return QApplication::exec();
}
