#include "dy_asset.h"
#include "dy_asset_obj.h"

#include <assert.h>
#include <unordered_map>
#include <string>

typedef int dy_asset_type;
enum : dy_asset_type
{
	DY_ASSET_MODEL,
	DY_ASSET_TEXTURE,
	
};

struct dy_asset
{
	dy_asset_type type;

	union
	{
		dy_model model;
		dy_texture* texture;
	};
};

// FIXME: std::string sucks!
static std::unordered_map<std::string, dy_asset*>* s_loadedAssets = nullptr;

void dy_asset_init()
{
	assert(s_loadedAssets == 0);
	s_loadedAssets = new std::unordered_map<std::string, dy_asset*>;

}

void dy_asset_shutdown()
{
	for (auto& p : *s_loadedAssets)
	{
		// FIXME: We're not freeing the actual data in the asset!
		if(p.second)
			delete p.second;
	}
	delete s_loadedAssets;
	s_loadedAssets = nullptr;
}



dy_model* dy_asset_model_load(const char* path)
{

	// FIXME: AAAAAA
	std::string s = std::string(path);
	auto f = s_loadedAssets->find(s);
	if (f != s_loadedAssets->end())
	{
		assert(f->second->type == DY_ASSET_MODEL);
		return &f->second->model;
	}

	dy_asset* asset = new dy_asset;
	asset->type = DY_ASSET_MODEL;
	
	// Right now we only have one type of model file supported
	// So let's just use it...
	asset->model = dy_asset_model_load_obj(path);

	// Remember this asset
	s_loadedAssets->insert({ s, asset });

	return &asset->model;
}


// FIXME: lol
extern dy_texture* dy_asset_loadpng(const char* path);
dy_texture* dy_asset_texture_load(const char* path)
{
	// FIXME: AAAAAA
	std::string s = std::string(path);
	auto f = s_loadedAssets->find(s);
	if (f != s_loadedAssets->end())
	{
		assert(f->second->type == DY_ASSET_TEXTURE);
		return &f->second->texture;
	}

	dy_asset* asset = new dy_asset;
	asset->type = DY_ASSET_TEXTURE;
	
	// Right now we only have one type of texture file supported
	// So let's just use it...
	asset->texture = dy_asset_loadpng(path);

	// Remember this asset
	s_loadedAssets->insert({ s, asset });

	return asset->texture;
}



void dy_asset_model_draw(dy_model* model)
{
	for (int i = 0; i < model->count; i++)
		dy_render_draw_mesh(&model->meshes[i]);
}
