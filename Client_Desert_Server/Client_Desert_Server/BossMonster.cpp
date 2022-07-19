#include "BossMonster.h"
#include "Session.h"

CBossMonster::CBossMonster()
{
	//SetScale(2.f, 2.f, 2.f);
	m_bActive = true;
	m_eCurAnim = BOSS::ANIM::IDLE;
	m_fAnimMaxTime = animTimes["Boss"][m_eCurAnim];
	
	m_hp = 100;
	SetPosition(BOSS_POS_INIT.x, BOSS_POS_INIT.y, BOSS_POS_INIT.z);
	SetScale(1.2f, 1.2f, 1.2f);

	m_fAnimElapsedTime = 0.f;
	m_fAttackCoolTime = 0.f;
	m_nowVerse = VERSE1;
	m_ePreAttack = BOSS::ATTACK2;
	m_targetId = 0;
	XMFLOAT3 playerPos = clients[m_targetId]._pObject->GetPosition();
	m_xmf3Target = playerPos;
}

CBossMonster::~CBossMonster()
{
}

void CBossMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;

	if (m_eCurAnim == CACTUS::DIE) {
		m_bActive = false;
		return;
	}

	m_fAnimElapsedTime += fTimeElapsed;
	m_fDamagedCoolTime += fTimeElapsed;

	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		m_fAnimElapsedTime = 0.f;
		Change_Animation(BOSS::ANIM::IDLE);
	}

	if (m_nowVerse == VERSE1) {
		// 두 플레이어와 거리 비교해서 BOSS_ATTACK_START_DISTANCE보다 작으면 verse바꾸기
		for (auto& client : clients)
		{
			XMFLOAT3 pos = client.second._pObject->GetPosition();
			if (Vector3::Distance(pos, GetPosition()) > BOSS_ATTACK_START_DISTANCE) {
				return;
			}
		}
		m_nowVerse = VERSE2;
	}
	else if (m_nowVerse == VERSE2)
	{
		XMFLOAT3 playerPos = clients[m_targetId]._pObject->GetPosition();
		m_xmf3Target = playerPos;

		m_fAttackCoolTime += fTimeElapsed;
		if (m_fAttackCoolTime > ATTACK_COOLTIME)
		{
			m_fAttackCoolTime = 0.f;
			if (BOSS::ATTACK2 == m_ePreAttack) {
				Change_Animation(BOSS::ANIM::ATTACK1);
				m_ePreAttack = BOSS::ATTACK1;
			}
			else {
				Change_Animation(BOSS::ANIM::ATTACK2);
				m_ePreAttack = BOSS::ATTACK2;
			}
		}
	}
}

void CBossMonster::Send_Packet_To_Clients(int c_id)
{
	SC_MOVE_MONSTER_PACKET p;
	p.id = 0;
	p.type = SC_MOVE_MONSTER;
	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
	p.eCurAnim = m_eCurAnim;
	p.xmf3Position = GetPosition();
	p.xmf3Look = m_xmf3Target;
	p.hp = m_hp;
	p.race = RACE_BOSS;
	p.verse = m_nowVerse;
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CBossMonster::Send_Remove_Packet_To_Clients(int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.type = SC_REMOVE_OBJECT;
	p.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	p.race = RACE_BOSS;
	p.id = 0; // 0
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CBossMonster::CheckCollision(int c_id)
{
	if (m_eCurAnim == CACTUS::ANIM::TAKE_DAMAGED)
		return;


	if (BoundingBox_Intersect(c_id) && m_fDamagedCoolTime > DAMAGE_COOLTIME && m_hp > 0)
	{
		m_hp -= 20.f;
		m_fDamagedCoolTime = 0.f;

		if (m_hp <= 0.f)
		{
			Change_Animation(BOSS::ANIM::DIE);
			return;
		}
		else
		{
			Change_Animation(BOSS::ANIM::TAKE_DAMAGED);
		}
	}
}

void CBossMonster::Change_Animation(BOSS::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = animTimes["Boss"][eNewAnim];
	m_eCurAnim = eNewAnim;
}
