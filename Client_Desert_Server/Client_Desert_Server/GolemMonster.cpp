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
	// Ÿ���� �Ѿư��� �ɷ� �̵� & ȸ��
	XMFLOAT3 mf3TargetPos = m_pTarget->GetPosition();
	m_xmf3Look = mf3TargetPos;

	if (m_eCurAnim == GOLEM::ANIM::RUN)
	{
		XMFLOAT3 subVectorNormal = Vector3::Subtract(mf3TargetPos, m_xmf3Position, true, true);
		XMVECTOR xmVecNormal = { subVectorNormal.x,subVectorNormal.y, subVectorNormal.z };

		xmVecNormal = xmVecNormal * fTimeElapsed * GOLEMSPEED;
		Move(Vector3::XMVectorToFloat3(xmVecNormal));
	}
	
	// Ÿ�ٰ� �����Ÿ� ���� �������� ����
	
	// �ǰ� �� �ǰ� �ִϸ��̼����� ����
	// �ǰ� �ִϸ��̼� ���� �� ATTACK1�� ���� (Ŭ�󿡼�)
	// ������ �÷��̾�� Ÿ�� ���� �� �ٽ� �Ѿư� & �ݺ�

}

void CGolemMonster::Move(XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}


