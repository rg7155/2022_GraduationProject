#include "Client_Desert_Server.h"
#include "GolemMonster.h"
#include "Player.h"


CGolemMonster::CGolemMonster(CPlayer* _pTarget)
	:m_pTarget(_pTarget)
{
	m_eCurAnim = GOLEM::ANIM::RUN;
	m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
	m_xmf3Look = m_pTarget->GetPosition();
}

void CGolemMonster::Update(float fTimeElapsed)
{
	// 타겟을 쫓아가는 걸로 이동 & 회전
	XMFLOAT3 mf3TargetPos = m_pTarget->GetPosition();
	m_xmf3Look = mf3TargetPos;

	if (m_eCurAnim == GOLEM::ANIM::RUN)
	{
		XMFLOAT3 subVectorNormal = Vector3::Subtract(mf3TargetPos, m_xmf3Position, true, true);
		XMVECTOR xmVecNormal = { subVectorNormal.x,subVectorNormal.y, subVectorNormal.z };

		xmVecNormal = xmVecNormal * fTimeElapsed * GOLEMSPEED;
		Move(Vector3::XMVectorToFloat3(xmVecNormal));
	}
	
	// 타겟과 일정거리 내로 좁혀지면 공격
	
	// 피격 시 피격 애니메이션으로 변경
	// 피격 애니메이션 종료 후 ATTACK1로 변경 (클라에서)
	// 공격한 플레이어로 타겟 변경 후 다시 쫓아감 & 반복

}

void CGolemMonster::Move(XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}


