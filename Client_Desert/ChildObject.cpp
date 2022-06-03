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
	//SkyboxCube �̹��� ���鶧 ����Ƽ ������ ��� Right, Left�ݴ�
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"SkyBox/SkyBox_Desert.dds", 0);

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
	//GameObject �Ҹ��ڿ��� ������Ʈ ref ����
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
	else if (m_strName.find("Plane") != string::npos)
	{
		m_isPlane = true;
		m_pComCollision->m_isCollisionIgnore = true;
	}

	//�ǹ� �İ���� �ʰԲ� �� Ű����?
	//m_pComCollision->m_xmLocalOOBB.Extents = Vector3::ScalarProduct(m_pComCollision->m_xmLocalOOBB.Extents, 1.1f, false);
	//XMFLOAT3 xmf3Add = { 0.1f, 0.0f, 0.1f };
	//m_pComCollision->m_xmLocalOOBB.Extents = Vector3::Add(m_pComCollision->m_xmLocalOOBB.Extents, xmf3Add);

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

	//�׸��ڸʿ� ���°ų�, ����̸� �ø� ���ϰ� �׸�
	if (CGameMgr::GetInstance()->m_isShadowMapRendering || m_isPlane)
		CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
	else
	{
		XMFLOAT3 xmf3Extents = m_pChild->m_pMesh->m_xmOOBB.Extents;
		float fMaxExtents = max(xmf3Extents.x, max(xmf3Extents.y, xmf3Extents.z));
		float fMaxRadius = max(m_xmf3Scale.x, max(m_xmf3Scale.y, m_xmf3Scale.z));//������ x,y,z �ٸ�����

		float fRadi = fMaxRadius * fMaxExtents; 

		//�ӽ÷�
		fRadi *= 1.51f;

		//������ ���� �ٸ��� �̻���
		if (static_cast<CFrustum*>(m_pComponent[COM_FRUSTUM])->Isin_Frustum_ForObject(pCamera, &GetPosition(), fRadi))
		{
			CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BLUE_COLOR4 1.f / 255.f, 165.f / 255.f, 172.f / 255.f, 0.f
//#define BLUE_COLOR4 1.f / 255.f, 102.f / 255.f, 200.f / 255.f, 0.f

#define GREEN_COLOR4 65.f / 255.f, 150.f / 255.f, 43.f / 255.f, 0.f

CTrailObject::CTrailObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int iVertexCount)
	: CGameObject(1)
{
	m_pTrailMesh = new CTrailMesh(pd3dDevice, pd3dCommandList, iVertexCount);
	SetMesh(m_pTrailMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Trail.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CTexture* pTexture2 = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture2->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Dissolve2.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture2, RP_TEXTURE2, false);

	CTrailShader* pShader = new CTrailShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	CMaterial* pMaterial = new CMaterial(2);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetTexture(pTexture2, 1);

	pMaterial->SetShader(pShader);
	SetMaterial(0, pMaterial);

	CreateShaderVariables_Sub(pd3dDevice, pd3dCommandList);
	//(CGameMgr::GetInstance()->GetId() == 0) ? m_xmf4Color = { BLUE_COLOR4 } : m_xmf4Color = { GREEN_COLOR4 };
}

CTrailObject::~CTrailObject()
{
	ReleaseShaderVariables_Sub();
}

void CTrailObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	SetCBVInfo(pd3dCommandList, CGameObject::CBV_COLOR, &m_xmf4Color);

	CGameObject::Render(pd3dCommandList, pCamera);
}

void CTrailObject::SetColor(bool isHero)
{
	//����
	if (CGameMgr::GetInstance()->GetId() == 0)
		isHero ? m_xmf4Color = { BLUE_COLOR4 } : m_xmf4Color = { GREEN_COLOR4 };
	else
		isHero ? m_xmf4Color = { GREEN_COLOR4 } : m_xmf4Color = { BLUE_COLOR4 };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMultiSpriteObject::CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, SPRITE_TYPE eType) : CGameObject(1)
{
	//m_fSpeed = 3.0f / (ppSpriteTextures[j]->m_nRows * ppSpriteTextures[j]->m_nCols);

	m_xmf4x4Texture = Matrix4x4::Identity();
	m_fSpeed = 0.001f;
	//m_xmf4Color = { 1.f, 1.f, 1.f, 1.f };

	m_eType = eType;
	switch (eType)
	{
	case CMultiSpriteObject::SPRITE_WAVE:
		break;
	case CMultiSpriteObject::SPRITE_HIT:
		m_isBiliboard = true;
		m_xmf4Color = { BLUE_COLOR4 }; //�÷� �����ָ� �ȳ���?
		break;
	}

	CreateShaderVariables_Sub(pd3dDevice, pd3dCommandList);
}


