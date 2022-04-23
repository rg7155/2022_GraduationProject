#pragma once

#include "stdafx.h"
#include "Object.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;
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
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

	virtual void CreateComponent() override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

public:
	bool		m_isPlane = false;

	string		m_strName = "";
	CCollision* m_pComCollision = nullptr;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//재질이 쉐이더를 가지는 오브젝트 - ex)스카이박스
class CTrailObject : public CGameObject
{
public:
	CTrailObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int iVertexCount);
	virtual ~CTrailObject();
public:
	CTrailMesh* m_pTrailMesh = nullptr;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CB_TEXTURE_INFO
{
	XMFLOAT4X4		m_xmf4x4TextureAnim;
};
class CMultiSpriteObject : public CGameObject
{
public:
	CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMultiSpriteObject();
public:

	void		AnimateRowColumn(float fTime);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseShaderVariables() override;

	virtual void Animate(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true) override;

public:
	ID3D12Resource* m_pd3dcbTexture = NULL;
	CB_TEXTURE_INFO* m_pcbMappedTexture = NULL;

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
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUIObject : public CGameObject
{
public:
	enum UI_TYPE { UI_FADE, UI_PLAYER, UI_PROFILE, UI_END };

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

private:
	float	m_fAlpha = 0.f;
	bool	m_isFadeIn = false;
	bool	m_isStartFade = false;
	bool	m_isChangeScene = false;
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