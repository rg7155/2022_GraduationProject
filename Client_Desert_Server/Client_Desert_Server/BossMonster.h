#pragma once
#include "GameObject.h"
constexpr auto BOSS_POS_INIT = XMFLOAT3(53.759f, 0.f, 60.787f);
constexpr auto BOSS_ATTACK_START_DISTANCE = 15.f;
constexpr auto ATTACK_COOLTIME = 3.f;
constexpr auto DASHTIME = 3.f;
constexpr auto IDLETIME = 1.f;
constexpr auto BOSS_SPEED = 6.f;

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
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float		m_fDamagedCoolTime;
	float		m_fAttackCoolTime;
	float		m_fIdleTime;
	int			m_nowVerse;
	BOSS::ANIM	m_ePreAttack;


public:
	int				m_targetId;

};

