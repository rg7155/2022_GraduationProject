#ifndef Component_h__
#define Component_h__

#include "stdafx.h"

class CComponent
{
protected:
	explicit CComponent(void) {};
	virtual ~CComponent(void) {};

public:
	virtual int	Update_Component(const float& fTimeDelta) { return 0; }

public:
	virtual CComponent* Clone(void) = 0;

protected:
	virtual void  Free(void) = 0;

};

///////////////////////////////////////////////////////////////////////////////

class CFrustum : public CComponent
{
private:
	explicit CFrustum();
	explicit CFrustum(const CFrustum& rhs);
	virtual ~CFrustum(void);

public:
	void		Ready_Frustum(void);
	bool		Isin_Frustum(XMFLOAT3* pPos);
	bool		Isin_Frustum(XMFLOAT3* pPos, const float& fRadius);
	bool		Isin_Frustum_ForObject(XMFLOAT3* pPos, XMFLOAT3& fRadius);

private:
	XMFLOAT4				GetPlane(XMFLOAT3& xmf3Pos1, XMFLOAT3& xmf3Pos2, XMFLOAT3& xmf3Pos3);

private
	XMFLOAT3				m_xmf3Point[8];
	XMFLOAT4				m_xmf4Plane[6];

public:
	static CFrustum*		Create();
	virtual CComponent*		Clone(void);
	virtual void			Free(void);
};
#endif // Component_h__
