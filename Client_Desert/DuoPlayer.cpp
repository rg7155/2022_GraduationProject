#include "DuoPlayer.h"
#include "GameMgr.h"
#include "Scene.h"

CDuoPlayer::CDuoPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	char fileName[2048];
	m_iId = *((int*)pContext);
	if (m_iId == 0)
		strcpy(fileName, "Model/Adventurer_Aland_Blue.bin");
	else
		strcpy(fileName, "Model/Adventurer_Aland_Green.bin");

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fileName, NULL);

	SetChild(pPlayerModel->m_pModelRootObject, true);

	m_pSword = FindFrame("Sword");

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

	SetPosition(15.f, 0.f, 15.f);


	CreateComponent(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//·»´õ¸µ ²°´Ù Ä×´Ù-> °ø°ÝÇÒ¶§¸¸ ³ª¿À°Ô º¯°æ
	if (IsNowAttack())
		m_pComTrail->SetRenderingTrail(true);
	else
		m_pComTrail->SetRenderingTrail(false);
	m_eCurAnim = PLAYER::ANIM::IDLE_RELAXED;

	m_bSkill1EffectOn = false;
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
	//·»´õ¸µ ²°´Ù Ä×´Ù-> °ø°ÝÇÒ¶§¸¸ ³ª¿À°Ô º¯°æ
	if (IsNowAttack())
		m_pComTrail->SetRenderingTrail(true);
	else
		m_pComTrail->SetRenderingTrail(false);

	// ¹Ù´Ú ÀÌÆåÆ®

	float fAnimElapseTime = m_pSkinnedAnimationController->m_fPosition[m_eCurAnim];
	cout << fAnimElapseTime << endl;
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
}

void CDuoPlayer::UpdateComponent(float fTimeElapsed)
{
	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	if (m_pComTrail)
	{
		XMFLOAT3 xmf3Top = Vector3::Add(m_pSword->GetPosition(), m_pSword->GetUp(), -1.f);
		m_pComTrail->AddTrail(xmf3Top, m_pSword->GetPosition());
	}
}

void CDuoPlayer::Server_SetParentAndAnimation(SC_MOVE_PLAYER_PACKET* packet)
{
	// Çà·Ä
	m_xmf4x4ToParent = packet->xmf4x4World;
	player_anim* _player_anim = packet->animInfo;
	m_eCurAnim = packet->eCurAnim;
	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackWeight(i, _player_anim[i].fWeight);
		m_pSkinnedAnimationController->SetTrackEnable(i, _player_anim[i].bEnable);
		m_pSkinnedAnimationController->m_fPosition[i] = _player_anim[i].fPosition;
	}
}

bool CDuoPlayer::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}
