#include "GameObject.h"

CGameObject::CGameObject()
{

}

CGameObject::~CGameObject()
{
}

//m_xmf4x4World = Matrix4x4::Identity();
//SetPosition(PLAYER_INIT_POSITION_X, PLAYER_INIT_POSITION_Z);
//for (int i = 0; i < PLAYER::ANIM::END; i++)
//{
//	m_eAnimInfo[i].sWeight = 0;
//	m_eAnimInfo[i].sPosition = 0;
//	m_eAnimInfo[i].bEnable = false;
//
//}
//m_eCurAnim = PLAYER::ANIM::IDLE;

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

void CGameObject::SetPosition(int x, int y, int z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

bool CGameObject::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}
