#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

// Ensure byte alignment
// We want it TIGHT
#pragma pack( push, 1 )

// TIFF is a little bit more complicated than TGA where its just a header
// Lets start with how its set up.
// This was compiled from the information available at..
// https://www.ibm.com/docs/en/ftmfm/3.0.2?topic=validation-tiff-image-characteristics-checking-capability

// First comes the Signature, which is pretty much always the same
// II, 42, 8
// The first IFD always comes after the Signature, so the offset to the IFD is always 8

struct File_TIFF_Signature
{
	uint8_t nEndian[2];	   // Byte order indicator ("II" for little-endian or "MM" for big-endian)
	uint16_t nMagicNumber; // TIFF magic number (42)
	uint32_t nOffsetIFD;   // Offset to the first Image File Directory (IFD) in the file
};

// Each IFD has an uncertain amount of Entries in it
// This is where the name Tagged mage Format comes from
// The Entry contains a TAG. Which is basically just a number
// That corresponds to a certain "thing"
// The field type determines the kind of data we have
// These are the possible field types, although you usually only ever see 1-5 and 7
// 1.	BYTE		(8 - bit unsigned integer) : An 8 - bit unsigned integer.
// 2.	ASCII		(8 - bit byte containing ASCII codes) : A sequence of 8 - bit bytes representing ASCII characters, with the last byte being null.
// 3.	SHORT		(16 - bit unsigned integer) : A 16 - bit(2 - byte) unsigned integer.
// 4.	LONG		(32 - bit unsigned integer) : A 32 - bit(4 - byte) unsigned integer.
// 5.	RATIONAL	(Two LONGs - numerator and denominator) : Two 32 - bit unsigned integers representing a fraction.
// 6.	SBYTE		(8 - bit signed integer) : An 8 - bit signed integer.
// 7.	UNDEFINED	(8 - bit byte that can contain anything) : An 8 - bit byte that can contain any value depending on the field definition.
// 8.	SSHORT		(16 - bit signed integer) : A 16 - bit(2 - byte) signed integer.
// 9.	SLONG		(32 - bit signed integer) : A 32 - bit(4 - byte) signed integer.
// 10.	SRATIONAL	(Two SLONGs - numerator and denominator) : Two 32 - bit signed integers representing a fraction.
// 11.	FLOAT		(32 - bit IEEE floating - point) : A 32 - bit IEEE floating - point value.
// 12.	DOUBLE		(64 - bit IEEE double - precision floating - point) : A 64 - bit IEEE double - precision floating - point value.
// The count is pretty much "how many"
// Offset or Value depends on the data this contains
// Usually it will contain the value directly, but often times like in case of bits per sample
// it will contain an offset relative to the start of the file to where the data is
// This is, extremely annoying when writing a TIF, because you will not know where is space for the data
// before you write the Entry. **VERY ANNOYING**
struct File_TIFF_IFD_Entry
{
	uint16_t nTag;
	uint16_t nFieldType;
	uint32_t nCount;

	//	https://docs.fileformat.com/image/tiff/
	// 	"The Value Offset, the file offset (in bytes) of the Value for the field.
	//	The Value is expected to begin on a word boundary; the correspond-ing Value Offset will thus be an even number.
	//	This file offset may point anywhere in the file, even after the image data"
	//	TLDR: Even number
	uint32_t nOffsetOrValue;
};

// There will be an IFD for each image file in the TIFF file
// There is always at least one in a tif file right after the signature
// the *aIFD is an array of IFD Entries of the size nEntryCount * sizeof(File_TIFF_IFD_Entry)
// do NOT read or write this struct from/into a file, you have to first read the entry count, then all the entries, THEN the offset to the next IFD
struct File_TIFF_IFD
{
	// This keeps track of how many IFD entries there are
	uint16_t nEntryCount;

	// Array of Entries in this IFD
	std::vector<File_TIFF_IFD_Entry> aIFD;

	uint32_t nNextIFD;
};

