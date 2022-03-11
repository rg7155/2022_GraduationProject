#pragma once

#include "stdafx.h"

class CScene;
class CPlayer;
class CCamera;

class CGameMgr
{
	DECLARE_SINGLETON(CGameMgr)

private:
	CGameMgr();
	virtual ~CGameMgr();
public:
	void			GameUpdate(float& fTimeElapsed);
public:
	CScene*			GetScene() { return m_pScene; }
	CPlayer*		GetPlayer() { return m_pPlayer; }
	CCamera*		GetCamera() { return m_pCamera; }
	ID3D12Device*	GetDevice() { return m_pd3dDevice; }


	void		SetScene(CScene* pScene) { m_pScene = pScene; }
	void		SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void		SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	void		SetDevice(ID3D12Device* pd3dDevice) { m_pd3dDevice = pd3dDevice; }


private:
	CScene*			m_pScene = nullptr;
	CPlayer*		m_pPlayer = nullptr;
	CCamera*		m_pCamera = nullptr;
	ID3D12Device*	m_pd3dDevice = nullptr;
public:
	bool		m_isShadowMapRendering = false;
	float		m_fElapsedTime = 0.0f;
};

