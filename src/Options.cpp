#include "Options.h"

#include <QApplication>
#include <QFileInfo>
#include <QMetaType>
#include <QStyle>

Q_DECLARE_METATYPE( QStringList )

QSettings *opts = nullptr;

bool Options::isStandalone()
{
	QFileInfo nonportable( QApplication::applicationDirPath() + "/.nonportable" );
	return !( nonportable.exists() && nonportable.isFile() );
}

void Options::setupOptions( QSettings &options )
{
	if ( !options.contains( OPT_START_MAXIMIZED ) )
	{
		options.setValue( OPT_START_MAXIMIZED, false );
	}

	if ( !options.contains( STR_OPEN_RECENT ) )
	{
		options.setValue( STR_OPEN_RECENT, QStringList {} );
	}

	opts = &options;
}

QSettings *Options::getOptions()
{
	return opts;
}

void Options::invert( std::string_view option )
{
	set( option, !get<bool>( option ) );
}
