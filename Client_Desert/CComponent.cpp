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
}

bool CFrustum::Isin_Frustum(XMFLOAT3* pPos)
{
	return false;
}

bool CFrustum::Isin_Frustum(XMFLOAT3* pPos, const float& fRadius)
{
	return false;
}

bool CFrustum::Isin_Frustum_ForObject(XMFLOAT3* pPos, XMFLOAT3& fRadius)
{
	return false;
}

CFrustum* CFrustum::Create()
{
	return nullptr;
}

void CFrustum::Free(void)
{
}
