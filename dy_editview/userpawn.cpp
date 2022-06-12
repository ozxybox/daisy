#include "userpawn.h"
#include "dy_debugtext.h"
#include "dy_engine.h"

// FIXME: bad
#include <unordered_map>
static int s_allUserPawnsIDSerial = 0;
static std::unordered_map<int, CUserPawn*> s_allUserPawns;


CUserPawn::CUserPawn()
{
	m_transform = { {0,0,0}, {0,0,0}, {32,32,32} };

	m_color = { 1,1,1 };
	m_username = XSTRL("unknown");

	// Track this pawn
	m_id = s_allUserPawnsIDSerial++;
	s_allUserPawns.insert({ m_id, this });
}

CUserPawn::~CUserPawn()
{

	// Untrack this pawn
	s_allUserPawns.erase(m_id);
}


#include "dy_editor.h"
extern dy_texture* s_redtexture;
extern dy_camera s_camera;

// TODO: We might want to just mush the two eye models together and just shift the eyes around using textures
//       Right now, moving the eyes with a static texture looks awful

void CUserPawn::DrawAll()
{
	// FIXME: yuck...
	static dy_model*   m_model     = dy_asset_model_load  ("C:/Dev/Calcifer/out/Debug/assets/binoculars.obj");
	static dy_model*   m_modeleyes = dy_asset_model_load  ("C:/Dev/Calcifer/out/Debug/assets/binoculars_eye.obj");
	static dy_texture* m_texmask   = dy_asset_texture_load("C:/Dev/Calcifer/out/Debug/assets/binoculars_mask.png");
	static dy_texture* m_texcolor  = dy_asset_texture_load("C:/Dev/Calcifer/out/Debug/assets/binoculars_color.png");
	static dy_texture* m_texeyes   = dy_asset_texture_load("C:/Dev/Calcifer/out/Debug/assets/eye_tileset.png");



	int tc = 0, tm = 1;
	dy_shader_set(DY_SHADERPARAM_TEXCOLOR, &tc);
	dy_shader_set(DY_SHADERPARAM_TEXMASK, &tm);
	dy_shader_set(DY_SHADERPARAM_MODEL, &m_model);


	unsigned int w = 0, h = 0;
	dy_texture_get_dimensions(m_texeyes, &w, &h);

	int square = w * 0.5;

	int eyepair = 1;

	float ey = eyepair * square / (float)h;
	float eh = square / (float)h;

	vec4 modeltexoffset    = { 0.0,0.0, 1.0,1.0 };
	vec4 righteyetexoffset = { 0.0, ey, 0.5, eh };
	vec4 lefteyetexoffset  = { 0, ey+0.5f, -0.5, -eh };

//	float t = dy_curtime();

	dy_transform tr;

	// Draw models
	for (auto& p : s_allUserPawns)
	{
		CUserPawn* pawn = p.second;

		mat4 m = pawn->m_transform.matrix();
		dy_shader_set(DY_SHADERPARAM_MODEL, &m);

		vec4 color = { pawn->m_color, 1 };
		dy_shader_set(DY_SHADERPARAM_COLOR, &color);
	
		// Body
		dy_texture_bind(m_texcolor, 0);
		dy_texture_bind(m_texmask, 1);
		dy_shader_set(DY_SHADERPARAM_TEXOFF, &modeltexoffset);
		dy_asset_model_draw(m_model);

		//vec3 off = { 8 * cosf(t), 0, 0 };

		// Eyes
		dy_texture_bind(m_texeyes, 0);
		dy_texture_bind(s_redtexture, 1);


		tr = pawn->m_transform;
		//tr.origin += off;
		m = tr.matrix();
		dy_shader_set(DY_SHADERPARAM_MODEL, &m);
		dy_shader_set(DY_SHADERPARAM_TEXOFF, &righteyetexoffset);
		dy_asset_model_draw(m_modeleyes);

		tr = pawn->m_transform;
		//tr.origin += off;
		tr.scale.x *= -1;
		tr.scale.y *= -1;
		m = tr.matrix();
		dy_shader_set(DY_SHADERPARAM_MODEL, &m);
		dy_shader_set(DY_SHADERPARAM_TEXOFF, &lefteyetexoffset);
		dy_asset_model_draw(m_modeleyes);

		
	}

	dy_shader_set(DY_SHADERPARAM_TEXOFF, &modeltexoffset);
	dy_texture_bind(s_redtexture, 1);
	
	// Draw name tags
	for (auto& p : s_allUserPawns)
	{
		CUserPawn* pawn = p.second;

		vec4 color = { 1 - pawn->m_color.x, 1 - pawn->m_color.y, 1 - pawn->m_color.z, 1 };
		dy_shader_set(DY_SHADERPARAM_COLOR, &color);
		
		dy_debugtext(pawn->m_username->str(), pawn->m_username->len, pawn->m_transform.origin + vec3{0,24,0}, s_camera.origin, 12);
	}
}
