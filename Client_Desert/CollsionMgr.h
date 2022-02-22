#pragma once

#include "stdafx.h"

class CGameObject;
class CCollsionMgr
{
	DECLARE_SINGLETON(CCollsionMgr)

private:
	CCollsionMgr();
	virtual ~CCollsionMgr();

public:
	bool	CheckCollsion(CGameObject* pObj1, CGameObject* pObj2);
	void	CheckCollsion(CGameObject* pObj, list<CGameObject*> listObj);
	void	CheckCollsion(list<CGameObject*> listObj1, list<CGameObject*> listObj2);


};

