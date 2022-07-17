//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "Player.h"
#include "Scene.h"
#include "InputDev.h"
#include "Monster.h"
#include "Object.h"


CShader::CShader(int nPipelineStates /*= 1*/)
{
	m_nPipelineStates = nPipelineStates;

	if(nPipelineStates >= 1)
		m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	if (m_ppd3dPipelineStates)
	{
		for (int i = 0; i < m_nPipelineStates; i++) 
			if (m_ppd3dPipelineStates[i]) 
				m_ppd3dPipelineStates[i]->Release();
		delete[] m_ppd3dPipelineStates;
	}
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateHullShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateDomainShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}



D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char *pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char *)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

#define _WITH_WFOPEN
//#define _WITH_STD_STREAM

#ifdef _WITH_STD_STREAM
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#endif

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob)
{
	UINT nReadBytes = 0;
#ifdef _WITH_WFOPEN
	FILE *pFile = NULL;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize = ::ftell(pFile);
	BYTE *pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	nReadBytes = (UINT)::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
#endif
#ifdef _WITH_STD_STREAM
	std::ifstream ifsFile;
	ifsFile.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	nReadBytes = (int)ifsFile.tellg();
	BYTE *pByteCode = new BYTE[*pnReadBytes];
	ifsFile.seekg(0);
	ifsFile.read((char *)pByteCode, nReadBytes);
	ifsFile.close();
#endif

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	if (ppd3dShaderBlob)
	{
		*ppd3dShaderBlob = NULL;
		HRESULT hResult = D3DCreateBlob(nReadBytes, ppd3dShaderBlob);
		memcpy((*ppd3dShaderBlob)->GetBufferPointer(), pByteCode, nReadBytes);
		d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
		d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
	}
	else
	{
		d3dShaderByteCode.BytecodeLength = nReadBytes;
		d3dShaderByteCode.pShaderBytecode = pByteCode;
	}

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout(int nPipelineState)
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}


D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_STREAM_OUTPUT_DESC CShader::CreateStreamOuputState(int nPipelineState)
{
	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	::ZeroMemory(&d3dStreamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));

	d3dStreamOutputDesc.NumEntries = 0;
	d3dStreamOutputDesc.NumStrides = 0;
	d3dStreamOutputDesc.pBufferStrides = NULL;
	d3dStreamOutputDesc.pSODeclaration = NULL;
	d3dStreamOutputDesc.RasterizedStream = 0;

	return(d3dStreamOutputDesc);
}

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, int nPipelineState /*= 0*/)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dGeometryShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nPipelineState);
	m_d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryShaderBlob, nPipelineState);//파티클
	m_d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, nPipelineState);
	m_d3dPipelineStateDesc.StreamOutput = CreateStreamOuputState(nPipelineState); //파티클
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(nPipelineState);
	m_d3dPipelineStateDesc.BlendState = CreateBlendState(nPipelineState);
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(nPipelineState);
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout(nPipelineState);
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	//m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//m_d3dPipelineStateDesc.NumRenderTargets = 1;
	//m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = GetPrimitiveTopologyType(nPipelineState); //2
	m_d3dPipelineStateDesc.NumRenderTargets = GetNumRenderTargets(nPipelineState); //3
	for (UINT i = 0; i < GetNumRenderTargets(nPipelineState); i++) m_d3dPipelineStateDesc.RTVFormats[i] = GetRTVFormat(nPipelineState, nPipelineState); //4
	m_d3dPipelineStateDesc.DSVFormat = GetDSVFormat(nPipelineState); //5

	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_ppd3dPipelineStates[nPipelineState]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState)
{
	if (m_ppd3dPipelineStates && m_ppd3dPipelineStates[nPipelineState]) 
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);
}

void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState /*= 0*/, bool isChangePipeline /*= true*/)
{
	OnPrepareRender(pd3dCommandList, nPipelineState);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkyBox", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStandardShader::CStandardShader(int nPipelineStates /*= 1*/)
	: CShader(nPipelineStates)
{
}

CStandardShader::~CStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader(int nPipelineStates /*= 1*/)
	:CStandardShader(nPipelineStates)
{
}

CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStandardObjectsShader::CStandardObjectsShader(int nPipelineStates)
	: CStandardShader(nPipelineStates)
{
}

CStandardObjectsShader::~CStandardObjectsShader()
{
}

void CStandardObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
}

