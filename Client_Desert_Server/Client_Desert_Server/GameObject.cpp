#include "GameObject.h"
#include "Session.h"

CGameObject::CGameObject()
{
	m_bActive = false;
	race = RACE_PLAYER;
	hp = 100, hpmax=100;

	m_xmf4x4World = Matrix4x4::Identity();
	//SetPosition(PLAYER_INIT_POSITION_X, PLAYER_INIT_POSITION_Z);
	//for (int i = 0; i < PLAYER::ANIM::END; i++)
	//{
	//	m_eAnimInfo[i].sWeight = 0;
	//	m_eAnimInfo[i].sPosition = 0;
	//	m_eAnimInfo[i].bEnable = false;
	//
	//}
	//m_eCurAnim = PLAYER::ANIM::IDLE;
}

CGameObject::~CGameObject()
{
}



void CGameObject::Update(float fTimeElapsed)
{
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetLook(float x, float y, float z)
{
	XMFLOAT3 targetPos = XMFLOAT3(x, y, z);
	XMFLOAT3 xmf3Pos = GetPosition(), xmf3Up = { 0.f, 1.f, 0.f };
	XMFLOAT3 xmf3Look = Vector3::Subtract(targetPos, xmf3Pos, true);
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);

	m_xmf4x4World._11 = xmf3Right.x; m_xmf4x4World._12 = xmf3Right.y; m_xmf4x4World._13 = xmf3Right.z;
	m_xmf4x4World._21 = xmf3Up.x; m_xmf4x4World._22 = xmf3Up.y; m_xmf4x4World._23 = xmf3Up.z;
	m_xmf4x4World._31 = xmf3Look.x; m_xmf4x4World._32 = xmf3Look.y; m_xmf4x4World._33 = xmf3Look.z;

	//SetScale(m_xmf3Scale);
}

bool CGameObject::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}

bool CGameObject::BoundingBox_Intersect(int c_id)
{
	UpdateBoundingBox(); 
	clients[c_id]._pObject->UpdateBoundingBox();

	BoundingOrientedBox targetOOBB = clients[c_id]._pObject->m_xmOOBB;
	if (m_xmOOBB.Intersects(targetOOBB))
		return true;
	
	return false;
}


void CGameObject::UpdateBoundingBox()
{
	m_xmLocalOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
	XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));

}
