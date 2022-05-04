#include "Client_Desert_Server.h"
#include "Player.h"

void CPlayer::Initialize()
{
	m_xmf4x4World = Matrix4x4::Identity();
	SetPosition(PLAYER_INIT_POSITION_X, PLAYER_INIT_POSITION_Z);
	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_eAnimInfo[i].sPosition = 0;
		m_eAnimInfo[i].sPosition = 0;
		m_eAnimInfo[i].bEnable = false;

	}
	m_eCurAnim = PLAYER::ANIM::IDLE;
}

XMFLOAT3 CPlayer::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CPlayer::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));

}

void CPlayer::SetPosition(int x, int z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._43 = z;

}

bool CPlayer::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}

