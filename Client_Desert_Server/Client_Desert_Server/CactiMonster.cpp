#include "CactiMonster.h"
#include "Session.h"
#include "CactusMonster.h"



constexpr float CACTISPEED = 7.f;
constexpr float RUNCOOLTIME = 3.f;

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
	m_fAnimMaxTime = animTimes["Cacti"][m_eCurAnim];

	m_hp = 100;
	SetLookAt(m_fAfterPos);
	SetScale(2.f, 2.f, 2.f);

	m_bActive = true;


}

void CCactiMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;

	m_fAnimElapsedTime += fTimeElapsed;
	m_fDamagedCoolTime += fTimeElapsed;
	m_fRunCoolTime += fTimeElapsed;

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
		XMFLOAT3 xmf3Look =GetLook();
		xmf3Look.x *= fTimeElapsed * CACTISPEED;
		xmf3Look.z *= fTimeElapsed * CACTISPEED;

		XMFLOAT3 newPos = Vector3::Add(xmf3Pos, xmf3Look);
		SetPosition(newPos.x, newPos.y, newPos.z);
		xmf3Pos = GetPosition();
		float dis = Vector3::Distance(xmf3Pos, m_fAfterPos);
		if (dis < 0.1f) {

			SetPosition(m_fAfterPos.x, m_fAfterPos.y, m_fAfterPos.z);
			m_nowVerse = VERSE3;
			Change_Animation(CACTI::IDLE);

			// 180�� ȸ�� �� Cactus �����ϴ� �κ�
			if (m_pCacti && (VERSE3 == m_pCacti->m_nowVerse)) {
				m_pCactus = new CCactusMonster();
				m_pCactus->m_xmLocalOOBB = oobbs["Cactus"];
				objects[OBJECT::OBJECT_MONSTER].push_back(m_pCactus);
				m_pCacti->m_pCactus = m_pCactus;
				if (m_index == 0) {
					m_pCactus->m_pCacti1 = this;
					m_pCactus->m_pCacti2 = m_pCacti;
				}
				else
				{
					m_pCactus->m_pCacti2 = this;
					m_pCactus->m_pCacti1 = m_pCacti;
				}

			}
		}
	}
	else if (VERSE4 == m_nowVerse) {
		// ����
		if (m_fRunCoolTime > RUNCOOLTIME)
		{
			m_bActive = false;
			return;
		}
		XMFLOAT3 xmf3Pos = GetPosition();
		XMFLOAT3 xmf3Look = GetLook();
		xmf3Look.x *= fTimeElapsed * CACTISPEED;
		xmf3Look.z *= fTimeElapsed * CACTISPEED;

		XMFLOAT3 newPos = Vector3::Add(xmf3Pos, xmf3Look);
		SetPosition(newPos.x, newPos.y, newPos.z);
		xmf3Pos = GetPosition();
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
	p.verse = m_nowVerse;
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CCactiMonster::Send_Remove_Packet_To_Clients(int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.type = SC_REMOVE_OBJECT;
	p.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	p.race = RACE_CACTI;
	p.id = m_index; // 0 , 1
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CCactiMonster::CheckCollision(int c_id) // �÷��̾ �����Ҷ� ȣ��
{
	if (m_fDamagedCoolTime < DAMAGE_COOLTIME || m_hp <= 0)
		return;

	if (BoundingBox_Intersect(c_id) && m_fDamagedCoolTime > DAMAGE_COOLTIME && m_hp > 0) {
		//m_hp -= 20.f;
		m_fDamagedCoolTime = 0.f;
		if (m_nowVerse == VERSE1) {
			Change_Animation(CACTI::ANIM::BITE);
		}
	}

}

void CCactiMonster::Change_Animation(CACTI::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = animTimes["Cacti"][eNewAnim];
	m_eCurAnim = eNewAnim;
}

void CCactiMonster::AttackProcess(CACTUS::ANIM eAnim)
{
	switch (eAnim)
	{
	case CACTUS::ATTACK1: {
		Change_Animation(CACTI::ATTACK1);
		break;
	}
	case CACTUS::ATTACK2: {
		Change_Animation(CACTI::ATTACK2);
		break;
	}
	case CACTUS::ATTACK3:
	default:
		break;
	}
}

void CCactiMonster::AddBullet(int _id)
{
	//CCactiBulletObject* pObj = static_cast<CCactiBulletObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"CactiBullet"));
	//CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();
	//XMFLOAT3 xmf3Target = pPlayer->GetPosition();
	//pObj->SetActiveState(true);

	//XMFLOAT3 regenPos;
	//regenPos.x = (float)(rand() % 20) * 0.05f * 2.f;
	//regenPos.y = (float)(rand() % 10) * 0.05f + 0.5f;
	//regenPos.z = (float)(rand() % 20) * 0.05f * 2.f;
	//regenPos = Vector3::Add(GetPosition(), regenPos);
	//pObj->SetPosition(regenPos);
	//pObj->SetTarget(pObj->GetPosition(), xmf3Target);

	//// look ���� ����
	//pObj->SetLookAt(xmf3Target, false);
	//pObj->m_fCreateTime = (float)(rand() % 5) * 0.2f;
	//pObj->m_fSpeed = (float)(rand() % 10) * 0.5f + 5.f;
}

void CCactiMonster::SetVerse(char _verse)
{
	m_nowVerse = _verse;

	if (m_nowVerse == VERSE4) {
		Change_Animation(CACTI::ANIM::WALK);
		m_fRunCoolTime = 0.f;
	}
}
