#pragma once
#include "stdafx.h"
#include "Player.h"

class CDuoPlayer : public CGameObject
{
public:
	CDuoPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext);
	~CDuoPlayer();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);

private:
	virtual void	CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual void	UpdateComponent(float fTimeElapsed);
	void			UpdateReadyTexture(float fTimeElapsed);

public:
	virtual void	Update_object_anim(object_anim* _object_anim);
	
private:
	CCollision* m_pComCollision = nullptr;
	CTrail* m_pComTrail = nullptr;
	CGameObject* m_pSword = NULL;
	CGameObject* m_pSwordTail = NULL;

	bool	m_bSkill1EffectOn;

	CGameObject*	m_pReadyTex = nullptr;

public:
	bool IsNowAttack();
	
	PLAYER::ANIM	m_eCurAnim;				// 현재 애니메이션
	PLAYER::ANIM	m_ePrevAnim;				// 현재 애니메이션

	bool			m_isReadyToggle = false;

};

