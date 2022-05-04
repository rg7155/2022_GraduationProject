#pragma once
#include "Protocol.h"

class CPlayer
{

public:
	void Initialize();

public:
	//void Check_Move();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	void SetPosition(int x, int z);
	bool IsNowAttack();
public:
	XMFLOAT4X4 m_xmf4x4World;
	player_anim m_eAnimInfo[PLAYER::ANIM::END];
	int	m_id;
	PLAYER::ANIM m_eCurAnim;
};

