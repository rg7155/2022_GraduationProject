#include "Monster.h"
#include "Scene.h"

CMonsterObject::CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CGameObject(1)
{
	SetChild(pModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pModel);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Dissolve.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CTexture* pTexture2 = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture2->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/burn_dissolve.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture2, 6, false);


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
	static bool bToggle = false;
	if(!bToggle)
		m_fDissolve += fTimeElapsed * 0.5f;
	else
		m_fDissolve -= fTimeElapsed * 0.5f;

	if (m_fDissolve >= 1.f || m_fDissolve <= 0.f)
		bToggle = !bToggle;

	//cout << m_fDissolve << endl;

	CGameObject::Animate(fTimeElapsed);
}

void CMonsterObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	//디졸브 텍스쳐 업데이트
	for (int i = 0; i < m_nMaterials; i++)
		if (m_ppMaterials[i])	m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera);
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
}

CBossObject::~CBossObject()
{
}

void CBossObject::Animate(float fTimeElapsed)
{
	CMonsterObject::Animate(fTimeElapsed);
}

void CBossObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CMonsterObject::Render(pd3dCommandList, pCamera);
}
