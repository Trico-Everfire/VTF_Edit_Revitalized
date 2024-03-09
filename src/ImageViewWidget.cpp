
#include "ImageViewWidget.h"

#include <QColorSpace>
#include <QPainter>
#include <QStyleOption>
#include <iostream>
#include <qfile.h>
using namespace VTFLib;

void ImageViewWidget::Animate()
{
	if ( !file_ )
		return;

	if ( frame_ < file_->GetFrameCount() )
		frame_++;
	else
		frame_ = 0;
	this->update();
}

ImageViewWidget::ImageViewWidget( QWidget *pParent ) :
	QOpenGLWidget( pParent ), QOpenGLFunctions_4_5_Core()

{
	setMinimumSize( { 256, 256 } );
}

void ImageViewWidget::startAnimation( int fps )
{
	animationTimer_ = startTimer( 1000 / fps );
}

void ImageViewWidget::stopAnimating()
{
	if ( animationTimer_ > 0 )
	{
		killTimer( animationTimer_ );
		animationTimer_ = -1;
	}
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

	setMinimumSize( { static_cast<int>( file->GetWidth() ), static_cast<int>( file->GetHeight() ) } );

	update_size();
}

void ImageViewWidget::initializeGL()
{
	// Set up the rendering context, load shaders and other resources, etc.:
	initializeOpenGLFunctions();

	QStyleOption opt;
	opt.initFrom( this );

	auto clearColor = opt.palette.color( QPalette::ColorRole::Text );
	glClearColor( clearColor.redF(), clearColor.greenF(), clearColor.blueF(), 1.0f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	//	glEnable( GL_DEPTH_TEST );
	//	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	//	glEnable( GL_COLOR_MATERIAL );

	unsigned int buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof( texCoords ), texCoords, GL_STATIC_DRAW );

	unsigned int ibo;
	glGenBuffers( 1, &ibo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( texIndeces ), texIndeces, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 8, 0 );

	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)( 6 * sizeof( float ) ) );
	glEnableVertexAttribArray( 2 );

	unsigned int vertexShader;
	vertexShader = glCreateShader( GL_VERTEX_SHADER );

	auto vshad = QFile( ":/vertex.glsl" );
	vshad.open( QFile::ReadOnly );
	const char *vShadSource = vshad.readAll().constData();

	glShaderSource( vertexShader, 1, &vShadSource, NULL );
	glCompileShader( vertexShader );

	vshad.close();

	int success;
	char infoLog[512];
	glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &success );

	if ( !success )
	{
		glGetShaderInfoLog( vertexShader, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

	auto fshad = QFile( ":/fragment.glsl" );
	fshad.open( QFile::ReadOnly );
	const char *fShadSource = fshad.readAll().constData();

	glShaderSource( fragmentShader, 1, &fShadSource, NULL );
	glCompileShader( fragmentShader );

	fshad.close();

	glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &success );

	if ( !success )
	{
		glGetShaderInfoLog( fragmentShader, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();

	glAttachShader( shaderProgram, vertexShader );
	glAttachShader( shaderProgram, fragmentShader );
	glLinkProgram( shaderProgram );

	glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );
	if ( !success )
	{
		glGetProgramInfoLog( shaderProgram, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::SHADERPROGRAM::INITIATION FAILED\n"
				  << infoLog << std::endl;
	}

	glUseProgram( shaderProgram );

	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );

	glGenTextures( 1, &texture );
}

void ImageViewWidget::resizeGL( int w, int h )
{
	// Update projection matrix and other size related settings:
	glViewport( 0, 0, w, h );
}

void ImageViewWidget::paintGL()
{
	// Draw the scene:
	glClear( GL_COLOR_BUFFER_BIT );

	int RGBAProcessing = glGetUniformLocation( shaderProgram, "RGBA" );

	glUseProgram( shaderProgram );

	glUniform1i( RGBAProcessing, rgba_ );

	if ( file_ )
	{
		GLuint width, height, whatever;
		CVTFFile::ComputeMipmapDimensions( file_->GetWidth(), file_->GetHeight(), 1, mip_, width, height, whatever );
		auto size = CVTFFile::ComputeImageSize( width, width, whatever, IMAGE_FORMAT_RGBA8888 );
		auto imgData = new vlByte[size];
		CVTFFile::ConvertToRGBA8888( file_->GetData( frame_, face_, 0, mip_ ), reinterpret_cast<vlByte *>( imgData ), width, height, file_->GetFormat() );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData );
		glGenerateMipmap( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE0 );

		glBindTexture( GL_TEXTURE_2D, texture );

		resizeGL( width * zoom_, height * zoom_ );
		setMinimumSize( { static_cast<int>( width * zoom_ ), static_cast<int>( height * zoom_ ) } );
		resize( { static_cast<int>( width * zoom_ ), static_cast<int>( height * zoom_ ) } );

		delete[] imgData;
	}
	else
	{
		static constexpr unsigned char buff[4] = { 0, 0, 0, 255 };
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff );
		glGenerateMipmap( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture );
		setMinimumSize( { 256, 256 } );
		resize( { 256, 256 } );
	}

	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr );
}

