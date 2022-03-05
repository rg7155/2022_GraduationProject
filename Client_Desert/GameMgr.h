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
	CScene*		GetScene() { return m_pScene; }
	CPlayer*	GetPlayer() { return m_pPlayer; }
	CCamera*	GetCamera() { return m_pCamera; }


	void		SetScene(CScene* pScene) { m_pScene = pScene; }
	void		SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void		SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }

private:
	CScene*		m_pScene = nullptr;
	CPlayer*	m_pPlayer = nullptr;
	CCamera*	m_pCamera = nullptr;

public:
	bool		m_isShadowMapRendering = false;
	float		m_fElapsedTime = 0.0f;
};

