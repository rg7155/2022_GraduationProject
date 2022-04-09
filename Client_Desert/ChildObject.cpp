#include "ChildObject.h"

#include "Shader.h"
#include "Scene.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"SkyBox/SkyBox_0.dds", 0);

	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 10, false);

	CMaterial* pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}


////////////////////////////////////////////////////////////////////////////////////////////////////

CMapObject::CMapObject()
{
}

CMapObject::~CMapObject()
{
	//GameObject 소멸자에서 컴포넌트 ref 감소
}

void CMapObject::Ready()
{
	m_eObjId = OBJ_MAP;

	CreateComponent();

	SetEffectsType(EFFECT_FOG, true);
}

void CMapObject::CreateComponent()
{
	//1
	m_pComponent[COM_FRUSTUM] = CFrustum::Create();

	//2
	m_pComponent[COM_COLLISION] = CCollision::Create();

	m_pComCollision = static_cast<CCollision*>(m_pComponent[COM_COLLISION]);
	m_pComCollision->m_isStaticOOBB = true;
	if (m_pChild && m_pChild->m_isRootModelObject)
		m_pComCollision->m_xmLocalOOBB = m_pChild->m_xmOOBB;
	m_pComCollision->m_pxmf4x4World = &m_xmf4x4World;

	if (m_strName.find("tree") != string::npos || m_strName.find("grass") != string::npos)
		m_pComCollision->m_isCollisionIgnore = true;

	if (m_strName.find("Plane") != string::npos)
	{
		m_isPlane = true;
		m_pComCollision->m_isCollisionIgnore = true;
	}

	m_pComCollision->UpdateBoundingBox();
}

void CMapObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
}


void CMapObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return;

	//BoundingOrientedBox if (m_xmOOBB.Intersects(iter->m_xmOOBB))
	//m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	CGameObject::Animate(fTimeElapsed);
}


void CMapObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	//그림자맵에 쓰는거나, 평면이면 컬링 안하고 그림
	if (CGameMgr::GetInstance()->m_isShadowMapRendering || m_isPlane)
		CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
	else
	{
		XMFLOAT3 xmf3Extents = m_pChild->m_pMesh->m_xmOOBB.Extents;
		float fMax = max(xmf3Extents.x, max(xmf3Extents.y, xmf3Extents.z));
		float fRadi = m_xmf3Scale.x * fMax; //스케일 x,y,z 다를수도

		if (static_cast<CFrustum*>(m_pComponent[COM_FRUSTUM])->Isin_Frustum_ForObject(pCamera, &GetPosition(), fRadi))
			CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTrailObject::CTrailObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int iVertexCount)
	: CGameObject(1)
{
	m_pTrailMesh = new CTrailMesh(pd3dDevice, pd3dCommandList, iVertexCount);
	SetMesh(m_pTrailMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	//pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Trail_Rot.dds", 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Trail.dds", 0);

	CTrailShader* pShader = new CTrailShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetShader(pShader);
	SetMaterial(0, pMaterial);
}

CTrailObject::~CTrailObject()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMultiSpriteObject::CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CGameObject(1)
{
	//m_fSpeed = 3.0f / (ppSpriteTextures[j]->m_nRows * ppSpriteTextures[j]->m_nCols);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_xmf4x4Texture = Matrix4x4::Identity();
	m_fSpeed = 0.01f;
}


CMultiSpriteObject::~CMultiSpriteObject()
{
	ReleaseShaderVariables();
}

void CMultiSpriteObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_TEXTURE_INFO) + 255) & ~255);
	m_pd3dcbTexture = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbTexture->Map(0, NULL, (void**)&m_pcbMappedTexture);
}

void CMultiSpriteObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &m_xmf4x4Texture, 34);

	XMStoreFloat4x4(&m_pcbMappedTexture->m_xmf4x4TextureAnim, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Texture)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbTexture->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(RP_TEXTUREANIM, d3dGpuVirtualAddress);
}

void CMultiSpriteObject::ReleaseShaderVariables()
{
	if (m_pd3dcbTexture)
	{
		m_pd3dcbTexture->Unmap(0, NULL);
		m_pd3dcbTexture->Release();
	}
}

void CMultiSpriteObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return;

	m_fTime += fTimeElapsed;
	if (m_fTime >= m_fSpeed)
		m_fTime = 0.0f;

	AnimateRowColumn(m_fTime);

	m_isBiliboard = true;
	if (m_isBiliboard)
	{
		XMFLOAT3 xmf3Target = CGameMgr::GetInstance()->GetCamera()->GetPosition();
		SetLookAt(xmf3Target);
	}

	CGameObject::Animate(fTimeElapsed);
}

void CMultiSpriteObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera);
}

void CMultiSpriteObject::AnimateRowColumn(float fTime)
{
	//	m_xmf4x4Texture = Matrix4x4::Identity();
	int nRows = m_ppMaterials[0]->m_ppTextures[0]->m_nRows;
	int nCols = m_ppMaterials[0]->m_ppTextures[0]->m_nCols;

	m_xmf4x4Texture._11 = 1.0f / float(nRows);
	m_xmf4x4Texture._22 = 1.0f / float(nCols);
	m_xmf4x4Texture._31 = float(m_nRow) / float(nRows);
	m_xmf4x4Texture._32 = float(m_nCol) / float(nCols);
	if (fTime == 0.0f)
	{
		//if (++m_nCol == nCols)
		//{
		//	m_nRow++;
		//	m_nCol = 0;
		//}
		if (++m_nRow == nRows)
		{
			m_nCol++;
			m_nRow = 0;
		}
		if (m_nCol == nCols)
		{
			//cout << "end" << endl;
			m_isActive = false;
			m_nCol = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CNPCObject::CNPCObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	SetChild(pModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pModel);

	//상호작용 ui
	m_pInteractionUI = new CGameObject(1);
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.5f, 0.5f, 0.f);
	m_pInteractionUI->SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Keyboard_R.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->m_ppPipelineShaders[CScene::PIPE_TEXTURE]);

	m_pInteractionUI->SetMaterial(0, pMaterial);

	XMFLOAT3 xmf3Pos = { 5.f, 0.f, 5.f };
	SetPosition(xmf3Pos);
	xmf3Pos = Vector3::Add(xmf3Pos, Vector3::ScalarProduct(GetRight(), -1.f));
	xmf3Pos = Vector3::Add(xmf3Pos, Vector3::ScalarProduct(GetUp(), 1.5f));

	m_pInteractionUI->SetPosition(xmf3Pos);
}

CNPCObject::~CNPCObject()
{
	if (m_pInteractionUI) delete m_pInteractionUI;
}

void CNPCObject::Animate(float fTimeElapsed)
{
	float fDistance = Vector3::Distance(CGameMgr::GetInstance()->GetPlayer()->GetPosition(), GetPosition());
	if (fDistance > 5.f) SetEffectsType(EFFECT_LIMLIGHT, m_isAbleInteraction = false);
	else SetEffectsType(EFFECT_LIMLIGHT, m_isAbleInteraction = true);

	CGameObject::Animate(fTimeElapsed);

	if (m_pInteractionUI)
	{
		XMFLOAT3 xmf3Target = CGameMgr::GetInstance()->GetCamera()->GetPosition();
		m_pInteractionUI->SetLookAt(xmf3Target);
		m_pInteractionUI->UpdateTransform(NULL);
	}
}

void CNPCObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	UpdateShaderVariables(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);

	if (m_isAbleInteraction && m_pInteractionUI) m_pInteractionUI->Render(pd3dCommandList, pCamera, true); //쉐이더 바꿔주기
}

void CNPCObject::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();

	//호출해줘야 메모리 릭 안남
	if (m_pInteractionUI)	m_pInteractionUI->ReleaseUploadBuffers();
}

void CNPCObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUIObject::CUIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject(1)
{
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.f);
	SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Keyboard_R.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	//pMaterial->SetShader(nullptr/*pShader*/);
	SetMaterial(0, pMaterial);

	SetOrthoWorld(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f);
}

