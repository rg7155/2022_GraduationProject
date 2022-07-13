#pragma once
#include "GameObject.h"

constexpr float CACTI_BULLET_TIME = 3.f;

class CCactiBullet : public CGameObject
{
public:
	CCactiBullet(int _index);
	~CCactiBullet();

public:
	virtual void Update(float fTimeElapsed);

public:
	virtual void Send_Packet_To_Clients(int c_id);
	virtual void CheckCollision(int c_id);

public:
	void	SetTarget(XMFLOAT3& xmf3Start, XMFLOAT3& xmf3Target, bool IsYFix = true);


public:
	XMFLOAT3	m_xmf3Target = {};
	float		m_fTime = 0;
	float		m_fSpeed;
	float		m_fCreateTime = 0.f;
	int			m_index = 0;
};

