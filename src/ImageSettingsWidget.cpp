#include "ImageSettingsWidget.h"

#include "flagsandformats.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
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
			file_->setStartFrame( startFrame_->value() );
			if ( !settingFile_ )
				emit fileModified();
		} );
	layout->addWidget( startFrame_, row, 1 );
	layout->addWidget( new QLabel( "Start Frame:" ), row, 0 );
	++row;
	QSpinBox *frameBox = new QSpinBox( this );
	frameBox->setMinimum( -144 );
	frameBox->setValue( 24 );
	frameBox->setMaximum( 144 ); // I don't think you can even run Source even supports 144
	layout->addWidget( frameBox, row, 1 );
	layout->addWidget( new QLabel( "Animation FPS:" ), row, 0 );
	++row;

	animateButton = new QPushButton( "Animate", this );

	connect( animateButton, &QPushButton::pressed, this, [&, viewer, frameBox]
			 {
				 if ( !file_ )
					 return;

				 if ( file_->getFrameCount() <= 1 )
					 return; // Do not animate when we do not have frames to animate, lol.

				 if ( animateButton->text() == "Animate" )
				 {
					 if ( frameBox->value() == 0 )
						 return;
					 viewer->startAnimation( frameBox->value() );
					 animateButton->setText( "Stop" );
				 }
				 else
				 {
					 viewer->stopAnimating();
					 animateButton->setText( "Animate" );
				 }
			 } );

	layout->addWidget( animateButton, row, 1 );

	// Flags list box
	++row;
	auto *flagsScroll = new QScrollArea( this );
	auto *flagsGroup = new QGroupBox( tr( "Flags" ), this );
	auto *flagsLayout = new QGridLayout( flagsGroup );

	for ( auto &flag : TEXTURE_FLAGS )
	{
		auto *check = new QCheckBox( flag.name, this );
		check->setCheckable( false );
		connect(
			check, &QCheckBox::stateChanged,
			[this, flag]( int newState )
			{
				if ( !file_ )
					return;
				if ( newState )
					file_->addFlags( flag.flag );
				else
					file_->removeFlags( flag.flag );
				if ( !settingFile_ )
					emit fileModified();
			} );
		flagChecks_.insert( { flag.flag, check } );
		flagsLayout->addWidget( check );
	}

	flagsScroll->setWidget( flagsGroup );
	layout->addWidget( flagsScroll, row, 0, 1, 2 );

	// Set the flags
	for ( auto &f : TEXTURE_FLAGS )
	{
		auto check = flagChecks_.find( f.flag )->second;
		check->setDisabled( vtfpp::VTF::FLAG_MASK_GENERATED & f.flag );
	}
}

void ImageSettingsWidget::set_vtf( const VTFContainer &file )
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
		file_ = file;
		animateButton->setText( "Animate" );
		return;
	}

	file_ = file;
	startFrame_->setValue( file->getStartFrame() );
	mip_->setValue( 0 );
	frame_->setValue( file->getStartFrame() );
	face_->setValue( 0 );

	// Configure ranges
	mip_->setRange( 0, file->getMipCount() - 1 );
	frame_->setRange( 0, file->getFrameCount() - 1 );
	face_->setRange( 1, file->getFaceCount() );
	startFrame_->setRange( 1, file->getFrameCount() );

	animateButton->setText( "Animate" ); // Tab switching stops animation, this reflects that.

	// Set the flags
	vtfpp::VTF::Flags flags = file->getFlags();
	for ( auto &f : TEXTURE_FLAGS )
	{
		auto check = flagChecks_.find( f.flag )->second;
		check->setCheckable( true );
		check->setChecked( f.flag & flags );
	}

	settingFile_ = false;
}
bool ImageSettingsWidget::aquireFFPS( int &frame, int &face, int &mip, int &slice )
{
	frame = -1;
	face = -1;
	mip = -1;
	slice = -1;

	if ( !file_ )
		return false;

	frame = this->frame_->value();
	face = this->face_->value();
	mip = this->mip_->value();
	slice = 0; // TF2 Medic: "Later."
	return true;
}
