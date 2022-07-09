#pragma once
#include "Client_Desert_Server.h"

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	virtual void Initialize() {};

public:
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	void SetPosition(int x, int y, int z);
	bool IsNowAttack();


public:
	char			race;
	int				hp, hpmax;
	XMFLOAT4X4		m_xmf4x4World;
	object_anim		m_eAnimInfo[10];
	int				m_eCurAnim;
};

