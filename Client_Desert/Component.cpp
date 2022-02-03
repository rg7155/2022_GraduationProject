#include "Component.h"
#include "Camera.h"

CFrustum::CFrustum()
{
}


CFrustum::~CFrustum(void)
{
}

void CFrustum::Ready_Frustum(void)
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
	pInstance->Ready_Frustum();
	pInstance->AddRef();
	return pInstance;
}



