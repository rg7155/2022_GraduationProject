#pragma once
#include "GameObject.h"

class CGolemMonster : public CGameObject
{
public:
	CGolemMonster(int _targetId);
	~CGolemMonster() {}


public:
	virtual void Update(float fTimeElapsed);
	virtual void Send_Packet_To_Clients(int c_id);
	virtual void CheckCollision(int c_id);

public:
	void Change_Animation(GOLEM::ANIM eNewAnim);

private:
	float			m_fAttackAnimTime;
	bool			m_bFollowStart;
private:
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간

private:
	float		m_fRunCoolTime;

};

