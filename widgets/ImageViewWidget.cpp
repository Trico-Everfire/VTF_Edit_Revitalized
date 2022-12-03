
#include "ImageViewWidget.h"

#include <QBitmap>
#include <QColorSpace>
#include <QPainter>
#include <iostream>

using namespace VTFLib;

ImageViewWidget::ImageViewWidget( QWidget *pParent ) :
	QWidget( pParent )
{
	setMinimumSize( 256, 256 );
}

void ImageViewWidget::set_pixmap( const QImage &pixmap )
{
	image_ = pixmap;
}

void ImageViewWidget::set_vtf( VTFLib::CVTFFile *file )
{
	file_ = file;
	// Force refresh of data
	currentFrame_ = -1;
	currentFace_ = -1;
	currentMip_ = -1;

	zoom_ = 1.f;
	pos_ = { 0, 0 };

	// No file, sad.
	if ( !file )
		return;

	update_size();
}

void ImageViewWidget::paintEvent( QPaintEvent *event )
{
	QPainter painter( this );

	if ( !file_ )
		return;

	// Compute draw size for this mip, frame, etc
	vlUInt imageWidth, imageHeight, imageDepth;
	CVTFFile::ComputeMipmapDimensions(
		file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), mip_, imageWidth, imageHeight, imageDepth );

	// Needs decode
	if ( frame_ != currentFrame_ || mip_ != currentMip_ || face_ != currentFace_ || requestColorChange )
	{
		const bool hasAlpha = CVTFFile::GetImageFormatInfo( file_->GetFormat() ).uiAlphaBitsPerPixel > 0;
		const VTFImageFormat format = hasAlpha ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGB888;
		auto size = file_->ComputeMipmapSize( file_->GetWidth(), file_->GetHeight(), 1, mip_, format );

		if ( imgBuf_ )
		{
			free( imgBuf_ );
		}
		// This buffer needs to persist- QImage does not own the mem you give it
		imgBuf_ = static_cast<vlByte *>( malloc( size ) );

		bool ok = CVTFFile::Convert(
			file_->GetData( frame_, face_, 0, mip_ ), (vlByte *)imgBuf_, imageWidth, imageHeight, file_->GetFormat(),
			format );

		if ( !ok )
		{
			std::cerr << "Could not convert image for display.\n";
			return;
		}

		image_ = QImage(
			(uchar *)imgBuf_, imageWidth, imageHeight, hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888 );

		if ( requestColorChange )
		{
			QImage alphaChannel = image_.convertToFormat( QImage::Format_Alpha8 );
			for ( int i = 0; i < ( image_.width() ); i++ )
				for ( int j = 0; j < image_.height(); j++ )
				{
					QColor QImageColor = QColor( image_.pixel( i, j ) );
					QRgb r = red_ ? QImageColor.red() : 0;
					QRgb g = green_ ? QImageColor.green() : 0;
					QRgb b = blue_ ? QImageColor.blue() : 0;
					QRgb a = alpha_ ? qAlpha( image_.pixel( i, j ) ) : 255;

					image_.setPixelColor( i, j, QColor( r, g, b, a ) );
				}
		}

		requestColorChange = false;
		currentFace_ = face_;
		currentFrame_ = frame_;
		currentMip_ = mip_;
	}

	QPoint destpt =
		QPoint( width() / 2, height() / 2 ) - QPoint( ( imageWidth * zoom_ ) / 2, ( imageHeight * zoom_ ) / 2 ) + pos_;
	QRect target = QRect( destpt.x(), destpt.y(), image_.width() * zoom_, image_.height() * zoom_ );

	painter.drawImage( target, image_, QRect( 0, 0, image_.width(), image_.height() ) );
}

void ImageViewWidget::zoom( float amount )
{
	if ( amount == 0 )
		return; // Skip expensive repaint
	zoom_ += amount;
	if ( zoom_ < 0.1f )
		zoom_ = 0.1f;
	update_size();
}

void ImageViewWidget::update_size()
{
	if ( !file_ )
		return;

	// Resize widget to be the same size as the image
	QSize sz( file_->GetWidth() * zoom_, file_->GetHeight() * zoom_ );
	resize( sz );
}
