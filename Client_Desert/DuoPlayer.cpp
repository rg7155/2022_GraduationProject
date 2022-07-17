#include "DuoPlayer.h"
#include "GameMgr.h"
#include "Scene.h"

CDuoPlayer::CDuoPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	char fileName[2048];
	int id = *(int*)pContext;
	if (id == 0)
		strcpy(fileName, "Model/Adventurer_Aland_Green.bin");
	else
		strcpy(fileName, "Model/Adventurer_Aland_Blue.bin");

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fileName, NULL);

	SetChild(pPlayerModel->m_pModelRootObject, true);

	m_pSword = FindFrame("Sword");
	m_pSwordTail = FindFrame("SwordTail");

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, PLAYER::ANIM::END, pPlayerModel);

	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);

	}
	m_pSkinnedAnimationController->SetTrackEnable(PLAYER::ANIM::IDLE_RELAXED, true);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pPlayerModel) delete pPlayerModel;

	SetPosition(XMFLOAT3(84.f, 0.f, 96.f));

	CreateComponent(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pComTrail->SetRenderingTrail(false);
	m_pReadyTex = new CTexturedObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CTexturedObject::TEXTURE_READY);

	m_eCurAnim = PLAYER::ANIM::IDLE_RELAXED;

	SetEffectsType(EFFECT_FOG, true);
	m_bSkill1EffectOn = false;

	CGameMgr::GetInstance()->SetDuoPlayer(this);
}

CDuoPlayer::~CDuoPlayer()
{
	ReleaseShaderVariables();

	if (m_pReadyTex)
	{
		m_pReadyTex->ReleaseUploadBuffers();
		m_pReadyTex->ReleaseShaderVariables();
		m_pReadyTex->Release();
	}
}

void CDuoPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CDuoPlayer::ReleaseShaderVariables()
{
}

void CDuoPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
}

void CDuoPlayer::Animate(float fTimeElapsed)
{
	if (m_eCurAnim != PLAYER::SKILL1)
		m_bSkill1EffectOn = false;
	if (m_eCurAnim != PLAYER::SKILL2)
		m_bSkill2EffectOn = false;

	UpdateComponent(fTimeElapsed);
	//렌더링 껐다 켰다-> 공격할때만 나오게 변경
	m_pComTrail->SetRenderingTrail(IsNowAttack());

	UpdateReadyTexture(fTimeElapsed);

	// 바닥 이펙트

	float fAnimElapseTime = m_pSkinnedAnimationController->m_fPosition[m_eCurAnim];
	if (m_eCurAnim == PLAYER::ANIM::SKILL1 && !m_bSkill1EffectOn && fAnimElapseTime > 1.0f)
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Shockwave");
		if (pObj) {
			XMFLOAT3 xmf3Pos = GetPosition();
			XMFLOAT3 xmf3Look = GetLook();
			xmf3Pos.x += xmf3Look.x;
			xmf3Pos.y += 0.1f;
			xmf3Pos.z += xmf3Look.z;
			pObj->SetPosition(xmf3Pos);
			static_cast<CMultiSpriteObject*>(pObj)->SetColor(false);
		}
		m_bSkill1EffectOn = true;

	}
	else if (m_eCurAnim == PLAYER::ANIM::SKILL2 && !m_bSkill2EffectOn && fAnimElapseTime > 0.5f)
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Skill2");
		if (pObj)
		{
			XMFLOAT3 xmf3Pos = GetPosition();
			//xmf3Pos.x += m_xmf3Look.x;
			xmf3Pos.y += 0.1f;
			//xmf3Pos.z += m_xmf3Look.z;
			pObj->SetPosition(xmf3Pos);
			static_cast<CMultiSpriteObject*>(pObj)->SetColor(false);
		}
		m_bSkill2EffectOn = true;
	}
	CGameObject::Animate(fTimeElapsed);
}

void CDuoPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);

	m_pComTrail->RenderTrail(pd3dCommandList, pCamera);
}

void CDuoPlayer::CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pComponent[COM_COLLISION] = CCollision::Create();

	m_pComCollision = static_cast<CCollision*>(m_pComponent[COM_COLLISION]);
	m_pComCollision->m_isStaticOOBB = false;
	if (m_pChild && m_pChild->m_isRootModelObject)
		m_pComCollision->m_xmLocalOOBB = m_pChild->m_xmOOBB;
	m_pComCollision->m_pxmf4x4World = &m_xmf4x4World;
	//m_pComCollision->m_xmf3OBBScale = { 10.f, 1.f, 10.f }; // 바운딩박스 스케일 키움
	m_pComCollision->UpdateBoundingBox();

	m_pComponent[COM_TRAIL] = CTrail::Create(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pComTrail = static_cast<CTrail*>(m_pComponent[COM_TRAIL]);
	m_pComTrail->SetColor(false);
}

void CDuoPlayer::UpdateComponent(float fTimeElapsed)
{
	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	XMFLOAT3 xmf3Corners[8];
	if (m_pComCollision)
	{
		m_pComCollision->UpdateBoundingBox();
		m_pComCollision->m_xmOOBB.GetCorners(xmf3Corners);
	}

	if (m_pComTrail)
		m_pComTrail->AddTrail(m_pSwordTail->GetPosition(), m_pSword->GetPosition());
}

void CDuoPlayer::UpdateReadyTexture(float fTimeElapsed)
{
	if (!m_pReadyTex || !m_isReadyToggle) return;

	m_pReadyTex->Animate(fTimeElapsed);

	CScene* pScene = CGameMgr::GetInstance()->GetScene();
	pScene->AddAlphaObjectToList(m_pReadyTex);

	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos.y += 2.f;
	m_pReadyTex->SetPosition(xmf3Pos);
}

void CDuoPlayer::Update_object_anim(object_anim* _object_anim)
{
	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackWeight(i, _object_anim[i].fWeight);
		m_pSkinnedAnimationController->SetTrackEnable(i, _object_anim[i].bEnable);
		m_pSkinnedAnimationController->m_fPosition[i] = _object_anim[i].fPosition;
	}
}

bool CDuoPlayer::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}
