//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "CollsionMgr.h"

ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
{
}

CScene::~CScene()
{
}


void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_eCurScene = SCENE_1;

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//전에는 각 쉐이더마다 DescriptorHeap을 만들었다. 지금은 씬에서 딱 한번만 만든다. 이게 편할수도
	//이러면 미리 텍스쳐 몇개 쓰는지 알아야함->오브젝트 추가 될때마다 늘려줘야함
	//미리 여유공간 만들어 놔도 됨->메모리 낭비?지만 터짐 방지
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 13+40); //skybox-1, terrain-2, player-1, map-3, depth-4, traill-1, explsion-1

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials();

	CreateShaders(pd3dDevice, pd3dCommandList);

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_pDuoPlayer = new CDuoPlayer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::CreateShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CShader* pShader = nullptr;

	//////////////////////////////////////////////////
	//오브젝트를 갖지 않는 쉐이더 구현하기, 파이프라인 변경 용
	//CTexturedShader, CPortalObject

	m_nPipelineShaders = PIPE_END;
	m_ppPipelineShaders = new CShader * [m_nPipelineShaders];

	pShader = new CTexturedShader();
	pShader->AddRef(); //다른 오브젝트에서도 참조하기 때문에
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_ppPipelineShaders[PIPE_TEXTURE] = pShader;

	//////////////////////////////////////////////////

	m_nShaders = 7;
	m_ppShaders = new CShader * [m_nShaders];

	int iIndex = 0;
	m_pMapObjectShader = new CMapObjectsShader();
	m_pMapObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pMapObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[iIndex++] = m_pMapObjectShader;

	m_pMonsterObjectShader = new CMonsterObjectsShader();
	m_pMonsterObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pMonsterObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[iIndex++] = m_pMonsterObjectShader;

	m_pNPCObjectShader = new CNPCObjectsShader();
	m_pNPCObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pNPCObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[iIndex++] = m_pNPCObjectShader;

	pShader = new CParticleShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[iIndex++] = pShader;

	//따로 쉐이더를 만들지 않는 오브젝트
	m_pStandardObjectShader = new CStandardObjectsShader(0); //파이프라인 안씀
	m_ppShaders[iIndex++] = m_pStandardObjectShader;
	CreateStandardObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_nAlphaShaderStartIndex = iIndex;
	m_pMultiSpriteObjectShader = new CMultiSpriteObjectsShader();
	m_pMultiSpriteObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pMultiSpriteObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[iIndex++] = m_pMultiSpriteObjectShader;

	m_pUIObjectShader = new CUIObjectsShader();
	m_pUIObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pUIObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[iIndex++] = m_pUIObjectShader;


	//////////////////////////////////////////////////
	//그림자 쉐이더
	m_pDepthRenderShader = new CDepthRenderShader(m_pMapObjectShader, m_pLights);
	m_pDepthRenderShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pDepthRenderShader->BuildObjects(pd3dDevice, pd3dCommandList, NULL);

	m_pShadowMapToViewport = new CTextureToViewportShader();
	m_pShadowMapToViewport->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
}

