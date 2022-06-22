#pragma once

#include "stdafx.h"
#include "Object.h"

class CTexturedObject;
class CMonsterObject : public CGameObject
{
public:
	CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CMonsterObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;
	virtual void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, CShader* pShader = NULL) override;

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

public:
	void LerpRotate(float fTimeElapsed);
	virtual void OnPrepareRender();
	void UpdateHpBar(float fTimeElapsed);
	void SetHp(int iDamage);
	void MakeHitEffect();

protected:
	float	m_fDissolve = 0.f; //0~1사이 값

protected:
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR m_xmVecNowRotate;
	XMVECTOR m_xmVecNewRotate;
	XMVECTOR m_xmVecSrc;
	float	m_fLerpSpeed;

	int							m_iHp = 0;
	int							m_iMaxHp = 0;
	CTexturedObject				*m_pHp = NULL;
	CTexturedObject				*m_pHpFrame = NULL;
	float						m_fHpOffsetY = 0.f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBossObject : public CMonsterObject/*CGameObject*/
{
public:
	CBossObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CBossObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;
};



class CGolemObject : public CMonsterObject
{
private:

public:
	CGolemObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CGolemObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

private:
	GOLEM::ANIM	m_ePrevAnim;			// 이전 애니메이션
	GOLEM::ANIM	m_eCurAnim;				// 현재 애니메이션
	float		m_fBlendingTime;		// 블렌딩 시간
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float		m_fLerpSpeed;
	bool		m_bLerpSpeedOn;

private:
	bool	m_bBlendingOn;

public:
	void Change_Animation(GOLEM::ANIM eNewAnim);
	void Blending_Animation(float fTimeElapsed);
	void SetNewRotate(XMFLOAT3 xmf3Look);

public:
	void Check_Collision();

private:
	bool	m_bAttack1On;
	bool	m_bAttack2On;
	float	m_fAttackTime;
	bool	m_bSkill1EffectOn;
public:
	short	m_targetId;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCactiBulletObject : public CGameObject
{
public:
	CCactiBulletObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CCactiBulletObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

public:
	void	SetTarget(XMFLOAT3& xmf3Start, XMFLOAT3& xmf3Target);

private:
	XMFLOAT3	m_xmf3Target = {};
	float		m_fTime = 0;

};