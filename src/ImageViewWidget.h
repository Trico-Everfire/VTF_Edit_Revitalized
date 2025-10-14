#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QWidget>
#include <vtfpp/vtfpp.h>

enum ColorSelection
{
	ALL,
	RGB,
	RED,
	GREEN,
	BLUE,
	ALPHA
};

inline QOpenGLTexture::PixelFormat mapVTFToPixelFormat( vtfpp::ImageFormat format )
{
	switch ( format )
	{
		case vtfpp::ImageFormat::RGBA8888:
			return QOpenGLTexture::RGBA;
		case vtfpp::ImageFormat::ABGR8888:
			return QOpenGLTexture::BGRA;
		case vtfpp::ImageFormat::RGB888:
			return QOpenGLTexture::RGB;
		case vtfpp::ImageFormat::BGR888:
			return QOpenGLTexture::BGR;
		case vtfpp::ImageFormat::RGB565:
			return QOpenGLTexture::RGB;
		case vtfpp::ImageFormat::I8:
			return QOpenGLTexture::PixelFormat::Red_Integer;
		case vtfpp::ImageFormat::IA88:
			return QOpenGLTexture::RG;
		case vtfpp::ImageFormat::P8:
			return QOpenGLTexture::PixelFormat::Red_Integer;
		case vtfpp::ImageFormat::A8:
			return QOpenGLTexture::PixelFormat::Red_Integer;
		case vtfpp::ImageFormat::RGB888_BLUESCREEN:
			return QOpenGLTexture::PixelFormat::RGB;
		case vtfpp::ImageFormat::BGR888_BLUESCREEN:
			return QOpenGLTexture::PixelFormat::BGR;
		case vtfpp::ImageFormat::ARGB8888:
		case vtfpp::ImageFormat::BGRA8888:
			return QOpenGLTexture::PixelFormat::RGBA;
		case vtfpp::ImageFormat::BGRX8888:
			return QOpenGLTexture::PixelFormat::BGRA;
		case vtfpp::ImageFormat::BGR565:
			break;
		case vtfpp::ImageFormat::BGRX5551:
			break;
		case vtfpp::ImageFormat::BGRA4444:
			break;
		case vtfpp::ImageFormat::BGRA5551:
			break;
		case vtfpp::ImageFormat::UV88:
			break;
		case vtfpp::ImageFormat::UVWQ8888:
			break;
		case vtfpp::ImageFormat::RGBA16161616F:
			break;
		case vtfpp::ImageFormat::RGBA16161616:
			break;
		case vtfpp::ImageFormat::UVLX8888:
			break;
		case vtfpp::ImageFormat::R32F:
			break;
		case vtfpp::ImageFormat::RGB323232F:
			break;
		case vtfpp::ImageFormat::RGBA32323232F:
			break;
		case vtfpp::ImageFormat::RG1616F:
			break;
		case vtfpp::ImageFormat::RG3232F:
			break;
		case vtfpp::ImageFormat::RGBX8888:
			break;
		case vtfpp::ImageFormat::EMPTY:
			break;
		case vtfpp::ImageFormat::ATI2N:
			break;
		case vtfpp::ImageFormat::ATI1N:
			break;
		case vtfpp::ImageFormat::RGBA1010102:
			break;
		case vtfpp::ImageFormat::BGRA1010102:
			break;
		case vtfpp::ImageFormat::R16F:
			break;
		case vtfpp::ImageFormat::CONSOLE_BGRX8888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_RGBA8888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_ABGR8888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_ARGB8888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_BGRA8888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_RGB888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_BGR888_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_BGRX5551_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_I8_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_RGBA16161616_LINEAR:
			break;
		case vtfpp::ImageFormat::CONSOLE_BGRX8888_LE:
			break;
		case vtfpp::ImageFormat::CONSOLE_BGRA8888_LE:
			break;
		case vtfpp::ImageFormat::R8:
			break;
		case vtfpp::ImageFormat::BC7:
			break;
		case vtfpp::ImageFormat::BC6H:
			break;
	}
}

