#include "CollsionMgr.h"
#include "Object.h"

IMPLEMENT_SINGLETON(CCollsionMgr)

CCollsionMgr::CCollsionMgr(void)
{

}

CCollsionMgr::~CCollsionMgr(void)
{
}

bool CCollsionMgr::CheckCollsion(CGameObject* pObj1, CGameObject* pObj2)
{
	CCollision* pCom1 = static_cast<CCollision*>(pObj1->m_pComponent[COM_COLLISION]);
	CCollision* pCom2 = static_cast<CCollision*>(pObj2->m_pComponent[COM_COLLISION]);

	if (pCom1->Check_Collision(pCom2))
	{
		pObj1->CollsionDetection(pObj2);
		pObj2->CollsionDetection(pObj1);

		//pCom1->m_isCollision = true; //자기것만 체크
		return true;
	}
	return false;
}

void CCollsionMgr::CheckCollsion(CGameObject* pObj, list<CGameObject*> listObj, bool isNearCheck /*= false*/)
{
	bool isCol = false;
	for (auto& iter : listObj)
	{
		if (isNearCheck)
		{
			if (20.f > Vector3::Length(Vector3::Subtract(iter->GetPosition(), pObj->GetPosition()))) //상수값 임시로
				if (CheckCollsion(pObj, iter))
					isCol = true;
		}
		else
		{
			if (CheckCollsion(pObj, iter))
				isCol = true;
		}
	}

	static_cast<CCollision*>(pObj->m_pComponent[COM_COLLISION])->m_isCollision = isCol;  //일단 자기것만 체크

}

void CCollsionMgr::CheckCollsion(list<CGameObject*> listObj1, list<CGameObject*> listObj2)
{
	for (auto& iter1 : listObj1)
	{
		for (auto& iter2 : listObj2)
		{
			CheckCollsion(iter1, iter2);
		}
	}
}


//bool CPlayer::CheckCollision(OBJ_ID eObjId)
//{
//	////CollisionMgr에서 충돌판정을 하고 충돌 했으면 그 오브젝트의 충돌함수를 불러서 eID로 switch문 돌리기
//	//switch (eObjId)
//	//{
//	//case OBJ_MAP:
//	//{
//	//	CMapObjectsShader* pMapObjectShader = CGameMgr::GetInstance()->GetScene()->m_pMapObjectShader;
//
//	//	XMFLOAT3 xmf3TempPos = Vector3::Add(m_xmf3Position, m_xmf3Velocity);
//
//	//	for (int i = 0; i < pMapObjectShader->m_nObjects; ++i)
//	//	{
//	//		CMapObject* pMapObject = static_cast<CMapObject*>(pMapObjectShader->m_ppObjects[i]);
//	//		if (!pMapObject->m_isCollisionIgnore)
//	//		{
//	//			if (m_pComCollision->Check_Collision_AfterMove(pMapObject->m_pComCollision->m_xmOOBB, xmf3TempPos, m_xmf4x4World))
//	//				return true;
//	//		}
//	//	}
//	//	break;
//	//}
//	//case OBJ_END:
//	//	break;
//	//}
//
//	return false;
//}


		////맵 충돌
		//if (CheckCollision(OBJ_ID::OBJ_MAP))
		//{
		//	XMFLOAT3 xmf3TempShift = xmf3Shift;
		//	xmf3TempShift = Vector3::ScalarProduct(xmf3TempShift, -1.5, false);
		//	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3TempShift);
		//}