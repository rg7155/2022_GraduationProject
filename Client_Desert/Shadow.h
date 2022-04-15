#pragma once

#include "stdafx.h"
#include "Shader.h"
#include "Scene.h"

struct TOOBJECTSPACEINFO
{
	XMFLOAT4X4						m_xmf4x4ToTexture;//������ǥ��� �ٲٴ� ���
	XMFLOAT4						m_xmf4Position;//������ġ
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

	void		RenderStaticShadow();
protected:
	//���� ���� �ؽ���
	CTexture* m_pDepthTexture = NULL;

	//���� ��ġ���� �������� �����ϱ����� ī�޶� 4��
	CCamera* m_ppDepthRenderCameras[MAX_DEPTH_TEXTURES] = {};

	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRtvCPUDescriptorHandles[MAX_DEPTH_TEXTURES] = {};

	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
	ID3D12Resource* m_pd3dDepthBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle = {};

	//���������� �ؽ��ķ� �ٲٴ� ���
	XMMATRIX						m_xmProjectionToTexture;

	LIGHT* m_pLights = NULL;

	TOLIGHTSPACES* m_pToLightSpaces;

	ID3D12Resource* m_pd3dcbToLightSpaces = NULL;
	TOLIGHTSPACES* m_pcbMappedToLightSpaces = NULL;

	D3D12_RESOURCE_STATES m_eCurResource[2];
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


