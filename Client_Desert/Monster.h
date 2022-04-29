#pragma once

#include "stdafx.h"
#include "Object.h"

class CMonsterObject : public CGameObject
{
public:
	CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CMonsterObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

public:
	void LerpRotate(float fTimeElapsed);
	virtual void OnPrepareRender();

protected:
	float	m_fDissolve = 0.f; //0~1���� ��

protected:
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR m_xmVecNowRotate;
	XMVECTOR m_xmVecNewRotate;
	XMVECTOR m_xmVecSrc;
	float	m_fLerpSpeed;

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
	GOLEM::ANIM	m_ePrevAnim;			// ���� �ִϸ��̼�
	GOLEM::ANIM	m_eCurAnim;				// ���� �ִϸ��̼�
	float		m_fBlendingTime;		// ���� �ð�
	float		m_fAnimMaxTime;			// ���� �ִϸ��̼��� ���� �ð�
	float		m_fAnimElapsedTime;		// ���� �ִϸ��̼��� �帥 �ð�
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

public:
	short	m_targetId;
};