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

		pObj1->CollsionDetection(pObj2, CheckLineCloseToPoint(pObj1, pCom2));
		pObj2->CollsionDetection(pObj1, CheckLineCloseToPoint(pObj2, pCom1));

		//pCom1->m_isCollision = true; //�ڱ�͸� üũ
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
			if (20.f > Vector3::Length(Vector3::Subtract(iter->GetPosition(), pObj->GetPosition()))) //����� �ӽ÷�
				if (CheckCollsion(pObj, iter))
					isCol = true;
		}
		else
		{
			if (CheckCollsion(pObj, iter))
				isCol = true;
		}
	}

	static_cast<CCollision*>(pObj->m_pComponent[COM_COLLISION])->m_isCollision = isCol;  //�ϴ� �ڱ�͸� üũ

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
	//�ٿ�� �ڽ��� �ظ����� �˻�, �ڳ�-0,1,4,5
	XMFLOAT3 xmf3Line[4][2];
	xmf3Line[0][0] = xmf3Corners[0], xmf3Line[0][1] = xmf3Corners[1];
	xmf3Line[1][0] = xmf3Corners[1], xmf3Line[1][1] = xmf3Corners[5];
	xmf3Line[2][0] = xmf3Corners[5], xmf3Line[2][1] = xmf3Corners[4];
	xmf3Line[3][0] = xmf3Corners[4], xmf3Line[3][1] = xmf3Corners[0];


	//����, ���� ������ ���� �Ÿ� - ����Ȯ,
	//������ ������ ������ ���� �Ÿ� ���� �̿�
	vector<float> vecDis;
	for (int i = 0; i < 4; ++i)
	{
		//XMFLOAT3 xmf3Mid = Vector3::ScalarProduct(Vector3::Add(xmf3Line[i][0], xmf3Line[i][1]), 0.5f, false); //���� ����
		//float fDis = Vector3::Distance(xmf3Point, xmf3Mid);
		//vecDis.emplace_back(fDis);
		
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
//	////CollisionMgr���� �浹������ �ϰ� �浹 ������ �� ������Ʈ�� �浹�Լ��� �ҷ��� eID�� switch�� ������
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


		////�� �浹
		//if (CheckCollision(OBJ_ID::OBJ_MAP))
		//{
		//	XMFLOAT3 xmf3TempShift = xmf3Shift;
		//	xmf3TempShift = Vector3::ScalarProduct(xmf3TempShift, -1.5, false);
		//	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3TempShift);
		//}