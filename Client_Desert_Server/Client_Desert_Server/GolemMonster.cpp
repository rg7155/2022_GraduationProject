#include "Client_Desert_Server.h"
#include "GameObject.h"
#include "GolemMonster.h"
#include "Session.h"
#define FOLLOW_DISTANCE 1.f

CGolemMonster::CGolemMonster(int _targetId)
{
	
	m_eCurAnim = GOLEM::ANIM::IDLE;
	m_fAnimMaxTime = animTimes["Golem"][m_eCurAnim];

	SetPosition(13.f, 0.f, 134.f);
	XMFLOAT3 playerPos = clients[_targetId]._pObject->GetPosition();
	m_xmf3Target = playerPos;
	m_fAttackAnimTime = 0.f;
	m_targetId = _targetId;
	m_fRunCoolTime = 0.f;
	m_fDamagedCoolTime = 0.f;
	m_hp = 100.f;
	m_bFollowStart = false;
}

void CGolemMonster::Update(float fTimeElapsed)
{
	if (!m_bActive)
		return;

	if (m_eCurAnim == GOLEM::DIE) {
		m_fDieCoolTime += fTimeElapsed;
		if (m_fDieCoolTime > 3.f)
			m_bActive = false;
		return;
	}
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
	if (m_fRunCoolTime > 1.f && m_eCurAnim != GOLEM::ANIM::RUN && m_eCurAnim != GOLEM::ANIM::DIE && m_bFollowStart)
		Change_Animation(GOLEM::ANIM::RUN);




	if (clients.count(m_targetId))
	{
		// Ÿ���� �Ѿư��� �ɷ� �̵� & ȸ��
		CGameObject* pTarget = clients[m_targetId]._pObject;
		XMFLOAT3 mf3TargetPos =pTarget ->GetPosition();

		if (m_eCurAnim == GOLEM::ANIM::RUN && Vector3::Distance(mf3TargetPos, GetPosition()) > FOLLOW_DISTANCE)
		{
			XMFLOAT3 subVectorNormal = Vector3::Subtract(mf3TargetPos, GetPosition(), true, true);
			XMVECTOR xmVecNormal = { subVectorNormal.x,subVectorNormal.y, subVectorNormal.z };

			xmVecNormal = xmVecNormal * fTimeElapsed * GOLEMSPEED;
			Move(Vector3::XMVectorToFloat3(xmVecNormal));
			m_xmf3Target = mf3TargetPos;
		}

		// Ÿ�ٰ� �����Ÿ� ���� �������� ����
		float fDis = Vector3::Distance(mf3TargetPos, GetPosition());
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
	p.xmf3Look = m_xmf3Target;
	p.target_id = m_targetId;
	p.hp = m_hp;
	p.race = RACE_GOLEM;
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}

void CGolemMonster::CheckCollision(int c_id)
{
	if (m_eCurAnim == GOLEM::ANIM::DAMAGED_LEFT ||
		m_eCurAnim == GOLEM::ANIM::DAMAGED_RIGHT)
		return;
	
	
	if (BoundingBox_Intersect(c_id) && m_fDamagedCoolTime > DAMAGE_COOLTIME && m_hp > 0)
	{
		m_hp -= 20.f;
		m_fDamagedCoolTime = 0.f;

		if (m_hp <= 0.f)
		{
			Change_Animation(GOLEM::ANIM::DIE);
			m_fRunCoolTime = 0.f;

			return;
		}
		else
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
	m_fAnimMaxTime = animTimes["Golem"][eNewAnim];
	m_eCurAnim = eNewAnim;
	m_fRunCoolTime = 0.f;

}


