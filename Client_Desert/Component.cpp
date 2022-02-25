#include "Component.h"
#include "Camera.h"
#include "Mesh.h"
#include "Object.h"
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

///////////////////////////////////////////////////////////////////////////////

CTrail::CTrail()
{
}

CTrail::~CTrail(void)
{
	if (m_pTrailObject)
	{
		m_pTrailObject->ReleaseUploadBuffers();
		delete m_pTrailObject;
	}
}

void CTrail::Ready_Component(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_iMaxCount = 100; //사각형은 1/2개
	m_fTime = TRAIL_CREATE_TIME + 1.f;

	//컴포넌트가 오브젝트를 가져도 되나.. 
	m_pTrailObject = new CTrailObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}

void CTrail::Update_Component(const float& fTimeDelta)
{
	m_fTime -= fTimeDelta;
}

void CTrail::AddTrail(XMFLOAT3& xmf3Top, XMFLOAT3& xmf3Bottom)
{
	if (m_fTime < 0.f)
	{
		m_fTime = TRAIL_CREATE_TIME;

		m_listPos.emplace_back(make_pair(xmf3Top, xmf3Bottom));
	
		//꽉차면 제일 첫번째 사각형 지우기
		size_t iCount = m_listPos.size();
		if (iCount >= m_iMaxCount)
		{
			for (int i = 0; i < 2; ++i)
			{
				m_listPos.pop_front();
			}
		}

		////사각형 못만들면
		//if (iCount % 2 == 1)
		//	return;
	}
}

void CTrail::RenderTrail(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	size_t iCount = m_listPos.size();
	if (iCount <= 1)
		return;
	
	auto iter = m_listPos.begin();

	while (iter != m_listPos.end())
	{
		XMFLOAT3 xmf3Pos[4];
		xmf3Pos[0] = (*(iter)).first; //Top1
		xmf3Pos[1] = (*(iter++)).second; //Bottom1

		if (iter == m_listPos.end())
			break;

		xmf3Pos[2] = (*(iter)).second; //Bottom2
		xmf3Pos[3] = (*(iter++)).first; //Top2

		m_pTrailObject->m_pTrailMesh->SetPosition(xmf3Pos[0], xmf3Pos[1], xmf3Pos[2], xmf3Pos[3]);
		m_pTrailObject->Render(pd3dCommandList, pCamera);
	}
}

CTrail* CTrail::Create(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CTrail* pInstance = new CTrail();
	pInstance->Ready_Component(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pInstance->AddRef();
	return pInstance;
}

