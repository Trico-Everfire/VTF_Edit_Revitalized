
#include "TiffSupport.h"

#include <fstream>
#include <iostream>
#include <vector>

bool TiffSupport::Load_TIFF( std::string_view fileName, TIFFFile &buff )
{
	buff.isValid = false;

	std::ifstream TIFF_File( fileName.data(), std::ios::binary );

	if ( !TIFF_File.is_open() )
		return false;

	File_TIFF_Signature Header {};
	TIFF_File.read( reinterpret_cast<char *>( &Header ), sizeof( File_TIFF_Signature ) );

	if ( Header.nMagicNumber != 42 )
	{
		std::cout << "Could not load " << fileName << ". Not a TIFF file." << std::endl;
		TIFF_File.close();
		return false;
	}

	if ( Header.nEndian[0] != 'I' || Header.nEndian[1] != 'I' )
	{
		std::cout << "Could not load " << fileName << ". Only little-endian TIFF files are supported." << std::endl;
		TIFF_File.close();
		return false;
	}

	std::vector<File_TIFF_IFD> IFDs;
	std::vector<uint32_t> nOffsetToIFDs;
	int nIFDs = 0; // Keeps track of the number of IFD's

	uint32_t nOffsetNextIFD = Header.nOffsetIFD;
	while ( nOffsetNextIFD != 0 )
	{
		TIFF_File.seekg( nOffsetNextIFD, std::ios::beg );
		File_TIFF_IFD CurrentIFD;

		TIFF_File.read( reinterpret_cast<char *>( &CurrentIFD.nEntryCount ), sizeof( uint16_t ) );

		TIFF_File.seekg( nOffsetNextIFD + sizeof( uint16_t ), std::ios::beg );

		CurrentIFD.aIFD.resize( CurrentIFD.nEntryCount );

		TIFF_File.read( reinterpret_cast<char *>( CurrentIFD.aIFD.data() ), CurrentIFD.nEntryCount * sizeof( File_TIFF_IFD_Entry ) );

		TIFF_File.read( reinterpret_cast<char *>( &CurrentIFD.nNextIFD ), sizeof( uint32_t ) );

		CurrentIFD.aIFD.push_back( {} );

		IFDs.push_back( CurrentIFD );
		nOffsetToIFDs.push_back( nOffsetNextIFD );

		nOffsetNextIFD = CurrentIFD.nNextIFD;
		nIFDs++;
	}

	uint16_t nPhotometricInterpretation;

	for ( int i = 0; i < nIFDs; i++ )
		for ( int j = 0; j < IFDs[i].nEntryCount; j++ )
		{
			// All the tags we check
			// 256 - Width
			// 257 - Height
			// 258 - BitsPerSample
			// 259 - Compression Scheme used on the image
			// 262 - Photometric Interpretation
			// All the fieldtypes we ever need
			// 3 - SHORT
			uint16_t Tag = IFDs[i].aIFD[j].nTag;
			uint16_t FieldType = IFDs[i].aIFD[j].nFieldType;
			uint32_t Count = IFDs[i].aIFD[j].nCount;
			uint32_t Value = IFDs[i].aIFD[j].nOffsetOrValue;

			if ( Tag == 256 && FieldType == 3 && Count == 1 )
			{
				buff.width = (unsigned int)static_cast<uint32_t>( Value );
			}
			// Height
			else if ( Tag == 257 && FieldType == 3 && Count == 1 )
			{
				buff.height = (unsigned int)static_cast<uint32_t>( Value );
			}
			// Bits Per Sample
			else if ( Tag == 258 && FieldType == 3 && Count > 0 )
			{
				// BitsPerSample tag (Tag 258), FieldType 3 (SHORT), Count > 0
				TIFF_File.seekg( IFDs[i].aIFD[j].nOffsetOrValue, std::ios::beg );
				std::vector<uint16_t> nBitsPerSample( IFDs[i].aIFD[j].nCount );
				TIFF_File.read( reinterpret_cast<char *>( nBitsPerSample.data() ), IFDs[i].aIFD[j].nCount * sizeof( uint16_t ) );

				std::cout << "BitsPerSample values: ";
				for ( uint16_t value : nBitsPerSample )
				{
					std::cout << value << " ";
				}
				std::cout << std::endl;

				if ( !nBitsPerSample.empty() )
				{
					// bits per sample refers to the number of bits used to
					// represent the intensity of color information in a single channel
					// In other words, bits per channel
					//					nBitsPerChannel = static_cast<int>( nBitsPerSample[0] );
					switch ( static_cast<int>( nBitsPerSample[0] ) )
					{
						case U8:
						case FP16:
						case FP32:
							buff.type = static_cast<TIFFImageType>( nBitsPerSample[0] );
							break;
						default:
							return false;
					}
				}
			}
			// Compression Scheme
			else if ( Tag == 259 && FieldType == 3 && Count == 1 )
			{
				switch ( IFDs[i].aIFD[j].nOffsetOrValue )
				{
					case 1:
						buff.compression = COMPRESSION_NONE;
						break;

					case 2:
						buff.compression = COMPRESSION_CCITT_GROUP3_1D;
						break;

					case 3:
						buff.compression = COMPRESSION_CCITT_GROUP3_2D;
						break;

					case4:
						buff.compression = COMPRESSION_CCITT_GROUP3_4D;
						break;

					case 5:
						buff.compression = COMPRESSION_LZW;
						break;

					case 6:
						buff.compression = COMPRESSION_JPEG_OLD;
						break;

					case 7:
						buff.compression = COMPRESSION_JPEG;
						break;

					case 8:
						buff.compression = COMPRESSION_DEFLATE;
						break;

					case 32946:
						buff.compression = COMPRESSION_DEFLATE_ITK_GDCM;
						break;

					case 32773:
						buff.compression = COMPRESSION_PACKBITS;
						break;
				}
			}
			// Photometric interpretation
			else if ( Tag == 262 && FieldType == 3 && Count == 1 )
			{
				bool bStore;
				if ( nPhotometricInterpretation == 2 )
					bStore = true;

				// TODO: This code makes no sense but it should work
				// Like what, read something as uint16 regardless?
				// If the value fits in the offset, use it directly
				if ( Count * sizeof( uint16_t ) <= sizeof( Value ) )
				{
					nPhotometricInterpretation = static_cast<uint16_t>( Value );
				}
				else
				{
					// Seek to the offset and read the value
					TIFF_File.seekg( Value, std::ios::beg );
					TIFF_File.read( reinterpret_cast<char *>( &nPhotometricInterpretation ), sizeof( uint16_t ) );
				}

				if ( nPhotometricInterpretation == 4 )
					buff.hasAlpha = true;

				if ( bStore )
					nPhotometricInterpretation = 2;
			}
		}

	int nCountChannels = buff.hasAlpha ? 4 : 3;

	buff.channelCount = nCountChannels;

	unsigned int bufferSize = buff.width * buff.height * ( buff.type / 8 ) * nCountChannels;

	buff.imageData.resize( bufferSize );

	bool bReadRGB = false;
	bool bReadAlpha = false;
	uint16_t TextureType = 0;

	std::vector<std::byte> tempAlphaBuffer;
	std::vector<std::byte> tempRGBBuffer;

	for ( int i = 0; i < nIFDs; i++ )
	{
		bool bHasData;
		bool bIsAlpha;

		// Check for StripOffsets and StripByteCounts Tags
		// Each IFD may only have one
		uint32_t nStripOffsets = 0;
		uint32_t nStripBytes = 0;

		// For each IFD Entry
		for ( int j = 0; j < IFDs[i].nEntryCount; j++ )
		{
			// Convert to easier to read format
			uint16_t Tag = IFDs[i].aIFD[j].nTag;
			uint16_t FieldType = IFDs[i].aIFD[j].nFieldType;
			uint32_t Count = IFDs[i].aIFD[j].nCount;
			uint32_t Value = IFDs[i].aIFD[j].nOffsetOrValue;

			if ( Tag == 262 && FieldType == 3 && Count == 1 )
			{
				// This IFD contains image data
				bHasData = true;

				// TODO: This code makes no sense but it should work
				// Like what, read something as uint16 regardless?
				// If the value fits in the offset, use it directly
				if ( Count * sizeof( uint16_t ) <= sizeof( Value ) )
				{
					TextureType = static_cast<uint16_t>( Value );
				}
				else
				{
					// Seek to the offset and read the value
					TIFF_File.seekg( Value, std::ios::beg );
					TIFF_File.read( reinterpret_cast<char *>( &TextureType ), sizeof( uint16_t ) );
				}

				// This IFD has Alpha Information? Yes:No
				bIsAlpha = TextureType == 4 ? true : false;
			}
			else if ( Tag == 273 )
			{
				nStripOffsets = Value;
			}
			else if ( Tag == 279 )
			{
				nStripBytes = Value;
			}
		}

		// We know where the image data is now and can read it
		if ( ( nStripOffsets != 0 ) && ( nStripBytes != 0 ) && bHasData )
		{
			// Move there first otherwise we read from random parts of the file
			TIFF_File.seekg( nStripOffsets, std::ios::beg );

			if ( bIsAlpha && !bReadAlpha )
			{
				// Debugging only
				if ( !buff.hasAlpha )
				{
					std::cout << "Found Alpha Strip without finding photometric interpretation for it. Something is wrong." << std::endl;
					continue;
				}

				// We will write the alpha after the rgb data. That way we don't have to mix the two dynamically
				// We can just fish it out again later when we need it. You just need to account for this when the input format is TIFF

				// 8f, 16f, 32f
				//				if ( buff.type == 8 )
				//					TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData + RGBSizeOffset * sizeof( char ) ), nStripBytes );
				//				else if ( ImageInMemory->nBitCountPerChannel == 16 )
				//					TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData16f + RGBSizeOffset * sizeof( uint16_t ) ), nStripBytes );
				//				else if ( ImageInMemory->nBitCountPerChannel == 32 )
				//					TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData32f + RGBSizeOffset * sizeof( float ) ), nStripBytes );
				tempAlphaBuffer.resize( nStripBytes );
				TIFF_File.read( reinterpret_cast<char *>( tempAlphaBuffer.data() ), nStripBytes );

				// We now have the data stop checking again
				bReadAlpha = true;
			}
			else if ( !bReadRGB ) // RGB Data
			{
				// 8f, 16f, 32f
				//				if ( ImageInMemory->nBitCountPerChannel == 8 )
				//					TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData ), nStripBytes );
				//				else if ( ImageInMemory->nBitCountPerChannel == 16 )
				//					TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData16f ), nStripBytes );
				//				else if ( ImageInMemory->nBitCountPerChannel == 32 )
				//					TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData32f ), nStripBytes );
				tempRGBBuffer.resize( nStripBytes );
				TIFF_File.read( reinterpret_cast<char *>( tempRGBBuffer.data() ), nStripBytes );

				std::cout << "successfully found rgb image data in tif file" << std::endl;

				// We now have the data stop checking again
				bReadRGB = true;
			}
		}
	}

	if ( tempRGBBuffer.empty() )
		return false;

	if ( !buff.hasAlpha )
	{
		//		if ( buff.type != FP16 )
		{
			buff.imageData = tempRGBBuffer;
			buff.isValid = true;
			return true;
		}

		//		struct sh_t
		//		{
		//			short dat = 32767;
		//
		//			union
		//			{
		//				std::byte one;
		//				std::byte two;
		//			};
		//		};
		//
		//		sh_t sh;

		//		for ( int i = 0; i < tempRGBBuffer.size(); i += sizeof( short ) * 3 )
		//		{
		//			buff.imageData.push_back( tempRGBBuffer[i] );
		//			buff.imageData.push_back( tempRGBBuffer[i + 1] );
		//			buff.imageData.push_back( tempRGBBuffer[i + 2] );
		//			buff.imageData.push_back( tempRGBBuffer[i + 3] );
		//			buff.imageData.push_back( tempRGBBuffer[i + 4] );
		//			buff.imageData.push_back( tempRGBBuffer[i + 5] );
		//			buff.imageData.push_back( sh.one );
		//			buff.imageData.push_back( sh.two );
		//		}

		buff.isValid = true;
		return true;
	}

	for ( int i = 0, k = 0; i < tempAlphaBuffer.size(); i += ( buff.type / 8 ), k += 3 * ( buff.type / 8 ) )
	{
		for ( int j = 0; j < ( buff.type / 8 ) * 3; j++ )
		{
			buff.imageData.push_back( tempRGBBuffer[k + j] );
		}
		for ( int j = 0; j < ( buff.type / 8 ); j++ )
		{
			buff.imageData.push_back( tempAlphaBuffer[i + j] );
		}
	}

	buff.isValid = true;
	return true;
}

