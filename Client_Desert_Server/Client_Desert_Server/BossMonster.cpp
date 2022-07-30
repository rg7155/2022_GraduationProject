#include "BossMonster.h"
#include "Session.h"

CBossMonster::CBossMonster()
{
	//SetScale(2.f, 2.f, 2.f);
	m_bActive = true;
	m_eCurAnim = BOSS::ANIM::IDLE;
	m_fAnimMaxTime = animTimes["Boss"][m_eCurAnim];
	
	m_hp = 2500;
	m_hpmax = m_hp;
	SetPosition(BOSS_POS_INIT.x, BOSS_POS_INIT.y, BOSS_POS_INIT.z);
	SetScale(0.8f, 0.8f, 0.8f);

	m_fAnimElapsedTime = 0.f;
	m_fAttackCoolTime = 0.f;
	m_nowVerse = VERSE1;
	m_ePreAttack = BOSS::ATTACK2;
	m_targetId = -1;

}

CBossMonster::~CBossMonster()
{
}

void CBossMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;

	if (m_eCurAnim == BOSS::DIE) {
		m_bActive = false;
		return;
	}

	m_fAnimElapsedTime += fTimeElapsed;
	m_fDamagedCoolTime += fTimeElapsed;

	// verse3는 피격 시 SPELL도 함
	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		m_fAnimElapsedTime = 0.f;
		if (VERSE3 == m_nowVerse) {
			if (BOSS::TAKE_DAMAGED == m_eCurAnim) {
				Change_Animation(BOSS::ANIM::SPELL);
			}
			else if (BOSS::SPELL == m_eCurAnim) {
				Change_Animation(BOSS::ANIM::IDLE);
				m_fIdleTime = 0.f;
			}
		}
		else
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
		// 처음 타겟 설정
		m_targetId = 0;
		m_nowVerse = VERSE2;
	}
	else if (m_nowVerse == VERSE2)
	{
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
			ChangeTarget();
		}
	}
	else if (m_nowVerse == VERSE3) {
		if (BOSS::IDLE == m_eCurAnim) {
			m_fIdleTime += fTimeElapsed;
			if (m_fIdleTime > IDLETIME) {
				Change_Animation(BOSS::ANIM::ATTACK3);
				m_fAttackCoolTime = 0.f;
				ChangeTarget();
			}
			return;
		}

		m_fAttackCoolTime += fTimeElapsed;
		if (m_fAttackCoolTime > DASHTIME) {
			m_fAttackCoolTime = 0.f;
			if(BOSS::IDLE != m_eCurAnim)
				Change_Animation(BOSS::ANIM::SPELL);
			return;
		}
		if (BOSS::ANIM::ATTACK3 == m_eCurAnim) {
			m_xmf3Look = Vector3::Subtract(m_xmf3Target, GetPosition(), true, true);
			XMVECTOR xmVecNormal = { m_xmf3Look.x,m_xmf3Look.y, m_xmf3Look.z };
			xmVecNormal *= fTimeElapsed * BOSS_SPEED;
			Move(Vector3::XMVectorToFloat3(xmVecNormal));	
		}

	}
	if (-1 != m_targetId) {
		for (auto& client : clients)
		{
			if (client.first == m_targetId) {
				XMFLOAT3 playerPos = client.second._pObject->GetPosition();
				m_xmf3Target = playerPos;
			}
		}

	}
	else
		Change_Animation(BOSS::ANIM::IDLE);


	
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
	p.attack_id = m_attackId;
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
	if (m_eCurAnim == BOSS::ANIM::TAKE_DAMAGED || m_eCurAnim == BOSS::ANIM::SPELL)
		return;

	bool bCol = BoundingBox_Intersect(c_id);

	CGameObject* pPlayer = clients[c_id]._pObject;

	// 플레이어가 공격
	if ((pPlayer->m_eCurAnim == PLAYER::ATTACK1 || pPlayer->m_eCurAnim == PLAYER::ATTACK2 ||
		pPlayer->m_eCurAnim == PLAYER::SKILL1 || pPlayer->m_eCurAnim == PLAYER::SKILL2) && CheckAttackAnimation(c_id)) {
		if (bCol && m_bColOn && m_hp > 0)
		{
			m_hp -= pPlayer->m_att;
			m_bColOn = false;
			m_attackId = c_id;

			if (CheckDamagedCoolTime())
			{
				if (m_hp <= 0)
				{
					m_hp = 0.f;
					Change_Animation(BOSS::ANIM::DIE);
					return;
				}
			}

			m_fDamagedCoolTime = 0.f;

			if (m_hp < (m_hpmax/2) && m_nowVerse == VERSE2)
			{
				m_nowVerse = VERSE3;

			}
			if (m_hp <= 0.f)
			{
				m_hp = 0.f;
				Change_Animation(BOSS::ANIM::DIE);
				return;
			}
			else
			{
				Change_Animation(BOSS::ANIM::TAKE_DAMAGED);

			}
		}
	}
	else if(BOSS::ATTACK1 == m_eCurAnim || BOSS::ATTACK2 == m_eCurAnim || BOSS::ATTACK3 == m_eCurAnim){ // 보스가 공격
		if (BOSS::ATTACK1 == m_eCurAnim || BOSS::ATTACK2 == m_eCurAnim) {
			bCol = BoundingBoxFront_Intersect(c_id, 2.f);
		}

		if (bCol) {
			if(BOSS::ATTACK3 == m_eCurAnim)
				Change_Animation(BOSS::ANIM::SPELL);
			clients[c_id].send_damaged_packet();
			m_targetId = 1 - m_targetId;

		}
		m_bColOn = true;
	}
	else
		m_bColOn = true;


}

void CBossMonster::Change_Animation(BOSS::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = animTimes["Boss"][eNewAnim];
	m_eCurAnim = eNewAnim;
}
