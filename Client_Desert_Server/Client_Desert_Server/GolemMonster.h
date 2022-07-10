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

	void Move(XMFLOAT3& xmf3Shift);
	virtual void CheckCollision(int c_id);
	void Change_Animation(GOLEM::ANIM eNewAnim);

public:
	int GetHp() { return m_iHp; }

public:
	int				m_targetId;
	XMFLOAT3		m_xmf3Look;

private:
	float			m_fAttackAnimTime;
	bool			m_bFollowStart;
private:
	float		m_fAnimMaxTime;			// ���� �ִϸ��̼��� ���� �ð�
	float		m_fAnimElapsedTime;		// ���� �ִϸ��̼��� �帥 �ð�

private:
	float		m_fRunCoolTime;
	float		m_fDamagedCoolTime;

private:
	int			m_iHp;
	int			m_iVerse;



};

