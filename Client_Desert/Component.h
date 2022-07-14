#ifndef Component_h__
#define Component_h__

#include "stdafx.h"

class CComponent
{
private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	explicit CComponent(void) {};
	virtual ~CComponent(void) {};

public:
	virtual void	Ready_Component(void) { return; }
	virtual void	Update_Component(const float& fTimeDelta) { return; }

public:
	//virtual CComponent* Clone(void) {} ;

};

///////////////////////////////////////////////////////////////////////////////

class CCamera;
class CFrustum : public CComponent
{
private:
	CFrustum();
	virtual ~CFrustum(void);

public:
	virtual void	Ready_Component(void) override;
	bool			Isin_Frustum(XMFLOAT3* pPos);
	bool			Isin_Frustum_ForObject(CCamera* pCamera, XMFLOAT3* pPos, float& fRadius);

	bool		GetIsRender() { return m_isRender; }
public:
	static CFrustum*		Create();

private:
	bool		m_isRender = true;
};

///////////////////////////////////////////////////////////////////////////////

class CCollision : public CComponent
{
private:
	CCollision();
	virtual ~CCollision(void);

public:
	virtual void	Ready_Component(void) override;
	virtual void	Update_Component(const float& fTimeDelta) override;

	bool			Check_Collision(CCollision* pCom);
	bool			Check_Collision(BoundingOrientedBox& xmTargetOOBB);
	bool			Check_Collision(BoundingOrientedBox& xmOOBB, BoundingOrientedBox& xmTargetOOBB);
	bool			Check_Collision_AfterMove(BoundingOrientedBox& xmTargetOOBB, XMFLOAT3& xmf3MovePos, XMFLOAT4X4& xmf4x4World);

	void			UpdateBoundingBox();
public:
	static CCollision* Create();

public:
	//바운딩박스 업데이트 한번만
	bool					m_isStaticOOBB = false; 
	bool					m_isCollision = false;
	bool					m_isCollisionIgnore = false;
	XMFLOAT4X4*				m_pxmf4x4World = nullptr;
	BoundingOrientedBox		m_xmLocalOOBB;
	BoundingOrientedBox		m_xmOOBB;
	XMFLOAT3				m_xmf3OBBScale = { 1.f, 1.f, 1.f };
private:
	bool					m_isOneUpdate = false;

};



///////////////////////////////////////////////////////////////////////////////
class CMesh;
class CTrailObject;
class CTrail : public CComponent
{
private:
	CTrail();
	virtual ~CTrail(void);

public:
	virtual void	Ready_Component(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual void	Update_Component(const float& fTimeDelta);

	void			AddTrail(XMFLOAT3& xmf3Top, XMFLOAT3& xmf3Bottom);
	void			RenderTrail(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void			SetRenderingTrail(bool isOn);
	void			SetColor(bool isHero);
public:
	static CTrail* Create(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

private:
	int								m_iMaxCount = 0;
	int								m_iDivide = 0;
	bool							m_bRender = false;
	float							m_fCreateTime = 0.f;
	float							m_fTime = 0.f;
	list<pair<XMFLOAT3, XMFLOAT3>>	m_listPos; //Top,Bottom
	list<pair<XMFLOAT3, XMFLOAT3>>	m_listRomPos; 
	CTrailObject*					m_pTrailObject = nullptr;
	bool							m_isZeroPlayer = true;
};





#endif // Component_h__
