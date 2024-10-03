#pragma once
#define STB_IMAGE_STATIC
#include <vtfpp/vtfpp.h>

constexpr struct TextureFlag
{
	vtfpp::VTF::Flags flag;
	const char *name;
} TEXTURE_FLAGS[] = {
	{ vtfpp::VTF::Flags::FLAG_POINT_SAMPLE, "Point Sample" },
	{ vtfpp::VTF::Flags::FLAG_TRILINEAR, "Trilinear" },
	{ vtfpp::VTF::Flags::FLAG_CLAMP_S, "Clamp S" },
	{ vtfpp::VTF::Flags::FLAG_CLAMP_T, "Clamp T" },
	{ vtfpp::VTF::Flags::FLAG_CLAMP_U, "Clamp U" },
	{ vtfpp::VTF::Flags::FLAG_ANISOTROPIC, "Anisotropic" },
	{ vtfpp::VTF::Flags::FLAG_HINT_DXT5, "Hint DXT5" },
	{ vtfpp::VTF::Flags::FLAG_SRGB, "sRGB" },
	{ vtfpp::VTF::Flags::FLAG_NO_COMPRESS, "Nocompress (Deprecated)" },
	{ vtfpp::VTF::Flags::FLAG_NORMAL, "Normal" },
	{ vtfpp::VTF::Flags::FLAG_NO_MIP, "No MIP" },
	{ vtfpp::VTF::Flags::FLAG_NO_LOD, "No LOD" },
	{ vtfpp::VTF::Flags::FLAG_MIN_MIP, "Min Mip" },
	{ vtfpp::VTF::Flags::FLAG_PROCEDURAL, "Procedural" },
	{ vtfpp::VTF::Flags::FLAG_ONE_BIT_ALPHA, "One-bit Alpha" },
	{ vtfpp::VTF::Flags::FLAG_MULTI_BIT_ALPHA, "Multi-bit Alpha" },
	{ vtfpp::VTF::Flags::FLAG_ENVMAP, "Envmap" },
	{ vtfpp::VTF::Flags::FLAG_RENDERTARGET, "Render Target" },
	{ vtfpp::VTF::Flags::FLAG_DEPTH_RENDERTARGET, "Depth Render Target" },
	{ vtfpp::VTF::Flags::FLAG_NO_DEBUG_OVERRIDE, "No Debug Override" },
	{ vtfpp::VTF::Flags::FLAG_SINGLE_COPY, "Single Copy" },
	//{ vtfpp::VTF::Flags::FLAG_ONE_OVER_MIP_LEVEL_IN_ALPHA, "One Over Mip Level Linear Alpha (Deprecated)" },
	//{ vtfpp::VTF::Flags::FLAG_PREMULTIPLY_COLOR_BY_ONE_OVER_MIP_LEVEL, "Pre-multiply Colors by One Over Mip Level (Deprecated)" },
	//	{ vtfpp::VTF::Flags::FLAG_NORMAL_TO_DUDV, "Normal To DuDv" },
	//{ vtfpp::VTF::Flags::FLAG_ALPHA_TEST_MIP_GENERATION, "Alpha Test Mip Generation (Deprecated)" },
	{ vtfpp::VTF::Flags::FLAG_NO_DEPTH_BUFFER, "No Depth Buffer" },
	//{ vtfpp::VTF::Flags::FLAG_NICE_FILTERED, "Nice Filtered (Deprecated)" },
	{ vtfpp::VTF::Flags::FLAG_VERTEX_TEXTURE, "Vertex Texture" },
	{ vtfpp::VTF::Flags::FLAG_SSBUMP, "SSBump" },
	//	{ vtfpp::VTF::Flags::FLAG_UNFILTERABLE_OK, "Unfilterable OK (Deprecated)" },
	{ vtfpp::VTF::Flags::FLAG_BORDER, "Border" },
	//{ vtfpp::VTF::Flags::FLAG_SPECVAR_RED, "Specvar Red (Deprecated)" },
	//{ vtfpp::VTF::Flags::FLAG_SPECVAR_ALPHA, "Specvar Alpha (Deprecated)" },
	{ vtfpp::VTF::Flags::FLAG_ONE_OVER_MIP_LEVEL_IN_ALPHA, "Unused 0" },
	{ vtfpp::VTF::Flags::FLAG_PREMULTIPLY_COLOR_BY_ONE_OVER_MIP_LEVEL, "Unused 1" },
	{ vtfpp::VTF::Flags::FLAG_NORMAL_TO_DUDV, "Unused 2" },
	{ vtfpp::VTF::Flags::FLAG_ALPHA_TEST_MIP_GENERATION, "Unused 3" },
	{ vtfpp::VTF::Flags::FLAG_NICE_FILTERED, "Unused 4" },
	{ vtfpp::VTF::Flags::FLAG_UNFILTERABLE_OK, "Unused 5" },
	{ vtfpp::VTF::Flags::FLAG_SPECVAR_RED, "Unused 6" },
	{ vtfpp::VTF::Flags::FLAG_SPECVAR_ALPHA, "Unused 7" } };

