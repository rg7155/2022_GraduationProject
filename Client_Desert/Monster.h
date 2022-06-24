#pragma once

#include "stdafx.h"
#include "Object.h"

constexpr auto CACTI_POS_INIT1 = XMFLOAT3(84.f, 0.f, 96.f);
constexpr auto CACTI_POS_INIT2 = XMFLOAT3(100.f, 0.f, 85.f);
constexpr auto CACTI_POS_AFTER1 = XMFLOAT3(127.f, 0.f, 105.f);
constexpr auto CACTI_POS_AFTER2 = XMFLOAT3(127.f, 0.f, 95.f);
constexpr auto CACTUS_POS_INIT = XMFLOAT3(127.f, 0.f, 100.f);

constexpr char CACTI1 = 1;
constexpr char CACTI2 = 2;

constexpr char VERSE1 = 0;
constexpr char VERSE2 = 1;
constexpr char VERSE3 = 2;
constexpr char VERSE4 = 3;

constexpr float ATTACK_COOLTIME = 1.5f;

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
	
	virtual void	CollsionDetection(CGameObject* pObj) override;
	virtual void	CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual void	UpdateComponent(float fTimeElapsed);

public:
	void ResetAttackCoolTime(bool bIgnore=true);
	void UpdateAttackCoolTime(float fTimeElapsed);

protected:
	float	fAttackCoolTime = 0.f;
	bool	bAttackInvalid = false;
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

protected:
	CCollision* m_pComCollision = nullptr;

public:
	char			m_nowVerse;


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
	virtual void	CollsionDetection(CGameObject* pObj) override;

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

class CCactiObject : public CMonsterObject
{
private:

public:
	CCactiObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, 
		CLoadedModelInfo* pModel, char type);
	virtual ~CCactiObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;
	virtual void	CollsionDetection(CGameObject* pObj) override;

private:
	CACTI::ANIM	m_ePrevAnim;			// 이전 애니메이션
	CACTI::ANIM	m_eCurAnim;				// 현재 애니메이션
	float		m_fBlendingTime;		// 블렌딩 시간
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float		m_fLerpSpeed;
	bool		m_bLerpSpeedOn;

private:
	bool		m_bBlendingOn;

public:
	XMFLOAT3		m_AfterPos;

public:
	void Change_Animation(CACTI::ANIM eNewAnim);
	void Blending_Animation(float fTimeElapsed);
	void SetNewRotate(XMFLOAT3 xmf3Look);

public:
	CGameObject*	m_pCactus;
	CGameObject*	m_pCacti;

};

class CCactusObject : public CMonsterObject
{
private:

public:
	CCactusObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel,
		CGameObject* pCacti1, CGameObject* pCacti2);
	virtual ~CCactusObject();
	virtual void	CollsionDetection(CGameObject* pObj) override;

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

private:
	CACTUS::ANIM	m_ePrevAnim;			// 이전 애니메이션
	CACTUS::ANIM	m_eCurAnim;				// 현재 애니메이션
	float		m_fBlendingTime;		// 블렌딩 시간
	float		m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float		m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float		m_fLerpSpeed;
	bool		m_bLerpSpeedOn;

private:
	bool	m_bBlendingOn;

public:
	void Change_Animation(CACTUS::ANIM eNewAnim);
	void Blending_Animation(float fTimeElapsed);
	void SetNewRotate(XMFLOAT3 xmf3Look);

public:
	CGameObject* m_pCacti1;
	CGameObject* m_pCacti2;
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