//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Animation.h"
#include "Component.h"
#include "GameMgr.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CShader;
class CStandardShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

struct SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0, int nRows = 1, int nCols = 1);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	int								m_nTextures = 0;
	ID3D12Resource					**m_ppd3dTextures = NULL;
	ID3D12Resource					**m_ppd3dTextureUploadBuffers;

	UINT*							m_pnResourceTypes = NULL;

	DXGI_FORMAT*					m_pdxgiBufferFormats = NULL;
	int*							m_pnBufferElements = NULL;
	int*							m_pnBufferStrides = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_pd3dSamplerGpuDescriptorHandles = NULL;


public:
	SRVROOTARGUMENTINFO				*m_pRootArgumentInfos = NULL;
	int 							m_nRows = 1; //몇곱몇인지, 이건 텍스쳐에
	int 							m_nCols = 1;
public:
	//만들자마자 해줘야함
	void AddRef() { 
		m_nReferences++;
	}
	void Release() { 
		if (--m_nReferences <= 0) 
			delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex, bool bIsDDSFile=true);
	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex);

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource *GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_MAP			0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_METALLIC_MAP			0x08
#define MATERIAL_EMISSION_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

#define EFFECT_FOG				0x01
#define EFFECT_LIMLIGHT			0x02
#define EFFECT_DISSOLVE			0x04

class CGameObject;

class CMaterial
{
public:
	CMaterial(int nTextures);
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CShader							*m_pShader = NULL;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	void SetShader(CShader *pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture *pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void ReleaseUploadBuffers();

public:
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	bool							m_isAnimationShader = false;

public:
	int 							m_nTextures = 0;
	_TCHAR							(*m_ppstrTextureNames)[64] = NULL;
	CTexture						**m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR *pwstrTextureName, CTexture **ppTexture, CGameObject *pParent, FILE *pInFile, CShader *pShader);

public:
	static CShader					*m_pStandardShader;
	static CShader					*m_pSkinnedAnimationShader;

	static void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { 
		CMaterial::SetShader(m_pSkinnedAnimationShader);
		m_isAnimationShader = true;
	}
};



class CAnimationSets;
class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

	CGameObject* m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationSets* m_pAnimationSets = NULL;

public:
	void PrepareSkinning();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAnimationController;
class CGameObject
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CGameObject();
	CGameObject(int nMaterials);
    virtual ~CGameObject();

public:
	char							m_pstrFrameName[64];

	CMesh							*m_pMesh = NULL;

	int								m_nMaterials = 0;
	CMaterial						**m_ppMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObject 					*m_pParent = NULL;
	CGameObject 					*m_pChild = NULL;
	CGameObject 					*m_pSibling = NULL;

	XMFLOAT3						m_xmf3Scale;

	BoundingOrientedBox				m_xmOOBB;
	bool							m_isRootModelObject = false;
	OBJ_ID							m_eObjId = OBJ_END;
	UINT							m_nEffectsType = 0x00;

	bool							m_isActive = false;

	void SetActiveState(bool isActve) { m_isActive = isActve; }
	void SetEffectsType(UINT nMask, bool isOn);

	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);

	void SetChild(CGameObject *pChild, bool bReferenceUpdate=false);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void Ready() {}
	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, bool isChangePipeline = true);
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, CShader* pShader = NULL);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3& xmf3Scale);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	void SetLookAt(XMFLOAT3& xmf3Target);

	CGameObject *GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent=NULL);
	CGameObject *FindFrame(char *pstrFrameName);

	CTexture *FindReplicatedTexture(_TCHAR *pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

public:
	CAnimationController 			*m_pSkinnedAnimationController = NULL;

	CSkinnedMesh *FindSkinnedMesh(char *pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader);

	static void LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes, bool isRootModelObj, CGameObject* pRootModelObj = nullptr);

	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

	static void PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent);

public:
	virtual void	CollsionDetection(CGameObject* pObj) {};

protected:
	virtual void	CreateComponent() {};

public:
	//vector<CComponent*>		m_pComponent;
	CComponent*		m_pComponent[COM_END];
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL, bool isChangePipeline = true) override;
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
	ID3D12Resource*		m_pd3dcbTexture = NULL;
	CB_TEXTURE_INFO*	m_pcbMappedTexture = NULL;

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

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

protected:
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUIObject : public CGameObject
{
public:
	CUIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CUIObject();

public:
	virtual void Animate(float fTimeElapsed) override;
};