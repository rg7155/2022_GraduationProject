#pragma once
#include "Protocol.h"

class CPlayer
{

public:
	void Initialize();

public:
	//void Check_Move();
	XMFLOAT3 GetPosition();
	void SetPosition(int x, int z);
public:
	XMFLOAT4X4 m_xmf4x4World;
	player_anim m_eAnimInfo[PLAYER::ANIM::END];

};

