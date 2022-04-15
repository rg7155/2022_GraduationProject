#include "Shadow.h"

//헤더에 다른 헤더들 선언하고, 전방선언 해주기

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDepthRenderShader::CDepthRenderShader(CStandardObjectsShader* pObjectsShader, LIGHT* pLights)
	: CSkinnedAnimationStandardShader(2) //파이프라인 2개 쓰겠다
{
	m_pObjectsShader = pObjectsShader;

	m_pLights = pLights;
	m_pToLightSpaces = new TOLIGHTSPACES;

	XMFLOAT4X4 xmf4x4ToTexture = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f };
	m_xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);
}

CDepthRenderShader::~CDepthRenderShader()
{
	if (m_pToLightSpaces) delete m_pToLightSpaces;
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	//애니메이션x
	if (nPipelineState == 0)
		return(CStandardShader::CreateVertexShader(ppd3dShaderBlob, nPipelineState));
	else if (nPipelineState == 1)
		return(CSkinnedAnimationStandardShader::CreateVertexShader(ppd3dShaderBlob, nPipelineState));
	else
		return CShader::CreateVertexShader(ppd3dShaderBlob, nPipelineState);
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shader_Shadow.hlsl", "PSDepthWriteShader", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CDepthRenderShader::CreateInputLayout(int nPipelineState)
{
	//애니메이션x
	if (nPipelineState == 0)
		return(CStandardShader::CreateInputLayout(nPipelineState));
	else if (nPipelineState == 1)
		return(CSkinnedAnimationStandardShader::CreateInputLayout(nPipelineState));
	else
		return CShader::CreateInputLayout(nPipelineState);
}

D3D12_DEPTH_STENCIL_DESC CDepthRenderShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //D3D12_COMPARISON_FUNC_LESS_EQUAL
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

D3D12_RASTERIZER_DESC CDepthRenderShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
#ifdef _WITH_RASTERIZER_DEPTH_BIAS
	d3dRasterizerDesc.DepthBias = 10000;
#endif
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void CDepthRenderShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 0);//건물
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 1);//애니메이션 모델
}

void CDepthRenderShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	{
		D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
		::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
		d3dDescriptorHeapDesc.NumDescriptors = MAX_DEPTH_TEXTURES; //조명마다 렌더타겟 만듬
		d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		d3dDescriptorHeapDesc.NodeMask = 0;
		//렌더타겟을 위한 서술자힙 생성
		HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

		m_pDepthTexture = new CTexture(MAX_DEPTH_TEXTURES, RESOURCE_TEXTURE2D_ARRAY, 0);
		m_pDepthTexture->AddRef();

		//깊이값만 써서 r32,
		D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R32_FLOAT, { 1.0f, 1.0f, 1.0f, 1.0f } };
		//화면 해상도와 같이
		for (UINT i = 0; i < MAX_DEPTH_TEXTURES; i++)
			m_pDepthTexture->CreateTexture(pd3dDevice, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, RESOURCE_TEXTURE2D, i);

		m_eCurResource[0] = m_eCurResource[1] = D3D12_RESOURCE_STATE_COMMON;

		D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
		d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
		d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;
		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < MAX_DEPTH_TEXTURES; i++)
		{
			ID3D12Resource* pd3dTextureResource = m_pDepthTexture->GetTexture(i);
			pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
			m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle;
			d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
		}

		d3dDescriptorHeapDesc.NumDescriptors = 1;
		d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		//DSV 서술자힙
		hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);

		//버퍼 만들고
		D3D12_RESOURCE_DESC d3dResourceDesc;
		d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		d3dResourceDesc.Alignment = 0;
		d3dResourceDesc.Width = _DEPTH_BUFFER_WIDTH;
		d3dResourceDesc.Height = _DEPTH_BUFFER_HEIGHT;
		d3dResourceDesc.DepthOrArraySize = 1;
		d3dResourceDesc.MipLevels = 1;
		d3dResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
		d3dResourceDesc.SampleDesc.Count = 1;
		d3dResourceDesc.SampleDesc.Quality = 0;
		d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES d3dHeapProperties;
		::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
		d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		d3dHeapProperties.CreationNodeMask = 1;
		d3dHeapProperties.VisibleNodeMask = 1;

		d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		d3dClearValue.DepthStencil.Depth = 1.0f;
		d3dClearValue.DepthStencil.Stencil = 0;

		pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthBuffer);

		//뷰를 만들고
		D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
		::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
		d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_d3dDsvDescriptorCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		pd3dDevice->CreateDepthStencilView(m_pd3dDepthBuffer, &d3dDepthStencilViewDesc, m_d3dDsvDescriptorCPUHandle);

		//카메라 생성
		for (int i = 0; i < MAX_DEPTH_TEXTURES; i++)
		{
			m_ppDepthRenderCameras[i] = new CCamera();
			m_ppDepthRenderCameras[i]->SetViewport(0, 0, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_ppDepthRenderCameras[i]->SetScissorRect(0, 0, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT);
			m_ppDepthRenderCameras[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//조명 위치에 따른 정보 초기화
	for (int j = 0; j < MAX_LIGHTS; j++)
	{
		//if (m_pLights[j].m_bEnable)
		//{
		XMFLOAT3 xmf3Position = m_pLights[0].m_xmf3Position;
		XMFLOAT3 xmf3Look = m_pLights[0].m_xmf3Direction;
		XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, +1.0f, 0.0f);

		XMMATRIX xmmtxView = XMMatrixLookToLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Up));

		float fNearPlaneDistance = 10.0f, fFarPlaneDistance = m_pLights[0].m_fRange;

		XMMATRIX xmmtxProjection = XMMatrixIdentity();
		if (m_pLights[0].m_nType == DIRECTIONAL_LIGHT)
		{
			float fWidth = _PLANE_WIDTH, fHeight = _PLANE_HEIGHT;
			//직교 투영
			xmmtxProjection = XMMatrixOrthographicLH(fWidth, fHeight, fNearPlaneDistance, fFarPlaneDistance);
		}

		m_ppDepthRenderCameras[j]->SetPosition(xmf3Position);
		XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4View, xmmtxView);
		XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4Projection, xmmtxProjection);

		XMMATRIX xmmtxToTexture = XMMatrixTranspose(xmmtxView * xmmtxProjection * m_xmProjectionToTexture);
		XMStoreFloat4x4(&m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4x4ToTexture, xmmtxToTexture);

		m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position = XMFLOAT4(xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);
		//}
		//else
		//{
		//	m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position.w = 0.0f;
		//}
	}

	CScene::CreateShaderResourceViews(pd3dDevice, m_pDepthTexture, RP_DEPTH_BUFFER, false);
}

void CDepthRenderShader::ReleaseObjects()
{
	for (int i = 0; i < MAX_DEPTH_TEXTURES; i++)
	{
		if (m_ppDepthRenderCameras[i])
		{
			m_ppDepthRenderCameras[i]->ReleaseShaderVariables();
			delete m_ppDepthRenderCameras[i];
		}
	}

	if (m_pDepthTexture) m_pDepthTexture->Release();
	if (m_pd3dDepthBuffer) m_pd3dDepthBuffer->Release();

	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
}

void CDepthRenderShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbDepthElementBytes;

	ncbDepthElementBytes = ((sizeof(TOLIGHTSPACES) + 255) & ~255); //256의 배수
	m_pd3dcbToLightSpaces = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbDepthElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbToLightSpaces->Map(0, NULL, (void**)&m_pcbMappedToLightSpaces);
}

void CDepthRenderShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedToLightSpaces, m_pToLightSpaces, sizeof(TOLIGHTSPACES));

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbToLightGpuVirtualAddress = m_pd3dcbToLightSpaces->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(RP_TO_LIGHT, d3dcbToLightGpuVirtualAddress); //ToLight

	if (m_pDepthTexture) m_pDepthTexture->UpdateShaderVariables(pd3dCommandList);
}

void CDepthRenderShader::ReleaseShaderVariables()
{
	if (m_pd3dcbToLightSpaces)
	{
		m_pd3dcbToLightSpaces->Unmap(0, NULL);
		m_pd3dcbToLightSpaces->Release();
	}
}

