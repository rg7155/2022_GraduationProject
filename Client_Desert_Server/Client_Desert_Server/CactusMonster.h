#pragma once
#include "GameObject.h"

constexpr auto CACTUS_POS_INIT = XMFLOAT3(127.f, 0.f, 100.f);
constexpr auto ATTACK_COOLTIME = 3.f;

class CCactiMonster;
class CCactusMonster : public CGameObject
{
public:
	CCactusMonster();
	~CCactusMonster();

public:
	virtual void Update(float fTimeElapsed);

public:
	virtual void Send_Packet_To_Clients(int c_id);
	virtual void CheckCollision(int c_id);

public:
	void Change_Animation(CACTUS::ANIM eNewAnim);
	void AddBullet();

private:
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float		m_fDamagedCoolTime;

	int			m_nowVerse;
public:
	CCactiMonster* m_pCacti1;
	CCactiMonster* m_pCacti2;

private:
	float				m_fAttackCoolTime;
	CACTUS::ANIM		m_ePreAttack;
};