CMultiSpriteObject::~CMultiSpriteObject()
{
	ReleaseShaderVariables_Sub();
}

void CMultiSpriteObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return;

	m_fTime += fTimeElapsed;
	if (m_fTime >= m_fSpeed)
		m_fTime = 0.0f;

	AnimateRowColumn(m_fTime);

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

	//CBV������Ʈ���� �ٲ���
	//UpdateShaderVariables(pd3dCommandList);
	SetCBVInfo(pd3dCommandList, CGameObject::CBV_TEX_ANIM, &m_xmf4x4Texture);
	SetCBVInfo(pd3dCommandList, CGameObject::CBV_COLOR, &m_xmf4Color);
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

void CMultiSpriteObject::SetColor(bool isHero /*= true*/)
{
	if (CGameMgr::GetInstance()->GetId() == 0)
		isHero ? m_xmf4Color = { BLUE_COLOR4 } : m_xmf4Color = { GREEN_COLOR4 };
	else
		isHero ? m_xmf4Color = { GREEN_COLOR4 } : m_xmf4Color = { BLUE_COLOR4 };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CNPCObject::CNPCObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	SetChild(pModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pModel);

	//��ȣ�ۿ� ui
	m_pInteractionUI = new CGameObject(1);
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.5f, 0.5f, 0.f);
	m_pInteractionUI->SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Keyboard_R.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));

	m_pInteractionUI->SetMaterial(0, pMaterial);

	XMFLOAT3 xmf3Pos = { 18.f, 0.f, 15.f };
	SetPosition(xmf3Pos);
	//xmf3Pos = Vector3::Add(xmf3Pos, Vector3::ScalarProduct(GetRight(), -1.f));
	xmf3Pos = Vector3::Add(xmf3Pos, Vector3::ScalarProduct(GetUp(), 2.f));

	m_pInteractionUI->SetPosition(xmf3Pos);

	SetEffectsType(EFFECT_FOG, true);
}

CNPCObject::~CNPCObject()
{
	if (m_pInteractionUI) delete m_pInteractionUI;
}

void CNPCObject::Animate(float fTimeElapsed)
{
	//if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_H))
	//{
	//	XMFLOAT3 xmf3Offset = { 0.f, 3.f, 5.f };
	//	static_cast<CThirdPersonCamera*>(CGameMgr::GetInstance()->GetCamera())->SetFocusOnTarget(true, GetPosition(), xmf3Offset);
	//}

	
	float fDistance = Vector3::Distance(CGameMgr::GetInstance()->GetPlayer()->GetPosition(), GetPosition());
	if (fDistance > 5.f) SetEffectsType(EFFECT_LIMLIGHT, m_isAbleInteraction = false);
	else SetEffectsType(EFFECT_LIMLIGHT, m_isAbleInteraction = true);

	if (!m_pUIQuest)
		m_pUIQuest = CGameMgr::GetInstance()->GetScene()->m_pUIObjectShader->GetObjectList(L"UI_Quest").front();
	if (m_isAbleInteraction && CInputDev::GetInstance()->KeyDown(DIKEYBOARD_R))
		m_pUIQuest->SetActiveState(true);

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

	if (m_isAbleInteraction && m_pInteractionUI)
	{
		//m_pInteractionUI->Render(pd3dCommandList, pCamera, true); //���̴� �ٲ��ֱ�
		CGameMgr::GetInstance()->GetScene()->AddAlphaObjectToList(m_pInteractionUI);
	}
}

void CNPCObject::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();

	//ȣ������� �޸� �� �ȳ�
	if (m_pInteractionUI)	m_pInteractionUI->ReleaseUploadBuffers();
}

void CNPCObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUIObject::CUIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UI_TYPE eType)
	: CGameObject(1)
{
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.f);
	SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

	m_eUIType = eType;
	m_fAlpha = 1.f;
	SetActiveState(true);
	switch (eType)
	{
	case CUIObject::UI_FADE:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Fade.dds", 0);
		SetOrthoWorld(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f);
		m_fAlpha = 0.f;
		break;
	case CUIObject::UI_PLAYER:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Outcircle.dds", 0);
		SetOrthoWorld(150, 150, 100.f, FRAME_BUFFER_HEIGHT * 0.85f);
		break;
	case CUIObject::UI_PROFILE:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Profile.dds", 0);
		SetOrthoWorld(300, 50, 300.f, FRAME_BUFFER_HEIGHT * 0.9f); 
		break;
	case CUIObject::UI_READY:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Outcircle.dds", 0);
		SetOrthoWorld(100, 100, FRAME_BUFFER_WIDTH - 100.f, FRAME_BUFFER_HEIGHT - 100.f);
		m_isClickedAble = true;
		break;
	case CUIObject::UI_QUEST:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Outcircle.dds", 0);
		SetOrthoWorld(200, 200, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f);
		SetActiveState(false);
	}

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	SetMaterial(0, pMaterial);

}

