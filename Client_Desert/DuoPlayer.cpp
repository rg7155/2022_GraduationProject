#include "DuoPlayer.h"

CDuoPlayer::CDuoPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_xmVecNowRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecTmpRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecNewRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecSrc = XMVectorSet(0.f, 0.f, 1.f, 1.f);

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Adventurer_Aland_Blue.bin", NULL);



	SetChild(pPlayerModel->m_pModelRootObject, true);

	m_pSword = FindFrame("Sword");

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 9, pPlayerModel);



	for (int i = 0; i < ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);

	}
	m_pSkinnedAnimationController->SetTrackEnable(ANIM::IDLE_RELAXED, true);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	//m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.001f, _T("Sound/Footstep01.wav"));
	//m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.125f, _T("Sound/Footstep02.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	/*CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);*/

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);



	if (pPlayerModel) delete pPlayerModel;


	m_bBattleOn = false;
	m_eCurAnim = ANIM::IDLE_RELAXED;
	m_ePrevAnim = ANIM::IDLE_RELAXED;
	m_bBlendingOn = false;
	m_bSkill1EffectOn = false;

	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 0.f;
	m_fBlendingTime = 0.f;
	///////////////////////////////////////////////
	//ÄÄÆ÷³ÍÆ®
	CreateComponent(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_eObjId = OBJ_PLAYER;
	///////////////////////////////////////////////
}

CDuoPlayer::~CDuoPlayer()
{
	ReleaseShaderVariables();

}

void CDuoPlayer::Move(ULONG nDirection, float fDistance, bool bVelocity)
{
}

void CDuoPlayer::Move(XMFLOAT3& xmf3Shift, bool bVelocity)
{
}

void CDuoPlayer::Move(float fxOffset, float fyOffset, float fzOffset)
{
}

void CDuoPlayer::Rotate(float x, float y, float z)
{
}

void CDuoPlayer::Update(float fTimeElapsed)
{
}

void CDuoPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
}

void CDuoPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CDuoPlayer::ReleaseShaderVariables()
{
}

void CDuoPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CDuoPlayer::OnPrepareRender()
{
}

void CDuoPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void CDuoPlayer::LerpRotate(float fTimeElapsed)
{
}

void CDuoPlayer::Blending_Animation(float fTimeElapsed)
{
}

XMFLOAT3 CDuoPlayer::MoveByDir(float fDistance)
{
	return XMFLOAT3();
}

bool CDuoPlayer::Check_Input(float fTimeElapsed)
{
	return false;
}

void CDuoPlayer::Change_Animation(ANIM eNewAnim)
{
}

bool CDuoPlayer::Check_MoveInput()
{
	return false;
}

void CDuoPlayer::CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
}

void CDuoPlayer::UpdateComponent(float fTimeElapsed)
{
}

bool CDuoPlayer::IsNowAttack()
{
	return false;
}
