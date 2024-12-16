#pragma once
#include <QMessageBox>
#include <utility>
#include <vtfpp/vtfpp.h>
// enum ImageDataType : uint8_t
//{
//	FRAME = 0,
//	FACE,
//	SLICE,
//	MIP
// };

// struct ImageInfo
//{
//	uint16_t frame;
//	uint8_t face;
//	uint16_t slice;
//	uint8_t mips;
//	//	ImageDataType dataType;
//	bool operator==( const ImageInfo &t ) const
//	{
//		return frame == t.frame && slice == t.slice && mips == t.mips && face == t.face;
//	}
//
//	auto operator<=>( const ImageInfo &other ) const
//	{
//		return frame == other.frame ? ( slice == other.slice ? ( mips == other.mips ? face <=> other.face : mips <=> other.mips ) : slice <=> other.slice ) : frame <=> other.frame;
//	}
//
//	//	bool operator<( uint32_t &t ) const
//	//	{
//	//		return frame < t && slice < 0 && mips < 0 && face < 0;
//	//	}
// };

struct VTFContainer
{
	vtfpp::VTF *vtf = nullptr;
	QString path = {};
	QString name = {};
	bool hasSaved = false;
	bool isOriginal = false;

	vtfpp::VTF *operator->() const
	{
		return this->vtf;
	}

	operator bool()
	{
		return vtf;
	}

	bool operator!() const
	{
		return vtf == nullptr;
	}
};

class VTFEImageContainer
{
	QString path;
	std::vector<std::byte> m_vImageData;
	uint16_t m_vWidth;
	uint16_t m_vHeight;
	uint32_t m_vSize;
	vtfpp::ImageFormat m_vFormat;

public:
	explicit VTFEImageContainer( const QString &path )
	{
		vtfpp::ImageFormat inputFormat;
		int inputWidth, inputHeight, inputFrameCount;
		auto imageData_ = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( path.toStdString() ), inputFormat, inputWidth, inputHeight, inputFrameCount );

		if ( inputFrameCount > 1 )
		{
			QMessageBox::critical( nullptr, "INVALID IMAGE", "Image file detected with too many frames." );
		}

		this->path = path;
		m_vSize = imageData_.size();
		m_vImageData = std::move( imageData_ );
		m_vWidth = inputWidth;
		m_vHeight = inputHeight;
		m_vFormat = inputFormat;
	}

	VTFEImageContainer( std::vector<std::byte> b, uint16_t width, uint16_t height, vtfpp::ImageFormat format )
	{
		m_vSize = b.size(); // vtfpp::ImageFormatDetails::getDataLength( format, width, height, 1 );
		m_vImageData = std::move( b );
		m_vWidth = width;
		m_vHeight = height;
		m_vFormat = format;
	}

	VTFEImageContainer( const std::byte *b, uint16_t width, uint16_t height, vtfpp::ImageFormat format )
	{
		auto size = vtfpp::ImageFormatDetails::getDataLength( format, width, height, 1 );
		m_vSize = size;
		m_vImageData = { b, b + m_vSize };
		m_vWidth = width;
		m_vHeight = height;
		m_vFormat = format;
	}

	VTFEImageContainer( const VTFEImageContainer &VTFEIF )
	{
		path = VTFEIF.path;
		m_vWidth = VTFEIF.m_vWidth;
		m_vHeight = VTFEIF.m_vHeight;
		m_vSize = VTFEIF.m_vSize;
		m_vFormat = VTFEIF.m_vFormat;
		m_vImageData = VTFEIF.m_vImageData;
	}

	[[nodiscard]] vtfpp::ImageFormat getFormat() const
	{
		return m_vFormat;
	}

	[[nodiscard]] uint16_t getWidth() const
	{
		return m_vWidth;
	}

	[[nodiscard]] uint16_t getHeight() const
	{
		return m_vHeight;
	}

	[[nodiscard]] uint32_t getSize() const
	{
		return m_vSize;
	}

	[[nodiscard]] const std::vector<std::byte> &getData() const
	{
		return m_vImageData;
	}

	[[nodiscard]] const QString getPath()
	{
		return path;
	}

	[[nodiscard]] bool hasData() const
	{
		return !m_vImageData.empty();
	}
};
