#include "ApplicationOptionsWidget.h"

#include <QApplication>
#include <QColorDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMetaEnum>
#include <QMetaType>
#include <QPainter>
#include <QPushButton>
#include <QStackedLayout>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOptionTabWidgetFrame>

// Q_DECLARE_METATYPE( QStringList )

// QSettings *opts = nullptr;

// bool Options::isStandalone()
//{
//	QFileInfo nonportable( QApplication::applicationDirPath() + "/.nonportable" );
//	return !( nonportable.exists() && nonportable.isFile() );
// }
//
// void Options::setupOptions( QSettings &options )
//{
//	if ( !options.contains( OPT_START_MAXIMIZED ) )
//	{
//		options.setValue( OPT_START_MAXIMIZED, false );
//	}
//
//	if ( !options.contains( OPT_IMPORT_BEHAVIOUR ) )
//	{
//		options.setValue( OPT_IMPORT_BEHAVIOUR, QList<QVariant> {} );
//	}
//
//	if ( !options.contains( STR_OPEN_RECENT ) )
//	{
//		options.setValue( STR_OPEN_RECENT, QStringList {} );
//	}
//
//	opts = &options;
// }
//
// QSettings *Options::getOptions()
//{
//	return opts;
// }
//
// void Options::invert( std::string_view option )
//{
//	set( option, !get<bool>( option ) );
// }
//
// template <typename T, qsizetype C>
// T Options::OptionList<T, C>::get( qsizetype i ) const
//{
//	return optionList->at( i );
// }
// template <typename T, qsizetype C>
// void Options::OptionList<T, C>::clear()
//{
//	optionList->clear();
// }
//
// template <typename T, qsizetype C>
// T Options::OptionList<T, C>::pull( qsizetype i )
//{
//	T val = optionList->at( i );
//	optionList->remove( i );
//	return nullptr;
// }
//
// template <typename T, qsizetype C>
// void Options::OptionList<T, C>::push( T type )
//{
//	if ( optionList->size() > C )
//		optionList->pop_front();
//	optionList->push_back( type );
// }
//
// template <typename T, qsizetype C>
// const QList<T> *Options::OptionList<T, C>::getList() const
//{
//	return optionList;
// }
// using namespace ui;

