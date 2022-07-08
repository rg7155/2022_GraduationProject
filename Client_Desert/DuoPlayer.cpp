#include "DuoPlayer.h"
#include "GameMgr.h"
#include "Scene.h"

CDuoPlayer::CDuoPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	char fileName[2048];
	int id = *(int*)pContext;
	if (id == 0)
		strcpy(fileName, "Model/Adventurer_Aland_Blue.bin");
	else
		strcpy(fileName, "Model/Adventurer_Aland_Green.bin");

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

	m_eCurAnim = PLAYER::ANIM::IDLE_RELAXED;

	SetEffectsType(EFFECT_FOG, true);
	m_bSkill1EffectOn = false;

	CGameMgr::GetInstance()->SetDuoPlayer(this);
}

CDuoPlayer::~CDuoPlayer()
{
	ReleaseShaderVariables();

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

	UpdateComponent(fTimeElapsed);
	//렌더링 껐다 켰다-> 공격할때만 나오게 변경
	m_pComTrail->SetRenderingTrail(IsNowAttack());

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
	m_pComponent[COM_TRAIL] = CTrail::Create(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pComTrail = static_cast<CTrail*>(m_pComponent[COM_TRAIL]);
	m_pComTrail->SetColor(false);
}

void CDuoPlayer::UpdateComponent(float fTimeElapsed)
{
	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	if (m_pComTrail)
		m_pComTrail->AddTrail(m_pSwordTail->GetPosition(), m_pSword->GetPosition());
}

void CDuoPlayer::set_stat_change(SC_STAT_CHANGE_PACKET* p)
{
	 
}

void CDuoPlayer::Change_Animation(PLAYER::ANIM eNewAnim)
{
	//m_ePrevAnim = m_eCurAnim;
	//m_eCurAnim = eNewAnim;


	//if (m_eCurAnim != PLAYER::SKILL1)
	//	m_bSkill1EffectOn = false;


	//m_fAnimElapsedTime = 0.f;
	//m_fBlendingTime = 0.f;
	//m_bBlendingOn = true;

	//// Prev, Cur 빼고 Enable
	//for (int i = 0; i < PLAYER::ANIM::END; i++)
	//{
	//	if (i == m_ePrevAnim || i == m_eCurAnim)
	//		continue;
	//	m_pSkinnedAnimationController->SetTrackEnable(i, false);
	//	m_pSkinnedAnimationController->SetTrackWeight(i, 0.f);
	//}

	//// 애니메이션 진행시간 
	//m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();


	//m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	//m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);	// 다음 애니메이션 true로, 이전도 아직 true

	//////	// 1 2 3 순으로 애니메이션 진행된다하면. 1,2 블렌딩 중에 3으로 바뀌면 2의 블렌딩값과 3의 1-2의블렌딩값으로 세팅되어야 함
	//if (m_pSkinnedAnimationController->GetTrackWeight(m_ePrevAnim) < 0.8f)
	//{
	//	m_fBlendingTime = m_pSkinnedAnimationController->GetTrackWeight(m_ePrevAnim);

	//	m_fAnimElapsedTime = 1.f - m_fBlendingTime;

	//	m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, m_fBlendingTime);

	//	m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fAnimElapsedTime);
	//	m_fBlendingTime = m_fAnimElapsedTime;
	//	m_fAnimElapsedTime = 0.f;
	//}
	//else
	//{
	//	m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f);	// 애니메이션 3개중첩 방지
	//	m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 0.f);
	//}
	//// 이전 애니메이션의 가중치가 1보다 작으면 1로 바꾸지말고 그때부터 보간해야함
}

bool CDuoPlayer::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}
