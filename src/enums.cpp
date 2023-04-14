
#include "enums.hpp"

const char *GetResourceName( vlUInt resource )
{
	switch ( resource )
	{
		case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
			return "Low-res Image (Legacy)";
		case VTF_LEGACY_RSRC_IMAGE:
			return "Image (Legacy)";
		case VTF_RSRC_SHEET:
			return "Sheet";
		case VTF_RSRC_CRC:
			return "CRC";
		case VTF_RSRC_TEXTURE_LOD_SETTINGS:
			return "Texture LOD Settings";
		case VTF_RSRC_TEXTURE_SETTINGS_EX:
			return "Texture Settings Extended";
		case VTF_RSRC_KEY_VALUE_DATA:
			return "KeyValue Data";
		default:
			return "";
	}
}