// void ImageViewWidget::paintEvent( QPaintEvent *event )
//{
//	QPainter painter( this );
//
//	if ( !file_ )
//		return;
//
//	// Compute draw size for this mip, frame, etc
//	vlUInt imageWidth, imageHeight, imageDepth;
//	CVTFFile::ComputeMipmapDimensions(
//		file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), mip_, imageWidth, imageHeight, imageDepth );
//
//	// Needs decode
//	if ( frame_ != currentFrame_ || mip_ != currentMip_ || face_ != currentFace_ || requestColorChange )
//	{
//		const bool hasAlpha = CVTFFile::GetImageFormatInfo( file_->GetFormat() ).uiAlphaBitsPerPixel > 0;
//		const VTFImageFormat format = hasAlpha ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGB888;
//		auto size = file_->ComputeMipmapSize( file_->GetWidth(), file_->GetHeight(), 1, mip_, format );
//
//		if ( imgBuf_ )
//		{
//			free( imgBuf_ );
//		}
//		// This buffer needs to persist- QImage does not own the mem you give it
//		imgBuf_ = static_cast<vlByte *>( malloc( size ) );
//
//		bool ok = CVTFFile::Convert(
//			file_->GetData( frame_, face_, 0, mip_ ), (vlByte *)imgBuf_, imageWidth, imageHeight, file_->GetFormat(),
//			format );
//
//		if ( !ok )
//		{
//			std::cerr << "Could not convert image for display.\n";
//			return;
//		}
//
//		image_ = QImage(
//			(uchar *)imgBuf_, imageWidth, imageHeight, hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888 );
//
//		if ( requestColorChange )
//		{
//			//			for ( int y = 0; y < image_.height(); ++y )
//			//			{
//			//				QRgb *line = reinterpret_cast<QRgb *>( image_.scanLine( y ) );
//			//				for ( int x = 0; x < image_.width(); ++x )
//			//				{
//			//					QRgb &rgb = line[x];
//			//					rgb = qRgba( qRed( red_ ? rgb : 0 ), qGreen( green_ ? rgb : 0 ), qBlue( blue_ ? rgb : 0 ), qAlpha( alpha_ ? rgb : 255 ) );
//			//				}
//			//			}
//
//			for ( int i = 0; i < ( image_.width() ); i++ )
//				for ( int j = 0; j < image_.height(); j++ )
//				{
//					QColor QImageColor = QColor( image_.pixel( i, j ) );
//					QRgb r = red_ ? QImageColor.red() : 0;
//					QRgb g = green_ ? QImageColor.green() : 0;
//					QRgb b = blue_ ? QImageColor.blue() : 0;
//					QRgb a = alpha_ ? qAlpha( image_.pixel( i, j ) ) : 255;
//
//					image_.setPixelColor( i, j, QColor( r, g, b, a ) );
//				}
//		}
//
//		requestColorChange = false;
//		currentFace_ = face_;
//		currentFrame_ = frame_;
//		currentMip_ = mip_;
//	}
//
//	QPoint destpt =
//		QPoint( width() / 2, height() / 2 ) - QPoint( ( imageWidth * zoom_ ) / 2, ( imageHeight * zoom_ ) / 2 ) + pos_;
//	QRect target = QRect( destpt.x(), destpt.y(), image_.width() * zoom_, image_.height() * zoom_ );
//
//	painter.drawImage( target, image_, QRect( 0, 0, image_.width(), image_.height() ) );
// }

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
	this->update();
	setMinimumSize( { static_cast<int>( file_->GetWidth() * zoom_ ), static_cast<int>( file_->GetHeight() * zoom_ ) } );
}
void ImageViewWidget::timerEvent( QTimerEvent *event )
{
	Animate();
	QObject::timerEvent( event );
}
