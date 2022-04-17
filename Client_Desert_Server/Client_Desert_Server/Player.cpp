#include "Client_Desert_Server.h"
#include "Player.h"

void CPlayer::Initialize()
{
	m_xmf4x4World = Matrix4x4::Identity();

	for (int i = 0; i < ANIM::END; i++)
	{
		m_eAnimInfo[i].fPosition = 0.f;
		m_eAnimInfo[i].fWeight = 0.f;
		m_eAnimInfo[i].bEnable = false;

	}
}
