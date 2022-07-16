#include "ChildObject.h"

#include "Shader.h"
#include "Scene.h"
#include "ServerManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	//SkyboxCube 이미지 만들때 유니티 에셋의 경우 Right, Left반대
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

	m_pComCollision->m_isCollisionIgnore = true;

	if (m_strName.find("stonedoor") != string::npos)
	{
		m_pComCollision->m_isCollisionIgnore = false;
	}
	else if (m_strName.find("CollisionBox") != string::npos)
	{
		m_pComCollision->m_isCollisionIgnore = false;
		m_isCollisionBox = true;
	}
	else if (m_strName.find("Plane") != string::npos)
		m_isPlane = true;

	//건물 파고들지 않게끔 좀 키워둠?
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
	if (!m_isActive/* || m_isCollisionBox*/)
		return;

	UpdateShaderVariables(pd3dCommandList);

	//그림자맵에 쓰는거나, 평면이면 컬링 안하고 그림
	if (CGameMgr::GetInstance()->m_isShadowMapRendering || m_isPlane)
		CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
	else
	{
		XMFLOAT3 xmf3Extents = m_pChild->m_pMesh->m_xmOOBB.Extents;
		float fMaxExtents = max(xmf3Extents.x, max(xmf3Extents.y, xmf3Extents.z));
		float fMaxRadius = max(m_xmf3Scale.x, max(m_xmf3Scale.y, m_xmf3Scale.z));//스케일 x,y,z 다를수도

		float fRadi = fMaxRadius * fMaxExtents; 

		//임시로
		//fRadi *= 1.51f;

		//포지션 축이 다르면 이상함
		if (static_cast<CFrustum*>(m_pComponent[COM_FRUSTUM])->Isin_Frustum_ForObject(pCamera, &GetPosition(), fRadi))
		{
			CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
		}
	}
}

#define DOOR_NORMAL 0
#define DOOR_UP		1
#define DOOR_DOWN	2



CFootHoldMapObject::CFootHoldMapObject()
{
}

CFootHoldMapObject::~CFootHoldMapObject()
{
}

void CFootHoldMapObject::Animate(float fTimeElapsed)
{
	bool bFoot = 0;
	bFoot = CServerManager::GetInstance()->m_bFoot[m_FootId];

	if (!m_isBeginOverlap && bFoot)
	{
		m_isBeginOverlap = true;
		for (auto& iter : m_vecStoneDoor)
			iter->m_iState = DOOR_DOWN;
	}
	else if (m_isBeginOverlap && !bFoot)
	{
		m_isBeginOverlap = false;
		for (auto& iter : m_vecStoneDoor)
			iter->m_iState = DOOR_UP;
	}
	cout << GetPosition().x << ' ' << GetPosition().y << ' '<<GetPosition().z << ' ' << endl;

	CMapObject::Animate(fTimeElapsed);
}

#define DOOR_MAX_Y 2.39
#define DOOR_MIN_Y -2.39

CStoneDoorMapObject::CStoneDoorMapObject()
{
	//2.39
}

CStoneDoorMapObject::~CStoneDoorMapObject()
{
}

