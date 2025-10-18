
#include "ImageViewWidget.h"

#include <QColorSpace>
#include <QOpenGLPixelTransferOptions>
#include <QPainter>
#include <QStyleOption>
#include <QWheelEvent>
#include <iostream>

#define remap( value, low1, high1, low2, high2 ) ( low2 + ( value - low1 ) * ( high2 - low2 ) / ( high1 - low1 ) )

void ImageViewWidget::Animate()
{
	if ( !file_ )
		return;

	if ( !animateReverse_ )
	{
		if ( frame_ < file_->getFrameCount() - 1 )
			frame_++;
		else
			frame_ = 0;
	}
	else
	{
		if ( frame_ > 1 )
			frame_--;
		else
			frame_ = file_->getFrameCount() - 1;
	}
	this->update();
	emit animated( frame_ );
}

ImageViewWidget::ImageViewWidget( QWidget *pParent ) :
	QOpenGLWidget( pParent ), QOpenGLFunctions_4_5_Core()

{
	setFocusPolicy( Qt::StrongFocus );
	this->background = QImage( ":/VTF_Forge_small_grayscale.png" );

	QPixmap transparent( this->background.size() );
	transparent.fill( Qt::transparent );
	QPainter p;
	p.begin( &transparent );
	p.setCompositionMode( QPainter::CompositionMode_Source );
	p.drawPixmap( 0, 0, QPixmap::fromImage( this->background ) );
	p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
	p.fillRect( transparent.rect(), QColor( 0, 0, 0, 40 ) );
	p.end();
	this->background = transparent.toImage();
}

void ImageViewWidget::startAnimation( int fps )
{
	animateReverse_ = fps < 1;
	animationTimer_ = startTimer( 1000 / std::abs( fps ) );
}

void ImageViewWidget::stopAnimating()
{
	if ( animationTimer_ > 0 )
	{
		killTimer( animationTimer_ );
		animationTimer_ = -1;
	}
}

void ImageViewWidget::set_vtf( vtfpp::VTF *file )
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
	{
		zoom_ = 1.6f;
		return;
	}

	update_size();
}

void ImageViewWidget::initializeGL()
{
	initializeOpenGLFunctions();

	shaderProgram = new QOpenGLShaderProgram( this );

	shaderProgram->addShaderFromSourceFile( QOpenGLShader::Vertex, ":/vertex.glsl" );
	shaderProgram->addShaderFromSourceFile( QOpenGLShader::Fragment, ":/fragment.glsl" );

	shaderProgram->link();
	shaderProgram->bind();

	this->vertices.create();
	this->vertices.bind();
	this->vertices.setUsagePattern( QOpenGLBuffer::StaticDraw );
	this->vertices.allocate( texCoords, sizeof( texCoords ) );
	this->vertices.release();

	this->indexes.create();
	this->indexes.bind();
	this->indexes.allocate( texIndeces, sizeof( texIndeces ) );
	this->indexes.release();
}

void ImageViewWidget::resizeGL( int w, int h )
{
	glViewport( 0, 0, w, h );
}