enum File_TIFF_CompressionScheme
{
	COMPRESSION_NONE = 0,
	COMPRESSION_CCITT_GROUP3_1D,  // Fax Encoding
	COMPRESSION_CCITT_GROUP3_2D,  // Fax Encoding
	COMPRESSION_CCITT_GROUP3_4D,  // Fax Encoding
	COMPRESSION_LZW,			  // Lempel-Ziv-Welch Compression
	COMPRESSION_JPEG_OLD,		  // Old JPEG Compression
	COMPRESSION_JPEG,			  // JPEG
	COMPRESSION_DEFLATE,		  // Adobe Deflate Compression
	COMPRESSION_DEFLATE_ITK_GDCM, // ITK GDCM Deflate Compression
	COMPRESSION_PACKBITS,
};

enum File_Format
{
	// All the formats we may ever need
	IMAGEFORMAT_TGA = 0, // Truevision Graphics Adapter
	IMAGEFORMAT_TIFF,	 // Tag Image File Format
	IMAGEFORMAT_DDS,	 // Direct Draw Surface ( Microsoft )

	// If you wish to expand on this :
	//    IMAGEFORMAT_PNG,        // Portable Network Graphic
	//    IMAGEFORMAT_JPEG,        // Joint Photographic Experts Group
	//    IMAGEFORMAT_BMP,        // BitMap
	//    IMAGEFORMAT_GIF,        // Graphics Interchange Format, soft g because its inventor said so
	//    IMAGEFORMAT_SVG,        // Scalable Vector Graphic
	//    IMAGEFORMAT_RAW,        // Raw image data
	//    IMAGEFORMAT_ICO,        // Icon File - Probably not worth it, ever
};

struct ImageData_t
{
	// Original File Format from which this texture was loaded
	// This is important because some formats like TGA or DDS store their data as BGR and not RGB
	// So we check this to convert the format in whatever the output format needs it to be
	File_Format Format;

	// If we have alpha
	bool bHasAlpha;

	// Whether or not we have mipmaps ( in case of dds )
	// If yes it will be high res -> low res
	// With the entire textures following after each other in power of 2 order
	bool bMipMaps;

	// Bit count is in bits per pixel
	// 8 b/c    = 24 b/p
	// 16 b/c    = 48 b/p
	// 32 b/c    = 96 b/p
	unsigned int nBitCountPerPixel;

	// bpp / 3 for without alpha and / 4 when with alpha
	unsigned int nBitCountPerChannel;

	// how many channels.
	// This exists just for easy math when writing
	unsigned int nCountChannels;

	// Only true when the data is valid
	bool bValidImageData;

	// Pointer to the image data, have to reinterpret based on other available data
	// Usually this will be an array of the size [nImageWidth * nImageHeight * (b/c)]
	// If you try to load multiple images, I suggest loading them each with their own ImageData_t
	// That way you could load custom mipmaps separately etc
	// char is for 8 bit data
	// uint16 for 16f, we merely use it as a storage vessel for this data
	// 32f is just raw float data so float it is
	char *pImageData;
	uint16_t *pImageData16f;
	float *pImageData32f;

	unsigned int nImageWidth;
	unsigned int nImageHeight;

	ImageData_t()
	{
		pImageData = nullptr;
		pImageData16f = nullptr;
		pImageData32f = nullptr;
	}
};

enum TIFFImageType
{
	U8 = 8,
	FP16 = 16,
	FP32 = 32

};

struct TIFFFile
{
	bool isValid;
	uint32_t width;
	uint32_t height;
	uint32_t channelCount;
	TIFFImageType type;
	File_TIFF_CompressionScheme compression;
	bool hasAlpha;

	std::vector<std::byte> imageData;
};

// Restore default byte alignment
// Now we don't care
#pragma pack( pop )

class TiffSupport
{
public:
	TiffSupport() = delete;
	static bool Load_TIFF( const char *cFileName, ImageData_t *ImageInMemory );
	static bool Load_TIFF( std::string_view fileName, TIFFFile &buff );
};
