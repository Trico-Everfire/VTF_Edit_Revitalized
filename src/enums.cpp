
#include "enums.hpp"

const char *GetResourceName( uint32_t resource )
{
	switch ( resource )
	{
		case vtfpp::Resource::Type::TYPE_THUMBNAIL_DATA:
			return "Low-res Image (Thumbnail)";
		case vtfpp::Resource::Type::TYPE_IMAGE_DATA:
			return "Image (Base Data)";
		case vtfpp::Resource::Type::TYPE_PARTICLE_SHEET_DATA:
			return "Sprite Sheet";
		case vtfpp::Resource::Type::TYPE_CRC:
			return "CRC Data";
		case vtfpp::Resource::Type::TYPE_LOD_CONTROL_INFO:
			return "Texture LOD Settings";
		case vtfpp::Resource::Type::TYPE_EXTENDED_FLAGS:
			return "Texture Settings Extended";
		case vtfpp::Resource::Type::TYPE_KEYVALUES_DATA:
			return "KeyValue Data";
		case vtfpp::Resource::Type::TYPE_AUX_COMPRESSION:
			return "Aux Compression";
		default:
			return "";
	}
}
