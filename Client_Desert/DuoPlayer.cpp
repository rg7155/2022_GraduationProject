#include "DuoPlayer.h"

CDuoPlayer::CDuoPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	//클라0번 파란색, 1번-빨간색
	//CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Adventurer_Alice_Red.bin", NULL);
	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Adventurer_Aland_Green.bin", NULL);


	SetChild(pPlayerModel->m_pModelRootObject, true);
	
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 9, pPlayerModel);

	for (int i = 0; i < ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);

	}
	m_pSkinnedAnimationController->SetTrackEnable(ANIM::IDLE_RELAXED, true);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pPlayerModel) delete pPlayerModel;

	SetPosition(15.f, 0.f, 15.f);
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
}

void CDuoPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CDuoPlayer::Server_SetParentAndAnimation(duoPlayer* _duoPlayer)
{
	// 행렬
	m_xmf4x4ToParent = _duoPlayer->xmf4x4World;
	player_anim* _player_anim = _duoPlayer->animInfo;

	for (int i = 0; i < ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackWeight(i, _player_anim[i].fWeight);
		m_pSkinnedAnimationController->SetTrackEnable(i, _player_anim[i].bEnable);
		m_pSkinnedAnimationController->m_fPosition[i] = _player_anim[i].fPosition;
	}
}
