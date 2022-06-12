#pragma once
#include "dy_asset.h"
#include "XIO.h"

class CUserPawn
{
public:
	CUserPawn();
	~CUserPawn();

	static void DrawAll();

	int m_id;

	xstr_t       m_username;
	vec3         m_color;
	dy_transform m_transform;

};