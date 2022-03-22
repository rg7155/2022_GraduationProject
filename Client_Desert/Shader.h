//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"

class CShader
{
public:
	CShader(int nPipelineStates = 1);
	virtual ~CShader();

private:
	int									m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { 
		if (--m_nReferences <= 0) 
			delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOuputState(int nPipelineState);

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(int nPipelineState) { return(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); }
	virtual UINT GetNumRenderTargets(int nPipelineState) { return(1); }
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget) { return(DXGI_FORMAT_R8G8B8A8_UNORM); }
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState) { return(DXGI_FORMAT_D24_UNORM_S8_UINT); }

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateHullShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateDomainShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob=NULL);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, int nPipelineState = 0);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0, bool isChangePipeline = true);

	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }

protected:
	ID3D12PipelineState**			m_ppd3dPipelineStates = NULL;
	int								m_nPipelineStates = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	float								m_fElapsedTime = 0.0f;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState) override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardShader : public CShader
{
public:
	CStandardShader(int nPipelineStates = 1);
	virtual ~CStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CStandardObjectsShader : public CStandardShader
{
public:
	CStandardObjectsShader();
	virtual ~CStandardObjectsShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0, bool isChangePipeline = true) override;
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CShader* pShader);

public:
	HRESULT		AddObject(const wchar_t* pObjTag, CGameObject* pGameObject);
	HRESULT		AddObjectOnlyKey(const wchar_t* pObjTag);
	list<CGameObject*>& GetObjectList(const wchar_t* pObjTag);

	virtual HRESULT		CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag) { return S_OK; }
	CGameObject*		SetActive(const wchar_t* pObjTag);
public:
	map<const wchar_t*, list<CGameObject*>>		m_mapObject;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMapObjectsShader : public CStandardObjectsShader
{
public:
	CMapObjectsShader();
	virtual ~CMapObjectsShader();

public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed) override;

	void LoadFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pFileName, bool isActive);
	void ChangeMap(SCENE eScene);

private:
	map<string, CLoadedModelInfo*> m_mapModelInfo;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMonsterObjectsShader : public CStandardObjectsShader
{
public:
	CMonsterObjectsShader();
	virtual ~CMonsterObjectsShader();

	HRESULT		CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag) override;

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0, bool isChangePipeline = true) override;

private:
	map<const wchar_t*, CLoadedModelInfo*>			m_mapModelInfo; //런타임중 로드 위해 사용

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CNPCObjectsShader : public CStandardObjectsShader
{
public:
	CNPCObjectsShader();
	virtual ~CNPCObjectsShader();

	HRESULT		CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag) override;

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);

private:
	map<const wchar_t*, CLoadedModelInfo*>			m_mapModelInfo;

};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedAnimationStandardShader : public CStandardShader
{
public:
	CSkinnedAnimationStandardShader(int nPipelineStates = 1);
	virtual ~CSkinnedAnimationStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkinnedAnimationObjectsShader : public CSkinnedAnimationStandardShader
{
public:
	CSkinnedAnimationObjectsShader();
	virtual ~CSkinnedAnimationObjectsShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0, bool isChangePipeline = true) override;

protected:
	CGameObject						**m_ppObjects = 0;
	int								m_nObjects = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct TOOBJECTSPACEINFO
{
	XMFLOAT4X4						m_xmf4x4ToTexture;//조명좌표계로 바꾸는 행렬
	XMFLOAT4						m_xmf4Position;//조명위치
};

struct TOLIGHTSPACES
{
	TOOBJECTSPACEINFO				m_pToLightSpaces[MAX_LIGHTS];
};

struct LIGHT;

#define _WITH_RASTERIZER_DEPTH_BIAS

#define DYNAMIC_SHADOW 0
#define STATIC_SHADOW 1
class CDepthRenderShader : public CSkinnedAnimationStandardShader
{
public:
	CDepthRenderShader(CStandardObjectsShader* pObjectsShader, LIGHT* pLights);
	virtual ~CDepthRenderShader();

	//virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(int nPipelineState) { return(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); }
	//virtual UINT GetNumRenderTargets(int nPipelineState) { return(1); }
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget) { return(DXGI_FORMAT_R32_FLOAT); }
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState) { return(DXGI_FORMAT_D32_FLOAT); }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState) override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState = 0)override;

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);
	virtual void ReleaseObjects();

	void PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList);
	void RenderToDepthTexture(ID3D12GraphicsCommandList* pd3dCommandList, int iIndex);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0, int iIndex = 0);

protected:
	//깊이 저장 텍스쳐
	CTexture* m_pDepthTexture = NULL;

	//조명 위치에서 깊이정보 저장하기위한 카메라 4개
	CCamera* m_ppDepthRenderCameras[MAX_DEPTH_TEXTURES] = {};

	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRtvCPUDescriptorHandles[MAX_DEPTH_TEXTURES] = {};

	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
	ID3D12Resource* m_pd3dDepthBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle = {};

	//프로젝션을 텍스쳐로 바꾸는 행렬
	XMMATRIX						m_xmProjectionToTexture;

	LIGHT* m_pLights = NULL;

	TOLIGHTSPACES* m_pToLightSpaces;

	ID3D12Resource* m_pd3dcbToLightSpaces = NULL;
	TOLIGHTSPACES* m_pcbMappedToLightSpaces = NULL;
public:
	CTexture* GetDepthTexture() { return(m_pDepthTexture); }
	ID3D12Resource* GetDepthTextureResource(UINT nIndex) { return(m_pDepthTexture->GetTexture(nIndex)); }

public:
	CStandardObjectsShader* m_pObjectsShader = NULL;
	CPlayer* m_pPlayer = NULL;
	bool	m_isStaticRender = false;
};

class CTextureToViewportShader : public CShader
{
public:
	CTextureToViewportShader();
	virtual ~CTextureToViewportShader();

	//virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget) { return DXGI_FORMAT_UNKNOWN; }

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState = 0);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0, bool isChangePipeline = true);

	bool		m_bRender = false;
protected:
	CTexture* m_pDepthTexture = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTexturedShader : public CStandardObjectsShader/*CShader*/
{
public:
	CTexturedShader();
	virtual ~CTexturedShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState) override;

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState = 0) override;

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState) override;
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTrailShader : public CTexturedShader
{
public:
	CTrailShader();
	virtual ~CTrailShader();

	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMultiSpriteObjectsShader : public CTexturedShader
{
public:
	CMultiSpriteObjectsShader();
	virtual ~CMultiSpriteObjectsShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState) override;

	HRESULT		CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag) override;

public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0, bool isChangePipeline = true) override;

protected:
	map<const wchar_t*, pair<CMesh*, CMaterial*>>	m_mapObjectInfo; 

};