CUIObject::~CUIObject()
{
}

void CUIObject::Animate(float fTimeElapsed)
{
	switch (m_eUIType)
	{
	case CUIObject::UI_FADE:
		if (m_isStartFade)
		{
			if (m_isFadeIn)
			{
				if (m_fAlpha > 0.f) m_fAlpha -= fTimeElapsed;
				else m_isStartFade = false;
			}
			else
			{
				//m_fAlpha 1�̸� ��ο�
				m_fAlpha += fTimeElapsed;
				if (m_fAlpha > 2.f) SetFadeState(true); //3�� �� �����
				else if (!m_isChangeScene && m_fAlpha > 1.f)
				{
					m_isChangeScene = true;
					CGameMgr::GetInstance()->GetScene()->ChangeScene(SCENE::SCENE_2);
				}
			}
		}
		break;
	case CUIObject::UI_READY:
		if (CInputDev::GetInstance()->LButtonDown())
		{
			XMFLOAT2 Cursor = CGameMgr::GetInstance()->m_xmf2CursorPos;
			if (Cursor.x > m_xmf2Pos.x - m_xmf2Size.x && Cursor.x < m_xmf2Pos.x + m_xmf2Size.x &&
				Cursor.y > m_xmf2Pos.y - m_xmf2Size.y && Cursor.y < m_xmf2Pos.y + m_xmf2Size.y)
				cout << "Cliked" << endl;
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
		//�����
		m_fAlpha = 1.f;
	}
	else
	{
		m_isStartFade = true;
		//���̵� �ƿ� - ��ο���
		m_fAlpha = 0.f;
	}
}

void CUIObject::SetOrthoWorld(float fSizeX, float fSizeY, float fPosX, float fPosY)
{
	m_xmf2Size.x = fSizeX, m_xmf2Size.y = fSizeY;
	m_xmf2Pos.x = fPosX, m_xmf2Pos.y = fPosY;

	// ��������
	m_xmf4x4ToParent._11 = fSizeX;
	m_xmf4x4ToParent._22 = fSizeY;
	m_xmf4x4ToParent._33 = 1.f;
	m_xmf4x4ToParent._41 = fPosX - FRAME_BUFFER_WIDTH * 0.5f;
	m_xmf4x4ToParent._42 = -fPosY + FRAME_BUFFER_HEIGHT * 0.5f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PARTICLE_POS 80.f, 0.01f, 23.f
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
	pMaterial->SetTexture(m_pRandowmValueTexture, 1); //1�־������!!!!!!!!!!!!

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, RP_TEXTURE, false);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, RP_RANDOM_BUFFER, false);

	SetMaterial(0, pMaterial);

	SetPosition(PARTICLE_POS);
}

CParticleObject::~CParticleObject()
{
	//if (m_pRandowmValueTexture)
	//	m_pRandowmValueTexture->ReleaseUploadBuffers();

}

void CParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CShader* pShader)
{
	if (!m_isActive)
		return;
	//CGameObject::Render(pd3dCommandList, pCamera);

	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);

	//0
	pShader->OnPrepareRender(pd3dCommandList, 0);

	m_pParticleMesh->PreRender(pd3dCommandList, 0); //Stream Output
	m_pParticleMesh->Render(pd3dCommandList, 0); //Stream Output

	//�����о����
	m_pParticleMesh->PostRender(pd3dCommandList, 0); //Stream Output

	//1
	pShader->OnPrepareRender(pd3dCommandList, 1);

	m_pParticleMesh->PreRender(pd3dCommandList, 1); //Draw
	m_pParticleMesh->Render(pd3dCommandList, 1); //Draw
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPortalObject::CPortalObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject(1)//�߰�!!!!!!!!!!!!!!!!!!!!!!!!!!
{
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 0.f, 1.f);
	SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Ring.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));

	SetMaterial(0, pMaterial);

	SetPosition(PARTICLE_POS);

	SetScale(3.f, 1.f, 3.f);

	m_fAlpha = 1.f;
}

CPortalObject::~CPortalObject()
{
}

