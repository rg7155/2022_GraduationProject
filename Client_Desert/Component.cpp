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
	m_fCreateTime = 0.001f;

	m_iMaxCount = 50; //사각형은 1/2개
	m_iDivide = 8; //하나를 몇개로 나눌껀지

	m_fTime = m_fCreateTime + 1.f;

	m_pTrailObject = new CTrailObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (m_iMaxCount - 1) * m_iDivide * 6 );
}

void CTrail::Update_Component(const float& fTimeDelta)
{
	if (!m_bRender)
		return;

	m_fTime -= fTimeDelta;
}

void CTrail::AddTrail(XMFLOAT3& xmf3Top, XMFLOAT3& xmf3Bottom)
{
	if (m_fTime < 0.f)
	{
		m_fTime = m_fCreateTime;

		m_listPos.emplace_back(make_pair(xmf3Top, xmf3Bottom));
	
		//꽉차면 제일 첫번째 사각형 지우기
		size_t iCount = m_listPos.size();
		if (iCount >= m_iMaxCount)
		{
			for (int i = 0; i < 2; ++i)
				m_listPos.pop_front();

			for (int i = 0; i < 2 * m_iDivide; ++i)
				m_listRomPos.pop_front();
		}
		
		//캣멀롬 
		//방법1 생성된 곳 한번만
		//방법2 생성되면 전체 다시해주기

		//방법1-1 
		iCount = m_listPos.size();
		if (iCount <= 3)
		{
			//임시로, 4개는 있어야 하니
			m_listRomPos.emplace_back(make_pair(xmf3Top, xmf3Bottom));
			return;
		}

		auto iter = m_listPos.end();
		XMFLOAT3 xmf3TopPos[4], xmf3BottomPos[4];
		//최신 점 4개
		for (int i = 3; i > -1; --i)
		{
			xmf3TopPos[i] = (*(--iter)).first;
			xmf3BottomPos[i] = (*(iter)).second;
		}

		for (int i = 0; i < m_iDivide; ++i)
		{
			float t = (i + 1) / (float)m_iDivide;
			XMFLOAT3 xmf3RomTopPos = Vector3::CatmullRom(xmf3TopPos[1], xmf3TopPos[2], xmf3TopPos[3], xmf3TopPos[3], t); //새로 들어온 하나만
			XMFLOAT3 xmf3RomBottomPos = Vector3::CatmullRom(xmf3BottomPos[1], xmf3BottomPos[2], xmf3BottomPos[3], xmf3BottomPos[3], t); //새로 들어온 하나만

			m_listRomPos.emplace_back(make_pair(xmf3RomTopPos, xmf3RomBottomPos));
		}

		//cout << m_listPos.size() << ", " << m_listRomPos.size() << endl;
	}
}

void CTrail::RenderTrail(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//나중에 렌더링을 끊기 보다는 자연스럽게 사라지도록 할 것.
	if (!m_bRender)
		return;

	size_t iCount = m_listRomPos.size();
	if (iCount <= 1)
		return;

	auto iter = m_listRomPos.begin();

	size_t iRectCount = iCount - 1;
	size_t iVertexCount = iRectCount * 6;//사각형 당 정점 6개
	CTexturedVertex* pVertices = new CTexturedVertex[iVertexCount];

	int i = 0, iLineIndex = 0; //사각형의 왼쪽
	while (iter != m_listRomPos.end())
	{
		XMFLOAT3 xmf3Pos[4];
		xmf3Pos[0] = (*(iter)).first; //Top1
		xmf3Pos[1] = (*(iter++)).second; //Bottom1

		//사각형 더 못그리면
		if (iter == m_listRomPos.end())
			break;

		xmf3Pos[2] = (*(iter)).second; //Bottom2
		xmf3Pos[3] = (*(iter)).first; //Top2

		int iNextIineIndex = iLineIndex + 1; //사각형의 오른쪽
		XMFLOAT2 xmf2UV[4];
		float fRatio = (float)iLineIndex / iRectCount;
		float fNextRatio = (float)iNextIineIndex / iRectCount;
		xmf2UV[0] = { fRatio,			0.f };
		//xmf2UV[1] = { fRatio,			1.f - (fRatio / 1.f) };
		//xmf2UV[2] = { fNextRatio / 1.f, 1.f - (fNextRatio / 1.f) };
		xmf2UV[1] = { fRatio,			(fRatio / 1.f) };
		xmf2UV[2] = { fNextRatio / 1.f, (fNextRatio / 1.f) };
		xmf2UV[3] = { fNextRatio / 1.f, 0.f };

		pVertices[i++] = CTexturedVertex(xmf3Pos[2], xmf2UV[2]);	//xmf3Bottom2,
		pVertices[i++] = CTexturedVertex(xmf3Pos[3], xmf2UV[3]);	//xmf3Top2,	
		pVertices[i++] = CTexturedVertex(xmf3Pos[0], xmf2UV[0]);	//xmf3Top1,	
		pVertices[i++] = CTexturedVertex(xmf3Pos[0], xmf2UV[0]);	//xmf3Top1,	
		pVertices[i++] = CTexturedVertex(xmf3Pos[1], xmf2UV[1]);	//xmf3Bottom1,
		pVertices[i++] = CTexturedVertex(xmf3Pos[2], xmf2UV[2]);	//xmf3Bottom2,

		iLineIndex++;
	}

	m_pTrailObject->m_pTrailMesh->SetVertices(pVertices, iVertexCount);
	m_pTrailObject->Render(pd3dCommandList, pCamera); //렌더링은 한번만

	delete[] pVertices;
}

void CTrail::SetRenderingTrail(bool isOn)
{
	m_bRender = isOn;

	if (!m_bRender)
	{
		m_listPos.clear();
		m_listRomPos.clear();
		m_fTime = m_fCreateTime;
	}
}

CTrail* CTrail::Create(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CTrail* pInstance = new CTrail();
	pInstance->Ready_Component(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pInstance->AddRef();
	return pInstance;
}