void CStoneDoorMapObject::Animate(float fTimeElapsed)
{

	XMFLOAT3 xmf3Pos = GetPosition();
	if (m_iState == DOOR_UP)
	{
		if (xmf3Pos.y < DOOR_MAX_Y)
			xmf3Pos.y += fTimeElapsed * 10.f;
	}
	else if (m_iState == DOOR_DOWN)
	{
		if (xmf3Pos.y > DOOR_MIN_Y)
			xmf3Pos.y -= fTimeElapsed * 10.f;
	}
	SetPosition(xmf3Pos);

	CMapObject::Animate(fTimeElapsed);
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
	//제로
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
		m_xmf4Color = { BLUE_COLOR4 }; //컬러 안해주면 안나옴?
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

	//CBV오브젝트에서 바꿔줌
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

	//상호작용 ui
	m_pInteractionUI = new CGameObject(1);
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.5f, 0.5f, 0.f, 0.f, 0.f, 0.f, true);
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

	//if (!m_pUIQuest)
	//	m_pUIQuest = CGameMgr::GetInstance()->GetScene()->m_pUIObjectShader->GetObjectList(L"UI_Quest").front();
	if (m_isAbleInteraction && CInputDev::GetInstance()->KeyDown(DIKEYBOARD_R))
		CGameMgr::GetInstance()->GetScene()->AddTextToUILayer(NPC_TEXT);

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
		//m_pInteractionUI->Render(pd3dCommandList, pCamera, true); //쉐이더 바꿔주기
		CGameMgr::GetInstance()->GetScene()->AddAlphaObjectToList(m_pInteractionUI);
	}
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
		SetOrthoWorld(150, 150, 100.f, FRAME_BUFFER_HEIGHT * 0.15f);
		break;
	case CUIObject::UI_PROFILE:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Profile.dds", 0);
		SetOrthoWorld(300, 50, 300.f, FRAME_BUFFER_HEIGHT * 0.1f); 
		break;
	case CUIObject::UI_READY_BTN:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/ReadyButton.dds", 0);
		SetOrthoWorld(300, 100, FRAME_BUFFER_WIDTH - 300.f, FRAME_BUFFER_HEIGHT - 100.f);
		break;
	case CUIObject::UI_READY_BTN_CLK:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/ReadyButton_Clicked.dds", 0);
		SetOrthoWorld(300, 100, FRAME_BUFFER_WIDTH - 300.f, FRAME_BUFFER_HEIGHT - 100.f);
		m_isActive = false;
		break;
	case CUIObject::UI_QUEST:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/TextBox.dds", 0);
		SetOrthoWorld(1000, 200, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.83f);
		SetActiveState(false);
		m_fAlpha = 0.2;
		break;
	case CUIObject::UI_CURSOR:
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Outcircle.dds", 0);
		break;
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
	if (!m_isActive) return;

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
				//m_fAlpha 1이면 어두움
				m_fAlpha += fTimeElapsed;
				if (m_fAlpha > 2.f) SetFadeState(true); //3초 후 밝아짐
				else if (!m_isChangeScene && m_fAlpha > 1.f)
				{
					m_isChangeScene = true;
					//m_isStartFade = false; //수정
					CGameMgr::GetInstance()->GetScene()->ChangeScene(SCENE::SCENE_2);
				}
			}
		}
		break;
	case CUIObject::UI_READY_BTN:
	case CUIObject::UI_READY_BTN_CLK:
		if (m_isOnceRender && CInputDev::GetInstance()->LButtonDown())
		{
			XMFLOAT2 Cursor = CGameMgr::GetInstance()->m_xmf2CursorPos;
			if (Cursor.x > m_xmf2Pos.x - m_xmf2Size.x && Cursor.x < m_xmf2Pos.x + m_xmf2Size.x &&
				Cursor.y > m_xmf2Pos.y - m_xmf2Size.y && Cursor.y < m_xmf2Pos.y + m_xmf2Size.y)
			{
				//cout << "Cliked" << endl;
				if (m_pButtonToggle)
				{
					m_isActive = false;
					m_pButtonToggle->m_isOnceRender = false;
					m_pButtonToggle->m_isActive = true;

				}
				CGameMgr::GetInstance()->GetPlayer()->ClickedReadyButton();
			}
		}
		break;
	case CUIObject::UI_CURSOR:
		XMFLOAT2 Cursor = CGameMgr::GetInstance()->m_xmf2CursorPos;

		SetOrthoWorld(21, 41, Cursor.x, Cursor.y);
		break;
	}

	CGameObject::Animate(fTimeElapsed);
}

void CUIObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isOnceRender) m_isOnceRender = true;

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
		m_fAlpha = 1.f;
	}
	else
	{
		m_isStartFade = true;
		//페이드 아웃 - 어두워짐
		m_fAlpha = 0.f;
	}
}

void CUIObject::SetOrthoWorld(float fSizeX, float fSizeY, float fPosX, float fPosY)
{
	m_xmf2Size.x = fSizeX, m_xmf2Size.y = fSizeY;
	m_xmf2Pos.x = fPosX, m_xmf2Pos.y = fPosY;

	// 직교투영
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
	pMaterial->SetTexture(m_pRandowmValueTexture, 1); //1넣어줘야지!!!!!!!!!!!!

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

	//갯수읽어오기
	m_pParticleMesh->PostRender(pd3dCommandList, 0); //Stream Output

	//1
	pShader->OnPrepareRender(pd3dCommandList, 1);

	m_pParticleMesh->PreRender(pd3dCommandList, 1); //Draw
	m_pParticleMesh->Render(pd3dCommandList, 1); //Draw
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPortalObject::CPortalObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject(1)//추가!!!!!!!!!!!!!!!!!!!!!!!!!!
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
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pUIObjectShader->GetObjectList(L"UI_Fade").front();
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
		pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.f, 0.5f, 0.f, 0.f, 0.f, 0.f, true);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Hp.dds", 0);
		pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));
		SetActiveState(true);
		break;

	case CTexturedObject::TEXTURE_HP_FRAME:
		pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.f, 0.5f, 0.f, 0.f, 0.f, 0.f, true);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/HpFrame.dds", 0); 
		pMaterial->SetShader(CGameMgr::GetInstance()->GetScene()->GetPipelineShader(CScene::PIPE_TEXTURE));
		SetActiveState(true);
		break;
	case CTexturedObject::TEXTURE_READY:
		pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 0.33f, 0.f, 0.f, 0.f, 0.f, true);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Ready3D.dds", 0);
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

		//2초동안 생성, 1초는 서서히 사라짐
		m_fAlpha -= fTimeElapsed;
		if (m_fAlpha < 0.f)
		{
			m_fAlpha =1.5f;
			m_isActive = false;
		}
		break;

	case CTexturedObject::TEXTURE_HP:
	case CTexturedObject::TEXTURE_HP_FRAME:
	case CTexturedObject::TEXTURE_READY:
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

	// CTexturedShader의 2번째 파이프라인 쓰겠다. PSAlphaTextured, 알파가 변하는
	CGameObject::Render(pd3dCommandList, pCamera, true);
}

void CTexturedObject::AlphaRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	// CTexturedShader의 2번째 파이프라인 쓰겠다. PSAlphaTextured, 알파가 변하는
	CGameObject::Render(pd3dCommandList, pCamera, true);
}

void CTexturedObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//PSAlphaTextured의 gfDissolve값 설정
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fAlpha, 34);
}


