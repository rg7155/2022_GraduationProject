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
	virtual int	Update_Component(const float& fTimeDelta) { return 0; }

public:
	virtual CComponent* Clone(void) = 0;

};

///////////////////////////////////////////////////////////////////////////////

class CCamera;
class CFrustum : public CComponent
{
private:
	explicit CFrustum();
	explicit CFrustum(const CFrustum& rhs);
	virtual ~CFrustum(void);

public:
	void		Ready_Frustum(void);
	bool		Isin_Frustum(XMFLOAT3* pPos);
	bool		Isin_Frustum(XMFLOAT3* pPos, float& fRadius);
	bool		Isin_Frustum_ForObject(CCamera* pCamera, XMFLOAT3* pPos, float& fRadius);

private:
	XMFLOAT4				GetPlane(XMFLOAT3& xmf3Pos1, XMFLOAT3& xmf3Pos2, XMFLOAT3& xmf3Pos3);

private:
	XMFLOAT3				m_xmf3Point[8];
	XMFLOAT4				m_xmf4Plane[6];

public:
	static CFrustum*		Create();
	virtual CComponent*		Clone(void);
};
#endif // Component_h__
