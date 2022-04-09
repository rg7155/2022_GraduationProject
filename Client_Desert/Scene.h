//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"
#include "Component.h"
#include "Shadow.h"
#include "DuoPlayer.h"
#include "ChildObject.h"

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

class CDepthRenderShader;
class CTextureToViewportShader;
class CScene
{
public:
	enum PIPELINE { PIPE_TEXTURE, PIPE_END };
public:
    CScene();
    ~CScene();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseUploadBuffers();

	CGameObject* SetActiveObjectFromShader(const wchar_t* pShaderTag, const wchar_t* pObjTag);
	void ChangeScene(SCENE eScene);

	////////////////////////////////////////////////////////////////////////
	void SetDescriptorRange(D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[], int iIndex, D3D12_DESCRIPTOR_RANGE_TYPE RangeType, UINT NumDescriptors, UINT BaseShaderRegister, UINT RegisterSpace);
	void SetRootParameterCBV(D3D12_ROOT_PARAMETER pd3dRootParameter[], int iIndex, UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility);
	void SetRootParameterDescriptorTable(D3D12_ROOT_PARAMETER pd3dRootParameter[], int iIndex, UINT NumDescriptorRanges, const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges, D3D12_SHADER_VISIBILITY ShaderVisibility);
	void SetRootParameterConstants(D3D12_ROOT_PARAMETER pd3dRootParameter[], int iIndex, UINT Num32BitValues, UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility);

	CPlayer								*m_pPlayer = NULL;
	CDuoPlayer							*m_pDuoPlayer = NULL;


	float								m_fElapsedTime = 0.0f;

	int									m_nShaders = 0;
	int									m_nAlphaShaderStartIndex = 0;

	CShader								**m_ppShaders = NULL;
	CMapObjectsShader					*m_pMapObjectShader = nullptr;
	CMonsterObjectsShader				*m_pMonsterObjectShader = nullptr;
	CNPCObjectsShader					*m_pNPCObjectShader = nullptr;
	CMultiSpriteObjectsShader			*m_pMultiSpriteObjectShader = nullptr;
	CUIObjectsShader					*m_pUIObjectShader = nullptr;

	int									m_nPipelineShaders = 0;
	CShader**							m_ppPipelineShaders = NULL;


	CSkyBox								*m_pSkyBox = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	//obj 그릴때 이걸 읽어서 뎁스들을 dp, sp비교해서 그림자 여부 판단
	CDepthRenderShader					*m_pDepthRenderShader = NULL; //쉐도우맵 만들기 위해 렌더해야하니깐 ㅇ

	//화면에 뎁스텍스쳐 그리기 위해, 디버깅 목적, 4개 다 그릴수도이씅ㅁ
	CTextureToViewportShader			*m_pShadowMapToViewport = NULL;

	SCENE								m_eCurScene = SCENE_END;

protected:
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;

public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

};