static inline constexpr struct
{
	vtfpp::ImageFormat format;
	const char *name;
} IMAGE_FORMATS[] = {
	{ vtfpp::ImageFormat::RGBA8888, "RGBA8888" },
	{ vtfpp::ImageFormat::ABGR8888, "ABGR8888" },
	{ vtfpp::ImageFormat::RGB888, "RGB888" },
	{ vtfpp::ImageFormat::BGR888, "BGR888" },
	{ vtfpp::ImageFormat::RGB565, "RGB565" },
	{ vtfpp::ImageFormat::I8, "I8" },
	{ vtfpp::ImageFormat::IA88, "IA88" },
	{ vtfpp::ImageFormat::P8, "P8" },
	{ vtfpp::ImageFormat::A8, "A8" },
	{ vtfpp::ImageFormat::RGB888_BLUESCREEN, "RGB888_BLUESCREEN" },
	{ vtfpp::ImageFormat::BGR888_BLUESCREEN, "BGR888_BLUESCREEN" },
	{ vtfpp::ImageFormat::ARGB8888, "ARGB8888" },
	{ vtfpp::ImageFormat::BGRA8888, "BGRA8888" },
	{ vtfpp::ImageFormat::DXT1, "DXT1" },
	{ vtfpp::ImageFormat::DXT3, "DXT3" },
	{ vtfpp::ImageFormat::DXT5, "DXT5" },
	{ vtfpp::ImageFormat::BGRX8888, "BGRX8888" },
	{ vtfpp::ImageFormat::BGR565, "BGR565" },
	{ vtfpp::ImageFormat::BGRX5551, "BGRX5551" },
	{ vtfpp::ImageFormat::BGRA4444, "BGRA4444" },
	{ vtfpp::ImageFormat::DXT1_ONE_BIT_ALPHA, "DXT1_ONEBITALPHA" },
	{ vtfpp::ImageFormat::BGRA5551, "BGRA5551" },
	{ vtfpp::ImageFormat::UV88, "UV88" },
	{ vtfpp::ImageFormat::UVWQ8888, "UVWQ8888" },
	{ vtfpp::ImageFormat::RGBA16161616F, "RGBA16161616F" },
	{ vtfpp::ImageFormat::RGBA16161616, "RGBA16161616" },
	{ vtfpp::ImageFormat::UVLX8888, "UVLX8888" },
	{ vtfpp::ImageFormat::R32F, "R32F" },
	{ vtfpp::ImageFormat::RGB323232F, "RGB323232F" },
	{ vtfpp::ImageFormat::RGBA32323232F, "RGBA32323232F" },
	//	{ vtfpp::ImageFormat::NV_DST16, "NV_DST16" },
	//	{ vtfpp::ImageFormat::NV_DST24, "NV_DST24" },
	//	{ vtfpp::ImageFormat::NV_INTZ, "NV_INTZ" },
	//	{ vtfpp::ImageFormat::NV_RAWZ, "NV_RAWZ" },
	//	{ vtfpp::ImageFormat::ATI_DST16, "ATI_DST16" },
	//	{ vtfpp::ImageFormat::ATI_DST24, "ATI_DST24" },
	{ vtfpp::ImageFormat::EMPTY, "NV_NULL" },
	{ vtfpp::ImageFormat::ATI2N, "ATI2N" },
	{ vtfpp::ImageFormat::ATI1N, "ATI1N" },
	//	{ vtfpp::ImageFormat::ATI2N_OLD, "ATI2N Old" },
	//	{ vtfpp::ImageFormat::ATI1N_OLD, "ATI1N Old" },
};

static inline constexpr const char *FILE_FIELDS[] = { "Size", "Version", "Compression Level" };

static inline constexpr const char *INFO_FIELDS[] = {
	"Width", "Height", "Depth", "Frames", "Faces", "Mips", "Reflectivity" };