CUIObject::~CUIObject()
{
}

void CUIObject::Animate(float fTimeElapsed)
{
	if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_F))
		SetFadeState(m_isFadeIn = !m_isFadeIn);

	//m_Alpha = 0.8f;
	switch (m_eUIType)
	{
	case CUIObject::UI_FADE:
		if (m_isFadeIn)
		{
			if (m_fAlpha < 1.f) m_fAlpha += fTimeElapsed;
		}
		else
		{
			if (m_fAlpha > 0.f) m_fAlpha -= fTimeElapsed;
			//m_fAlpha -= fTimeElapsed;
			//if (m_fAlpha < -2.f)
			//{
			//	SetFadeState(m_isFadeIn = !m_isFadeIn);
			//}

		}
		break;
	}

	CGameObject::Animate(fTimeElapsed);
}

void CUIObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	UpdateShaderVariables(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CUIObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fAlpha, 34);
}

void CUIObject::SetFadeState(bool isIn)
{
	m_isFadeIn = isIn;

	if (isIn)
	{
		//밝아짐
		m_fAlpha = 0.f;
	}
	else
	{
		//페이드 아웃 - 어두워짐
		m_fAlpha = 1.f;
	}
}

void CUIObject::SetOrthoWorld(float fSizeX, float fSizeY, float fPosX, float fPosY)
{
	// 직교투영
	m_xmf4x4ToParent._11 = fSizeX;
	m_xmf4x4ToParent._22 = fSizeY;
	m_xmf4x4ToParent._33 = 1.f;
	m_xmf4x4ToParent._41 = fPosX - FRAME_BUFFER_WIDTH * 0.5f;
	m_xmf4x4ToParent._42 = -fPosY + FRAME_BUFFER_HEIGHT * 0.5f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CParticleObject::CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CParticleMesh* pMesh = m_pParticleMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.5f, 0.5f), 1.0f);
	SetMesh(pMesh);

	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/RoundParticle.dds", 0);
	//pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Fade.dds", 0);


	XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1000];
	for (int i = 0; i < 1000; i++)
		pxmf4RandomValues[i] = XMFLOAT4(RandomValue(-1.0f, 1.0f), RandomValue(-1.0f, 1.0f), RandomValue(-1.0f, 1.0f), RandomValue(0.0f, 1.0f));

	m_pRandowmValueTexture = new CTexture(1, RESOURCE_BUFFER, 0, 1);
	m_pRandowmValueTexture->LoadBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1000, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);


	CMaterial* pMaterial = new CMaterial(2);
	pMaterial->SetTexture(pParticleTexture);
	pMaterial->SetTexture(m_pRandowmValueTexture, 1); //1넣어줘야지!!!!!!!!!!!!

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, RP_TEXTURE, false);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, RP_RANDOM_BUFFER, false);

	SetMaterial(0, pMaterial);

	SetPosition(10.f, 0.f, 10.f);
}

CParticleObject::~CParticleObject()
{
	//if (m_pRandowmValueTexture)
	//	m_pRandowmValueTexture->ReleaseUploadBuffers();

}

void CParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CShader* pShader)
{

	//CGameObject::Render(pd3dCommandList, pCamera);

	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);

	//0
	pShader->OnPrepareRender(pd3dCommandList, 0);

	m_pParticleMesh->PreRender(pd3dCommandList, 0); //Stream Output
	m_pParticleMesh->Render(pd3dCommandList, 0); //Stream Output

	//갯수읽어오기
	m_pParticleMesh->PostRender(pd3dCommandList, 0); //Stream Output

	//1
	pShader->OnPrepareRender(pd3dCommandList, 1);

	m_pParticleMesh->PreRender(pd3dCommandList, 1); //Draw
	m_pParticleMesh->Render(pd3dCommandList, 1); //Draw
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPortalObject::CPortalObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.f);
	SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Fade.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	SetMaterial(0, pMaterial);
}

CPortalObject::~CPortalObject()
{
}

void CPortalObject::Animate(float fTimeElapsed)
{
}

void CPortalObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
}

void CPortalObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}
