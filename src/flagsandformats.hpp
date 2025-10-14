#pragma once
#define STB_IMAGE_STATIC
#include <vtfpp/vtfpp.h>

inline std::array<std::string_view, 32> getPrettyFlagNamesForFlags( uint16_t minorVersion, vtfpp::VTF::Platform platform )
{
	std::array<std::string_view, 32> flags {
		"Point Sample",
		"Trilinear",
		"Clamp S",
		"Clamp T",
		"Anisotropic",
		"(vtex) Hint DXT5",
		"(vtex) No Compress",
		"Normal",
		"No Mips",
		"No LOD",
		"Load Small Mips",
		"Procedural",
		"One-bit Alpha",
		"Multi-bit Alpha",
		"Envmap",
		"Unused (1<<15)",
		"Unused (1<<16)",
		"Unused (1<<17)",
		"Unused (1<<18)",
		"Unused (1<<19)",
		"Unused (1<<20)",
		"Unused (1<<21)",
		"Unused (1<<22)",
		"Unused (1<<23)",
		"Unused (1<<24)",
		"Unused (1<<25)",
		"Unused (1<<26)",
		"Unused (1<<27)",
		"Unused (1<<28)",
		"Unused (1<<29)",
		"Unused (1<<30)",
		"Unused (1<<31)",
	};
	if ( minorVersion >= 1 )
	{
		flags[15] = "Rendertarget";
		flags[16] = "Depth Rendertarget";
		flags[17] = "No Debug Override";
		flags[18] = "Single Copy";
		flags[19] = "(vtex) One Over Mip Level in Alpha";
		flags[20] = "(vtex) Premultiply Color by One Over Mip Level in Alpha";
		flags[21] = "(vtex) Convert Normal to DUDV";
	}
	if ( minorVersion >= 2 )
	{
		flags[22] = "(vtex) Alpha Test Mip Generation";
		flags[23] = "No Depth Buffer";
		flags[24] = "(vtex) NICE Filtered";
		flags[25] = "Clamp U";
		if ( platform == vtfpp::VTF::Platform::PLATFORM_XBOX )
		{
			flags[26] = "[XBOX] (vtex) Preswizzled";
			flags[27] = "[XBOX] Cacheable";
			flags[28] = "[XBOX] Unfilterable OK";
		}
	}
	if ( minorVersion >= 3 )
	{
		flags[10] = "Load All Mips";
		flags[26] = "Vertex Texture";
		flags[27] = "SSBump";
		flags[28] = "Unused (1<<28)";
		flags[29] = "Border";
	}
	if ( minorVersion >= 4 )
	{
		flags[5] = "Unused (1<<5)";
		flags[6] = "sRGB";
		flags[19] = "[TF2] Staging Memory";
		flags[20] = "[TF2] Immediate Cleanup";
		flags[21] = "[TF2] Ignore mat_picmip";
		flags[22] = "Unused (1<<22)";
		flags[24] = "Unused (1<<24)";
		flags[30] = "[TF2] Streamable (Coarse)";
	}
	if ( minorVersion >= 5 )
	{
		flags[6] = "PWL Corrected";
		flags[10] = "Unused (1<<10)";
		flags[19] = "sRGB";
		flags[20] = "Default Pool";
		flags[21] = "[CS:GO] Combined";
		flags[22] = "[CS:GO] Async Download";
		flags[24] = "[CS:GO] Skip Initial Download";
		flags[28] = "Load Most Mips";
		flags[30] = "[CS:GO] YCoCg";
		flags[31] = "[CS:GO] Async Skip Initial Low Res";
	}
	if ( minorVersion >= 6 )
	{
		flags[21] = "Combined";
		flags[22] = "Async Download";
		flags[24] = "Skip Initial Download";
		flags[30] = "YCoCg";
		flags[31] = "Async Skip Initial Low Res";
	}
	return flags;
}
// constexpr struct TextureFlag
//{
//	uint32_t flag;
//	const char *name;
// } TEXTURE_FLAGS[] = {
//	{ vtfpp::VTF::FlagsV0::FLAG_POINT_SAMPLE, "Point Sample" },
//	{ vtfpp::VTF::FlagsV0::FLAG_TRILINEAR, "Trilinear" },
//	{ vtfpp::VTF::FlagsV0::FLAG_CLAMP_S, "Clamp S" },
//	{ vtfpp::VTF::FlagsV0::FLAG_CLAMP_T, "Clamp T" },
//	{ vtfpp::VTF::FLAG_V2_CLAMP_U, "Clamp U" },
//	{ vtfpp::VTF::FlagsV0::FLAG_ANISOTROPIC, "Anisotropic" },
//	{ vtfpp::VTF::FlagsV0::FLAG_HINT_DXT5, "Hint DXT5" },
//	{ vtfpp::VTF::FLAG_V5_SRGB, "sRGB" },
//	//	{ vtfpp::VTF::FlagsV0::FLAG_NO_COMPRESS, "Nocompress (Deprecated)" },
//	{ vtfpp::VTF::FlagsV0::FLAG_NORMAL, "Normal" },
//	{ vtfpp::VTF::FlagsV0::FLAG_NO_MIP, "No MIP" },
//	{ vtfpp::VTF::FlagsV0::FLAG_NO_LOD, "No LOD" },
//	{ vtfpp::VTF::FLAG_V5_LOAD_MOST_MIPS, "Min Mip" },
//	{ vtfpp::VTF::FlagsV0::FLAG_PROCEDURAL, "Procedural" },
//	{ vtfpp::VTF::FlagsV0::FLAG_ONE_BIT_ALPHA, "One-bit Alpha" },
//	{ vtfpp::VTF::FlagsV0::FLAG_MULTI_BIT_ALPHA, "Multi-bit Alpha" },
//	{ vtfpp::VTF::FlagsV0::FLAG_ENVMAP, "Envmap" },
//	{ vtfpp::VTF::FlagsV0::FLAG_RENDERTARGET, "Render Target" },
//	{ vtfpp::VTF::FlagsV0::FLAG_DEPTH_RENDERTARGET, "Depth Render Target" },
//	{ vtfpp::VTF::FlagsV0::FLAG_NO_DEBUG_OVERRIDE, "No Debug Override" },
//	{ vtfpp::VTF::FlagsV0::FLAG_SINGLE_COPY, "Single Copy" },
//	//{ vtfpp::VTF::FlagsV0::FLAG_ONE_OVER_MIP_LEVEL_IN_ALPHA, "One Over Mip Level Linear Alpha (Deprecated)" },
//	//{ vtfpp::VTF::FlagsV0::FLAG_PREMULTIPLY_COLOR_BY_ONE_OVER_MIP_LEVEL, "Pre-multiply Colors by One Over Mip Level (Deprecated)" },
//	//	{ vtfpp::VTF::FlagsV0::FLAG_NORMAL_TO_DUDV, "Normal To DuDv" },
//	//{ vtfpp::VTF::FlagsV0::FLAG_ALPHA_TEST_MIP_GENERATION, "Alpha Test Mip Generation (Deprecated)" },
//	{ vtfpp::VTF::FLAG_V2_NO_DEPTH_BUFFER, "No Depth Buffer" },
//	//{ vtfpp::VTF::FlagsV0::FLAG_NICE_FILTERED, "Nice Filtered (Deprecated)" },
//	{ vtfpp::VTF::FLAG_V3_VERTEX_TEXTURE, "Vertex Texture" } };
//	{ vtfpp::VTF::FlagsV0::FLAG_SSBUMP, "SSBump" },
//	{ vtfpp::VTF::FlagsV0::FLAG_UNFILTERABLE_OK, "Unfilterable OK (Deprecated)" },
//	{ vtfpp::VTF::FlagsV0::FLAG_BORDER, "Border" },
//{ vtfpp::VTF::FlagsV0::FLAG_SPECVAR_RED, "Specvar Red (Deprecated)" },
//{ vtfpp::VTF::FlagsV0::FLAG_SPECVAR_ALPHA, "Specvar Alpha (Deprecated)" },
//	{ vtfpp::VTF::FlagsV0::FLAG_ONE_OVER_MIP_LEVEL_IN_ALPHA, "Unused 0" },
//	{ vtfpp::VTF::FlagsV0::FLAG_PREMULTIPLY_COLOR_BY_ONE_OVER_MIP_LEVEL, "Unused 1" },
//	{ vtfpp::VTF::FlagsV0::FLAG_NORMAL_TO_DUDV, "Unused 2" },
//	{ vtfpp::VTF::FlagsV0::FLAG_ALPHA_TEST_MIP_GENERATION, "Unused 3" },
//	{ vtfpp::VTF::FlagsV0::FLAG_NICE_FILTERED, "Unused 4" },
//	{ vtfpp::VTF::FlagsV0::FLAG_UNFILTERABLE_OK, "Unused 5" },
//	{ vtfpp::VTF::FlagsV0::FLAG_SPECVAR_RED, "Unused 6" },
//	{ vtfpp::VTF::FlagsV0::FLAG_SPECVAR_ALPHA, "Unused 7" } };

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
	{ vtfpp::ImageFormat::BC6H, "BC6H" },
	{ vtfpp::ImageFormat::BC7, "BC7" } };
// TODO: Add strata specific new format.

static inline constexpr const char *FILE_FIELDS[] = { "Size", "Version", "Compression Type", "Compression Level" };

static inline constexpr const char *INFO_FIELDS[] = {
	"Width", "Height", "Depth", "Frames", "Faces", "Mips", "Reflectivity" };