#define RANGE 2.5f
void CPortalObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return;

	m_fAlpha -= fTimeElapsed;
	if (m_fAlpha <= 0.f)
		m_fAlpha = 1.f;

	float fDistanceToPlayer = Vector3::Distance(CGameMgr::GetInstance()->GetPlayer()->GetPosition(), GetPosition());
	float fDistanceToDuo = 0.f;
#ifdef USE_SERVER
	CGameObject* pObj = CGameMgr::GetInstance()->GetDuoPlayer();
	if (pObj)
		fDistanceToDuo = Vector3::Distance(pObj->GetPosition(), GetPosition());
#endif // USE_SERVER

	if (fDistanceToPlayer < RANGE && fDistanceToDuo < RANGE && !m_isOverlap)
	{
		m_isOverlap = true;
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pUIObjectShader->GetObjectList(L"UI_Info").back();
		static_cast<CUIObject*>(pObj)->SetFadeState(false);
		m_isActive = false;
	}
	CGameObject::Animate(fTimeElapsed);
}

void CPortalObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isActive)
		return; 
	
	UpdateShaderVariables(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CPortalObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fAlpha, 34);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTexturedObject::CTexturedObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, TEXTURE_TYPE eType)
	: CGameObject(1)
{
	CMesh* pMesh = NULL;
	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	CMaterial* pMaterial = new CMaterial(1);

	m_eTextureType = eType;
	switch (eType)
	{
	case CTexturedObject::TEXTURE_QUAKE:
		pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 0.f, 1.f);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Earthquake.dds", 0);
		//pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/vfx_ImpactCrack_A.dds", 0);

		pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));
		pMaterial->m_iPipelineState = 1;

		SetScale(2.f, 1.f, 2.f); 
		m_fAlpha = 1.5f;
		m_isAlphaObject = true;
		break;

	case CTexturedObject::TEXTURE_HP:
		pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.f, 0.5f, 0.f);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Hp.dds", 0);
		pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));
		SetActiveState(true);
		break;

	case CTexturedObject::TEXTURE_HP_FRAME:
		pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.f, 0.5f, 0.f);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/HpFrame.dds", 0); 
		pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));
		SetActiveState(true);
		break;
	}

	SetMesh(pMesh);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);
	pMaterial->SetTexture(pTexture);
	SetMaterial(0, pMaterial);
}

CTexturedObject::~CTexturedObject()
{
}

void CTexturedObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return;

	switch (m_eTextureType)
	{
	case TEXTURE_TYPE::TEXTURE_QUAKE:
		CGameMgr::GetInstance()->GetScene()->AddAlphaObjectToList(this);

		//2�ʵ��� ����, 1�ʴ� ������ �����
		m_fAlpha -= fTimeElapsed;
		if (m_fAlpha < 0.f)
		{
			m_fAlpha =1.5f;
			m_isActive = false;
		}
		break;

	case CTexturedObject::TEXTURE_HP:

	case CTexturedObject::TEXTURE_HP_FRAME:
		SetLookAt(CGameMgr::GetInstance()->GetCamera()->GetPosition());
		UpdateTransform(NULL); 
		break;
	}

	CGameObject::Animate(fTimeElapsed);
}

void CTexturedObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isActive || m_isAlphaObject)
		return;

	UpdateShaderVariables(pd3dCommandList);

	// CTexturedShader�� 2��° ���������� ���ڴ�. PSAlphaTextured, ���İ� ���ϴ�
	CGameObject::Render(pd3dCommandList, pCamera, true);
}

void CTexturedObject::AlphaRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	// CTexturedShader�� 2��° ���������� ���ڴ�. PSAlphaTextured, ���İ� ���ϴ�
	CGameObject::Render(pd3dCommandList, pCamera, true);
}

void CTexturedObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//PSAlphaTextured�� gfDissolve�� ����
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fAlpha, 34);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDamageFontObject::CDamageFontObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject(1)
{
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.f);
	SetMesh(pMesh);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	CMaterial* pMaterial = new CMaterial(1);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Number.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);
	pMaterial->SetTexture(pTexture);
	SetMaterial(0, pMaterial);

	pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));
	pMaterial->m_iPipelineState = 2;

	SetPosition(25.0f, 2.f, 25.0f);
	SetDamageFont(973);

	//SetActiveState(true);

	CreateShaderVariables_Sub(pd3dDevice, pd3dCommandList);

	//CBV Set�� ���� ������Ʈ, ������ �ִ� 3�ڸ���
	for (int i = 0; i < 2; ++i)
	{
		CGameObject* pObj = new CGameObject();
		pObj->CreateShaderVariables_Sub(pd3dDevice, pd3dCommandList);
		vecSubObject.emplace_back(pObj);
	}
}

