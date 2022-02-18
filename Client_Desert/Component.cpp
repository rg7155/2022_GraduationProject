#include "Component.h"
#include "Camera.h"

CFrustum::CFrustum()
{
}


CFrustum::~CFrustum(void)
{
}

void CFrustum::Ready_Component(void)
{
}

bool CFrustum::Isin_Frustum(XMFLOAT3* pPos)
{
	//float	fDistance = 0.f;

	//for (int i = 0; i < 6; ++i)
	//{
	//	fDistance = D3DXPlaneDotCoord(&m_Plane[i], pPos);

	//	if (fDistance > 0.f) //면 앞에 있다
	//		return false;
	//}

	return true;
}



bool CFrustum::Isin_Frustum_ForObject(CCamera* pCamera, XMFLOAT3* pPos, float& fRadius)
{
	float	fDistance = 0.f;

	for (int i = 0; i < 6; ++i)
	{
		//fDistance = D3DXPlaneDotCoord(&m_xmf4Plane[i], pPos);
		fDistance = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&(pCamera->m_xmf4Plane[i])), XMLoadFloat3(pPos)));
		if (fDistance > fRadius)
			return false;
	}

	return true;
	//m_isRender = true;
}

CFrustum* CFrustum::Create()
{
	CFrustum* pInstance = new CFrustum();
	pInstance->Ready_Component();
	pInstance->AddRef();
	return pInstance;
}

///////////////////////////////////////////////////////////////////////////////


CCollision::CCollision()
{
}


CCollision::~CCollision(void)
{
}

void CCollision::Ready_Component(void)
{
}

void CCollision::Update_Component(const float& fTimeDelta)
{
	UpdateBoundingBox();
}

//주로 이걸 사용할듯?
bool CCollision::Check_Collision(CCollision* pCom)
{
	if (m_isCollisionIgnore || pCom->m_isCollisionIgnore)
		return false;

	if (m_xmOOBB.Intersects(pCom->m_xmOOBB))
		return true;

	return false;
}

bool CCollision::Check_Collision(BoundingOrientedBox& xmTargetOOBB)
{
	if (m_isCollisionIgnore)
		return false;

	if (m_xmOOBB.Intersects(xmTargetOOBB))
		return true;

	return false;
}

bool CCollision::Check_Collision(BoundingOrientedBox& xmOOBB, BoundingOrientedBox& xmTargetOOBB)
{
	if (m_isCollisionIgnore)
		return false;
		
	if (xmOOBB.Intersects(xmTargetOOBB))
		return true;

	return false;
}

//Me-Player, Target-Map
bool CCollision::Check_Collision_AfterMove(BoundingOrientedBox& xmTargetOOBB, XMFLOAT3& xmf3MovePos, XMFLOAT4X4& xmf4x4World)
{
	if (m_isCollisionIgnore)
		return false; 
	
	BoundingOrientedBox xmMeOOBB;
	xmf4x4World._41 = xmf3MovePos.x;
	xmf4x4World._42 = xmf3MovePos.y;
	xmf4x4World._43 = xmf3MovePos.z;
	m_xmLocalOOBB.Transform(xmMeOOBB, XMLoadFloat4x4(&xmf4x4World));
	XMStoreFloat4(&xmMeOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&xmMeOOBB.Orientation)));
	
	if (Check_Collision(xmMeOOBB, xmTargetOOBB))
		return true;

	return false;
}

void CCollision::UpdateBoundingBox()
{
	if (m_isCollisionIgnore)
		return;
	
	if (m_isStaticOOBB)
	{
		if (m_isOneUpdate)
			return;
		m_isOneUpdate = true;
	}
	
	m_xmLocalOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(m_pxmf4x4World));
	XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
}

CCollision* CCollision::Create()
{
	CCollision* pInstance = new CCollision();
	pInstance->Ready_Component();
	pInstance->AddRef();
	return pInstance;
}
