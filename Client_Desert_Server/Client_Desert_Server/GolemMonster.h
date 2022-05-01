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
	void CheckCollision(CPlayer* pAttackPlayer);
	void Change_Animation(GOLEM::ANIM eNewAnim);

public:
	int GetHp() { return m_iHp; }
public:
	CPlayer*		m_pTarget;
	XMFLOAT3		m_xmf3Position;
	XMFLOAT3		m_xmf3Look;
	GOLEM::ANIM		m_eCurAnim;

private:
	float			m_fAttackAnimTime;

private:
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간

public:
	short		m_targetId;

private:
	float		m_fDamagedCoolTime;

private:
	int			m_iHp;


};

