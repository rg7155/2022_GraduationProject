#pragma once
#include "stdafx.h"
#include "Object.h"

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

public:
	void Server_SetParentAndAnimation(SC_MOVE_PLAYER_PACKET* packet);

private:
	CCollision* m_pComCollision = nullptr;
	CTrail* m_pComTrail = nullptr;
	CGameObject* m_pSword = NULL;
	bool	m_bSkill1EffectOn;
public:
	bool IsNowAttack();

public:
	int				m_iId;
	
	PLAYER::ANIM	m_eCurAnim;				// 현재 애니메이션

};

