#pragma once

#include <QSettings>
#include <string_view>

// Options
constexpr std::string_view OPT_START_MAXIMIZED = "start_maximized";

// Storage
constexpr std::string_view STR_OPEN_RECENT = "open_recent";

namespace Options
{

	bool isStandalone();

	void setupOptions( QSettings &options );

	QSettings *getOptions();

	template <typename T>
	T get( std::string_view option )
	{
		return getOptions()->value( option ).value<T>();
	}

	template <typename T>
	void set( std::string_view option, T value )
	{
		getOptions()->setValue( option, value );
	}

	// Only use for booleans!
	void invert( std::string_view option );

} // namespace Options
