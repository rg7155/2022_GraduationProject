#pragma once
#include "Client_Desert_Server.h"
constexpr float DAMAGE_COOLTIME = 1.2f;

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
	bool IsNowAttack();
	void Move(XMFLOAT3& xmf3Shift);

public:
	virtual void Send_Packet_To_Clients(int c_id) {};
	virtual void CheckCollision(int c_id) {};
	bool BoundingBox_Intersect(int c_id);
	void UpdateBoundingBox();

public:
	char			m_race;
	int				m_hp, m_hpmax;
	XMFLOAT4X4		m_xmf4x4World;
	XMFLOAT3		m_xmf3Target;
	object_anim		m_eAnimInfo[10];
	int				m_eCurAnim;


public:
	bool					m_bActive;
	float					m_fDieCoolTime;

public:
	BoundingOrientedBox		m_xmLocalOOBB;
	BoundingOrientedBox		m_xmOOBB;



};

