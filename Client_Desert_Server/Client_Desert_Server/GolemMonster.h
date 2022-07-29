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
	float		m_fAnimMaxTime;			// ���� �ִϸ��̼��� ���� �ð�
	float		m_fAnimElapsedTime;		// ���� �ִϸ��̼��� �帥 �ð�

private:
	float		m_fRunCoolTime;

};

