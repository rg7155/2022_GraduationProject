#pragma once

#include "stdafx.h"

class CGameObject;
class CCollision;
class CCollsionMgr
{
	DECLARE_SINGLETON(CCollsionMgr)

private:
	CCollsionMgr();
	virtual ~CCollsionMgr();

public:
	bool	CheckCollsion(CGameObject* pObj1, CGameObject* pObj2);
	void	CheckCollsion(CGameObject* pObj, list<CGameObject*> listObj, bool isNearCheck = false, bool isCheckOnce = false);
	void	CheckCollsion(list<CGameObject*> listObj1, list<CGameObject*> listObj2);

private:
	//두 점을 반환하는 함수
	XMFLOAT3* CheckLineCloseToPoint(CGameObject* pObj1, CCollision* pCol2);
	XMFLOAT3* CheckLineCloseToPoint(XMFLOAT3& xmf3Point, XMFLOAT3* xmf3Corners);

};

