#pragma once
#include "../libs/VTFLib/VTFLib/VTFLib.h"
#include "../libs/VTFLib/VTFLib/stdafx.h"
class VTFEImageFormat
{
	~VTFEImageFormat()
	{
		delete[] m_vImageData;
	}

	vlByte *m_vImageData;
	vlUInt m_vWidth;
	vlUInt m_vHeight;
	vlUInt m_vDepth;
	vlUInt m_vSize;
	VTFImageFormat m_vformat;

public:
	VTFEImageFormat( vlByte *b, vlUInt width, vlUInt height, vlUInt depth, VTFImageFormat format )
	{
		m_vSize = VTFLib::CVTFFile::ComputeImageSize( width, height, 1, format );
		m_vImageData = new vlByte[m_vSize];
		memcpy( m_vImageData, b, m_vSize );
		m_vWidth = width;
		m_vHeight = height;
		m_vDepth = depth;
		m_vformat = format;
	}

	VTFEImageFormat( const VTFEImageFormat &VTFEIF )
	{
		m_vWidth = VTFEIF.m_vWidth;
		m_vHeight = VTFEIF.m_vHeight;
		m_vDepth = VTFEIF.m_vDepth;
		m_vSize = VTFEIF.m_vSize;
		m_vformat = VTFEIF.m_vformat;
		m_vImageData = VTFEIF.m_vImageData;
	}

	VTFImageFormat getFormat()
	{
		return m_vformat;
	}

	vlUInt getWidth()
	{
		return m_vWidth;
	}

	vlUInt getHeight()
	{
		return m_vHeight;
	}

	vlUInt getDepth()
	{
		return m_vDepth;
	}

	vlUInt getSize()
	{
		return m_vSize;
	}

	vlByte *getData()
	{
		return m_vImageData;
	}

	bool hasData()
	{
		return !!m_vImageData;
	}
};