void CScene::CreateStandardObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//포탈
	CPortalObject* pObj = new CPortalObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pObj->SetActiveState(true);
	m_pStandardObjectShader->AddObject(L"Portal", pObj);
}


ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];
	SetDescriptorRange(pd3dDescriptorRanges, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0);//t6: gtxtAlbedoTexture
	SetDescriptorRange(pd3dDescriptorRanges, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0);//t7: gtxtSpecularTexture
	SetDescriptorRange(pd3dDescriptorRanges, 2, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 0);//t8: gtxtNormalTexture
	SetDescriptorRange(pd3dDescriptorRanges, 3, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0);//t9: gtxtMetallicTexture = Texture2
	SetDescriptorRange(pd3dDescriptorRanges, 4, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10, 0);//t10: gtxtEmissionTexture = RandomBuffer
	SetDescriptorRange(pd3dDescriptorRanges, 5, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11, 0);//t11: gtxtDetailAlbedoTexture
	SetDescriptorRange(pd3dDescriptorRanges, 6, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12, 0);//t12: gtxtDetailNormalTexture
	SetDescriptorRange(pd3dDescriptorRanges, 7, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13, 0);//t13: gtxtSkyBoxTexture
	SetDescriptorRange(pd3dDescriptorRanges, 8, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 14, 0);//t14: gtxtDepthTextures
	SetDescriptorRange(pd3dDescriptorRanges, 9, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);//t0: gtxtTexture

	D3D12_ROOT_PARAMETER pd3dRootParameters[18];
	SetRootParameterCBV(pd3dRootParameters, 0, 1, 0, D3D12_SHADER_VISIBILITY_ALL);//Camera
	SetRootParameterConstants(pd3dRootParameters, 1, 35, 2, 0, D3D12_SHADER_VISIBILITY_ALL);//b2 GameObject, 34개 씀
	SetRootParameterCBV(pd3dRootParameters, 2, 4, 0, D3D12_SHADER_VISIBILITY_ALL);//b4 Lights
	SetRootParameterDescriptorTable(pd3dRootParameters, 3, 1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	SetRootParameterDescriptorTable(pd3dRootParameters, 4, 1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	SetRootParameterDescriptorTable(pd3dRootParameters, 5, 1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_PIXEL);
	SetRootParameterDescriptorTable(pd3dRootParameters, RP_TEXTURE2, 1, &pd3dDescriptorRanges[3], D3D12_SHADER_VISIBILITY_ALL); 
	SetRootParameterDescriptorTable(pd3dRootParameters, RP_RANDOM_BUFFER, 1, &pd3dDescriptorRanges[4], D3D12_SHADER_VISIBILITY_ALL);//기하
	SetRootParameterDescriptorTable(pd3dRootParameters, 8, 1, &pd3dDescriptorRanges[5], D3D12_SHADER_VISIBILITY_PIXEL);
	SetRootParameterDescriptorTable(pd3dRootParameters, 9, 1, &pd3dDescriptorRanges[6], D3D12_SHADER_VISIBILITY_PIXEL);
	SetRootParameterDescriptorTable(pd3dRootParameters, 10, 1, &pd3dDescriptorRanges[7], D3D12_SHADER_VISIBILITY_PIXEL);
	SetRootParameterCBV(pd3dRootParameters, 11, 7, 0, D3D12_SHADER_VISIBILITY_VERTEX);//Skinned Bone Offsets
	SetRootParameterCBV(pd3dRootParameters, 12, 8, 0, D3D12_SHADER_VISIBILITY_VERTEX);//Skinned Bone Transforms
	SetRootParameterDescriptorTable(pd3dRootParameters, RP_DEPTH_BUFFER, 1, &pd3dDescriptorRanges[8], D3D12_SHADER_VISIBILITY_PIXEL); //t14: gtxtDepthTextures
	SetRootParameterCBV(pd3dRootParameters, RP_TO_LIGHT, 3, 0, D3D12_SHADER_VISIBILITY_ALL);//b3 ToLight
	SetRootParameterCBV(pd3dRootParameters, RP_FRAMEWORK_INFO, 5, 0, D3D12_SHADER_VISIBILITY_ALL);//b5 FRAMEWORKInfo
	SetRootParameterDescriptorTable(pd3dRootParameters, RP_TEXTURE, 1, &pd3dDescriptorRanges[9], D3D12_SHADER_VISIBILITY_ALL);
	SetRootParameterCBV(pd3dRootParameters, RP_TEXTUREANIM, 6, 0, D3D12_SHADER_VISIBILITY_ALL);//b6 TextureAnim


	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//비교 필터
	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0.0f;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	//텍스처 읽은색과 현재 깊이와 비교 - 깊이보다 더 작으면 성공 ->그림자가 아님
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}


void CScene::AnimateObjects(float fTimeElapsed)
{
	if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_C))
	{
		//static_cast<CUIObject*>(m_pUIObjectShader->GetObjectList(L"UI_Fade").front())->SetFadeState(true);
		ChangeScene(SCENE_2);
	}

	m_fElapsedTime = fTimeElapsed;
	CGameMgr::GetInstance()->m_fElapsedTime = fTimeElapsed;

	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	m_pDuoPlayer->Animate(fTimeElapsed);

	///////////////////////////////////////////////////////////////////////////////////
	//플레이어-맵 충돌
	switch (m_eCurScene)
	{
	case SCENE_1:
		CCollsionMgr::GetInstance()->CheckCollsion(m_pPlayer, m_pMapObjectShader->GetObjectList(L"Map"), true);
		break;
	case SCENE_2:
		CCollsionMgr::GetInstance()->CheckCollsion(m_pPlayer, m_pMapObjectShader->GetObjectList(L"Map2"), true);
		break;
	}
	///////////////////////////////////////////////////////////////////////////////////

}

