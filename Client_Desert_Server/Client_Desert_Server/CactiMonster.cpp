#include "CactiMonster.h"
#include "Session.h"

constexpr int VERSE1 = 1;
constexpr int VERSE2 = 2;
constexpr int VERSE3 = 3;

constexpr float CACTISPEED = 5.f;

CCactiMonster::CCactiMonster(int _index)
{
	m_nowVerse = VERSE1;
	m_index = _index;
	if (m_index == 0) {
		SetPosition(CACTI_POS_INIT1.x, CACTI_POS_INIT1.y, CACTI_POS_INIT1.z);
		m_fAfterPos = CACTI_POS_AFTER1;
	}
	else
	{
		SetPosition(CACTI_POS_INIT2.x, CACTI_POS_INIT2.y, CACTI_POS_INIT2.z);
		m_fAfterPos = CACTI_POS_AFTER2;
	}
	m_fAnimElapsedTime = 0.f;
	m_fDamagedCoolTime = 0.f;
	m_eCurAnim = CACTI::IDLE;
	m_hp = 100;

	m_bActive = true;
}

void CCactiMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;

	m_fAnimElapsedTime += fTimeElapsed;
	m_fDamagedCoolTime += fTimeElapsed;
	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		m_fAnimElapsedTime = 0.f;
		if (m_eCurAnim == CACTI::ANIM::BITE)
		{
			Change_Animation(CACTI::ANIM::WALK);

			m_nowVerse = VERSE2;
		}
		if (m_nowVerse == VERSE3) {
			Change_Animation(CACTI::ANIM::IDLE);

		}
	}
	if (VERSE2 == m_nowVerse) {
		XMFLOAT3 xmf3Pos = GetPosition();
		XMFLOAT3 xmf3Look = GetLook();
		XMFLOAT3 moveSize = xmf3Look;
		moveSize.x *= fTimeElapsed * CACTISPEED;
		moveSize.z *= fTimeElapsed * CACTISPEED;

		XMFLOAT3 newPos = Vector3::Add(xmf3Pos, moveSize);
		SetPosition(newPos.x, newPos.y, newPos.z);
		xmf3Pos = GetPosition();
		float dis = Vector3::Distance(xmf3Pos, m_fAfterPos);
		if (dis < 0.1f) {

			SetPosition(m_fAfterPos.x, m_fAfterPos.y, m_fAfterPos.z);
			m_nowVerse = VERSE3;
			Change_Animation(CACTI::IDLE);

			// 180도 회전 후 Cactus 생성하는 부분
			//Rotate(0.f, 180.f, 0.f);
			//if (m_pCacti && VERSE3 == static_cast<CMonsterObject*>(m_pCacti)->m_nowVerse) {
			//	m_pCactus->SetActiveState(true);
			//}
		}
	}
}

void CCactiMonster::Send_Packet_To_Clients(int c_id)
{
	SC_MOVE_MONSTER_PACKET p;
	p.id = m_index;
	p.type = SC_MOVE_MONSTER;
	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
	p.eCurAnim = m_eCurAnim;
	p.xmf3Position = GetPosition();
	p.xmf3Look = m_xmf3Target;
	p.hp = m_hp;
	p.race = RACE_CACTI;
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CCactiMonster::CheckCollision(int c_id) // 플레이어가 공격할때 호출
{
	if (m_fDamagedCoolTime < DAMAGE_COOLTIME || m_hp <= 0)
		return;

	if (BoundingBox_Intersect(c_id) && m_fDamagedCoolTime > DAMAGE_COOLTIME && m_hp > 0) {
		m_hp -= 20.f;
		m_fDamagedCoolTime = 0.f;
		if (m_nowVerse == VERSE1) {
			Change_Animation(CACTI::ANIM::BITE);
		}
	}

}

void CCactiMonster::Change_Animation(CACTI::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 1.f;
	m_eCurAnim = eNewAnim;
}