inline QPalette ApplicationOptions::getTheme( ApplicationOptions::ApplicationPaletteOptions opt )
{
	if ( opt == ApplicationOptions::VTF_FORGE_DARK )
	{
		QPalette palette { QApplication::palette() };
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
		return palette;
	}
	if ( opt == ApplicationOptions::VTF_FORGE_LIGHT )
	{
		QPalette palette { QApplication::palette() };
		palette.setColor( QPalette::Text, Qt::black );
		palette.setColor( QPalette::WindowText, Qt::black );
		palette.setColor( QPalette::ToolTipText, Qt::black );
		palette.setColor( QPalette::ButtonText, Qt::black );
		palette.setColor( QPalette::BrightText, Qt::black );
		palette.setColor( QPalette::HighlightedText, Qt::black );
		palette.setColor( QPalette::Active, QPalette::BrightText, Qt::black );

		palette.setColor( QPalette::Dark, 0xfbfbfb );
		palette.setColor( QPalette::Light, 0xfbfbfb );

		palette.setColor( QPalette::Base, 0xefefef );
		palette.setColor( QPalette::Window, 0xefefef );
		palette.setColor( QPalette::Button, 0xfbfbfb );
		palette.setColor( QPalette::Button, QColor( 240, 240, 240 ) );
		palette.setColor( QPalette::AlternateBase, 0xefefef );

		palette.setColor( QPalette::Disabled, QPalette::ButtonText, QColor( 150, 150, 150 ) );
		palette.setColor( QPalette::Disabled, QPalette::WindowText, QColor( 150, 150, 150 ) );
		palette.setColor( QPalette::Disabled, QPalette::Text, QColor( 150, 150, 150 ) );

		return palette;
	}
	if ( opt == ApplicationOptions::VTF_FORGE_VGUI )
	{
		QPalette palette {};
		palette.setColor( QPalette::Text, QColor( 216, 222, 211 ) );
		palette.setColor( QPalette::WindowText, QColor( 216, 222, 211 ) );
		palette.setColor( QPalette::ToolTipText, QColor( 216, 222, 211 ) );
		palette.setColor( QPalette::ButtonText, QColor( 216, 222, 211 ) );
		palette.setColor( QPalette::BrightText, QColor( 255, 255, 255 ) );
		palette.setColor( QPalette::HighlightedText, QColor( 255, 255, 255 ) );
		palette.setColor( QPalette::Active, QPalette::BrightText, QColor( 196, 181, 80 ) );
		palette.setColor( QPalette::Disabled, QPalette::Text, QColor( 117, 128, 111 ) );
		palette.setColor( QPalette::Disabled, QPalette::WindowText, QColor( 137, 148, 131 ) );
		palette.setColor( QPalette::Disabled, QPalette::ButtonText, QColor( 137, 148, 131 ) );

		palette.setColor( QPalette::Dark, QColor( 40, 46, 34 ) );
		palette.setColor( QPalette::Light, QColor( 136, 145, 128 ) );
		palette.setColor( QPalette::Shadow, QColor( 66, 78, 58 ) );

		palette.setColor( QPalette::Base, QColor( 76, 88, 68 ) );
		palette.setColor( QPalette::Button, QColor( 76, 88, 68 ) );
		palette.setColor( QPalette::AlternateBase, QColor( 90, 106, 80 ) );
		palette.setColor( QPalette::Window, QColor( 62, 70, 55 ) );
		palette.setColor( QPalette::NoRole, Qt::darkRed );
		return palette;
	}
	if ( opt == VTF_FORGE_GALAXY )
	{
		QPixmap background( ":/Galaxy_2k.png" ); // We want to load the background once. It's a big image and SHOULD account for most screen sizes.
		auto darkerBackground = background.copy();

		//		darkerBackground.fill( qRgba( 255, 255, 255, 10 ) );
		QPainter p( &background );
		p.setCompositionMode( QPainter::CompositionMode_Overlay );
		p.fillRect( QRect { 0, 0, darkerBackground.size().width(), darkerBackground.size().height() }, QBrush( QColor( 255, 255, 255, 255 ) ) );
		//				p.end();
		QPalette palette { QApplication::palette() };

		//		palette.setColor( QPalette::Window, QColor( 49, 54, 59 ) );
		//		palette.setColor( QPalette::WindowText, Qt::white );
		//		palette.setColor( QPalette::Base, QColor( 27, 30, 32 ) );
		//		palette.setColor( QPalette::AlternateBase, QColor( 49, 54, 59 ) );
		//		palette.setColor( QPalette::ToolTipBase, Qt::black );
		//		palette.setColor( QPalette::ToolTipText, Qt::white );
		//		palette.setColor( QPalette::Text, Qt::white );
		//		palette.setColor( QPalette::Button, QColor( 49, 54, 59 ) );
		//		palette.setColor( QPalette::ButtonText, Qt::white );
		//		palette.setColor( QPalette::BrightText, Qt::red );
		//
		//		palette.setColor( QPalette::Dark, QColor( 79, 84, 89 ) );
		//		palette.setColor( QPalette::Light, QColor( 79, 84, 89 ) );
		//
		//		palette.setColor( QPalette::Link, QColor( 42, 130, 218 ) );
		//		palette.setColor( QPalette::Highlight, QColor( 42, 130, 218 ) );
		//		palette.setColor( QPalette::HighlightedText, Qt::black );
		//		palette.setColor( QPalette::Active, QPalette::Button, QColor( 49, 54, 59 ) );
		//		palette.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::darkGray );
		//		palette.setColor( QPalette::Disabled, QPalette::WindowText, Qt::darkGray );
		//		palette.setColor( QPalette::Disabled, QPalette::Text, Qt::darkGray );
		//		palette.setColor( QPalette::Disabled, QPalette::Light, QColor( 49, 54, 59 ) );

		palette.setBrush( QPalette::Window, background );
		palette.setBrush( QPalette::WindowText, Qt::white );
		palette.setBrush( QPalette::Base, background );
		palette.setBrush( QPalette::AlternateBase, background );
		palette.setBrush( QPalette::ToolTipBase, background );
		palette.setBrush( QPalette::Button, background );

		palette.setBrush( QPalette::Dark, darkerBackground );
		palette.setBrush( QPalette::Light, darkerBackground );

		palette.setBrush( QPalette::Active, QPalette::Button, background );
		palette.setBrush( QPalette::Disabled, QPalette::Light, background );
		return palette;
	}

	return QApplication::palette();
}

