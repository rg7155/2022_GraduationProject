#pragma once
#include "Protocol.h"

class CPlayer;
class CGolemMonster
{
public:
	CGolemMonster(CPlayer* _pTarget);
	~CGolemMonster() {}


public:
	void Update(float fTimeElapsed);
	void Move(XMFLOAT3& xmf3Shift);

public:
	CPlayer*		m_pTarget;
	XMFLOAT3		m_xmf3Position;
	XMFLOAT3		m_xmf3Look;
	GOLEM::ANIM		m_eCurAnim;

private:
	float			m_fAttackAnimTime;

public:
	char			send_buf[BUFSIZE];

};

