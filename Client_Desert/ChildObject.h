#pragma once

#include "stdafx.h"
#include "Object.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMapObject : public CGameObject
{
public:
	CMapObject();
	virtual ~CMapObject();

public:
	virtual void Ready() override;
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);

	virtual void CreateComponent() override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

public:
	bool		m_isPlane = false;
	bool		m_isCollisionBox = false;
	bool		m_isCollisionBoxRender = false;

	string		m_strName = "";
	CCollision* m_pComCollision = nullptr;

};

class CStoneDoorMapObject;
class CFootHoldMapObject : public CMapObject
{
public:
	CFootHoldMapObject();
	virtual ~CFootHoldMapObject();

public:
	//virtual void Ready() override;
	virtual void Animate(float fTimeElapsed) override;
	//virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);

public:
	vector<CStoneDoorMapObject*> m_vecStoneDoor;
	bool	m_isBeginOverlap = false;

};

class CStoneDoorMapObject : public CMapObject
{
public:
	CStoneDoorMapObject();
	virtual ~CStoneDoorMapObject();

public:
	//virtual void Ready() override;
	virtual void Animate(float fTimeElapsed) override;
	//virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);

public:
	int		m_iState = 0;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//재질이 쉐이더를 가지는 오브젝트 - ex)스카이박스
class CTrailObject : public CGameObject
{
public:
	CTrailObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int iVertexCount);
	virtual ~CTrailObject();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	void		SetColor(bool isPlayerZero);
public:
	CTrailMesh* m_pTrailMesh = nullptr;
	XMFLOAT4	m_xmf4Color;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMultiSpriteObject : public CGameObject
{
public:
	enum SPRITE_TYPE { SPRITE_WAVE, SPRITE_HIT, SPRITE_END };

public:
	CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, SPRITE_TYPE eType);
	virtual ~CMultiSpriteObject();
public:
	void		AnimateRowColumn(float fTime);

	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);

	void SetColor(bool isHero = true);

public:
	SPRITE_TYPE			m_eType = SPRITE_END;

	XMFLOAT4			m_xmf4Color;

	int 				m_nRow = 0; //시간에 의해 바뀌는
	int 				m_nCol = 0;

	XMFLOAT4X4			m_xmf4x4Texture;

	float				m_fSpeed = 0.1f;
	float				m_fTime = 0.0f;

	bool				m_isBiliboard = false;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CNPCObject : public CGameObject
{
public:
	CNPCObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CNPCObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	virtual void ReleaseUploadBuffers() override;

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

private:
	CGameObject* m_pInteractionUI = NULL;
	bool		m_isAbleInteraction = false;

	CGameObject* m_pUIQuest = NULL;

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUIObject : public CGameObject
{
public:
	enum UI_TYPE { UI_FADE, UI_PLAYER, UI_PROFILE, UI_READY_BTN, UI_READY_BTN_CLK, UI_QUEST, UI_CURSOR, UI_HIT_EFFECT, UI_END };

	CUIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UI_TYPE eType);
	virtual ~CUIObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

public:
	void		SetOrthoWorld(float fSizeX, float fSizeY, float fPosX, float fPosY);
	void		SetFadeState(bool isIn);

public:
	UI_TYPE		m_eUIType = UI_END;
	CUIObject* m_pButtonToggle = nullptr;
	bool		m_isHit = false;

private:
	float	m_fAlpha = 0.f;
	bool	m_isFadeIn = false;
	bool	m_isStartFade = false;
	bool	m_isChangeScene = false;

	XMFLOAT2	m_xmf2Size = { 0.f, 0.f };
	XMFLOAT2	m_xmf2Pos = { 0.f, 0.f };

	bool		m_isOnceRender = false;

	float		m_fValue = 0.f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CParticleObject : public CGameObject
{
public:
	CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CParticleObject();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CShader* pShader);

public:
	CTexture* m_pRandowmValueTexture = NULL;
	CParticleMesh* m_pParticleMesh = NULL;

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPortalObject : public CGameObject
{
public:
	CPortalObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CPortalObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

private:
	float m_fAlpha = 0.f;
	bool	m_isOverlap = false;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTexturedObject : public CGameObject
{
public:
	enum TEXTURE_TYPE { TEXTURE_QUAKE, TEXTURE_HP, TEXTURE_HP_FRAME, TEXTURE_READY, TEXTURE_END };

	CTexturedObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, TEXTURE_TYPE eType);
	virtual ~CTexturedObject();

public:
	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;
	virtual void AlphaRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

public:
	TEXTURE_TYPE		m_eTextureType = TEXTURE_END;
private:
	float	m_fAlpha = 0.f;
	bool	m_isAlphaObject = false;
};