inline QOpenGLTexture::TextureFormat mapVTFToGLFormat( vtfpp::ImageFormat format )
{
	switch ( format )
	{
		case vtfpp::ImageFormat::EMPTY:
			return QOpenGLTexture::NoFormat;
		case vtfpp::ImageFormat::CONSOLE_BGRX8888_LE:
		case vtfpp::ImageFormat::CONSOLE_BGRA8888_LE:
		case vtfpp::ImageFormat::CONSOLE_BGRX8888_LINEAR:
		case vtfpp::ImageFormat::CONSOLE_RGBA8888_LINEAR:
		case vtfpp::ImageFormat::CONSOLE_ABGR8888_LINEAR:
		case vtfpp::ImageFormat::CONSOLE_ARGB8888_LINEAR:
		case vtfpp::ImageFormat::CONSOLE_BGRA8888_LINEAR:
		case vtfpp::ImageFormat::RGBX8888:
		case vtfpp::ImageFormat::UVLX8888:
		case vtfpp::ImageFormat::UVWQ8888:
		case vtfpp::ImageFormat::BGRX8888:
		case vtfpp::ImageFormat::ARGB8888:
		case vtfpp::ImageFormat::BGRA8888:
		case vtfpp::ImageFormat::RGBA8888:
		case vtfpp::ImageFormat::ABGR8888:
			return QOpenGLTexture::RGBA8_UNorm;
		case vtfpp::ImageFormat::CONSOLE_RGB888_LINEAR:
		case vtfpp::ImageFormat::CONSOLE_BGR888_LINEAR:
		case vtfpp::ImageFormat::RGB888_BLUESCREEN:
		case vtfpp::ImageFormat::BGR888_BLUESCREEN:
		case vtfpp::ImageFormat::RGB888:
		case vtfpp::ImageFormat::BGR888:
			return QOpenGLTexture::RGB8_UNorm;
		case vtfpp::ImageFormat::BGR565:
		case vtfpp::ImageFormat::RGB565:
			return QOpenGLTexture::R5G6B5;
		case vtfpp::ImageFormat::I8:
		case vtfpp::ImageFormat::A8:
		case vtfpp::ImageFormat::R8:
		case vtfpp::ImageFormat::P8:
		case vtfpp::ImageFormat::CONSOLE_I8_LINEAR:
			return QOpenGLTexture::R8_UNorm;
		case vtfpp::ImageFormat::CONSOLE_BGRX5551_LINEAR:
		case vtfpp::ImageFormat::BGRA5551:
		case vtfpp::ImageFormat::BGRX5551:
			return QOpenGLTexture::RGB5A1;
		case vtfpp::ImageFormat::UV88:
		case vtfpp::ImageFormat::IA88:
			return QOpenGLTexture::RG8I;
		case vtfpp::ImageFormat::RGBA16161616:
		case vtfpp::ImageFormat::CONSOLE_RGBA16161616_LINEAR:
			return QOpenGLTexture::RGBA16I;
		case vtfpp::ImageFormat::RGBA1010102:
		case vtfpp::ImageFormat::BGRA1010102:
			return QOpenGLTexture::RGB10A2;
		case vtfpp::ImageFormat::DXT1:
			return QOpenGLTexture::RGBA_DXT1;
		case vtfpp::ImageFormat::DXT1_ONE_BIT_ALPHA:
			return QOpenGLTexture::RGBA_DXT1;
		case vtfpp::ImageFormat::DXT3:
			return QOpenGLTexture::RGBA_DXT3;
		case vtfpp::ImageFormat::DXT5:
			return QOpenGLTexture::RGBA_DXT5;
		case vtfpp::ImageFormat::BGRA4444:
			return QOpenGLTexture::RGBA4;
		case vtfpp::ImageFormat::RGBA16161616F:
			return QOpenGLTexture::RGBA16F;
		case vtfpp::ImageFormat::R32F:
			return QOpenGLTexture::R32F;
		case vtfpp::ImageFormat::RGB323232F:
			return QOpenGLTexture::RGB32F;
		case vtfpp::ImageFormat::RGBA32323232F:
			return QOpenGLTexture::RGBA32F;
		case vtfpp::ImageFormat::RG1616F:
			return QOpenGLTexture::RG16F;
		case vtfpp::ImageFormat::RG3232F:
			return QOpenGLTexture::RG32F;
		case vtfpp::ImageFormat::ATI2N:
			return QOpenGLTexture::RG_ATI2N_UNorm;
		case vtfpp::ImageFormat::ATI1N:
			return QOpenGLTexture::R_ATI1N_UNorm;
		case vtfpp::ImageFormat::R16F:
			return QOpenGLTexture::R16F;
		case vtfpp::ImageFormat::BC7:
			return QOpenGLTexture::RGB_BP_UNorm;
		case vtfpp::ImageFormat::BC6H:
			return QOpenGLTexture::RGB_BP_SIGNED_FLOAT;
	}
}