bool TiffSupport::Load_TIFF( const char *ccFileName, ImageData_t *ImageInMemory )
{
	// Load the file from the disk
	std::ifstream TIFF_File( ccFileName, std::ios::binary );

	if ( TIFF_File.is_open() )
	{
		// Store that we are using TIFF
		// This is important because we will be adding alpha to the tail of the entire rgb data
		ImageInMemory->Format = IMAGEFORMAT_TIFF;

		// Read the header of the TIFF File
		File_TIFF_Signature Header;
		TIFF_File.read( reinterpret_cast<char *>( &Header ), sizeof( File_TIFF_Signature ) );

		// Little Endian
		if ( Header.nEndian[0] != 'I' || Header.nEndian[1] != 'I' )
		{
			std::cout << "Could not load " << ccFileName << ". Only little-endian TIFF files are supported." << std::endl;
			TIFF_File.close();
			return false;
		}

		// we need to dynamically add to this IFD list
		// There is not a given amount of IFD's so we just have to iterate through all of them...
		std::vector<File_TIFF_IFD> IFDs;
		std::vector<uint32_t> nOffsetToIFDs;
		int nIFDs = 0; // Keeps track of the number of IFD's

		// This has to be 8, otherwise your signature is wrong
		std::cout << "Size of Signature is " << sizeof( File_TIFF_Signature ) << std::endl;

		// This also has to be 8, the first IFD MUST come after the signature
		std::cout << "First offset is " << Header.nOffsetIFD << std::endl;

		// Offset to the next IFD; TIFF allows chaining an infinite amount. When we reach an offset of 0 it means we reached the end
		uint32_t nOffsetNextIFD = Header.nOffsetIFD;
		while ( nOffsetNextIFD != 0 )
		{
			TIFF_File.seekg( nOffsetNextIFD, std::ios::beg );

			// Heres a fun issue with TIF:
			// The entries are in the middle of the IFD...
			// Since we use a pointer instead of all the entries, we have to read the file as such:
			File_TIFF_IFD CurrentIFD;

			// Read the entry count, we already seeked to the start of the IFD so its here right now
			TIFF_File.read( reinterpret_cast<char *>( &CurrentIFD.nEntryCount ), sizeof( uint16_t ) );

			TIFF_File.seekg( nOffsetNextIFD + sizeof( uint16_t ), std::ios::beg );

			// Allocate the memory now and then load all the entries
			CurrentIFD.aIFD[CurrentIFD.nEntryCount] = {};
			TIFF_File.read( reinterpret_cast<char *>( &CurrentIFD.aIFD[CurrentIFD.nEntryCount] ), CurrentIFD.nEntryCount * sizeof( File_TIFF_IFD_Entry ) );

			// Debugging : Prints out all the Entry's of an IFD
#if 0
			for (uint16_t bl = 0; bl < CurrentIFD.nEntryCount; bl++)
			{
				std::cout << "--- Entry " << (bl + 1) << " ---" << std::endl;
				std::cout << "Tag " << (int)CurrentIFD.aIFD[bl].nTag << std::endl;
				std::cout << "FieldType " << (int)CurrentIFD.aIFD[bl].nFieldType << std::endl;
				std::cout << "Count " << (int)CurrentIFD.aIFD[bl].nCount << std::endl;
				std::cout << "Value " << (int)CurrentIFD.aIFD[bl].nOffsetOrValue << std::endl;
			}
#endif

			// Now read the offset to the next entry... which is practically just nOffsetNextIFD + sizeof()
			TIFF_File.read( reinterpret_cast<char *>( &CurrentIFD.nNextIFD ), sizeof( uint32_t ) );

			IFDs.push_back( CurrentIFD );
			nOffsetToIFDs.push_back( nOffsetNextIFD );

			// Visual Studio complains that nIFD is not declared
			// Since its value will be set in the loop
			// But setting its value to 0 will somehow break this entire thing
			// So I will simply ignore the warning since it will still work
			if ( nIFDs == 0 )
			{
				std::cout << CurrentIFD.nEntryCount << " Entries" << std::endl;
				std::cout << CurrentIFD.nNextIFD << " Offset to next IFD" << std::endl;
			}
			else if ( nIFDs < 25 )
				std::cout << "offset to next IFD : " << CurrentIFD.nNextIFD << std::endl;

			// Now move to the next IFD...
			nOffsetNextIFD = CurrentIFD.nNextIFD;
			nIFDs++;
		}

		std::cout << nIFDs << " IFD's found." << std::endl;

#if 0
		// Now that we populated the main list, lets get IFD-Entries on each IFD
		// So for each IFD there is,
		for (int i = 0; i < nIFDs; i++)
		{
			// Allocate memory for the IFD's Entries
			// NOTE: You have to delete[] this later
			IFDs[i].aIFD = new File_TIFF_IFD_Entry[IFDs[i].nEntryCount];

			// Go to the current IFD, then add the size of that IFD so we land right behind it
			// Then we just read however many entries there supposedly are
			TIFF_File.seekg(nOffsetToIFDs[i] + sizeof(File_TIFF_IFD), ios::beg);
			TIFF_File.read(reinterpret_cast<char*>(&IFDs[i].aIFD), IFDs[i].nEntryCount * sizeof(File_TIFF_IFD_Entry));
		}
#endif

		// Now this is kind of... interesting... ( really really annoying )
		// We have to check the tag on all entries for whether there is LZW compression
		// and all entries must be checked to find out if we have 3 channels or 4
		// and what resolution we have to expect
		// In theory we would just have an image buffer for each layer
		// But we don't want 15 layers, what am I supposed to do with that?
		File_TIFF_CompressionScheme Compression = COMPRESSION_NONE;
		unsigned int nBitsPerChannel = 0;	 // Bits per Sample
		uint16_t nPhotometricInterpretation; // 1 or 2 but never 3 or 4
		bool bHasAlpha;						 // True when Photometric Interpretation would have been 4

		// For each IFD...
		for ( int i = 0; i < nIFDs; i++ )
		{
			// For each IFD Entry
			for ( int j = 0; j < IFDs[i].nEntryCount; j++ )
			{
				// Convert to easier to read format
				uint16_t Tag = IFDs[i].aIFD[j].nTag;
				uint16_t FieldType = IFDs[i].aIFD[j].nFieldType;
				uint32_t Count = IFDs[i].aIFD[j].nCount;
				uint32_t Value = IFDs[i].aIFD[j].nOffsetOrValue;

				// All the tags we check
				// 256 - Width
				// 257 - Height
				// 258 - BitsPerSample
				// 259 - Compression Scheme used on the image
				// 262 - Photometric Interpretation
				// All the fieldtypes we ever need
				// 3 - SHORT
				// Count just declares how many

				// There may be multiple widths and heights.
				// Hopefully they will all be the same in your file
				// Width
				if ( Tag == 256 && FieldType == 3 && Count == 1 )
				{
					ImageInMemory->nImageWidth = (unsigned int)static_cast<uint32_t>( Value );
				}
				// Height
				else if ( Tag == 257 && FieldType == 3 && Count == 1 )
				{
					ImageInMemory->nImageHeight = (unsigned int)static_cast<uint32_t>( Value );
				}
				// Bits Per Sample
				else if ( Tag == 258 && FieldType == 3 && Count > 0 )
				{
					// BitsPerSample tag (Tag 258), FieldType 3 (SHORT), Count > 0
					TIFF_File.seekg( IFDs[i].aIFD[j].nOffsetOrValue, std::ios::beg );
					std::vector<uint16_t> nBitsPerSample( IFDs[i].aIFD[j].nCount );
					TIFF_File.read( reinterpret_cast<char *>( nBitsPerSample.data() ), IFDs[i].aIFD[j].nCount * sizeof( uint16_t ) );

					std::cout << "BitsPerSample values: ";
					for ( uint16_t value : nBitsPerSample )
					{
						std::cout << value << " ";
					}
					std::cout << std::endl;

					if ( !nBitsPerSample.empty() )
					{
						// bits per sample refers to the number of bits used to
						// represent the intensity of color information in a single channel
						// In other words, bits per channel
						nBitsPerChannel = static_cast<int>( nBitsPerSample[0] );
					}
				}
				// Compression Scheme
				else if ( Tag == 259 && FieldType == 3 && Count == 1 )
				{
					switch ( IFDs[i].aIFD[j].nOffsetOrValue )
					{
						case 1:
							Compression = COMPRESSION_NONE;
							break;

						case 2:
							Compression = COMPRESSION_CCITT_GROUP3_1D;
							break;

						case 3:
							Compression = COMPRESSION_CCITT_GROUP3_2D;
							break;

						case4:
							Compression = COMPRESSION_CCITT_GROUP3_4D;
							break;

						case 5:
							Compression = COMPRESSION_LZW;
							break;

						case 6:
							Compression = COMPRESSION_JPEG_OLD;
							break;

						case 7:
							Compression = COMPRESSION_JPEG;
							break;

						case 8:
							Compression = COMPRESSION_DEFLATE;
							break;

						case 32946:
							Compression = COMPRESSION_DEFLATE_ITK_GDCM;
							break;

						case 32773:
							Compression = COMPRESSION_PACKBITS;
							break;
					}
				}
				// Photometric interpretation
				else if ( Tag == 262 && FieldType == 3 && Count == 1 )
				{
					bool bStore;
					if ( nPhotometricInterpretation == 2 )
						bStore = true;

					// TODO: This code makes no sense but it should work
					// Like what, read something as uint16 regardless?
					// If the value fits in the offset, use it directly
					if ( Count * sizeof( uint16_t ) <= sizeof( Value ) )
					{
						nPhotometricInterpretation = static_cast<uint16_t>( Value );
					}
					else
					{
						// Seek to the offset and read the value
						TIFF_File.seekg( Value, std::ios::beg );
						TIFF_File.read( reinterpret_cast<char *>( &nPhotometricInterpretation ), sizeof( uint16_t ) );
					}

					if ( nPhotometricInterpretation == 4 )
						bHasAlpha = true;

					if ( bStore )
						nPhotometricInterpretation = 2;
				}
			}
		}

		if ( Compression != COMPRESSION_NONE )
		{
			std::cout << "TIFF File with Compression was parsed, this is not supported." << std::endl;
			std::cout << "This will memory leak the data from this tif, because I haven't written a dedicated function for cleanup" << std::endl;
		}

		// Lets find out how many bits we have to deal with
		// RGB * Bits + Alpha * Bits
		ImageInMemory->nBitCountPerPixel = nBitsPerChannel * 3 + bHasAlpha * nBitsPerChannel;
		ImageInMemory->nBitCountPerChannel = nBitsPerChannel; // This is straight forward on TIFF
		ImageInMemory->bHasAlpha = bHasAlpha;

		ImageInMemory->nCountChannels = bHasAlpha ? 4 : 3;

		// Now that we know, allocate the memory for the data
		if ( nBitsPerChannel == 8 )
			ImageInMemory->pImageData = new char[ImageInMemory->nImageWidth * ImageInMemory->nImageHeight * ImageInMemory->nBitCountPerChannel];
		else if ( nBitsPerChannel == 16 ) // 16f
			ImageInMemory->pImageData16f = new uint16_t[ImageInMemory->nImageWidth * ImageInMemory->nImageHeight * ImageInMemory->nBitCountPerChannel];
		else if ( nBitsPerChannel == 32 ) // 32f
			ImageInMemory->pImageData32f = new float[ImageInMemory->nImageWidth * ImageInMemory->nImageHeight * ImageInMemory->nBitCountPerChannel];

		// We only want to read one layer in for both alpha and rgb in total.
		// Not sure what to do with multiple layers
		bool bReadRGB = false;
		bool bReadAlpha = false;
		uint16_t TextureType = 0;

		// For each IFD...
		for ( int i = 0; i < nIFDs; i++ )
		{
			bool bHasData;
			bool bIsAlpha;

			// Check for StripOffsets and StripByteCounts Tags
			// Each IFD may only have one
			uint32_t nStripOffsets = 0;
			uint32_t nStripBytes = 0;

			// For each IFD Entry
			for ( int j = 0; j < IFDs[i].nEntryCount; j++ )
			{
				// Convert to easier to read format
				uint16_t Tag = IFDs[i].aIFD[j].nTag;
				uint16_t FieldType = IFDs[i].aIFD[j].nFieldType;
				uint32_t Count = IFDs[i].aIFD[j].nCount;
				uint32_t Value = IFDs[i].aIFD[j].nOffsetOrValue;

				if ( Tag == 262 && FieldType == 3 && Count == 1 )
				{
					// This IFD contains image data
					bHasData = true;

					// TODO: This code makes no sense but it should work
					// Like what, read something as uint16 regardless?
					// If the value fits in the offset, use it directly
					if ( Count * sizeof( uint16_t ) <= sizeof( Value ) )
					{
						TextureType = static_cast<uint16_t>( Value );
					}
					else
					{
						// Seek to the offset and read the value
						TIFF_File.seekg( Value, std::ios::beg );
						TIFF_File.read( reinterpret_cast<char *>( &TextureType ), sizeof( uint16_t ) );
					}

					// This IFD has Alpha Information? Yes:No
					bIsAlpha = TextureType == 4 ? true : false;
				}
				else if ( Tag == 273 )
				{
					nStripOffsets = Value;
				}
				else if ( Tag == 279 )
				{
					nStripBytes = Value;
				}
			}

			// We know where the image data is now and can read it
			if ( ( nStripOffsets != 0 ) && ( nStripBytes != 0 ) && bHasData )
			{
				// Move there first otherwise we read from random parts of the file
				TIFF_File.seekg( nStripOffsets, std::ios::beg );

				if ( bIsAlpha && !bReadAlpha )
				{
					// Debugging only
					if ( !ImageInMemory->bHasAlpha )
					{
						std::cout << "Found Alpha Strip without finding photometric interpretation for it. Something is wrong." << std::endl;
						continue;
					}

					// We will write the alpha after the rgb data. That way we don't have to mix the two dynamically
					// We can just fish it out again later when we need it. You just need to account for this when the input format is TIFF
					size_t RGBSizeOffset = ImageInMemory->nImageWidth * ImageInMemory->nImageHeight * 3;

					// 8f, 16f, 32f
					if ( ImageInMemory->nBitCountPerChannel == 8 )
						TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData + RGBSizeOffset * sizeof( char ) ), nStripBytes );
					else if ( ImageInMemory->nBitCountPerChannel == 16 )
						TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData16f + RGBSizeOffset * sizeof( uint16_t ) ), nStripBytes );
					else if ( ImageInMemory->nBitCountPerChannel == 32 )
						TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData32f + RGBSizeOffset * sizeof( float ) ), nStripBytes );

					// We now have the data stop checking again
					bReadAlpha = true;
				}
				else if ( !bReadRGB ) // RGB Data
				{
					// 8f, 16f, 32f
					if ( ImageInMemory->nBitCountPerChannel == 8 )
						TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData ), nStripBytes );
					else if ( ImageInMemory->nBitCountPerChannel == 16 )
						TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData16f ), nStripBytes );
					else if ( ImageInMemory->nBitCountPerChannel == 32 )
						TIFF_File.read( reinterpret_cast<char *>( ImageInMemory->pImageData32f ), nStripBytes );

					std::cout << "successfully found rgb image data in tif file" << std::endl;

					// We now have the data stop checking again
					bReadRGB = true;
				}
			}
		}

		std::cout << "Successfully read " << ccFileName << std::endl;
		std::cout << "I hope." << std::endl;

		TIFF_File.close();
		return true;
	}
	else
	{
		std::cout << "Error 002 : TIFF File not open, for whatever reason." << std::endl;
		std::cout << "Name of the file as recognised by this software : " << ccFileName << std::endl;
		TIFF_File.close();
		return false;
	}
}