ApplicationOptions::ApplicationOptions( QWidget *parent ) :
	QDialog( parent )
{
	auto baseLayout = QGridLayout( this );

	auto optionsTab = new QTabWidget( this );
	// connect( optionsTab, &QTabWidget::currentChanged, optionsTab, &ApplicationOptionsTab::updateGeometry );
	//	optionsTab->setLayout( new QGridLayout() );

	auto themeWidget = new QWidget( this );

	optionsTab->addTab( themeWidget, "Themes" );

	auto optionsLayout = new QGridLayout( themeWidget );

	QFileInfo nonportable( QApplication::applicationDirPath() + "/.nonportable" );
	if ( !nonportable.exists() || !nonportable.isFile() )
	{
		auto configPath = QApplication::applicationDirPath() + "/config.ini";
		qInfo() << QApplication::applicationDirPath() + "/config.ini";
		options = std::make_unique<QSettings>( configPath, QSettings::Format::IniFormat );
	}
	else
	{
		options = std::make_unique<QSettings>();
	}

	auto themeOptions = get( OPT_THEME_SETTINGS, themeSettingsDefault );

	auto selectedPalette = getTheme( themeOptions.value( "theme" ).toVariant().value<ApplicationPaletteOptions>() );

	qInfo() << themeOptions.toVariantMap();

	QApplication::setPalette( selectedPalette );

	QMetaObject paletteTypeMeta = QPalette::staticMetaObject;
	auto paletteMetaEnum = paletteTypeMeta.enumerator( paletteTypeMeta.indexOfEnumerator( "ColorRole" ) );

	auto paletteGroupEnum = paletteTypeMeta.enumerator( paletteTypeMeta.indexOfEnumerator( "ColorGroup" ) );

	QGroupBox *themeBox = new QGroupBox( this );
	auto themeBoxLayout = new QGridLayout( themeBox );

	auto customLayoutWidget = new QWidget( this );
	QGridLayout *paletteLayout = new QGridLayout( customLayoutWidget );

	for ( int j = 0; j < paletteGroupEnum.keyCount(); j++ )
	{
		QGroupBox *paletteGroupBox = new QGroupBox( paletteGroupEnum.key( j ), this );
		QGridLayout *paletteGroupLayout = new QGridLayout( paletteGroupBox );

		for ( int k = 0, i = 0; k < paletteMetaEnum.keyCount(); k++, i++ )
		{
			auto colorLayout = new QGridLayout();

			QLabel *keyLabel = new QLabel( paletteMetaEnum.key( k ) );
			colorLayout->addWidget( keyLabel, 0, 1 );
			QPushButton *color = new QPushButton();
			auto role = static_cast<QPalette::ColorRole>( paletteMetaEnum.value( k ) );
			QPixmap square { 16, 16 };
			square.fill( selectedPalette.color( role ) );
			color->setIcon( { square } );
			color->setFixedSize( 24, 24 );
			colorLayout->addWidget( color, 0, 0 );
			if ( i == 5 )
				i = 0;
			paletteGroupLayout->addLayout( colorLayout, i + 2, std::floor( k / 5 ) );
		}
		paletteLayout->addWidget( paletteGroupBox, j, 0 );
	}
	themeBoxLayout->addWidget( customLayoutWidget );

	QApplication::setStyle( themeOptions.value( "style" ).toString() );

	optionsLayout->addWidget( themeBox, 0, 0 );

	baseLayout.addWidget( optionsTab, 0, 0 );
}
QSize ApplicationOptions::ApplicationOptionsTab::minimumSizeHint() const
{
	return this->sizeHint();
}
QSize ApplicationOptions::ApplicationOptionsTab::sizeHint() const
{
	//	return QTabWidget::sizeHint();
	auto lc = QSize( 0, 0 );
	auto rc = QSize( 0, 0 );
	auto opt = new QStyleOptionTabWidgetFrame();
	this->initStyleOption( opt );

	if ( this->cornerWidget( Qt::TopLeftCorner ) )
		lc = this->cornerWidget( Qt::TopLeftCorner )->sizeHint();
	if ( this->cornerWidget( Qt::TopRightCorner ) )
		lc = this->cornerWidget( Qt::TopRightCorner )->sizeHint();

	if ( !this->widget( 0 ) )
		return QTabWidget::sizeHint();

	auto layout = this->currentWidget()->layout();
	auto layoutHint = layout->widget()->sizeHint();
	auto tabHint = this->tabBar()->sizeHint();

	QSize size;

	if ( this->tabPosition() & North | South )
	{
		size = QSize( std::max( layoutHint.width(), tabHint.width() + rc.width() + lc.width() ), layoutHint.height() + std::max( rc.height(), std::max( lc.height(), tabHint.height() ) ) );
	}
	else
	{
		size = QSize(
			layoutHint.width() + std::max( rc.width(), std::max( lc.width(), tabHint.width() ) ),
			std::max( layoutHint.height(), tabHint.height() + rc.height() + lc.height() ) );
	}

	return size;
}