class ImageViewWidget : public QOpenGLWidget,
						protected QOpenGLFunctions_4_5_Core
{
	Q_OBJECT;

	static constexpr GLfloat texCoords[] = {
		// positions          // colors           // texture coords
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,	  // top right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
		-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f	  // top left
	};

	QImage background;

	static constexpr GLbyte texIndeces[] = {
		0, 1, 2, 2, 3, 0 };

public:
	ImageViewWidget( QWidget *pParent = nullptr );

	void timerEvent( QTimerEvent *event ) override;

	void set_vtf( vtfpp::VTF *file );

	void initializeGL() override;

	void resizeGL( int w, int h ) override;

	void paintGL() override;

	void wheelEvent( QWheelEvent *event ) override;

	bool event( QEvent * ) override;

	void set_red( bool red )
	{
		if ( red == hasRed_ )
			return;

		rgba_ += red ? 8 : -8;
		hasRed_ = red;
		this->update();
	}

	void set_green( bool green )
	{
		if ( green == hasGreen_ )
			return;
		rgba_ += green ? 4 : -4;
		hasGreen_ = green;
		this->update();
	}

	void set_blue( bool blue )
	{
		if ( blue == hasBlue_ )
			return;
		rgba_ += blue ? 2 : -2;
		hasBlue_ = blue;
		this->update();
	}

	void set_alpha( bool alpha )
	{
		if ( alpha == hasAlpha_ )
			return;
		rgba_ += alpha ? 1 : -1;
		hasAlpha_ = alpha;
		this->update();
	}

	void set_rgba( bool r, bool g, bool b, bool a )
	{
		if ( r == hasRed_ && g == hasGreen_ && b == hasBlue_ && hasAlpha_ == a )
			return;

		if ( a != hasAlpha_ )
		{
			rgba_ += a ? 1 : -1;
			hasAlpha_ = !hasAlpha_;
		}
		{
			rgba_ += b ? 2 : -2;
			hasBlue_ = !hasBlue_;
		}
		{
			rgba_ += g ? 4 : -4;
			hasGreen_ = !hasGreen_;
		}
		{
			rgba_ += r ? 8 : -8;
			hasRed_ = !hasRed_;
		}

		this->update();
	}

	void set_frame( int f )
	{
		frame_ = f;
		this->update();
	}

	int get_frame() const
	{
		return frame_;
	}

	void set_face( int f )
	{
		if ( m_animating )
			return; // We do not allow the face to be set/messed with publicly when it's animating.
		face_ = f;
		this->update();
	}
	void set_mip( int f )
	{
		mip_ = f;
		this->update();
	}

	float getZoom() const;

	void zoom( float amount );

	void setXOffset( int offset )
	{
		xOffset_ = offset;
		this->update();
	};

	void setYOffset( int offset )
	{
		yOffset_ = offset;
		this->update();
	}

	int getHDRGamma()
	{
		return gamma_;
	}

	void setHDRGamma( int gamma )
	{
		gamma_ = gamma;
		this->update();
	}

	void setSpriteSheet( vtfpp::SHT::Sequence::Frame::Bounds sheet, bool is )
	{
		hasSpriteSheetLocation_ = is;
		if ( !hasSpriteSheetLocation_ )
		{
			update();
			return;
		}
		this->spriteSheet_ = sheet;
		update();
	}

	void startAnimation( int fps );

	void stopAnimating();

private:
	void update_size();

	QOpenGLTexture texture { QOpenGLTexture::Target2D };
	QOpenGLShaderProgram *shaderProgram;
	vtfpp::VTF *file_ = nullptr;

	bool m_animating = false;

	QOpenGLBuffer vertices { QOpenGLBuffer::Type::VertexBuffer };

	QOpenGLBuffer indexes { QOpenGLBuffer::Type::IndexBuffer };

	float zoom_ = 1.6f;
	QPoint pos_;

	int frame_ = 0;
	int face_ = 0;
	int mip_ = 0;
	int rgba_ = 16;
	float xOffset_ = 0;
	float yOffset_ = 0;
	int gamma_ = 220;
	bool hasRed_ = true;
	bool hasGreen_ = true;
	bool hasBlue_ = true;
	bool hasAlpha_ = true;

	bool hasSpriteSheetLocation_ = false;
	vtfpp::SHT::Sequence::Frame::Bounds spriteSheet_ = { -1.f, -1.f, -1.f, -1.f };

	int animationTimer_ = -1;
	bool animateReverse_;

	bool m_isCTRLHeld = false;

	int currentFrame_ = 0;
	int currentFace_ = 0;
	int currentMip_ = 0;
	bool requestColorChange = false;
	void Animate();

protected:
	void mousePressEvent( QMouseEvent *event ) override;

signals:
	void animated( int frame );
	void zoomChanged( float zoom );
	void onRightClick();
};