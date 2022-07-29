#pragma once
#include "GameObject.h"
constexpr auto CACTI_POS_INIT1 = XMFLOAT3(84.f, 0.f, 96.f);
constexpr auto CACTI_POS_INIT2 = XMFLOAT3(100.f, 0.f, 85.f);
constexpr auto CACTI_POS_AFTER1 = XMFLOAT3(127.f, 0.f, 105.f);
constexpr auto CACTI_POS_AFTER2 = XMFLOAT3(127.f, 0.f, 95.f);

class CCactusMonster;

class CCactiMonster : public CGameObject
{
public:
	CCactiMonster(int _index);
	~CCactiMonster() {}

public:
	virtual void Update(float fTimeElapsed);

public:
	virtual void Send_Packet_To_Clients(int c_id);
	virtual void Send_Remove_Packet_To_Clients(int c_id);
	virtual void CheckCollision(int c_id);

public:
	void Change_Animation(CACTI::ANIM eNewAnim);

public:
	void AttackProcess(CACTUS::ANIM eAnim);
	void AddBullet(int _id);
	void SetVerse(char _verse);

private:
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float		m_fRunCoolTime;
	int			m_nowVerse;

	XMFLOAT3	m_fAfterPos;

public:
	int			m_index;
	CCactiMonster*	m_pCacti;
	CCactusMonster* m_pCactus;

};

