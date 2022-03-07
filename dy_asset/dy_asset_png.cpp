#include "dy_texture.h"
#include <lodepng.h>

dy_texture* dy_asset_loadpng(const char* path)
{
	unsigned int w = 0;
	unsigned int h = 0;

	std::vector<unsigned char> pix;
	lodepng::decode(pix, w, h, path);

	return dy_texture_create(DY_TEXTURE_FORMAT_RGBA8, pix.data(), w, h);
}