CDamageFontObject::~CDamageFontObject()
{
	ReleaseShaderVariables_Sub();
	for (auto& i : vecSubObject)
	{
		i->ReleaseShaderVariables_Sub();
		i->ReleaseUploadBuffers();
		i->Release();
	}
}

void CDamageFontObject::Animate(float fTimeElapsed)
{
//	WorldToViewPort();
	if (!m_isActive)
		return;

	CGameMgr::GetInstance()->GetScene()->AddAlphaObjectToList(this);

	//SetLookAt(CGameMgr::GetInstance()->GetCamera()->GetPosition());

	CGameObject::Animate(fTimeElapsed);
}

void CDamageFontObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	return;
}

void CDamageFontObject::AlphaRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	//for �ڸ�����ŭ, ����� Right�ุŭ �̵���Ű��
	XMFLOAT4X4 xmf4x4WorldTemp = m_xmf4x4ToParent;
	for (int i = 0; i < (int)(m_strDamage.size()); ++i)
	{
		float iNum = m_strDamage[i] - '0';

		CGameObject* pObj = nullptr;
		i == 0 ? pObj = this : pObj = vecSubObject[i - 1];
		pObj->SetCBVInfo(pd3dCommandList, CGameObject::CBV_DAMAGE_NUMBER, &iNum);

		//XMFLOAT4X4 xmf4x4World = m_xmf4x4ToParent;
		//XMFLOAT3 xmf3CameraRight = CGameMgr::GetInstance()->GetCamera()->GetRightVector();
		XMFLOAT3 xmf3CameraRight = GetRight();

		XMFLOAT3 xmf3Left = Vector3::ScalarProduct(xmf3CameraRight, 0.3f * i, true);
		XMFLOAT3 xmf3Pos = Vector3::Add(GetPosition(), xmf3Left);
		SetPosition(xmf3Pos);

		WorldToViewPort(xmf3Pos);

		//TODO-�ѹ��� ���������� VS���� �ٲ��ֱ�, �Ÿ��� ���� ������ ����?

		//XMFLOAT3 xmf3Scale = { 3.f, 1.f, 1.f };
		//SetScale(xmf3Scale);

		CGameObject::Render(pd3dCommandList, pCamera, true);

		m_xmf4x4ToParent = xmf4x4WorldTemp;
		UpdateTransform(NULL);
	}


}


void CDamageFontObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//PSAlphaTextured�� gfDissolve�� ����
	m_fAlpha = 1.f;
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fAlpha, 34);
}

void CDamageFontObject::SetDamageFont(int iDamage)
{
	m_iDamage = iDamage;
	m_strDamage = to_string(m_iDamage);
}

void CDamageFontObject::WorldToViewPort(XMFLOAT3 xmf3Pos)
{
	XMFLOAT4X4 xmf4x4View = CGameMgr::GetInstance()->GetCamera()->GetViewMatrix();
	XMFLOAT4X4 xmf4x4OrthoProj = CGameMgr::GetInstance()->GetCamera()->GetProjectionMatrix();
	XMFLOAT4X4 xmf4x4ViewPort = CGameMgr::GetInstance()->GetCamera()->GetViewPortMatrix();
	XMFLOAT4X4 WorldViewProj = Matrix4x4::Multiply(Matrix4x4::Multiply(Matrix4x4::Multiply(m_xmf4x4World, xmf4x4View), xmf4x4OrthoProj), xmf4x4ViewPort);





	XMFLOAT4X4 temp = Matrix4x4::Identity();
	XMFLOAT3 xmf3Project;
	XMVECTOR vecPos = XMLoadFloat3(&xmf3Pos);
	XMStoreFloat3(&xmf3Project, XMVector3Project(vecPos, 0.f, 0.f, (float)FRAME_BUFFER_WIDTH, (float)FRAME_BUFFER_HEIGHT, 0.f, 1.f,
		XMLoadFloat4x4(&xmf4x4OrthoProj), XMLoadFloat4x4(&xmf4x4View), XMLoadFloat4x4(&temp)));

	float x = xmf3Project.x , y = xmf3Project.y, z = xmf3Project.z;
	cout << x << "," << y <<  "," << z << endl;

	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4ToParent._11 = 50.f;
	m_xmf4x4ToParent._22 = 50.f;
	m_xmf4x4ToParent._33 = 0.f;
	m_xmf4x4ToParent._41 = x - FRAME_BUFFER_WIDTH * 0.5f;
	m_xmf4x4ToParent._42 = -y + FRAME_BUFFER_HEIGHT * 0.5f;
	m_xmf4x4ToParent._43 = z; //����Ʈ z�� �־������


	UpdateTransform(NULL);
}

