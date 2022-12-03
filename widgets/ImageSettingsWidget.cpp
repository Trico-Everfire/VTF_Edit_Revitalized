#include "ImageSettingsWidget.h"

#include "../common/flagsandformats.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>

ImageSettingsWidget::ImageSettingsWidget( ImageViewWidget *viewer, QWidget *parent ) :
	QWidget( parent )
{
	setup_ui( viewer );
}

void ImageSettingsWidget::setup_ui( ImageViewWidget *viewer )
{
	auto *layout = new QGridLayout( this );

	int row = 0;
	frame_ = new QSpinBox( this );
	connect(
		frame_, &QSpinBox::textChanged,
		[viewer, this]( const QString & )
		{
			viewer->set_frame( frame_->value() );
		} );
	layout->addWidget( frame_, row, 1 );
	layout->addWidget( new QLabel( "Frame:" ), row, 0 );

	++row;
	mip_ = new QSpinBox( this );
	connect(
		mip_, &QSpinBox::textChanged,
		[viewer, this]( const QString & )
		{
			viewer->set_mip( mip_->value() );
		} );
	layout->addWidget( mip_, row, 1 );
	layout->addWidget( new QLabel( "Mip:" ), row, 0 );

	++row;
	face_ = new QSpinBox( this );
	connect(
		face_, &QSpinBox::textChanged,
		[viewer, this]( const QString & )
		{
			viewer->set_face( face_->value() );
		} );
	layout->addWidget( face_, row, 1 );
	layout->addWidget( new QLabel( "Face:" ), row, 0 );

	++row;
	startFrame_ = new QSpinBox( this );
	connect(
		startFrame_, &QSpinBox::textChanged,
		[viewer, this]( const QString & )
		{
			if ( !file_ )
				return;
			file_->SetStartFrame( startFrame_->value() );
			if ( !settingFile_ )
				emit fileModified();
		} );
	layout->addWidget( startFrame_, row, 1 );
	layout->addWidget( new QLabel( "Start Frame:" ), row, 0 );

	// Flags list box
	++row;
	auto *flagsScroll = new QScrollArea( this );
	auto *flagsGroup = new QGroupBox( tr( "Flags" ), this );
	auto *flagsLayout = new QGridLayout( flagsGroup );

	for ( auto &flag : TEXTURE_FLAGS )
	{
		auto *check = new QCheckBox( flag.name, this );
		check->setCheckable( true );
		connect(
			check, &QCheckBox::stateChanged,
			[this, flag]( int newState )
			{
				if ( !file_ )
					return;
				file_->SetFlag( (VTFImageFlag)flag.flag, newState );
				if ( !settingFile_ )
					emit fileModified();
			} );
		flagChecks_.insert( { flag.flag, check } );
		flagsLayout->addWidget( check );
	}

	flagsScroll->setWidget( flagsGroup );
	layout->addWidget( flagsScroll, row, 0, 1, 2 );
}

void ImageSettingsWidget::set_vtf( VTFLib::CVTFFile *file )
{
	// Hack to ensure we don't emit fileModified when setting defaults
	settingFile_ = true;

	// Set some sensible defaults in the event no file is loaded
	if ( !file )
	{
		startFrame_->setValue( 0 );
		startFrame_->setRange( 0, 0 );
		mip_->setValue( 0 );
		mip_->setRange( 0, 0 );
		face_->setValue( 0 );
		face_->setRange( 0, 0 );
		frame_->setValue( 0 );
		frame_->setRange( 0, 0 );
		for ( auto &check : flagChecks_ )
		{
			check.second->setChecked( false );
			check.second->setCheckable( false );
		}
		settingFile_ = false;
		return;
	}

	file_ = file;
	startFrame_->setValue( file->GetStartFrame() );
	mip_->setValue( 0 );
	frame_->setValue( file->GetStartFrame() );
	face_->setValue( 0 );

	// Configure ranges
	mip_->setRange( 0, file->GetMipmapCount() );
	frame_->setRange( 1, file->GetFrameCount() );
	face_->setRange( 1, file->GetFaceCount() );
	startFrame_->setRange( 1, file->GetFrameCount() );

	// Set the flags
	uint32_t flags = file->GetFlags();
	for ( auto &f : TEXTURE_FLAGS )
	{
		auto check = flagChecks_.find( f.flag )->second;
		check->setChecked( !!( flags & f.flag ) );
		check->setCheckable( true );
	}

	settingFile_ = false;
}