void CStandardObjectsShader::ReleaseObjects()
{
	for (auto& iter : m_mapObject)
		for (auto& iterSec : iter.second)
			iterSec->Release();
	//m_listObjects.clear();

}

void CStandardObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	for (auto& iter : m_mapObject)
	{
		for (auto& iterSec : iter.second)
		{
			if (!iterSec->m_isActive)
				continue;
			iterSec->Animate(m_fElapsedTime);
			iterSec->UpdateTransform(NULL);
		}
	}

	//auto& mapiter_begin = m_mapObject.begin();//맵 이터
	//auto& mapiter_end = m_mapObject.end();

	//for (; mapiter_begin != mapiter_end; )
	//{
	//	auto& iter_begin = mapiter_begin->second.begin();//리스트이터
	//	auto& iter_end = mapiter_begin->second.end();

	//	for (; iter_begin != iter_end; )
	//	{
	//		(*iter_begin)->Animate(fTimeElapsed);
	//		(*iter_begin)->UpdateTransform(NULL);
	//		if ((*iter_begin)->m_isDead)
	//		{
	//			delete (*iter_begin);
	//			iter_begin = mapiter_begin->second.erase(iter_begin);
	//		}
	//		else
	//			++iter_begin;
	//	}
	//	++mapiter_begin;
	//}
}

void CStandardObjectsShader::ReleaseUploadBuffers()
{
	for (auto& iter : m_mapObject) 
		for (auto& iterSec : iter.second)
			iterSec->ReleaseUploadBuffers();
}

void CStandardObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState /*= 0*/, bool isChangePipeline /*= true*/)
{
	if(isChangePipeline)
		CStandardShader::Render(pd3dCommandList, pCamera, nPipelineState);

	for (auto & iter : m_mapObject)
	{
		for (auto& iterSec : iter.second)
		{
			if (!iterSec->m_isActive)
				continue; 
			iterSec->UpdateTransform(NULL);
			iterSec->Render(pd3dCommandList, pCamera, isChangePipeline);
		}
	}

}

void CStandardObjectsShader::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CShader* pShader)
{
	for (auto& iter : m_mapObject)
	{
		for (auto& iterSec : iter.second)
		{
			if (!iterSec->m_isActive)
				continue; 
			iterSec->UpdateTransform(NULL);
			iterSec->ShadowRender(pd3dCommandList, pCamera, pShader);
		}
	}
}

HRESULT CStandardObjectsShader::AddObject(const wchar_t* pObjTag, CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	auto iter = m_mapObject.find(pObjTag);
	if (iter == m_mapObject.end())
	{
		list<CGameObject*> ListObj;
		ListObj.emplace_back(pGameObject);
		m_mapObject.emplace(pObjTag, ListObj);
	}
	else
		iter->second.emplace_back(pGameObject);

	//pGameObject->Ready();

	return S_OK;
}


HRESULT CStandardObjectsShader::AddObjectOnlyKey(const wchar_t* pObjTag)
{
	//충돌위해 find할때 못찾을 상황 방지
	list<CGameObject*> ListObj;
	m_mapObject.emplace(pObjTag, ListObj);

	return S_OK;
}

list<CGameObject*>& CStandardObjectsShader::GetObjectList(const wchar_t* pObjTag)
{
	return m_mapObject.find(pObjTag)->second;
}

CGameObject* CStandardObjectsShader::SetActive(const wchar_t* pObjTag)
{
	auto& iter = m_mapObject.find(pObjTag);
	if (iter == m_mapObject.end())
		return nullptr;

	for (auto& list_iter : iter->second)
	{
		if (!list_iter->m_isActive)
		{
			//list_iter->m_isActive = true;
			list_iter->SetActiveState(true);
			return list_iter;
		}
	}
	
	//모든 오브젝트가 활동중일때 예외처리 필요
	cout << "All Object Actived" << endl;

	return nullptr;
}

