#include "InfoWidget.h"

#include "flagsandformats.hpp"
#include "fmt/format.h"
#include "util.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <vtfpp/vtfpp.h>

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
	this->spriteSheetGroupBox->setDisabled( true );
	this->enableSpritesheetDisplay->setChecked( false );
	emit spriteSheetInfoUpdated( {}, false );

	if ( !file )
		return;

	this->vtfFile = file;

	if ( vtfpp::ImageFormatDetails::large( file->getFormat() ) )
	{
		this->slider->setDisabled( false );
		this->sliderLabel->setDisabled( false );
	}

	if ( auto resource = file->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
	{
		spriteSheetGroupBox->setDisabled( false );
		auto spriteSheet = resource->getSpriteSheet();
		this->spriteSheetSequence->valueChanged( 0 );
		this->spriteSheetSequence->setValue( 0 );
		this->spriteSheetFrame->setValue( 0 );
		this->spriteSheetSequence->setMaximum( spriteSheet.getSequences().size() - 1 );
		this->spriteSheetPositions->setValue( 0 );
		this->spriteSheetPositions->setMaximum( spriteSheet.getVersion() != 0 ? 3 : 0 );
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
	this->spriteSheetGroupBox = new QGroupBox( tr( "Image Info" ), this );
	spriteSheetGroupBox->setDisabled( true );

	auto *fileGroupLayout = new QGridLayout( fileGroupBox );
	auto *imageGroupLayout = new QGridLayout( imageGroupBox );
	auto *spriteSheetLayout = new QGridLayout( spriteSheetGroupBox );
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

	this->enableSpritesheetDisplay = new QCheckBox( "Display Spritesheet.", this );
	spriteSheetLayout->addWidget( this->enableSpritesheetDisplay, 0, 0 );

	this->spriteSheetSequence = new QSpinBox( this );
	this->spriteSheetSequence->setPrefix( "Sequence: " );
	this->spriteSheetSequence->setMinimum( 0 );
	spriteSheetLayout->addWidget( this->spriteSheetSequence, 1, 0 );

	this->spriteSheetFrame = new QSpinBox( this );
	this->spriteSheetFrame->setPrefix( "Frame: " );
	this->spriteSheetFrame->setMinimum( 0 );
	spriteSheetLayout->addWidget( this->spriteSheetFrame, 2, 0 );

	this->spriteSheetPositions = new QSpinBox( this );
	this->spriteSheetPositions->setPrefix( "imagePositions: " );
	this->spriteSheetPositions->setMinimum( 0 );
	spriteSheetLayout->addWidget( this->spriteSheetPositions, 3, 0 );

	this->spriteSheetAnimate = new QPushButton( "Animate", this );
	spriteSheetLayout->addWidget( this->spriteSheetAnimate, 4, 0 );

	layout->addWidget( fileGroupBox );
	layout->addWidget( imageGroupBox );
	layout->addWidget( spriteSheetGroupBox );

	// Prevent space being added to the bottom of the file metadata group box
	layout->addStretch( 1 );

	connect( this->spriteSheetSequence, &QSpinBox::valueChanged, spriteSheetGroupBox, [&]( int i )
			 {
				 this->canTriggerInternal();
				 if ( !vtfFile )
					 return;

				 if ( auto resource = vtfFile->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
				 {
					 auto spriteSheet = resource->getSpriteSheet();
					 this->spriteSheetFrame->setMaximum( spriteSheet.getSequences()[i].getFrames().size() - 1 );
				 }
			 } );
	connect( this->enableSpritesheetDisplay, &QCheckBox::clicked, this, &InfoWidget::canTriggerInternal );
	connect( this->spriteSheetFrame, &QSpinBox::valueChanged, this, &InfoWidget::canTriggerInternal );
	connect( this->spriteSheetPositions, &QSpinBox::valueChanged, this, &InfoWidget::canTriggerInternal );
	connect( this->spriteSheetAnimate, &QPushButton::pressed, this, [&]
			 {
				 if ( this->spriteSheetAnimate->text() == "Animate" )
					 return Animate();

				 if ( this->spriteSheetAnimate->text() == "Stop" )
					 return this->spriteSheetAnimate->setText( "Animate" );
			 } );
}
void InfoWidget::canTriggerInternal()
{
	if ( !this->vtfFile )
	{
		emit spriteSheetInfoUpdated( {}, false );
		return;
	}

	if ( !this->vtfFile->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
	{
		emit spriteSheetInfoUpdated( {}, false );
		return;
	}

	if ( !this->enableSpritesheetDisplay->isChecked() )
	{
		emit spriteSheetInfoUpdated( {}, false );
		return;
	}

	auto resourceData = this->vtfFile->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA )->getSpriteSheet();
	emit spriteSheetInfoUpdated( resourceData.getSequences()[this->spriteSheetSequence->value()].getFrames()[this->spriteSheetFrame->value()].getSpriteImages()[this->spriteSheetPositions->value()], true );
}

void InfoWidget::Animate()
{
	if ( !this->vtfFile )
		return this->spriteSheetAnimate->setText( "Animate" );

	if ( !this->vtfFile->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
		return this->spriteSheetAnimate->setText( "Animate" );

	auto spriteSheet = this->vtfFile->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA )->getSpriteSheet();

	if ( spriteSheet.getSequences().size() - 1 < this->spriteSheetSequence->value() )
		return this->spriteSheetAnimate->setText( "Animate" );

	this->spriteSheetAnimate->setText( "Stop" );

	auto sequence = spriteSheet.getSequences()[this->spriteSheetSequence->value()];

	int i = 0;
	QTimer timer;
	connect( &timer, &QTimer::timeout, this, [this, sequence, &i, &timer]
			 {
				 if ( i > sequence.getFrames().size() - 1 && sequence.getLoop() )
				 {
					 i = 0;
				 }

				 if ( i > sequence.getFrames().size() - 1 || this->spriteSheetAnimate->text() == "Animate" )
				 {
					 timer.stop();
					 return this->spriteSheetAnimate->setText( "Animate" );
				 }

				 this->spriteSheetFrame->setValue( i );
				 this->canTriggerInternal();
				 i++;
				 timer.stop();
				 timer.start( sequence.getFrames()[i].duration * 60 );
			 } );

	timer.start( 1 );

	while ( timer.isActive() )
		QApplication::processEvents();
}
