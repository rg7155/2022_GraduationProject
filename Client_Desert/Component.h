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
	void		Ready_Frustum(void);
	bool		Isin_Frustum(XMFLOAT3* pPos);
	bool		Isin_Frustum_ForObject(CCamera* pCamera, XMFLOAT3* pPos, float& fRadius);

	bool		GetIsRender() { return m_isRender; }
public:
	static CFrustum*		Create();

private:
	bool		m_isRender = true;
};
#endif // Component_h__