void CDepthRenderShader::PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameMgr::GetInstance()->m_isShadowMapRendering = true;

	//1번은 정적(맵), 한번만
	if (!m_isStaticRender)
	{
		RenderToDepthTexture(pd3dCommandList, STATIC_SHADOW);
		m_isStaticRender = true;
	}


	//0번은 동적(플레이어, 몬스터 등), 계속
	RenderToDepthTexture(pd3dCommandList, DYNAMIC_SHADOW);

	CGameMgr::GetInstance()->m_isShadowMapRendering = false;
}

void CDepthRenderShader::RenderToDepthTexture(ID3D12GraphicsCommandList* pd3dCommandList, int iIndex)
{
	::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetTexture(iIndex), m_eCurResource[iIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);

	FLOAT pfClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	pd3dCommandList->ClearRenderTargetView(m_pd3dRtvCPUDescriptorHandles[iIndex], pfClearColor, 0, NULL);
	pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);
	pd3dCommandList->OMSetRenderTargets(1, &m_pd3dRtvCPUDescriptorHandles[iIndex], TRUE, &m_d3dDsvDescriptorCPUHandle);

	if (iIndex == DYNAMIC_SHADOW)
	{
		////1번에 그린 깊이를 0번으로 복사
		::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetTexture(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

		pd3dCommandList->CopyResource(m_pDepthTexture->GetTexture(0), m_pDepthTexture->GetTexture(1));

		::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetTexture(0), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	//조명의 위치에서 씬을 렌더
	Render(pd3dCommandList, m_ppDepthRenderCameras[iIndex], 0, iIndex);

	if (iIndex == DYNAMIC_SHADOW)
		::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetTexture(iIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, m_eCurResource[iIndex] = D3D12_RESOURCE_STATE_COMMON);
	else
		::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetTexture(iIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, m_eCurResource[iIndex] = D3D12_RESOURCE_STATE_COPY_SOURCE);


}


void CDepthRenderShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState/*= 0*/, int iIndex /*= 0*/)
{
	CShader::Render(pd3dCommandList, pCamera, nPipelineState);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	CScene* pScene = CGameMgr::GetInstance()->GetScene();
	if (iIndex == STATIC_SHADOW)
	{
		m_pObjectsShader->Render(pd3dCommandList, pCamera, nPipelineState, false);
		pScene->m_pNPCObjectShader->Render(pd3dCommandList, pCamera, nPipelineState, false);
	}
	else if (iIndex == DYNAMIC_SHADOW)
	{
		m_pPlayer->ShadowRender(pd3dCommandList, pCamera, this);//플레이어는 애님쉐이더, 칼은 스탠다드 쉐이더
		pScene->m_pMonsterObjectShader->ShadowRender(pd3dCommandList, pCamera, this);// ShadowRender(pd3dCommandList, pCamera, this);
	}

}

void CDepthRenderShader::RenderStaticShadow()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTextureToViewportShader::CTextureToViewportShader()
{
}

CTextureToViewportShader::~CTextureToViewportShader()
{
}

D3D12_DEPTH_STENCIL_DESC CTextureToViewportShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
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

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shader_Shadow.hlsl", "VSTextureToViewport", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shader_Shadow.hlsl", "PSTextureToViewport", "ps_5_1", ppd3dShaderBlob));
}

void CTextureToViewportShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	//m_nPipelineStates = 1;
	//m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

void CTextureToViewportShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, bool isChangePipeline /*= true*/)
{
	////f7 on/off
	if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_F7))
		m_bRender = !m_bRender;

	if (!m_bRender)
		return;

	float fSize = FRAME_BUFFER_WIDTH / 4.f;
	D3D12_VIEWPORT d3dViewport = { 0.0f, 0.0f, fSize, fSize, 0.0f, 1.0f };
	D3D12_RECT d3dScissorRect = { 0, 0, (LONG)fSize, (LONG)fSize };
	pd3dCommandList->RSSetViewports(1, &d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &d3dScissorRect);

	CShader::Render(pd3dCommandList, pCamera, nPipelineState);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}