//각 프레임마다 제일 먼저 호출됨
void CScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	//각 조명에서 쉐도우맵 만드는 역할, 
	m_pDepthRenderShader->PrepareShadowMap(pd3dCommandList);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//쉐도우맵이 갖고있는 텍스쳐 정보를 set
	m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);


	//그림자 그린다-오브젝트 그린다
	//1.맵 2.이펙트 3.플레이어  / 투명은 나중에 그려야함

	//1.불투명-맵, 몬스터
	for (int i = 0; i < m_nAlphaShaderStartIndex; i++)
		m_ppShaders[i]->Render(pd3dCommandList, pCamera);

	m_pDuoPlayer->Render(pd3dCommandList, pCamera);

	//트레일 렌더링 따로 빼야함, 아님 야매로 불투명 젤 마지막에 렌더하던지..
	m_pPlayer->Render(pd3dCommandList, pCamera);

	//이걸 먼저 해야 그려짐?? 
	for (auto& iter : m_listAlphaObject)
		iter->Render(pd3dCommandList, pCamera, true);
	m_listAlphaObject.clear();
	

	//2.투명-이펙트
	for (int i = m_nAlphaShaderStartIndex; i < m_nShaders; i++)
		m_ppShaders[i]->Render(pd3dCommandList, pCamera);


	//화면에 뎁스 텍스쳐 그린다, 디버깅 용
	//TODO- 투명이펙트에 가려진다. 수정할것.
	if (m_pShadowMapToViewport) m_pShadowMapToViewport->Render(pd3dCommandList, pCamera);


	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);
}




CGameObject* CScene::SetActiveObjectFromShader(const wchar_t* pShaderTag, const wchar_t* pObjTag)
{
	if (!wcscmp(pShaderTag, L"Monster"))
		return m_pMapObjectShader->SetActive(pObjTag);
	else if (!wcscmp(pShaderTag, L"MultiSprite"))
		return m_pMultiSpriteObjectShader->SetActive(pObjTag);
	else
		return nullptr;
}

void CScene::ChangeScene(SCENE eScene)
{
	m_eCurScene = eScene;

	switch (eScene)
	{
	case SCENE_1:
		break;
	case SCENE_2:
		m_pMapObjectShader->ChangeMap(eScene);
		m_pDepthRenderShader->m_isStaticRender = false; //정적 맵 다시 그려라
		break;
	}
}

void CScene::AddAlphaObjectToList(CGameObject* pObj)
{
	m_listAlphaObject.emplace_back(pObj);
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 2;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(-(_PLANE_WIDTH * 0.5f), 300.0f, (_PLANE_WIDTH * 0.5f));
	//m_pLights[0].m_xmf3Position = XMFLOAT3((_PLANE_WIDTH * 0.5f), 150.0f, (_PLANE_WIDTH * 0.5f));
	m_pLights[0].m_fRange = 700.0f;

	m_pLights[1].m_bEnable = false;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
}


