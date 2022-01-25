#include "CComponent.h"

CFrustum::CFrustum()
{
}

CFrustum::CFrustum(const CFrustum& rhs)
{
}

CFrustum::~CFrustum(void)
{
}

void CFrustum::Ready_Frustum(void)
{
	m_xmf3Point[0] = { -1.f, 1.f, 0.f };
	m_xmf3Point[1] = { 1.f, 1.f, 0.f };
	m_xmf3Point[2] = { 1.f, -1.f, 0.f };
	m_xmf3Point[3] = { -1.f, -1.f, 0.f };

	m_xmf3Point[4] = { -1.f, 1.f, 1.f };
	m_xmf3Point[5] = { 1.f, 1.f, 1.f };
	m_xmf3Point[6] = { 1.f, -1.f, 1.f };
	m_xmf3Point[7] = { -1.f, -1.f, 1.f };
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

	//return true;
}

bool CFrustum::Isin_Frustum(XMFLOAT3* pPos, const float& fRadius)
{
	float	fDistance = 0.f;

	for (int i = 0; i < 6; ++i)
	{
		//fDistance = D3DXPlaneDotCoord(&m_xmf4Plane[i], pPos);
		//XMVectorSet()
		fDistance = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_xmf4Plane[i]), XMLoadFloat3(&pPos[i])));
		if (fDistance > fRadius)
			return false;
	}

	return true;
}

bool CFrustum::Isin_Frustum_ForObject(XMFLOAT3* pPos, XMFLOAT3& fRadius)
{
	//Ready_Frustum();

	//_matrix		matProj, matView;

	//m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
	//m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);

	//D3DXMatrixInverse(&matProj, NULL, &matProj);
	//D3DXMatrixInverse(&matView, NULL, &matView);

	//for (_ulong i = 0; i < 8; ++i)
	//{
	//	D3DXVec3TransformCoord(&m_vPoint[i], &m_vPoint[i], &matProj);
	//	D3DXVec3TransformCoord(&m_vPoint[i], &m_vPoint[i], &matView);
	//}

	// x+
	m_xmf4Plane[0] = GetPlane(m_xmf3Point[1], m_xmf3Point[5], m_xmf3Point[6]);
	// x-
	m_xmf4Plane[1] = GetPlane(m_xmf3Point[4], m_xmf3Point[0], m_xmf3Point[3]);
	// y+
	m_xmf4Plane[2] = GetPlane(m_xmf3Point[4], m_xmf3Point[5], m_xmf3Point[1]);
	// y-
	m_xmf4Plane[3] = GetPlane(m_xmf3Point[3], m_xmf3Point[2], m_xmf3Point[6]);
	// z+
	m_xmf4Plane[4] = GetPlane(m_xmf3Point[7], m_xmf3Point[6], m_xmf3Point[5]);
	// z-
	m_xmf4Plane[5] = GetPlane(m_xmf3Point[0], m_xmf3Point[1], m_xmf3Point[2]);


	return false;
}

XMFLOAT4 CFrustum::GetPlane(XMFLOAT3& xmf3Pos1, XMFLOAT3& xmf3Pos2, XMFLOAT3& xmf3Pos3)
{
	XMFLOAT4 xmf4Plane;
	XMStoreFloat4(&xmf4Plane, (XMPlaneFromPoints(XMLoadFloat3(&xmf3Pos1), XMLoadFloat3(&xmf3Pos2), XMLoadFloat3(&xmf3Pos3))));
	return  xmf4Plane;
}

CFrustum* CFrustum::Create()
{
	CFrustum* pInstance = new CFrustum;
	pInstance->Ready_Frustum();
	return pInstance;
}

CComponent* CFrustum::Clone(void)
{
	return new CFrustum(*this);
}

void CFrustum::Free(void)
{
}
