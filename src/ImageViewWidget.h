#pragma once
#include "../libs/VTFLib/VTFLib/VTFLib.h"
#include "vulkan/vulkan.h"

#include <QVulkanWindow>
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

class ImageViewWidget : public QVulkanWindow
{
	Q_OBJECT;

public:
	ImageViewWidget( QWindow *pParent = nullptr );
	QVulkanWindowRenderer *createRenderer() override;

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

	void set_red( bool red )
	{
		red_ = red;
		requestColorChange = true;
	}

	void set_green( bool green )
	{
		green_ = green;
		requestColorChange = true;
	}

	void set_blue( bool blue )
	{
		blue_ = blue;
		requestColorChange = true;
	}

	void set_alpha( bool alpha )
	{
		alpha_ = alpha;
		requestColorChange = true;
	}

	void set_rgba( bool r, bool g, bool b, bool a )
	{
		red_ = r;
		green_ = g;
		blue_ = b;
		alpha_ = a;
		requestColorChange = true;
	}

	void set_frame( int f )
	{
		frame_ = f;
	}
	void set_face( int f )
	{
		face_ = f;
	}
	void set_mip( int f )
	{
		mip_ = f;
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

class VulkanRenderer : public QVulkanWindowRenderer
{
public:
	VulkanRenderer( QVulkanWindow *w );

	void initResources() override;
	//	void initSwapChainResources() override;
	//	void releaseSwapChainResources() override;
	//	void releaseResources() override;

	void startNextFrame() override;

private:
	QVulkanWindow *m_window;
	QVulkanDeviceFunctions *m_devFuncs;
	float m_green = 0;
};