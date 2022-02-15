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

	bool			Check_Collision(BoundingOrientedBox& xmOOBB, BoundingOrientedBox& xmTargetOOBB);
	bool			Check_Collision_AfterMove(BoundingOrientedBox& xmTargetOOBB, XMFLOAT3& xmf3MovePos, XMFLOAT4X4& xmf4x4World);

	void			UpdateBoundingBox();
public:
	static CCollision* Create();

public:
	//바운딩박스 업데이트 한번만
	bool					m_isStaticOOBB = false; 
	bool					m_isCollision = false;
	XMFLOAT4X4*				m_pxmf4x4World = nullptr;
	BoundingOrientedBox		m_xmLocalOOBB;
	BoundingOrientedBox		m_xmOOBB;
private:
	bool					m_isOneUpdate = false;

};


#endif // Component_h__
