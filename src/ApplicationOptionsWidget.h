#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QSettings>
#include <QTabWidget>
#include <string_view>
#include <vtfpp/vtfpp.h>

// Options
constexpr std::string_view OPT_START_MAXIMIZED = "start_maximized";
constexpr std::string_view OPT_IMPORT_SETTINGS = "import_settings";
constexpr std::string_view OPT_IMPORT_COUNT = "import_amount";
constexpr std::string_view OPT_THEME_SETTINGS = "theme_settings";
constexpr std::string_view OPT_CUSTOM_THEME = "theme_custom";
// Advanced
constexpr std::string_view ADV_SRATA_SOURCE = "adv_support_strata";
constexpr std::string_view ADV_ALLOW_NON_PO2 = "adv_allow_non_po2";
// Settings
constexpr std::string_view IMPORT_MENU_SETTINGS = "import_menu_settings";
// Storage
constexpr std::string_view STR_OPEN_RECENT = "open_recent";

const QJsonObject IMPORT_DEFAULTS = {
	{ "format", (int)vtfpp::ImageFormat::RGB888 },
	{ "format_alpha", (int)vtfpp::ImageFormat::RGBA8888 },
	{ "texture_type", 0 },
	{ "vtf_version", 5 },
	{ "enable_compression", false },
	{ "compression_method", 0 },
	{ "compression_level", 10 },
	{ "srgb", false },
	{ "generate_thumbnail", true },
	{ "generate_reflectivity", true },
	{ "red_lumen", 0.299 },
	{ "green_lumen", 0.587 },
	{ "blue_lumen", 0.114 },
	{ "resize_method", 3 },
	{ "resize_filter", 0 },
	{ "should_clamp", false },
	{ "clamp_width", 11 },
	{ "clamp_height", 11 },
	{ "gen_mipmaps", true },
	{ "mipmap_filter", 0 },
	{ "lod_control_enabled", false },
	{ "lod_strength", 2.0f },
	{ "lod_threshold", 0.5f },
	{ "information_enabled", false },
	{ "information_author", "" },
	{ "information_contact", "" }

}

;

class ApplicationOptions : public QObject
{
	Q_OBJECT
	std::unique_ptr<QSettings> options;
	//	uint8_t rememberPathLength = 6;
	ApplicationOptions();

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

	//	class ApplicationOptionsTab : public QTabWidget
	//	{
	//		using QTabWidget::QTabWidget;
	//
	//		QSize minimumSizeHint() const override;
	//
	//		QSize sizeHint() const override;
	//	};

	//	explicit ApplicationOptions();

	static ApplicationOptions *getInstance()
	{
		static auto opts = new ApplicationOptions();
		return opts;
	}

	void set( std::string_view key, const QJsonValue &val )
	{
		options->setValue( key, val );
	}

	QJsonValue get( std::string_view str )
	{
		return options->value( str ).value<QJsonValue>();
	}

	QJsonValue get( std::string_view str, QJsonValue defVal )
	{
		if ( !options->contains( str ) )
		{
			options->setValue( str, defVal );
			return defVal;
		}
		return options->value( str ).value<QJsonValue>();
	}

	static QPalette getTheme( ApplicationPaletteOptions opt );
};
//} // namespace ui

// namespace Options
//{
//	template <typename QJsonValue, qsizetype C = -1>
//	class OptionList
//	{
//		QList<QJsonValue> *optionList;
//
//	public:
//		explicit OptionList( QList<QJsonValue> d ) :
//			optionList { d } {};
//
//		QJsonValue get( qsizetype i = 0 ) const;
//		void push( QJsonValue );
//		QJsonValue pull( qsizetype i = 0 );
//
//		const QList<QJsonValue> *getList() const;
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
//
//	QJsonValue get( std::string_view option )
//	{
//		return getOptions()->value( option ).value<QJsonValue>();
//	}
//
//
//	void set( std::string_view option, QJsonValue value )
//	{
//		getOptions()->setValue( option, value );
//	}
//
//	template <qsizetype QJsonValue>
//	OptionList<QString, QJsonValue> getRecentlyUsedList()
//	{
//		static auto var = getOptions()->;
//		auto list = var.toStringList();
//		return OptionList<QString, QJsonValue>( list );
//	}
//
//	// Only use for booleans!
//	void
//	invert( std::string_view option );

//} // namespace Options
