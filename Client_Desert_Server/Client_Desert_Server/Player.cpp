#include "Client_Desert_Server.h"
#include "Player.h"

void CPlayer::Initialize()
{
	m_xmf4x4World = Matrix4x4::Identity();

	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_eAnimInfo[i].fPosition = 0.f;
		m_eAnimInfo[i].fWeight = 0.f;
		m_eAnimInfo[i].bEnable = false;

	}
}

XMFLOAT3 CPlayer::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