void ImageViewWidget::paintGL()
{
	// Draw the scene:
	QStyleOption opt;
	opt.initFrom( this );
	auto clearColor = opt.palette.color( QPalette::ColorRole::Window );
	// Todo: check for transparent/translucent window flag and apply clearColor Alpha then.
	this->glClearColor( clearColor.redF(), clearColor.greenF(), clearColor.blueF(), 1.0f );
	this->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	this->glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	shaderProgram->bind();

	float aspect = (float)this->width() / (float)this->height();
	QVector4D sheetData;
	if ( !hasSpriteSheetLocation_ )
		sheetData = QVector4D( 0.f, 1.f, 1.f, 0.f );

	else
		sheetData = QVector4D( this->spriteSheet_.x1, this->spriteSheet_.y1, this->spriteSheet_.x2, this->spriteSheet_.y2 );

	GLfloat texCoords[] = {
		// positions          // colors           // texture coords
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, sheetData.z(), sheetData.y(),	// top right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, sheetData.z(), sheetData.w(),	// bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, sheetData.x(), sheetData.w(), // bottom left
		-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, sheetData.x(), sheetData.y()	// top left
	};

	this->vertices.create();
	this->vertices.bind();
	this->vertices.setUsagePattern( QOpenGLBuffer::StaticDraw );
	this->vertices.allocate( texCoords, sizeof( texCoords ) );
	this->vertices.release();

	QMatrix4x4 projectionMatrix = {
		1.f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.f };

	int TexProcessing = shaderProgram->uniformLocation( "TexMat" ); // glGetUniformLocation( shaderProgram, "RGBA" );

	shaderProgram->setUniformValue( TexProcessing, sheetData );

	// TODO: figure this out properly, this feels terrible.
	int startWidht = this->width();
	int startHeight = this->height();

	float scalarX = (float)startWidht / this->width();
	float scalarY = (float)startHeight / this->height();

	float zoomFactor = 1 / zoom_;
	if ( zoomFactor > 10 )
		zoomFactor = 10;

	if ( zoomFactor < 0.00000001 )
		zoomFactor = 0.00000001;

	float xSpan = zoomFactor;
	float ySpan = zoomFactor;

	if ( aspect > 1 )
	{
		xSpan *= aspect;
	}
	else
	{
		ySpan = xSpan / aspect;
	}

	projectionMatrix.ortho( -1 * xSpan, xSpan, -1 * ySpan, ySpan, -0, 1 );

	int RGBAProcessing = shaderProgram->uniformLocation( "RGBA" ); // glGetUniformLocation( shaderProgram, "RGBA" );

	shaderProgram->setUniformValue( RGBAProcessing, rgba_ );

	int GammaLocation = shaderProgram->uniformLocation( "gamma" ); // glGetUniformLocation( shaderProgram, "RGBA" );

	float offs = ( 0.5f / aspect );
	QVector2D offsets = { remap( xOffset_, 0, 4096, offs * ( zoom_ + ( 1 - scalarX ) ), -offs * ( zoom_ + ( 1 - scalarX ) ) ), remap( yOffset_, 0, 4096, -offs * ( zoom_ + ( 1 - scalarY ) ), offs * ( zoom_ + ( 1 - scalarY ) ) ) };

	int OFFSETProcessing = shaderProgram->uniformLocation( "OFFSET" ); // glGetUniformLocation( shaderProgram, "RGBA" );

	shaderProgram->setUniformValue( OFFSETProcessing, offsets );

	indexes.bind();
	vertices.bind();

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 8, 0 );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)( 3 * sizeof( float ) ) );

	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)( 6 * sizeof( float ) ) );
	if ( file_ )
	{
		GLuint width, height;
		width = vtfpp::ImageDimensions::getMipDim( mip_, file_->getWidth() );
		height = vtfpp::ImageDimensions::getMipDim( mip_, file_->getHeight() );
		//		if ( vtfpp::ImageFormatDetails::compressed( file_->getFormat() ) )
		//		{
		//			auto dat = file_->getImageDataRaw( mip_, frame_, face_ - 1, 0 );
		//			texture.create();
		//			texture.setFormat( mapVTFToGLFormat( file_->getFormat() ) );
		//			texture.setSize( width, height, 1 );
		//			texture.allocateStorage();
		//			texture.setCompressedData( dat.size(), dat.data() );
		//			// texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::Float32, dat.data() );
		//			shaderProgram->setUniformValue( GammaLocation, (float)getHDRGamma() / 100 );
		//		}
		//		else if ( this->hasSpriteSheetLocation_ && file_->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
		//		{
		//			projectionMatrix.setColumn( 1, { 0, -projectionMatrix.column( 1 )[1], 0, 0 } );
		//
		//			auto dat = file_->getImageDataAsRGBA8888( mip_, frame_, face_ - 1, 0 );
		//			texture.setMinMagFilters( QOpenGLTexture::Linear, QOpenGLTexture::Linear );
		//			texture.create();
		//			texture.setSize( width, height, 1 );
		//			texture.setFormat( QOpenGLTexture::RGBA8_UNorm );
		//			texture.allocateStorage();
		//			texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, dat.data() );
		//			shaderProgram->setUniformValue( GammaLocation, -1.0f );
		//		}
		//		else
		//			if ( vtfpp::ImageFormatDetails::large( file_->getFormat() ) )
		//		{
		//			float vtfAspect = (float)width / (float)height;
		//			projectionMatrix.setColumn( 0, { projectionMatrix.column( 0 )[0] * vtfAspect, 0, 0, 0 } );
		//
		//			auto dat = file_->getImageDataAs( vtfpp::ImageFormat::RGBA32323232F, mip_, frame_, face_ - 1, 0 );
		//			texture.create();
		//
		//			texture.setSize( width, height, 1 );
		//			texture.setFormat( QOpenGLTexture::RGBA32F );
		//			texture.allocateStorage();
		//			texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::Float32, dat.data() );
		//			shaderProgram->setUniformValue( GammaLocation, (float)getHDRGamma() / 100 );
		//		}
		//		else

		auto fmt = mapVTFToGLFormat( file_->getFormat() );

		std::vector<std::byte> dat;

		texture.create();
		texture.setFormat( fmt );
		texture.setSize( width, height, 1 );
		texture.allocateStorage();

		if ( this->hasSpriteSheetLocation_ && file_->getResource( vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA ) )
		{
			texture.setMinMagFilters( QOpenGLTexture::Linear, QOpenGLTexture::Linear );
			projectionMatrix.setColumn( 1, { 0, -projectionMatrix.column( 1 )[1], 0, 0 } );
		}
		else
		{
			float vtfAspect = (float)width / (float)height;
			projectionMatrix.setColumn( 0, { projectionMatrix.column( 0 )[0] * vtfAspect, 0, 0, 0 } );
		}

		if ( vtfpp::ImageFormatDetails::large( file_->getFormat() ) )
			shaderProgram->setUniformValue( GammaLocation, (float)getHDRGamma() / 100 );
		else
			shaderProgram->setUniformValue( GammaLocation, -1.0f );

		switch ( fmt )
		{
			case QOpenGLTexture::RGBA8_UNorm:
				dat = file_->getImageDataAsRGBA8888( mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, dat.data() );
				break;

			case QOpenGLTexture::RGB8_UNorm:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGB888, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGB, QOpenGLTexture::UInt8, dat.data() );
				break;

			case QOpenGLTexture::R5G6B5:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGB565, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGB, QOpenGLTexture::UInt16_R5G6B5, dat.data() );
				break;

			case QOpenGLTexture::R8_UNorm:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::R8, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::Red, QOpenGLTexture::UInt8, dat.data() );
				break;

			case QOpenGLTexture::RGB5A1:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::BGRA5551, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::BGRA, QOpenGLTexture::UInt16_RGB5A1, dat.data() );
				break;

			case QOpenGLTexture::RG8I:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::UV88, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RG, QOpenGLTexture::UInt8, dat.data() );
				break;

			case QOpenGLTexture::RGBA16I:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGBA16161616, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, dat.data() );
				break;

			case QOpenGLTexture::RGB10A2:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGBA1010102, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGB, QOpenGLTexture::UInt32_RGB10A2, dat.data() );
				break;

			case QOpenGLTexture::RGBA4:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::BGRA4444, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::BGRA, QOpenGLTexture::UInt16_RGBA4, dat.data() );
				break;

			case QOpenGLTexture::RGBA16F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGBA16161616F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::Float16, dat.data() );
				break;

			case QOpenGLTexture::R32F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::R32F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::Red, QOpenGLTexture::Float32, dat.data() );
				break;
			case QOpenGLTexture::RGB32F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGB323232F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGB, QOpenGLTexture::Float32, dat.data() );
				break;

			case QOpenGLTexture::RGBA32F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RGBA32323232F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::Float32, dat.data() );
				break;

			case QOpenGLTexture::RG16F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RG1616F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RG, QOpenGLTexture::Float16, dat.data() );
				break;

			case QOpenGLTexture::RG32F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::RG3232F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RG, QOpenGLTexture::Float32, dat.data() );
				break;

			case QOpenGLTexture::R16F:
				dat = file_->getImageDataAs( vtfpp::ImageFormat::R16F, mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::Red, QOpenGLTexture::Float16, dat.data() );
				break;

			default:
			{
				if ( vtfpp::ImageFormatDetails::compressed( file_->getFormat() ) )
				{
					auto rawDat = file_->getImageDataRaw( mip_, frame_, face_ - 1, 0 );
					texture.setCompressedData( rawDat.size(), rawDat.data() );
					break;
				}
				// Well, we tried.
				dat = file_->getImageDataAsRGBA8888( mip_, frame_, face_ - 1, 0 );
				texture.setData( QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, dat.data() );
				break;
			}
		}
		texture.bind( 0 );
	}
	else
	{
		float vtfAspect = (float)this->background.width() / (float)this->background.height();
		projectionMatrix.setColumn( 0, { projectionMatrix.column( 0 )[0] * vtfAspect, 0, 0, 0 } );

		shaderProgram->setUniformValue( GammaLocation, -1.0f );

		texture.create();
		texture.setData( this->background );
		texture.bind( 0 );
	}

	int AspectRatioLocation = shaderProgram->uniformLocation( "ProjMat" ); // glGetUniformLocation( shaderProgram, "RGBA" );
	shaderProgram->setUniformValue( AspectRatioLocation, projectionMatrix );

	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr );
	indexes.release();
	vertices.release();
	texture.release( 0 );
	texture.destroy();
	shaderProgram->release();
}

