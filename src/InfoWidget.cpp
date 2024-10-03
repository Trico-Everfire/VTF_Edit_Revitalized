#include "InfoWidget.h"

#include "flagsandformats.hpp"
#include "fmt/format.h"
#include "util.hpp"

#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

InfoWidget::InfoWidget( QWidget *pParent ) :
	QWidget( pParent )
{
	setup_ui();
}

void InfoWidget::update_info( vtfpp::VTF *file )
{
	for ( auto &pair : fields_ )
	{
		pair.second->clear();
	}

	this->slider->setDisabled( true );
	this->sliderLabel->setDisabled( true );

	if ( !file )
		return;

	if ( vtfpp::ImageFormatDetails::large( file->getFormat() ) )
	{
		this->slider->setDisabled( false );
		this->sliderLabel->setDisabled( false );
	}
	find( "Width" )->setText( QString::number( file->getWidth() ) );
	find( "Height" )->setText( QString::number( file->getHeight() ) );
	find( "Depth" )->setText( QString::number( file->getSliceCount() ) );
	find( "Frames" )->setText( QString::number( file->getFrameCount() ) );
	find( "Faces" )->setText( QString::number( file->getFaceCount() ) );
	find( "Mips" )->setText( QString::number( file->getMipCount() ) );

	find( "Version" )->setText( QString::number( file->getMajorVersion() ) + "." + QString::number( file->getMinorVersion() ) );
	auto clevel = find( "Compression Level" );
	if ( file->getMinorVersion() >= 6 )
	{
		clevel->setDisabled( false );
		clevel->setText( QString( std::to_string( file->getCompressionLevel() ).c_str() ) );
	}
	else
	{
		clevel->setText( "Not Supported" );
		clevel->setDisabled( true );
	}

	auto size = file->bake().size();
	find( "Size" )->setText(
		fmt::format( FMT_STRING( "{:.2f} MiB ({:.2f} KiB)" ), size / ( 1024.f * 1024.f ), size / 1024.f ).c_str() );

	float x, y, z;
	auto flt = file->getReflectivity();

	find( "Reflectivity" )->setText( fmt::format( FMT_STRING( "{:.3f} {:.3f} {:.3f}" ), flt[0], flt[1], flt[2] ).c_str() );

	// Select the correct image format
	for ( int i = 0; i < util::ArraySize( IMAGE_FORMATS ); ++i )
	{
		if ( IMAGE_FORMATS[i].format == file->getFormat() )
		{
			formatCombo_->setCurrentIndex( i );
			break;
		}
	}

	this->slider->setValue( 220 );
}

void InfoWidget::setup_ui()
{
	auto *layout = new QVBoxLayout( this );
	auto *fileGroupBox = new QGroupBox( tr( "File Metadata" ), this );
	auto *imageGroupBox = new QGroupBox( tr( "Image Info" ), this );

	auto *fileGroupLayout = new QGridLayout( fileGroupBox );
	auto *imageGroupLayout = new QGridLayout( imageGroupBox );
	fileGroupLayout->setColumnStretch( 1, 1 );
	imageGroupLayout->setColumnStretch( 1, 1 );

	// Prevent rows from expanding on resize
	fileGroupLayout->setRowStretch( util::ArraySize( FILE_FIELDS ), 1 );
	imageGroupLayout->setRowStretch( util::ArraySize( INFO_FIELDS ), 1 );

	// File meta info
	int row = 0;
	for ( auto &f : FILE_FIELDS )
	{
		auto *label = new QLabel( QString( f ) + ":", fileGroupBox );
		auto *edit = new QLineEdit( this );
		edit->setReadOnly( true );

		fileGroupLayout->addWidget( label, row, 0 );
		fileGroupLayout->addWidget( edit, row, 1 );
		++row;

		fields_.insert( { f, edit } );
	}

	// Image contents info group box below here
	row = 0;

	// Image format dropdown box
	formatCombo_ = new QComboBox( this );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		formatCombo_->addItem( fmt.name, (int)fmt.format );
	}
	formatCombo_->setDisabled( true );
	imageGroupLayout->addWidget( new QLabel( "Image format:", this ), row, 0 );
	imageGroupLayout->addWidget( formatCombo_, row, 1 );
	++row;

	for ( auto &f : INFO_FIELDS )
	{
		auto *label = new QLabel( QString( f ) + ":", imageGroupBox );
		auto *edit = new QLineEdit( this );
		edit->setReadOnly( true );

		imageGroupLayout->addWidget( label, row, 0 );
		imageGroupLayout->addWidget( edit, row, 1 );
		++row;

		fields_.insert( { f, edit } );
	}

	this->sliderLabel = new QLabel( "HDR Slider:", imageGroupBox );
	this->slider = new QSlider( Qt::Horizontal );
	slider->setMinimum( 0 );
	slider->setMaximum( 1000 );
	slider->setValue( 220 );
	imageGroupLayout->addWidget( sliderLabel, row, 0 );
	imageGroupLayout->addWidget( slider, row, 1 );
	++row;

	this->slider->setDisabled( true );
	this->sliderLabel->setDisabled( true );

	layout->addWidget( fileGroupBox );
	layout->addWidget( imageGroupBox );

	// Prevent space being added to the bottom of the file metadata group box
	layout->addStretch( 1 );
}