void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_ppPipelineShaders)
	{
		for (int i = 0; i < m_nPipelineShaders; i++)
		{
			m_ppPipelineShaders[i]->Release();
		}
		delete[] m_ppPipelineShaders;
	}

	if (m_pDepthRenderShader)
	{
		m_pDepthRenderShader->ReleaseShaderVariables();
		m_pDepthRenderShader->ReleaseObjects();
		m_pDepthRenderShader->Release();
	}
	if (m_pShadowMapToViewport)
	{
		m_pShadowMapToViewport->ReleaseShaderVariables();
		m_pShadowMapToViewport->ReleaseObjects();
		m_pShadowMapToViewport->Release();
	}


	if (m_pSkyBox) delete m_pSkyBox;

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;

	if (m_pDuoPlayer) m_pDuoPlayer->Release();

}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nPipelineShaders; i++) m_ppPipelineShaders[i]->ReleaseUploadBuffers();

	if (m_pDepthRenderShader) m_pDepthRenderShader->ReleaseUploadBuffers();

	if (m_pDuoPlayer) m_pDuoPlayer->ReleaseUploadBuffers();
}


void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType, CTexture* pTexture, int nIndex)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
		case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
		case RESOURCE_TEXTURE2D_ARRAY:
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
			break;
		case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
			d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	/*		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
			d3dShaderResourceViewDesc.Buffer.NumElements = 0;
			d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;*/
			d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
			d3dShaderResourceViewDesc.Buffer.NumElements = pTexture->m_pnBufferElements[nIndex];
			d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;

			d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			d3dShaderResourceViewDesc.Format = pTexture->m_pdxgiBufferFormats[nIndex]; //포멧 임시 지정
			break;
	}
	return(d3dShaderResourceViewDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType, pTexture, i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

void CScene::SetDescriptorRange(D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[], int iIndex, D3D12_DESCRIPTOR_RANGE_TYPE RangeType, UINT NumDescriptors, UINT BaseShaderRegister, UINT RegisterSpace)
{
	pd3dDescriptorRanges[iIndex].RangeType = RangeType;
	pd3dDescriptorRanges[iIndex].NumDescriptors = NumDescriptors;
	pd3dDescriptorRanges[iIndex].BaseShaderRegister = BaseShaderRegister;
	pd3dDescriptorRanges[iIndex].RegisterSpace = RegisterSpace;
	pd3dDescriptorRanges[iIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void CScene::SetRootParameterCBV(D3D12_ROOT_PARAMETER pd3dRootParameter[], int iIndex, UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility)
{
	pd3dRootParameter[iIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[iIndex].Descriptor.ShaderRegister = ShaderRegister;
	pd3dRootParameter[iIndex].Descriptor.RegisterSpace = RegisterSpace;
	pd3dRootParameter[iIndex].ShaderVisibility = ShaderVisibility;
}

void CScene::SetRootParameterDescriptorTable(D3D12_ROOT_PARAMETER pd3dRootParameter[], int iIndex, UINT NumDescriptorRanges, const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges, D3D12_SHADER_VISIBILITY ShaderVisibility)
{
	pd3dRootParameter[iIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameter[iIndex].DescriptorTable.NumDescriptorRanges = NumDescriptorRanges;
	pd3dRootParameter[iIndex].DescriptorTable.pDescriptorRanges = pDescriptorRanges;
	pd3dRootParameter[iIndex].ShaderVisibility = ShaderVisibility;
}

void CScene::SetRootParameterConstants(D3D12_ROOT_PARAMETER pd3dRootParameter[], int iIndex, UINT Num32BitValues, UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility)
{
	pd3dRootParameter[iIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameter[iIndex].Constants.Num32BitValues = Num32BitValues;
	pd3dRootParameter[iIndex].Constants.ShaderRegister = ShaderRegister;
	pd3dRootParameter[iIndex].Constants.RegisterSpace = RegisterSpace;
	pd3dRootParameter[iIndex].ShaderVisibility = ShaderVisibility;
}
