#include "Monster.h"
#include "Scene.h"

CMonsterObject::CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CGameObject(1)
{
	SetChild(pModel->m_pModelRootObject, true);
	

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Dissolve.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CTexture* pTexture2 = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture2->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/burn_dissolve.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture2, RP_TEXTURE2, false);


	CMaterial* pMaterial = new CMaterial(2);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetTexture(pTexture2, 1);

	//pMaterial->SetShader(nullptr/*pShader*/);
	SetMaterial(0, pMaterial);

	//if(m_ppMaterials)
	//	m_ppMaterials[0]->SetTexture(pTexture);

	SetEffectsType(EFFECT_DISSOLVE, true);
	m_fDissolve = 0.f; //1에 가까울수록 사라짐


}

CMonsterObject::~CMonsterObject()
{

}

void CMonsterObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return; 

	//m_fDissolve = 0.5;
	//static bool bToggle = false;
	//if(!bToggle)
	//	m_fDissolve += fTimeElapsed * 0.5f;
	//else
	//	m_fDissolve -= fTimeElapsed * 0.5f;

	//if (m_fDissolve >= 1.f || m_fDissolve <= 0.f)
	//	bToggle = !bToggle;

	//cout << m_fDissolve << endl;

	CGameObject::Animate(fTimeElapsed);
	//cout << m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pSkinnedAnimationController->m_pAnimationTracks[0].m_nAnimationSet]->m_fPosition << endl;

}

void CMonsterObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	//디졸브 텍스쳐 업데이트
	for (int i = 0; i < m_nMaterials; i++)
		if (m_ppMaterials[i])	m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CMonsterObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fDissolve, 34);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CBossObject::CBossObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel)
{

	//m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 10, pModel);
	//for (int i = 0; i < 10; i++)
	//{
	//	m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	//	m_pSkinnedAnimationController->SetTrackEnable(i, false);

	//}
	//m_pSkinnedAnimationController->SetTrackEnable(3, true);
}

CBossObject::~CBossObject()
{
}

void CBossObject::Animate(float fTimeElapsed)
{
	CMonsterObject::Animate(fTimeElapsed);
}

void CBossObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	CMonsterObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}


CGolemObject::CGolemObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel)
{
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, GOLEM::ANIM::END, pModel);
	for (int i = 0; i < GOLEM::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);

	}
	m_pSkinnedAnimationController->SetTrackEnable(GOLEM::ANIM::IDLE, true);

	m_eCurAnim = GOLEM::ANIM::IDLE;
	m_ePrevAnim = GOLEM::ANIM::IDLE;
	m_bBlendingOn = false;
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 0.f;
	m_fBlendingTime = 0.f;

	m_bAttack1On = false;
	m_bAttack2On = false;

	m_fAttackTime = 0.f;

}

CGolemObject::~CGolemObject()
{
}

void CGolemObject::Animate(float fTimeElapsed)
{
	if ((m_ePrevAnim == GOLEM::IDLE && m_eCurAnim == GOLEM::RUN) ||
		(m_eCurAnim == GOLEM::IDLE && m_ePrevAnim == GOLEM::RUN))
	{
		Blending_Animation(fTimeElapsed);
	}

	m_fAttackTime += fTimeElapsed;
	m_fAnimElapsedTime += fTimeElapsed;

	if (m_fAttackTime > 8.f)
	{
		Change_Animation(GOLEM::ANIM::ATTACK1);

		m_fAttackTime = 0.f;
	}

	if (m_eCurAnim != GOLEM::IDLE && m_eCurAnim != GOLEM::RUN && m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		Change_Animation(GOLEM::ANIM::IDLE);

	}

	CGameObject::Animate(fTimeElapsed);



}

void CGolemObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	CMonsterObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CGolemObject::Change_Animation(GOLEM::ANIM eNewAnim)
{
	m_ePrevAnim = m_eCurAnim;
	m_eCurAnim = eNewAnim;

	m_fAnimElapsedTime = 0.f;
	m_fBlendingTime = 0.f;
	m_bBlendingOn = true;
	for (int i = 0; i < GOLEM::ANIM::END; i++)
	{
		if (i == m_ePrevAnim || i == m_eCurAnim)
			continue;
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pSkinnedAnimationController->SetTrackWeight(i, 0.f);
	}

	// 애니메이션 진행시간 
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();
	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);	// 다음 애니메이션 true로, 이전도 아직 true

	if ((m_ePrevAnim == GOLEM::IDLE && m_eCurAnim == GOLEM::RUN) ||
		(m_eCurAnim == GOLEM::IDLE && m_ePrevAnim == GOLEM::RUN))
	{
		////	// 1 2 3 순으로 애니메이션 진행된다하면. 1,2 블렌딩 중에 3으로 바뀌면 2의 블렌딩값과 3의 1-2의블렌딩값으로 세팅되어야 함
		if (m_pSkinnedAnimationController->GetTrackWeight(m_ePrevAnim) < 0.8f)
		{
			m_fBlendingTime = m_pSkinnedAnimationController->GetTrackWeight(m_ePrevAnim);

			m_fAnimElapsedTime = 1.f - m_fBlendingTime;

			m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, m_fBlendingTime);

			m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fAnimElapsedTime);
			m_fBlendingTime = m_fAnimElapsedTime;
			m_fAnimElapsedTime = 0.f;
		}
		else
		{
			m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f);	// 애니메이션 3개중첩 방지
			m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 0.f);
		}
		// 이전 애니메이션의 가중치가 1보다 작으면 1로 바꾸지말고 그때부터 보간해야함
	}
	else
	{
		m_pSkinnedAnimationController->SetTrackEnable(m_ePrevAnim, false);
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 0.f);
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 1.f);	// 애니메이션 3개중첩 방지

	}
	
}

void CGolemObject::Blending_Animation(float fTimeElapsed)
{
	if (!m_bBlendingOn)
		return;

	m_fBlendingTime += fTimeElapsed * 2.f;

	if (m_fBlendingTime >= 1.f)
	{
		m_bBlendingOn = false;
		m_pSkinnedAnimationController->SetTrackEnable(m_ePrevAnim, false);
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 0.f);
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 1.f);

		return;
	}


	m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f - m_fBlendingTime);
	m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fBlendingTime);
}
