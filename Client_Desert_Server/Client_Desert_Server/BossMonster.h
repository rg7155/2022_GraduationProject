#pragma once
#include "GameObject.h"
constexpr auto BOSS_POS_INIT = XMFLOAT3(53.759f, 0.f, 60.787f);
constexpr auto ATTACK_COOLTIME = 3.f;

class CBossMonster : public CGameObject
{
public:
	CBossMonster();
	~CBossMonster();

public:
	virtual void Update(float fTimeElapsed);

public:
	virtual void Send_Packet_To_Clients(int c_id);
	virtual void Send_Remove_Packet_To_Clients(int c_id);

	virtual void CheckCollision(int c_id);

public:
	void Change_Animation(BOSS::ANIM eNewAnim);

private:
	float		m_fAnimMaxTime;			// ���� �ִϸ��̼��� ���� �ð�
	float		m_fAnimElapsedTime;		// ���� �ִϸ��̼��� �帥 �ð�
	float		m_fDamagedCoolTime;
	float		m_fAttackCoolTime;
	int			m_nowVerse;

	BOSS::ANIM	m_ePreAttack;

};
