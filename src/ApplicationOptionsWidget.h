#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QSettings>
#include <QTabWidget>
#include <string_view>

// Options
constexpr std::string_view OPT_START_MAXIMIZED = "start_maximized";
constexpr std::string_view OPT_IMPORT_SETTINGS = "import_settings";
constexpr std::string_view OPT_IMPORT_COUNT = "import_amount";
constexpr std::string_view OPT_THEME_SETTINGS = "theme_settings";
constexpr std::string_view OPT_CUSTOM_THEME = "theme_custom";
// Storage
constexpr std::string_view STR_OPEN_RECENT = "open_recent";

// namespace ui
//{
class ApplicationOptions : public QDialog
{
	Q_OBJECT
	std::unique_ptr<QSettings> options;
	uint8_t rememberPathLength = 6;

public:
	enum ApplicationPaletteOptions
	{
		VTF_FORGE_DARK,
		VTF_FORGE_LIGHT,
		VTF_FORGE_VGUI,
		VTF_FORGE_GALAXY,
		VTF_FORGE_P2CE,
		VTF_FORGE_MOMENTUM,
		OS_PALETTE,
		VTF_FORGE_CUSTOM
	};

	inline const static QJsonObject themeSettingsDefault {
		{ "style", "Fusion" },
		{ "theme", VTF_FORGE_DARK } };

	class ApplicationOptionsTab : public QTabWidget
	{
		using QTabWidget::QTabWidget;

		QSize minimumSizeHint() const override;

		QSize sizeHint() const override;
	};

	explicit ApplicationOptions( QWidget *parent );

	template <typename T>
	void set( std::string_view key, T val )
	{
		options->setValue( key, val );
	}

	template <typename T>
	T get( std::string_view str )
	{
		return options->value( str ).value<T>();
	}

	template <typename T>
	T get( std::string_view str, T defVal )
	{
		if ( !options->contains( str ) )
		{
			options->setValue( str, defVal );
			return defVal;
		}
		return options->value( str ).value<T>();
	}

	QPalette getTheme( ApplicationPaletteOptions opt );
};
//} // namespace ui

// namespace Options
//{
//	template <typename T, qsizetype C = -1>
//	class OptionList
//	{
//		QList<T> *optionList;
//
//	public:
//		explicit OptionList( QList<T> d ) :
//			optionList { d } {};
//
//		T get( qsizetype i = 0 ) const;
//		void push( T );
//		T pull( qsizetype i = 0 );
//
//		const QList<T> *getList() const;
//
//		void clear();
//	};
//
//	bool isStandalone();
//
//	void setupOptions( QSettings &options );
//
//	QSettings *getOptions();
//
//	template <typename T>
//	T get( std::string_view option )
//	{
//		return getOptions()->value( option ).value<T>();
//	}
//
//	template <typename T>
//	void set( std::string_view option, T value )
//	{
//		getOptions()->setValue( option, value );
//	}
//
//	template <qsizetype T>
//	OptionList<QString, T> getRecentlyUsedList()
//	{
//		static auto var = getOptions()->;
//		auto list = var.toStringList();
//		return OptionList<QString, T>( list );
//	}
//
//	// Only use for booleans!
//	void
//	invert( std::string_view option );

//} // namespace Options
