#include "Component.h"
#include "Camera.h"

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

	return true;
}

bool CFrustum::Isin_Frustum(XMFLOAT3* pPos, float& fRadius)
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

bool CFrustum::Isin_Frustum_ForObject(CCamera* pCamera, XMFLOAT3* pPos, float& fRadius)
{
	Ready_Frustum();

	XMFLOAT4X4	xmf4x4View, xmf4x4Proj;

	xmf4x4View = Matrix4x4::Inverse(pCamera->GetViewMatrix());
	xmf4x4Proj = Matrix4x4::Inverse(pCamera->GetProjectionMatrix());
	
	for (int i = 0; i < 8; ++i)
	{
		Vector3::TransformCoord(m_xmf3Point[i], xmf4x4Proj);
		Vector3::TransformCoord(m_xmf3Point[i], xmf4x4View);
		//XMStoreFloat3(&m_xmf3Point[i], XMVector3TransformCoord(XMLoadFloat3(&m_xmf3Point[i]), XMLoadFloat4x4(&xmf4x4Proj)));
		//XMStoreFloat3(&m_xmf3Point[i], XMVector3TransformCoord(XMLoadFloat3(&m_xmf3Point[i]), XMLoadFloat4x4(&xmf4x4View)));
	}

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


	return Isin_Frustum(pPos, fRadius);
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
