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
		//XMFLOAT3* xmf3Line[2] = { CheckLineCloseToPoint(pObj1, pCom2) , CheckLineCloseToPoint(pObj2, pCom1) };
		//if (pObj2->m_eObjId == OBJ_ID::OBJ_MAP)
		//{
		//	XMFLOAT3 xmf3Pos = pObj2->GetPosition();
		//	cout << xmf3Pos.x << "," << xmf3Pos.z << " - ";
		//}
		pObj1->CollsionDetection(pObj2, CheckLineCloseToPoint(pObj1, pCom2));
		pObj2->CollsionDetection(pObj1, CheckLineCloseToPoint(pObj2, pCom1));

		//pCom1->m_isCollision = true; //자기것만 체크
		return true;
	}
	return false;
}

void CCollsionMgr::CheckCollsion(CGameObject* pObj, list<CGameObject*> listObj, bool isNearCheck /*= false*/, bool isCheckOnce /*= false*/)
{
	bool isCol = false;
	for (auto& iter : listObj)
	{
		if (!iter->m_isActive)
			continue;

		if (isNearCheck)
		{
			if (20.f > Vector3::Length(Vector3::Subtract(iter->GetPosition(), pObj->GetPosition()))) //상수값 임시로
				if (CheckCollsion(pObj, iter))
				{
					isCol = true;

					if (isCheckOnce)
						break;
				}
		}
		else
		{
			if (CheckCollsion(pObj, iter))
			{
				isCol = true;

				if (isCheckOnce)
					break;
			}
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

XMFLOAT3* CCollsionMgr::CheckLineCloseToPoint(CGameObject* pObj1, CCollision* pCol2)
{
	XMFLOAT3 xmf3Corners[8];
	pCol2->m_xmOOBB.GetCorners(xmf3Corners);

	return CheckLineCloseToPoint(pObj1->GetPosition(), xmf3Corners);
}

XMFLOAT3* CCollsionMgr::CheckLineCloseToPoint(XMFLOAT3& xmf3Point, XMFLOAT3* xmf3Corners)
{
	//바운딩 박스의 밑면으로 검사, 코너-0,1,4,5 ->회전시 인덱스 달라짐
	vector<XMFLOAT3> vecTemp(4);
	vector<XMFLOAT3> vecSortY(8);

	for (int i = 0; i < 8; ++i)
		vecSortY[i] = xmf3Corners[i];

	sort(vecSortY.begin(), vecSortY.end(), [](auto& a, auto& b) { return a.y < b.y; });

	//밑면 점 4개
	for (int i = 0; i < 4; ++i)
		vecTemp[i] = vecSortY[i];

	sort(vecTemp.begin(), vecTemp.end(), [](auto& a, auto& b) { return a.x < b.x; });

	//밑면 점 사각형으로 만들기
	vector<XMFLOAT3> vecBottom(4);
	if(vecTemp[0].z < vecTemp[1].z)
	{
		vecBottom[0] = vecTemp[0];
		vecBottom[1] = vecTemp[1];
	}
	else
	{
		vecBottom[0] = vecTemp[1];
		vecBottom[1] = vecTemp[0];                                     
	}

	float dis2 = Vector3::Distance(vecBottom[1], vecTemp[2]);
	float dis3 = Vector3::Distance(vecBottom[1], vecTemp[3]);
	if (dis2 < dis3)
	{
		vecBottom[2] = vecTemp[2];
		vecBottom[3] = vecTemp[3];
	}
	else
	{
		vecBottom[2] = vecTemp[3];
		vecBottom[3] = vecTemp[2];
	}


	XMFLOAT3 xmf3Line[4][2];
	xmf3Line[0][0] = vecBottom[0], xmf3Line[0][1] = vecBottom[1];
	xmf3Line[1][0] = vecBottom[1], xmf3Line[1][1] = vecBottom[2];
	xmf3Line[2][0] = vecBottom[2], xmf3Line[2][1] = vecBottom[3];
	xmf3Line[3][0] = vecBottom[3], xmf3Line[3][1] = vecBottom[0];


	//선분, 선분 중점과 점과 거리 - 불정확,
	//두점을 지나는 직선과 점의 거리 공식 이용
	vector<float> vecDis;
	for (int i = 0; i < 4; ++i)
	{
		float x0 = xmf3Point.x, y0 = xmf3Point.z;
		float x1 = xmf3Line[i][0].x, y1 = xmf3Line[i][0].z;
		float x2 = xmf3Line[i][1].x, y2 = xmf3Line[i][1].z;
		float fDis = abs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1)) / sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
		vecDis.emplace_back(fDis);
	}
	int iMinIndex = int(min_element(vecDis.begin(), vecDis.end()) - vecDis.begin());
	//cout << iMinIndex << endl;
	return xmf3Line[iMinIndex];
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