void CStandardObjectsShader::SetInactiveAllObject()
{
	for (auto& iter : m_mapObject)
		for (auto& iterSec : iter.second)
			iterSec->SetActiveState(false);
}

void CStandardObjectsShader::SetActiveStateObjects(const wchar_t* pObjTag, bool isActive)
{
	auto& iter = m_mapObject.find(pObjTag);
	if (iter == m_mapObject.end())
		return;

	for (auto& list_iter : iter->second)
		list_iter->SetActiveState(isActive);

}

void CStandardObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& iter : m_mapObject)
	{
		for (auto& iterSec : iter.second)
		{
			if (!iterSec->m_isActive)
				continue;
			iterSec->UpdateShaderVariables(pd3dCommandList);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMapObjectsShader::CMapObjectsShader()
{
}

CMapObjectsShader::~CMapObjectsShader()
{
}


void CMapObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	
	//LoadFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Data/SceneTest_Transform.bin", true);
	//LoadFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Data/Scene1_Transform_NoCol.bin", true);
	LoadFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Data/Scene0_Transform.bin", 0);
	LoadFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Data/Scene1_Transform.bin", 1);
	LoadFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Data/Scene2_Transform.bin", 2);

}

void CMapObjectsShader::AnimateObjects(float fTimeElapsed)
{
	CStandardObjectsShader::AnimateObjects(fTimeElapsed);
}

void CMapObjectsShader::LoadFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pFileName, int iSceneIndex)
{
	FILE* pInFile = NULL;

	if (::fopen_s(&pInFile, pFileName, "rb"))
		return;
	::rewind(pInFile); //스트림 맨위로

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	int nObjects = ReadIntegerFromFile(pInFile);

	vector<CStoneDoorMapObject*> vecStoneDoor;
	vector<CFootHoldMapObject*> vecFootHold;

	for (int i = 0; i < nObjects; ++i)
	{
		if (ReadStringFromFile(pInFile, pstrToken))
		{
			CLoadedModelInfo* pMapModel = NULL;
			int iLength = (int)strlen(pstrToken);

			for (int j = 0; j < iLength; j++)
			{
				if (pstrToken[j] == ' ')
				{
					pstrToken[j] = '\0';
					break;
				}
			}

			string str(pstrToken);
			auto iter = m_mapModelInfo.find(str);
			if (iter == m_mapModelInfo.end())
			{
				//삽입
				char pName[64] = "Model/Map/";

				strcat_s(pName, pstrToken);
				strcat_s(pName, ".bin");

				pMapModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pName, this);
				m_mapModelInfo.emplace(str, pMapModel);
			}
			else
			{
				pMapModel = iter->second;
			}

			CGameObject* pGameObject = nullptr;
			if (str.find("foothold") != string::npos)
			{
				pGameObject = new CFootHoldMapObject();
				vecFootHold.emplace_back(static_cast<CFootHoldMapObject*>(pGameObject));
			}
			else if (str.find("stonedoor") != string::npos)
			{
				pGameObject = new CStoneDoorMapObject();
				vecStoneDoor.emplace_back(static_cast<CStoneDoorMapObject*>(pGameObject));
			}
			else
				pGameObject = new CMapObject();

			pGameObject->SetChild(pMapModel->m_pModelRootObject, true);

			//크자이로 읽어옴
			XMFLOAT3 xmf3Scale = ReadVectorFromFile(pInFile, 3);
			XMFLOAT3 xmf3Rotaion = ReadVectorFromFile(pInFile, 3);
			pGameObject->Rotate(xmf3Rotaion.x, xmf3Rotaion.y, xmf3Rotaion.z);
			pGameObject->SetScale(xmf3Scale);

			XMFLOAT3 xmf3Position = ReadVectorFromFile(pInFile, 3);
			pGameObject->SetPosition(xmf3Position);

			CMapObject* pMapObject = static_cast<CMapObject*>(pGameObject);
			pMapObject->m_strName = str;
			pMapObject->Ready();

			pMapObject->SetActiveState(iSceneIndex == 0 ? true : false);

			if(iSceneIndex == 0)
				AddObject(L"Lobby", pGameObject);
			else if (iSceneIndex == 1)
				AddObject(L"Map", pGameObject);
			else if (iSceneIndex == 2)
				AddObject(L"Map2", pGameObject); 
		}
	}

	//발판과 돌문 연결
	int cnt = 0;
	for (auto& iter1 : vecFootHold) {
		iter1->m_FootId = cnt++;
		for (auto& iter2 : vecStoneDoor)
			iter1->m_vecStoneDoor.emplace_back(iter2);
	}
		

}

