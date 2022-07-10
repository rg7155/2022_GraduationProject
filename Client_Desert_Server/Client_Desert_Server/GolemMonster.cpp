#include "Client_Desert_Server.h"
#include "GameObject.h"
#include "GolemMonster.h"
#include "Session.h"
#define FOLLOW_DISTANCE 1.f
#define VERSE1 1
#define VERSE2 2

CGolemMonster::CGolemMonster(int _targetId)
{
	
	m_eCurAnim = GOLEM::ANIM::IDLE;
	m_xmf3Position = XMFLOAT3(13.f, 0.f, 134.f);
	m_xmf3Look = clients[_targetId]._pObject->GetPosition();
	m_fAttackAnimTime = 0.f;
	m_targetId = clients[_targetId]._id;
	m_fRunCoolTime = 0.f;
	m_fDamagedCoolTime = 0.f;
	m_iHp = 100.f;
	m_bFollowStart = false;
	m_iVerse = VERSE1;
}

void CGolemMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;

	m_fAnimElapsedTime += fTimeElapsed;
	m_fRunCoolTime += fTimeElapsed;
	m_fDamagedCoolTime += fTimeElapsed;

	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		if (m_eCurAnim == GOLEM::ANIM::DAMAGED_LEFT || m_eCurAnim == GOLEM::ANIM::DAMAGED_RIGHT ||
			m_eCurAnim == GOLEM::ANIM::ATTACK1 || m_eCurAnim == GOLEM::ANIM::ATTACK2)
		{
			// �÷��̾ ���� -> Ŭ�󿡼� ó��
			Change_Animation(GOLEM::ANIM::IDLE);

		}

		else if (m_eCurAnim != GOLEM::IDLE && m_eCurAnim != GOLEM::RUN && m_eCurAnim != GOLEM::DIE)
		{
			Change_Animation(GOLEM::ANIM::RUN);
		}
	}

	// ���̵� -> �ǰ� -> ���̵� -> �� -> ���� -> ���̵� -> ��
	// �ǰ� -> ���̵� -> ��
	if (m_fRunCoolTime > 1.5f && m_eCurAnim != GOLEM::ANIM::RUN && m_eCurAnim != GOLEM::ANIM::DIE && m_bFollowStart)
		Change_Animation(GOLEM::ANIM::RUN);




	if (clients.count(m_targetId))
	{
		// Ÿ���� �Ѿư��� �ɷ� �̵� & ȸ��
		CGameObject* pTarget = clients[m_targetId]._pObject;
		XMFLOAT3 mf3TargetPos =pTarget ->GetPosition();

		if (m_eCurAnim == GOLEM::ANIM::RUN && Vector3::Distance(mf3TargetPos, m_xmf3Position) > FOLLOW_DISTANCE)
		{
			XMFLOAT3 subVectorNormal = Vector3::Subtract(mf3TargetPos, m_xmf3Position, true, true);
			XMVECTOR xmVecNormal = { subVectorNormal.x,subVectorNormal.y, subVectorNormal.z };

			xmVecNormal = xmVecNormal * fTimeElapsed * GOLEMSPEED;
			Move(Vector3::XMVectorToFloat3(xmVecNormal));
			m_xmf3Look = mf3TargetPos;
		}

		// Ÿ�ٰ� �����Ÿ� ���� �������� ����
		float fDis = Vector3::Distance(mf3TargetPos, m_xmf3Position);
		if (fDis < 15.f && !m_bFollowStart)
		{
			m_bFollowStart = true;
			Change_Animation(GOLEM::ANIM::RUN);
		}
		if (fDis < GOLEM_ATTACK_DISTANCE && GOLEM::ANIM::RUN == m_eCurAnim && !pTarget->IsNowAttack())
		{
			// �� �� �� �����ϰ�
			int iAttackRand = rand() % 2;
			GOLEM::ANIM eAnim = iAttackRand == 0 ? GOLEM::ANIM::ATTACK1 : GOLEM::ANIM::ATTACK2;
			Change_Animation(eAnim);
			m_fAttackAnimTime = 0.f;
			// ���� �� Ÿ�� �ٲٱ�
			m_targetId = 1 - m_targetId;

		}
	}

	// �ǰ� �� �ǰ� �ִϸ��̼����� ����
	// �ǰ� �ִϸ��̼� ���� �� ATTACK1�� ���� (Ŭ�󿡼�)
	// ������ �÷��̾�� Ÿ�� ���� �� �ٽ� �Ѿư� & �ݺ�

}

void CGolemMonster::Send_Packet_To_Clients(int c_id)
{
	SC_MOVE_MONSTER_PACKET p;
	p.id = 0;
	p.type = SC_MOVE_MONSTER;
	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
	p.eCurAnim = m_eCurAnim;
	p.xmf3Position = GetPosition();
	p.xmf3Look = GetLook();
	p.target_id = m_targetId;
	p.hp = m_iHp;

	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CGolemMonster::Move(XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}

void CGolemMonster::CheckCollision(int c_id)
{
	if (m_eCurAnim == GOLEM::ANIM::DAMAGED_LEFT ||
		m_eCurAnim == GOLEM::ANIM::DAMAGED_RIGHT)
		return;
	
	CGameObject* pObject = clients[c_id]._pObject;
	// �ִϸ��̼� position ����ؼ� �˻�
	if (m_eCurAnim == GOLEM::ATTACK1)
	{
		if (pObject->m_eAnimInfo[pObject->m_eCurAnim].fPosition < 8000)
			return;
	}
	else
	{
		if (pObject->m_eAnimInfo[pObject->m_eCurAnim].fPosition < 5000 ||
			pObject->m_eAnimInfo[pObject->m_eCurAnim].fPosition > 10000)
			return;
	}

	

	XMFLOAT3 fPlayerPos = pObject->GetPosition();
	float fDis = Vector3::Distance(fPlayerPos, m_xmf3Position);

	// �÷��̾� Look�� �÷��̾�� ���� Dir ��
	XMFLOAT3 fPlayerLook = pObject->GetLook();
	XMFLOAT3 fDir = Vector3::Subtract(m_xmf3Position, fPlayerPos, true, true);

	float fAngle = Vector3::Angle(fPlayerLook, fDir);

	if (m_eCurAnim == GOLEM::ANIM::ATTACK1 || m_eCurAnim == GOLEM::ANIM::ATTACK2)
		fAngle = 0.f;


	if (fDis < PLAYER_ATTACK_DISTANCE && m_fDamagedCoolTime > 0.8f && abs(fAngle) < 90.f)
	{
		m_iHp -= 20.f;
		m_fDamagedCoolTime = 0.f;

		if (m_iHp <= 0.f)
		{
			Change_Animation(GOLEM::ANIM::DIE);
			m_fRunCoolTime = 0.f;
			return;
		}
		else if (m_fRunCoolTime > 1.5f)
		{
			Change_Animation(GOLEM::ANIM::DAMAGED_LEFT);
			m_fRunCoolTime = 0.f;

			// Ÿ�� �����ϰ� �ٲٱ�
			m_targetId = rand() % 2;
		}

	}
}

void CGolemMonster::Change_Animation(GOLEM::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 1.f;
	m_eCurAnim = eNewAnim;
	m_fRunCoolTime = 0.f;

}


