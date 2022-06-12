#pragma once

#include <stdint.h>


// Somewhat arbitrary limits on paths to make life better and exploits harder
#define VPK_MAX_EXTENSION_LEN 128
#define VPK_MAX_DIRECTORY_LEN 128
#define VPK_MAX_FILETITLE_LEN 128


// PACK STRUCTS
// We need these structs packed with no padding
#ifdef _MSC_VER
#pragma pack(push, 1)
#elif defined(__GNUC__)
// lol
#define struct struct __attribute__((packed))
#else
#error UNSUPPORTED_PLATFORM
#endif

struct vpk_v1_header
{
	const uint32_t signature;
	const uint32_t version;

	uint32_t treedictsize;
};


struct vpk_v2_header
{
	const uint32_t signature;
	const uint32_t version;

	uint32_t treedict_size;

	uint32_t aa[4];
};


#define VPK_ARCHIVE_DATA_FOLLOWS 0x7FFF

struct vpk_direntry
{
	uint32_t crc;
	uint16_t preload_length;
	uint16_t archive_index; // If this is equal to VPK_ARCHIVE_DATA_FOLLOWS, our one element archive is right after this direntry
	uint32_t data_offset;   // Offset from start of archive
	uint32_t data_length;   // Length of the data
	const uint16_t terminator; // 0xFFFF
};

#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__GNUC__)
// lol
#undef struct
#else
#error UNSUPPORTED_PLATFORM
#endif
