#include "Client_Desert_Server.h"
#include "GolemMonster.h"
#include "Player.h"


CGolemMonster::CGolemMonster(CPlayer* _pTarget)
	:m_pTarget(_pTarget)
{
	m_eCurAnim = GOLEM::ANIM::RUN;
	m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
	m_xmf3Look = m_pTarget->GetPosition();
	m_fAttackAnimTime = 0.f;
}

void CGolemMonster::Update(float fTimeElapsed)
{
	m_fAnimElapsedTime += fTimeElapsed;

	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		if (m_eCurAnim == GOLEM::ANIM::DAMAGED_LEFT || m_eCurAnim == GOLEM::ANIM::DAMAGED_RIGHT)
		{
			// 플레이어를 공격 -> 클라에서 처리
			Change_Animation(GOLEM::ANIM::ATTACK1);

		}

		else if (m_eCurAnim != GOLEM::IDLE && m_eCurAnim != GOLEM::RUN)
		{
			Change_Animation(GOLEM::ANIM::RUN);
		}
	}

	// 타겟을 쫓아가는 걸로 이동 & 회전
	XMFLOAT3 mf3TargetPos = m_pTarget->GetPosition();

	if (m_eCurAnim == GOLEM::ANIM::RUN)
	{
		XMFLOAT3 subVectorNormal = Vector3::Subtract(mf3TargetPos, m_xmf3Position, true, true);
		XMVECTOR xmVecNormal = { subVectorNormal.x,subVectorNormal.y, subVectorNormal.z };

		xmVecNormal = xmVecNormal * fTimeElapsed * GOLEMSPEED;
		Move(Vector3::XMVectorToFloat3(xmVecNormal));
		m_xmf3Look = mf3TargetPos;
	}
	
	// 타겟과 일정거리 내로 좁혀지면 공격
	float fDis = Vector3::Distance(mf3TargetPos, m_xmf3Position);
	if (fDis < ATTACK_DISTANCE && GOLEM::ANIM::RUN == m_eCurAnim)
	{
		Change_Animation(GOLEM::ANIM::ATTACK2);
		m_fAttackAnimTime = 0.f;

		// 공격 후 타겟 바꾸기
		CPlayer* pPlayer = Get_Player(1 - m_pTarget->m_id);
		m_pTarget = pPlayer;
	}
	// 피격 시 피격 애니메이션으로 변경
	// 피격 애니메이션 종료 후 ATTACK1로 변경 (클라에서)
	// 공격한 플레이어로 타겟 변경 후 다시 쫓아감 & 반복

}

void CGolemMonster::Move(XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}

void CGolemMonster::CheckCollision(CPlayer* pAttackPlayer)
{
	if (m_eCurAnim == GOLEM::ANIM::DAMAGED_LEFT ||
		m_eCurAnim == GOLEM::ANIM::DAMAGED_RIGHT)
		return;
	float fDis = Vector3::Distance(pAttackPlayer->GetPosition(), m_xmf3Position);
	if (fDis < PLAYER_ATTACK_DISTANCE)
	{
		m_eCurAnim = GOLEM::ANIM::DAMAGED_LEFT;
	}
}

void CGolemMonster::Change_Animation(GOLEM::ANIM eNewAnim)
{
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 1.f;
	m_eCurAnim = eNewAnim;

}