void ImageViewWidget::wheelEvent( QWheelEvent *event )
{
	if ( event->angleDelta().y() > 0 ) // up Wheel
	{
		if ( m_isCTRLHeld )
		{
			zoom( 0.1 );
			event->ignore();
			return;
		}
	}
	else if ( event->angleDelta().y() < 0 ) // down Wheel
	{
		if ( m_isCTRLHeld )
		{
			zoom( -0.1 );
			event->ignore();
			return;
		}
	}
	QOpenGLWidget::wheelEvent( event );
}

bool ImageViewWidget::event( QEvent *event )
{
	if ( event->type() == QEvent::KeyPress )
	{
		auto ke = static_cast<QKeyEvent *>( event );
		if ( ( ke->key() == Qt::Key_Control ) )
			m_isCTRLHeld = true;
	}

	if ( event->type() == QEvent::KeyRelease )
	{
		auto ke = static_cast<QKeyEvent *>( event );
		if ( ( ke->key() == Qt::Key_Control ) )
			m_isCTRLHeld = false;
	}

	// When we lose focus, we can no longer check for key events, so to prevent
	// weird behaviour upon defocus, we set m_isCTRLHeld to false.
	if ( event->type() == QEvent::FocusOut )
	{
		auto fe = static_cast<QFocusEvent *>( event );
		if ( fe->lostFocus() )
			m_isCTRLHeld = false;
	}

	return QOpenGLWidget::event( event );
}

void ImageViewWidget::zoom( float amount )
{
	if ( amount == 0 || !file_ )
		return; // Skip expensive update

	zoom_ += amount;
	if ( zoom_ < 0.01f )
		zoom_ = 0.01f;

	emit zoomChanged( zoom_ );

	update_size();
}

void ImageViewWidget::update_size()
{
	if ( !file_ )
		return;

	this->update();
}
void ImageViewWidget::timerEvent( QTimerEvent *event )
{
	Animate();
	QObject::timerEvent( event );
}
float ImageViewWidget::getZoom() const
{
	return zoom_;
}
void ImageViewWidget::mousePressEvent( QMouseEvent *event )
{
	if ( event->button() == Qt::RightButton )
	{
		emit onRightClick();
	}

	QWidget::mousePressEvent( event );
}
