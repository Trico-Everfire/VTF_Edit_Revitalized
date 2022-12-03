#include "InfoWidget.h"

#include "../common/flagsandformats.hpp"
#include "../common/util.hpp"
#include "fmt/format.h"

#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

InfoWidget::InfoWidget( QWidget *pParent ) :
	QWidget( pParent )
{
	setup_ui();
}

void InfoWidget::update_info( VTFLib::CVTFFile *file )
{
	for ( auto &pair : fields_ )
	{
		pair.second->clear();
	}

	if ( !file )
		return;

	find( "Width" )->setText( QString::number( file->GetWidth() ) );
	find( "Height" )->setText( QString::number( file->GetHeight() ) );
	find( "Depth" )->setText( QString::number( file->GetDepth() ) );
	find( "Frames" )->setText( QString::number( file->GetFrameCount() ) );
	find( "Faces" )->setText( QString::number( file->GetFaceCount() ) );
	find( "Mips" )->setText( QString::number( file->GetMipmapCount() ) );

	find( "Version" )->setText( QString::number( file->GetMajorVersion() ) + "." + QString::number( file->GetMinorVersion() ) );
	auto clevel = find( "Compression Level" );
	if ( file->GetMinorVersion() >= 6 )
	{
		clevel->setDisabled( false );
		clevel->setText( QString( std::to_string( file->GetAuxCompressionLevel() ).c_str() ) );
	}
	else
	{
		clevel->setText( "Not Supported" );
		clevel->setDisabled( true );
	}

	auto size = file->GetSize();
	find( "Size" )->setText(
		fmt::format( FMT_STRING( "{:.2f} MiB ({:.2f} KiB)" ), size / ( 1024.f * 1024.f ), size / 1024.f ).c_str() );

	vlSingle x, y, z;
	file->GetReflectivity( x, y, z );
	find( "Reflectivity" )->setText( fmt::format( FMT_STRING( "{:.3f} {:.3f} {:.3f}" ), x, y, z ).c_str() );

	// Select the correct image format
	for ( int i = 0; i < util::ArraySize( IMAGE_FORMATS ); ++i )
	{
		if ( IMAGE_FORMATS[i].format == file->GetFormat() )
		{
			formatCombo_->setCurrentIndex( i );
			break;
		}
	}
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

	layout->addWidget( fileGroupBox );
	layout->addWidget( imageGroupBox );

	// Prevent space being added to the bottom of the file metadata group box
	layout->addStretch( 1 );
}