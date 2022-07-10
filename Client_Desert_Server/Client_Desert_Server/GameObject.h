#pragma once
#include "Client_Desert_Server.h"

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	virtual void Initialize() {};
	virtual void Update(float fTimeElapsed);

public:
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	void SetPosition(float x, float y, float z);
	void SetLook(float x, float y, float z);
	bool IsNowAttack();

public:
	virtual void Send_Packet_To_Clients(int c_id) {};
	virtual void CheckCollision(int c_id) {};
	bool BoundingBox_Intersect(int c_id);
	void UpdateBoundingBox();
public:
	char			race;
	int				hp, hpmax;
	XMFLOAT4X4		m_xmf4x4World;
	object_anim		m_eAnimInfo[10];
	int				m_eCurAnim;


public:
	bool					m_bActive;

public:
	BoundingOrientedBox		m_xmLocalOOBB;
	BoundingOrientedBox		m_xmOOBB;

};

