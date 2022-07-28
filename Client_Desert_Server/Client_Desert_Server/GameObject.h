#pragma once
#include "Client_Desert_Server.h"
constexpr float DAMAGE_COOLTIME = 2.f;
constexpr int VERSE1 = 0;
constexpr int VERSE2 = 1;
constexpr int VERSE3 = 2;
constexpr int VERSE4 = 3;

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
	void SetLookAt(XMFLOAT3& xmf3Target);
	void SetPosition(float x, float y, float z);
	void SetScale(float x, float y, float z);
	bool IsNowAttack();
	void Move(XMFLOAT3& xmf3Shift);

public:
	virtual void Send_Packet_To_Clients(int c_id) {};
	virtual void Send_Remove_Packet_To_Clients(int c_id) {};

	virtual void CheckCollision(int c_id) {};
	bool BoundingBox_Intersect(int c_id);
	bool BoundingBoxFront_Intersect(int c_id, float fDis);

	void UpdateBoundingBox();
	void ChangeTarget();

public:
	char			m_race;
	int				m_hp, m_hpmax;
	int				m_att;
	XMFLOAT4X4		m_xmf4x4World;
	XMFLOAT3		m_xmf3Target;
	object_anim		m_eAnimInfo[10];
	int				m_eCurAnim;
	XMFLOAT3		m_xmf3Look;

public:
	int				m_ePreColAnim;

public:
	bool					m_bActive;
	float					m_fDieCoolTime;
public:
	int				m_targetId;
public:
	BoundingOrientedBox		m_xmLocalOOBB;
	BoundingOrientedBox		m_xmOOBB;



};

