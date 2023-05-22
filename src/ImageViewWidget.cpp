
#include "ImageViewWidget.h"

#include <QBitmap>
#include <QColorSpace>
#include <QPainter>
#include <QVulkanDeviceFunctions>
#include <iostream>

using namespace VTFLib;

ImageViewWidget::ImageViewWidget( QWindow *pParent ) :
	QVulkanWindow( pParent )
{
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

QVulkanWindowRenderer *ImageViewWidget::createRenderer()
{
	return new VulkanRenderer( this );
}

VulkanRenderer::VulkanRenderer( QVulkanWindow *w ) :
	m_window( w )
{
}

void VulkanRenderer::initResources()
{
	qDebug( "initResources" );

	m_devFuncs = m_window->vulkanInstance()->deviceFunctions( m_window->device() );
}

void VulkanRenderer::startNextFrame()
{
	m_green += 0.005f;
	if ( m_green > 1.0f )
		m_green = 0.0f;

	VkClearColorValue clearColor = { { 0.0f, m_green, 0.0f, 1.0f } };
	VkClearDepthStencilValue clearDS = { 1.0f, 0 };
	VkClearValue clearValues[2];
	memset( clearValues, 0, sizeof( clearValues ) );
	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = clearDS;

	VkRenderPassBeginInfo rpBeginInfo;
	memset( &rpBeginInfo, 0, sizeof( rpBeginInfo ) );
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = m_window->defaultRenderPass();
	rpBeginInfo.framebuffer = m_window->currentFramebuffer();
	const QSize sz = m_window->swapChainImageSize();
	rpBeginInfo.renderArea.extent.width = sz.width();
	rpBeginInfo.renderArea.extent.height = sz.height();
	rpBeginInfo.clearValueCount = 2;
	rpBeginInfo.pClearValues = clearValues;
	VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
	m_devFuncs->vkCmdBeginRenderPass( cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

	// Do nothing else. We will just clear to green, changing the component on
	// every invocation. This also helps verifying the rate to which the thread
	// is throttled to. (The elapsed time between startNextFrame calls should
	// typically be around 16 ms. Note that rendering is 2 frames ahead of what
	// is displayed.)

	m_devFuncs->vkCmdEndRenderPass( cmdBuf );

	m_window->frameReady();
	m_window->requestUpdate(); // render continuously, throttled by the presentation rate
}