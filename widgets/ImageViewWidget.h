#pragma once
#include "../libs/VTFLib/VTFLib/VTFLib.h"

#include <QWidget>

enum ColorSelection
{
	ALL,
	RGB,
	RED,
	GREEN,
	BLUE,
	ALPHA
};

class ImageViewWidget : public QWidget
{
	Q_OBJECT;

public:
	ImageViewWidget( QWidget *pParent = nullptr );

	void set_pixmap( const QImage &pixmap );
	void set_vtf( VTFLib::CVTFFile *file );

	inline const QImage &pixmap() const
	{
		return image_;
	};
	inline QImage &pixmap()
	{
		return image_;
	};

	void paintEvent( QPaintEvent *event ) override;

	void set_red( bool red )
	{
		red_ = red;
		requestColorChange = true;
		repaint();
	}

	void set_green( bool green )
	{
		green_ = green;
		requestColorChange = true;
		repaint();
	}

	void set_blue( bool blue )
	{
		blue_ = blue;
		requestColorChange = true;
		repaint();
	}

	void set_alpha( bool alpha )
	{
		alpha_ = alpha;
		requestColorChange = true;
		repaint();
	}

	void set_frame( int f )
	{
		frame_ = f;
		repaint();
	}
	void set_face( int f )
	{
		face_ = f;
		repaint();
	}
	void set_mip( int f )
	{
		mip_ = f;
		repaint();
	}

	void zoom( float amount );

private:
	void update_size();

	QImage image_;
	void *imgBuf_ = nullptr;
	VTFLib::CVTFFile *file_ = nullptr;

	float zoom_ = 1.0f;
	QPoint pos_;

	int frame_ = 0;
	int face_ = 0;
	int mip_ = 0;
	bool red_ = true;
	bool green_ = true;
	bool blue_ = true;
	bool alpha_ = true;

	int currentFrame_ = 0;
	int currentFace_ = 0;
	int currentMip_ = 0;
	bool requestColorChange = false;
};