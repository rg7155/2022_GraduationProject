#include "CactusMonster.h"
#include "CactiMonster.h"
#include "Session.h"
CCactusMonster::CCactusMonster()
{
	SetScale(2.f, 2.f, 2.f);
	m_bActive = true;
	m_eCurAnim = CACTUS::ANIM::SPAWN;
	m_fAnimMaxTime = animTimes["Cactus"][m_eCurAnim];
	m_hp = 100;
	SetPosition(CACTUS_POS_INIT.x, CACTUS_POS_INIT.y, CACTUS_POS_INIT.z);
	
}

CCactusMonster::~CCactusMonster()
{
}

void CCactusMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;
	if (m_eCurAnim == CACTUS::DIE) {
		m_bActive = false;
		m_pCacti1->SetVerse(VERSE4);
		m_pCacti2->SetVerse(VERSE4);

		return;
	}
	m_fAnimElapsedTime += fTimeElapsed;
	m_fDamagedCoolTime += fTimeElapsed;
	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		m_fAnimElapsedTime = 0.f;
		Change_Animation(CACTUS::ANIM::IDLE);
		m_nowVerse = VERSE2;
	}

	if (m_nowVerse == VERSE2) {
		m_fAttackCoolTime += fTimeElapsed;
		if (m_fAttackCoolTime > ATTACK_COOLTIME)
		{
			m_fAttackCoolTime = 0.f;
			CACTUS::ANIM eNext = m_ePreAttack == CACTUS::ATTACK3 ? CACTUS::ATTACK1 : (CACTUS::ANIM)(m_ePreAttack + 1);
			Change_Animation(eNext);
			m_ePreAttack = eNext;
			if (eNext != CACTUS::ATTACK3) {
				m_pCacti1->AttackProcess(eNext);
				m_pCacti2->AttackProcess(eNext);
			}
			
		}
	}
}

void CCactusMonster::Send_Packet_To_Clients(int c_id)
{
	SC_MOVE_MONSTER_PACKET p;
	p.id = 0;
	p.type = SC_MOVE_MONSTER;
	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
	p.eCurAnim = m_eCurAnim;
	p.xmf3Position = GetPosition();
	p.xmf3Look = m_xmf3Target;
	p.hp = m_hp;
	p.race = RACE_CACTUS;
	p.verse = m_nowVerse;
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CCactusMonster::CheckCollision(int c_id)
{
	if (m_eCurAnim == CACTUS::ANIM::TAKE_DAMAGED)
		return;


	if (BoundingBox_Intersect(c_id) && m_fDamagedCoolTime > DAMAGE_COOLTIME && m_hp > 0)
	{
		m_hp -= 20.f;
		m_fDamagedCoolTime = 0.f;

		if (m_hp <= 0.f)
		{
			Change_Animation(CACTUS::ANIM::DIE);
			return;
		}
		else
		{
			Change_Animation(CACTUS::ANIM::TAKE_DAMAGED);
		}
	}
}

void CCactusMonster::Change_Animation(CACTUS::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = animTimes["Cactus"][eNewAnim];
	m_eCurAnim = eNewAnim;

}

void CCactusMonster::AddBullet()
{
	//CCactiBulletObject* pObj = static_cast<CCactiBulletObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"CactiBullet"));
	//CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();
	//XMFLOAT3 xmf3Target = pPlayer->GetPosition();
	//pObj->SetActiveState(true);

	//XMFLOAT3 regenPos;
	//regenPos.x = (float)(rand() % 40) * 0.5f - 10.f;
	//regenPos.y = (float)(rand() % 10) * 0.1f + 10.f;
	//regenPos.z = (float)(rand() % 40) * 0.5f - 10.f;
	//regenPos = Vector3::Add(xmf3Target, regenPos);
	//pObj->SetPosition(regenPos);
	//xmf3Target = regenPos;
	//xmf3Target.y = -1.f;
	//pObj->SetTarget(pObj->GetPosition(), xmf3Target, false);
	//pObj->Rotate(90.f, 0.f, 0.f);
	//pObj->SetScale(4.f, 4.f, 4.f);
	////pObj->m_fCreateTime = (float)(rand() % 5) * 0.2f;
	//pObj->m_fSpeed = (float)(rand() % 10) * 0.5f + 4.f;
}
 