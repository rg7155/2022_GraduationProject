#include "DuoPlayer.h"

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
	
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 9, pPlayerModel);

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

void CDuoPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	if (m_bDead)
		return;

	UpdateShaderVariables(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CDuoPlayer::Server_SetParentAndAnimation(SC_MOVE_PLAYER_PACKET* packet)
{
	// За·Д
	m_xmf4x4ToParent = packet->xmf4x4World;
	player_anim* _player_anim = packet->animInfo;

	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackWeight(i, _player_anim[i].fWeight);
		m_pSkinnedAnimationController->SetTrackEnable(i, _player_anim[i].bEnable);
		m_pSkinnedAnimationController->m_fPosition[i] = _player_anim[i].fPosition;
	}
}