void CMapObjectsShader::ActiveObjectByChangeScene(SCENE eScene)
{
	SetInactiveAllObject();

	const wchar_t* pObjTag = L"";
	switch (eScene)
	{
	case SCENE_0:
		pObjTag = L"Lobby";
		break;
	case SCENE_1:
		pObjTag = L"Map";
		break;
	case SCENE_2:
		pObjTag = L"Map2";
		break;
	}

	auto& list = GetObjectList(pObjTag);

	for (auto& iter : list)
		iter->SetActiveState(true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMonsterObjectsShader::CMonsterObjectsShader()
{
}

CMonsterObjectsShader::~CMonsterObjectsShader()
{
	m_mapModelInfo.clear();
}

HRESULT CMonsterObjectsShader::CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag)
{

	return S_OK;
}

void CMonsterObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	//CLoadedModelInfo* pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Cacti.bin", NULL);
	//m_mapModelInfo.emplace(L"Monster", pModel);

	//CMonsterObject* pObj = new CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
	////pObj->Rotate(90.f, 0.f, 0.f); // Boss 몬스터에 추가해야함
	//AddObject(L"Monster", pObj);
	//pObj->SetActiveState(true);


	CLoadedModelInfo* pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/ms01_Golem_5.bin", NULL);
	m_mapModelInfo.emplace(L"Golem", pModel);

	CMonsterObject* pObj = new CGolemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
	//pObj->SetActiveState(true);
	//pObj->SetPosition(CACTI_POS_AFTER1);
	AddObject(L"Golem", pObj);
	
	// Cacti
	pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Cacti.bin", NULL);
	m_mapModelInfo.emplace(L"Cacti", pModel);

	CGameObject* pCacti1 = new CCactiObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel, CACTI1);
	pCacti1->SetActiveState(true);
	AddObject(L"Cacti", pCacti1);

	pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Cacti.bin", NULL);
	m_mapModelInfo.emplace(L"Cacti", pModel);

	CGameObject* pCacti2 = new CCactiObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel, CACTI2);
	pCacti2->SetActiveState(true);
	AddObject(L"Cacti", pCacti2);

	// Cactus
	pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Cactus.bin", NULL);
	m_mapModelInfo.emplace(L"Cactus", pModel);

	CGameObject* pCactus = new CCactusObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel, pCacti1, pCacti2);
	pCactus->SetActiveState(false);
	pCactus->SetPosition(CACTUS_POS_INIT);
	AddObject(L"Cactus", pCactus);

	// Cactic 세팅
	static_cast<CCactiObject*>(pCacti1)->m_pCactus = pCactus;
	static_cast<CCactiObject*>(pCacti2)->m_pCactus = pCactus;
	static_cast<CCactiObject*>(pCacti1)->m_pCacti = pCacti2;
	static_cast<CCactiObject*>(pCacti2)->m_pCacti = pCacti1;

}

void CMonsterObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState/* = 0*/, bool isChangePipeline /*= true*/)
{
	//파이프라인을 바꾸지 않음
	CStandardObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState, isChangePipeline);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CNPCObjectsShader::CNPCObjectsShader()
{
}

CNPCObjectsShader::~CNPCObjectsShader()
{
	m_mapModelInfo.clear();
}

HRESULT CNPCObjectsShader::CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag)
{

	return S_OK;
}

void CNPCObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CLoadedModelInfo* pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Dwarf_Warrior_Orange.bin", NULL);
	m_mapModelInfo.emplace(L"Dwarf", pModel);

	CNPCObject* pObj = new CNPCObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
	AddObject(L"NPC", pObj);
	pObj->SetActiveState(true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkinnedAnimationObjectsShader::CSkinnedAnimationObjectsShader()
{
}

CSkinnedAnimationObjectsShader::~CSkinnedAnimationObjectsShader()
{
}

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
}

void CSkinnedAnimationObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CSkinnedAnimationObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CSkinnedAnimationObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState /*= 0*/, bool isChangePipeline /*= true*/)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			//m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTexturedShader::CTexturedShader(int nPipelineStates)
	: CStandardObjectsShader(nPipelineStates)
{
}

CTexturedShader::~CTexturedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTexturedShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CTexturedShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; //컬링안함
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CTexturedShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE; // True면 포함여부를 계산할때 알파값 사용, 다중샘플링 위한
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC CTexturedShader::CreateDepthStencilState(int nPipelineState)
{
	//return CStandardObjectsShader::CreateDepthStencilState(nPipelineState);

	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	if (nPipelineState == 2) //데미지 폰트
	{
		d3dDepthStencilDesc.DepthEnable = FALSE; //깊이 검사x
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; 
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3dDepthStencilDesc.StencilEnable = FALSE; //스텐실 검사x
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	}
	else
	{
		d3dDepthStencilDesc.DepthEnable = TRUE;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; //깊이 쓰기x, 다른거 가리지 않게끔
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3dDepthStencilDesc.StencilEnable = FALSE;
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	}
	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CTexturedShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 2) //데미지 폰트
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDamageFontTextured", "vs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if(nPipelineState == 0 )
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
	else if (nPipelineState == 1 || nPipelineState == 2)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSAlphaTextured", "ps_5_1", ppd3dShaderBlob));
	else
		return(CShader::CreatePixelShader(ppd3dShaderBlob, nPipelineState));

}

void CTexturedShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState /*= 0*/)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nPipelineState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTrailShader::CTrailShader()
{
}

CTrailShader::~CTrailShader()
{
}

D3D12_SHADER_BYTECODE CTrailShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedTrail", "ps_5_1", ppd3dShaderBlob));
}

D3D12_DEPTH_STENCIL_DESC CTrailShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE; 
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; //깊이 쓰기x, 다른거 가리지 않게끔
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE; 
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMultiSpriteObjectsShader::CMultiSpriteObjectsShader()
{
}

CMultiSpriteObjectsShader::~CMultiSpriteObjectsShader()
{
}

D3D12_SHADER_BYTECODE CMultiSpriteObjectsShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteAnimation", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CMultiSpriteObjectsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if(nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSpriteAnimationShockwave", "ps_5_1", ppd3dShaderBlob));
	else
		return(CShader::CreatePixelShader(ppd3dShaderBlob, nPipelineState));

}

void CMultiSpriteObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 0); //쇼크웨이브
	//CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 1); //
}

HRESULT CMultiSpriteObjectsShader::CreateObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pObjTag)
{
	CMesh* pMesh = m_mapObjectInfo.find(pObjTag)->second.first;
	CMaterial* pMaterial = m_mapObjectInfo.find(pObjTag)->second.second;

	//분기문 태우기
	CGameObject* pObject = nullptr;
	CMultiSpriteObject::SPRITE_TYPE eType = CMultiSpriteObject::SPRITE_TYPE::SPRITE_END;

	if (!wcscmp(pObjTag, L"Shockwave"))
		eType = CMultiSpriteObject::SPRITE_TYPE::SPRITE_SKILL1;
	else if (!wcscmp(pObjTag, L"HitEffect"))
		eType = CMultiSpriteObject::SPRITE_TYPE::SPRITE_HIT;
	else if (!wcscmp(pObjTag, L"Skill2"))
		eType = CMultiSpriteObject::SPRITE_TYPE::SPRITE_SKILL2;
	else if (!wcscmp(pObjTag, L"Wind"))
		eType = CMultiSpriteObject::SPRITE_TYPE::SPRITE_WIND;

	pObject = new CMultiSpriteObject(pd3dDevice, pd3dCommandList, eType);
	pObject->SetMesh(pMesh);
	pObject->SetMaterial(0, pMaterial);

	AddObject(pObjTag, pObject);

	return S_OK;
}

void CMultiSpriteObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	//방법1-오브젝트를 미리 만들어둔다-오브젝트풀링
	//방법2-텍스쳐와 매쉬만 미리 만들고, 오브젝트는 런타임중에 만든다

	//쇼크웨이브
	CMesh* pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList,5.f, 0.f, 5.f);
	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 8, 8);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/vfx_shockwave_B-x8.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	m_mapObjectInfo.emplace(L"Shockwave", make_pair(pMesh, pMaterial));
	for(int i = 0; i < 5; ++i) CreateObject(pd3dDevice, pd3dCommandList, L"Shockwave");


	//히트 이펙트
	pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.f);
	pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 2, 5);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/ShapeFX16.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	m_mapObjectInfo.emplace(L"HitEffect", make_pair(pMesh, pMaterial));
	for (int i = 0; i < 5; ++i) CreateObject(pd3dDevice, pd3dCommandList, L"HitEffect");


	//스킬2 이펙트
	pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5.f, 0.f, 5.f);
	pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 2, 7);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/ShapeFX36.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	m_mapObjectInfo.emplace(L"Skill2", make_pair(pMesh, pMaterial));
	for (int i = 0; i < 5; ++i) CreateObject(pd3dDevice, pd3dCommandList, L"Skill2");


	//보스 바람 이펙트
	pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5.f, 0.f, 5.f);
	pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 2, 7);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/ShapeFX11.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	pMaterial = new CMaterial(1);
	pMaterial->SetTexture(pTexture);

	m_mapObjectInfo.emplace(L"Wind", make_pair(pMesh, pMaterial));
	for (int i = 0; i < 10; ++i) CreateObject(pd3dDevice, pd3dCommandList, L"Wind");
}

void CMultiSpriteObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, bool isChangePipeline)
{
	//CStandardObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState, isChangePipeline);

	//TODO키값에 따라 파이프라인 바꿔주기
	if (isChangePipeline)
		CStandardShader::Render(pd3dCommandList, pCamera);

	for (auto& iter : m_mapObject)
	{
		for (auto& iterSec : iter.second)
		{
			if (!iterSec->m_isActive)
				continue;
			iterSec->UpdateTransform(NULL);
			iterSec->Render(pd3dCommandList, pCamera, isChangePipeline);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CUIObjectsShader::CUIObjectsShader()
{
}

CUIObjectsShader::~CUIObjectsShader()
{
}

D3D12_SHADER_BYTECODE CUIObjectsShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSOrthoTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUIObjectsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSAlphaTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_DEPTH_STENCIL_DESC CUIObjectsShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE; //깊이 검사x
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //깊이 쓰기x
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE; //스텐실 검사x
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);

	//::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	//d3dDepthStencilDesc.DepthEnable = TRUE;
	//d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//d3dDepthStencilDesc.StencilEnable = FALSE;
	//d3dDepthStencilDesc.StencilReadMask = 0x00;
	//d3dDepthStencilDesc.StencilWriteMask = 0x00;
	//d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	//d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

}

void CUIObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CGameObject* pObject = nullptr;

	//pObject = new CDamageFontObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//pObject->SetPosition(25.0f, 0, 25.0f);
	//AddObject(L"DamageFont", pObject);

	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_PROFILE);
	AddObject(L"UI_Info", pObject); 
	pObject->SetActiveState(false);
	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_PLAYER);
	AddObject(L"UI_Info", pObject);
	pObject->SetActiveState(false);

	//퀘스트
	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_QUEST);
	AddObject(L"UI_Quest", pObject);

	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_READY_BTN);
	AddObject(L"UI_Button", pObject);
	CGameObject*  pObject1 = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_READY_BTN_CLK);
	AddObject(L"UI_Button", pObject1);

	static_cast<CUIObject*>(pObject)->m_pButtonToggle = static_cast<CUIObject*>(pObject1);
	static_cast<CUIObject*>(pObject1)->m_pButtonToggle = static_cast<CUIObject*>(pObject);


	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_HIT_EFFECT);
	AddObject(L"UI_Hit_Effect", pObject);

	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_CURSOR);
	AddObject(L"UI_Cursor", pObject);


	//젤 마지막에 삽입
	pObject = new CUIObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CUIObject::UI_TYPE::UI_FADE);
	AddObject(L"UI_Fade", pObject);

	

}

void CUIObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, bool isChangePipeline)
{
	CStandardObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState, isChangePipeline);
}

void CUIObjectsShader::ActiveObjectByChangeScene(SCENE eScene)
{
	switch (eScene)
	{
	case SCENE_0:
		break;
	case SCENE_1:
	{
		SetActiveStateObjects(L"UI_Button", false);
		SetActiveStateObjects(L"UI_Info", true);
	}
		break;
	case SCENE_2:
		break;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CParticleShader::CParticleShader()
	: CTexturedShader(2)
{
}

CParticleShader::~CParticleShader()
{
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE CParticleShader::GetPrimitiveTopologyType(int nPipelineState)
{
	return(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
}

UINT CParticleShader::GetNumRenderTargets(int nPipelineState)
{
	return((nPipelineState == 0) ? 0 : 1);  //스트림0, 렌더1
}

DXGI_FORMAT CParticleShader::GetRTVFormat(int nPipelineState, int nRenderTarget)
{
	return((nPipelineState == 0) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R8G8B8A8_UNORM);
}

DXGI_FORMAT CParticleShader::GetDSVFormat(int nPipelineState)
{
	return(DXGI_FORMAT_D24_UNORM_S8_UINT);
}

D3D12_SHADER_BYTECODE CParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleStreamOutput", "vs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSParticleStreamOutput", "gs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSParticleDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CreatePixelShader(ppd3dShaderBlob, 0)); //PS필요 없음
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSParticleDraw", "ps_5_1", ppd3dShaderBlob));
}

D3D12_DEPTH_STENCIL_DESC CParticleShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_INPUT_LAYOUT_DESC CParticleShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 8;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "ACCELERATION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "AGELIFETIME", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "PARTICLETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[7] = { "ALPHA", 0, DXGI_FORMAT_R32_FLOAT, 0, 68, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC CParticleShader::CreateStreamOuputState(int nPipelineState)
{
	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	::ZeroMemory(&d3dStreamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));

	if (nPipelineState == 0)
	{
		UINT nStreamOutputDecls = 8;
		D3D12_SO_DECLARATION_ENTRY* pd3dStreamOutputDecls = new D3D12_SO_DECLARATION_ENTRY[nStreamOutputDecls];
		pd3dStreamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 }; //float3, 0번 슬롯 버퍼로 출력
		pd3dStreamOutputDecls[1] = { 0, "COLOR", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[2] = { 0, "VELOCITY", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[3] = { 0, "ACCELERATION", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[4] = { 0, "SIZE", 0, 0, 2, 0 };
		pd3dStreamOutputDecls[5] = { 0, "AGELIFETIME", 0, 0, 2, 0 };
		pd3dStreamOutputDecls[6] = { 0, "PARTICLETYPE", 0, 0, 1, 0 };
		pd3dStreamOutputDecls[7] = { 0, "ALPHA", 0, 0, 1, 0 };

		UINT* pBufferStrides = new UINT[1];
		pBufferStrides[0] = sizeof(CParticleVertex); //여기도 추가해주기

		d3dStreamOutputDesc.NumEntries = nStreamOutputDecls;
		d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDecls;
		d3dStreamOutputDesc.NumStrides = 1;
		d3dStreamOutputDesc.pBufferStrides = pBufferStrides;
		d3dStreamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM; //래라 넘기지 않음
	}

	return(d3dStreamOutputDesc);
}

void CParticleShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState /*= 0*/)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 0); //Stream Output Pipeline State
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 1); //Draw Pipeline State
}

void CParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CParticleObject* pObject = new CParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pObject->SetActiveState(true);
	AddObject(L"Particle", pObject);
}

void CParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, bool isChangePipeline)
{
	for (auto& iter : m_mapObject)
		for (auto& iterSec : iter.second)
			static_cast<CParticleObject*>(iterSec)->Render(pd3dCommandList, pCamera, this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float Random()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
	float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
	xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
	xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
	xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}

