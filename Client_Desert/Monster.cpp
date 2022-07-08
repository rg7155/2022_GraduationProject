#include "Monster.h"
#include "Scene.h"
#include "ChildObject.h"
#include "ServerManager.h"

CMonsterObject::CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CGameObject(1)
{
	SetChild(pModel->m_pModelRootObject, true);
	

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/Dissolve.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, RP_TEXTURE, false);

	CTexture* pTexture2 = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture2->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Images/burn_dissolve.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture2, RP_TEXTURE2, false);


	CMaterial* pMaterial = new CMaterial(2);
	pMaterial->SetTexture(pTexture);
	pMaterial->SetTexture(pTexture2, 1);

	//pMaterial->SetShader(nullptr/*pShader*/);
	SetMaterial(0, pMaterial);

	//if(m_ppMaterials)
	//	m_ppMaterials[0]->SetTexture(pTexture);

	SetEffectsType(EFFECT_DISSOLVE, true);
	SetEffectsType(EFFECT_FOG, true);

	m_pHp = new CTexturedObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CTexturedObject::TEXTURE_HP);
	m_pHpFrame = new CTexturedObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CTexturedObject::TEXTURE_HP_FRAME);
	//m_pHp = static_cast<CTexturedObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"Hp"));
	//m_pHpFrame = static_cast<CTexturedObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"HpFrame"));

	//각 몬스터에 따라 변경할 것
	m_fHpOffsetY = 3.f;
	m_iHp = 100;
	m_iMaxHp = m_iHp;

	m_fDissolve = 0.f; //1에 가까울수록 사라짐

	m_xmVecNowRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmf3Look = { 0.f, 0.f, 1.f };
	m_xmVecNewRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecSrc = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_eObjId = OBJ_MONSTER;

	//SetPosition(XMFLOAT3(10.f, 0.f, 20.f));

	CreateComponent(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}

CMonsterObject::~CMonsterObject()
{
	if (m_pHp)
	{
		m_pHp->ReleaseUploadBuffers();
		m_pHp->ReleaseShaderVariables();
		m_pHp->Release();
	}
	if (m_pHpFrame)
	{
		m_pHpFrame->ReleaseUploadBuffers();
		m_pHpFrame->ReleaseShaderVariables();
		m_pHpFrame->Release();
	}
}

void CMonsterObject::Animate(float fTimeElapsed)
{
	if (!m_isActive)
		return; 

	UpdateHpBar(fTimeElapsed);
	UpdateComponent(fTimeElapsed);
	UpdateAttackCoolTime(fTimeElapsed);


	//if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_H))
	//{
	//	SetDamaged(10);
	//}

	// m_fDissolve 0 - 1
	
	//m_fDissolve = 0.2f;
	//static bool bToggle = false;
	//if(!bToggle)
	//	m_fDissolve += fTimeElapsed * 0.5f;
	//else
	//	m_fDissolve -= fTimeElapsed * 0.5f;

	//if (m_fDissolve >= 1.f || m_fDissolve <= 0.f)
	//	bToggle = !bToggle;

	//cout << m_fDissolve << endl;

	//static bool bToggle = false;
	//if (!bToggle)
	//{
	//	CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"Quake");
	//	XMFLOAT3 xmf3Pos = CGameMgr::GetInstance()->GetPlayer()->GetPosition();
	//	xmf3Pos.y += 0.01f;
	//	pObj->SetPosition(xmf3Pos);
	//	bToggle = true;
	//}


	//LerpRotate(fTimeElapsed);
	CGameObject::Animate(fTimeElapsed);
	//cout << m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pSkinnedAnimationController->m_pAnimationTracks[0].m_nAnimationSet]->m_fPosition << endl;

}

void CMonsterObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	if (!m_isActive)
		return;

	UpdateShaderVariables(pd3dCommandList);

	//디졸브 텍스쳐 업데이트
	for (int i = 0; i < m_nMaterials; i++)
		if (m_ppMaterials[i])	m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CMonsterObject::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CShader* pShader)
{
	//디졸브 시작되면 그림자 그리지 않기
	if (m_fDissolve > 0.f)
		return;

	CGameObject::ShadowRender(pd3dCommandList, pCamera, pShader);
}

void CMonsterObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_fDissolve, 34);
}

void CMonsterObject::LerpRotate(float fTimeElapsed)
{
	m_xmVecNowRotate = XMVector3Normalize(m_xmVecNowRotate);
	m_xmVecNowRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecNewRotate, fTimeElapsed * 10.f);

	XMStoreFloat3(&m_xmf3Look, m_xmVecNowRotate);

	XMVECTOR xmVecRight = XMVector3Cross(XMLoadFloat3(&m_xmf3Up), XMLoadFloat3(&m_xmf3Look));
	XMStoreFloat3(&m_xmf3Right, xmVecRight);


	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Up = Vector3::Normalize(m_xmf3Up);
	m_xmf3Right = Vector3::Normalize(m_xmf3Right);
}

void CMonsterObject::OnPrepareRender()
{
	//m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	//m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	//m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	//////m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	//m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CMonsterObject::UpdateHpBar(float fTimeElapsed)
{
	if (!m_pHp || !m_pHpFrame)
		return;

	m_pHp->Animate(fTimeElapsed);
	m_pHpFrame->Animate(fTimeElapsed);

	CScene* pScene = CGameMgr::GetInstance()->GetScene();
	pScene->AddAlphaObjectToList(m_pHp);
	pScene->AddAlphaObjectToList(m_pHpFrame);

	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos.y += m_fHpOffsetY;
	m_pHp->SetPosition(xmf3Pos);
	m_pHpFrame->SetPosition(xmf3Pos);

}

void CMonsterObject::SetHp(int hp)
{
	//피격 상태
	if (m_iHp != hp)
		MakeHitEffect();

	m_iHp = hp;
	if (m_iHp < 0)
		m_iHp = 0;

	float fRatio = (m_iHp / (float)m_iMaxHp);
	m_pHp->SetScale(fRatio, 1.f, 1.f);
}


void CMonsterObject::MakeHitEffect()
{
	CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"HitEffect");
	if (!pObj)
		return;

	XMFLOAT3 xmf3Pos = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Pos.x += xmf3Look.x;
	xmf3Pos.y += 1.f;
	xmf3Pos.z += xmf3Look.z;
	pObj->SetPosition(xmf3Pos);

	cout << "MakeEff" << endl;
}

void CMonsterObject::CollsionDetection(CGameObject* pObj)
{
	OBJ_ID eObjId = pObj->m_eObjId;

	switch (eObjId)
	{
	case OBJ_SWORD:
		//cout << "칼과 충돌 했다!" << endl;
		ResetAttackCoolTime();
		break;
	default:
		break;
	}

}

void CMonsterObject::CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pComponent[COM_COLLISION] = CCollision::Create();

	m_pComCollision = static_cast<CCollision*>(m_pComponent[COM_COLLISION]);
	m_pComCollision->m_isStaticOOBB = false;
	if (m_pChild && m_pChild->m_isRootModelObject)
		m_pComCollision->m_xmLocalOOBB = m_pChild->m_xmOOBB;
	m_pComCollision->m_pxmf4x4World = &m_xmf4x4World;
	m_pComCollision->UpdateBoundingBox();
}

void CMonsterObject::UpdateComponent(float fTimeElapsed)
{
	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	if (m_pComCollision)
		m_pComCollision->UpdateBoundingBox();
}

void CMonsterObject::ResetAttackCoolTime(bool bIgnore /*= true*/)
{
	m_pComCollision->m_isCollisionIgnore = bIgnore;
	bAttackInvalid = bIgnore;
	fAttackCoolTime = 0.f;
}

void CMonsterObject::UpdateAttackCoolTime(float fTimeElapsed)
{
	fAttackCoolTime += fTimeElapsed;
	if (fAttackCoolTime > ATTACK_COOLTIME)
	{
		ResetAttackCoolTime(false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CBossObject::CBossObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel)
{

	//m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 10, pModel);
	//for (int i = 0; i < 10; i++)
	//{
	//	m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	//	m_pSkinnedAnimationController->SetTrackEnable(i, false);

	//}
	//m_pSkinnedAnimationController->SetTrackEnable(3, true);
}

CBossObject::~CBossObject()
{
}

void CBossObject::Animate(float fTimeElapsed)
{
	CMonsterObject::Animate(fTimeElapsed);
}

void CBossObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline /*= true*/)
{
	CMonsterObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}


CGolemObject::CGolemObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
	: CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel)
{
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, GOLEM::ANIM::END, pModel);
	for (int i = 0; i < GOLEM::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);

	}


	m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[GOLEM::ANIM:: DIE]->m_nType = ANIMATION_TYPE_ONCE;
	m_eCurAnim = GOLEM::ANIM::IDLE;
	m_ePrevAnim = GOLEM::ANIM::IDLE;

	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);
	m_bBlendingOn = false;
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 0.f;
	m_fBlendingTime = 0.f;

	m_bAttack1On = false;
	m_bAttack2On = false;

	m_fAttackTime = 0.f;
	m_bSkill1EffectOn = false;
	SetLookAt(XMFLOAT3(0.f, 0.f, -1.f));
	SetPosition(XMFLOAT3(23.f, 0.f, 114.f));

	m_isActive = false;

}

CGolemObject::~CGolemObject()
{
}

void CGolemObject::Animate(float fTimeElapsed)
{
	if (m_eCurAnim != GOLEM::ANIM::ATTACK1)
		m_bSkill1EffectOn = false;

	if ((m_ePrevAnim == GOLEM::IDLE && m_eCurAnim == GOLEM::RUN) ||
		(m_eCurAnim == GOLEM::IDLE && m_ePrevAnim == GOLEM::RUN))
	{
		Blending_Animation(fTimeElapsed);
	}
	// 바닥 이펙트

	float fAnimElapseTime = m_pSkinnedAnimationController->m_fPosition[m_eCurAnim];

	if (m_eCurAnim == GOLEM::ANIM::ATTACK1 && !m_bSkill1EffectOn && fAnimElapseTime > 0.5f)
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"Quake");
		if (pObj) {
			XMFLOAT3 xmf3Pos = GetPosition();
			XMFLOAT3 xmf3Look = GetLook();
			xmf3Pos.x += xmf3Look.x;
			xmf3Pos.y += 0.1f;
			xmf3Pos.z += xmf3Look.z;
			pObj->SetPosition(xmf3Pos);
		}
		m_bSkill1EffectOn = true;

	}
	
	// m_fDissolve 0 - 1
	
	if (m_eCurAnim == GOLEM::ANIM::DIE)
	{
		float fLength = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_eCurAnim]->GetLength();
		if (fAnimElapseTime >= fLength - EPSILON)
		{
			m_fDissolve += fTimeElapsed * 0.5f;
			if (m_fDissolve > 1.f)
			{
				m_fDissolve = 1.f;
				m_isActive = false;
			}
		}
	}

	//포인트 라이트
	CGameMgr::GetInstance()->GetScene()->SetPointLightPos(GetPosition());


	//cout << m_fDissolve << endl;
	
	////m_fAttackTime += fTimeElapsed;
	//m_fAnimElapsedTime += fTimeElapsed;

	///*if (m_fAttackTime > 4.f)
	//{
	//	Change_Animation(GOLEM::ANIM::ATTACK1);

	//	m_fAttackTime = 0.f;
	//}*/
	//if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	//{
	//	if(m_eCurAnim == GOLEM::ANIM::DAMAGED_LEFT || m_eCurAnim == GOLEM::ANIM::DAMAGED_RIGHT)
	//		Change_Animation(GOLEM::ANIM::ATTACK1);

	//	else if (m_eCurAnim != GOLEM::IDLE && m_eCurAnim != GOLEM::RUN)
	//	{
	//		Change_Animation(GOLEM::ANIM::RUN);

	//	}
	//}

	

	CMonsterObject::Animate(fTimeElapsed);



}

void CGolemObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	CMonsterObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CGolemObject::CollsionDetection(CGameObject* pObj)
{
	OBJ_ID eObjId = pObj->m_eObjId;

	switch (eObjId)
	{
	case OBJ_SWORD:
		cout << "칼과 충돌 했다!" << endl;
		ResetAttackCoolTime();
		break;
	default:
		break;
	}
}

void CGolemObject::Change_Animation(GOLEM::ANIM eNewAnim)
{
	//if (m_eCurAnim == GOLEM::ANIM::ATTACK2 || m_eCurAnim == GOLEM::ANIM::ATTACK1)
	//{
	//	eNewAnim = GOLEM::ANIM::IDLE;
	//}
	if (m_eCurAnim == eNewAnim)
		return;

	// 플레이어가 공격 중이면 공격하지 않음
	CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();
	int id = CServerManager::GetInstance()->m_myid;

	if (!pPlayer->IsNowAttack())
	{
		// 근접공격은 타겟 무조건 공격

		if (eNewAnim == GOLEM::ANIM::ATTACK2)
		{
			// 타겟이면

			if (id == m_targetId)
			{
				pPlayer->Change_Animation(PLAYER::ANIM::TAKE_DAMAGED);

			}
		}

		// 원격공격은 거리 계산해서 공격
		if (eNewAnim == GOLEM::ANIM::ATTACK1)
		{
			Check_Collision();
		}
	}
	
	m_ePrevAnim = m_eCurAnim;
	m_eCurAnim = eNewAnim;

	m_fAnimElapsedTime = 0.f;
	m_fBlendingTime = 0.f;
	m_bBlendingOn = true;
	for (int i = 0; i < GOLEM::ANIM::END; i++)
	{
		if (i == m_ePrevAnim || i == m_eCurAnim)
			continue;
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pSkinnedAnimationController->SetTrackWeight(i, 0.f);
	}

	// 애니메이션 진행시간 
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();
	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);	// 다음 애니메이션 true로, 이전도 아직 true

	if ((m_ePrevAnim == GOLEM::IDLE && m_eCurAnim == GOLEM::RUN) ||
		(m_eCurAnim == GOLEM::IDLE && m_ePrevAnim == GOLEM::RUN))
	{
		////	// 1 2 3 순으로 애니메이션 진행된다하면. 1,2 블렌딩 중에 3으로 바뀌면 2의 블렌딩값과 3의 1-2의블렌딩값으로 세팅되어야 함
		if (m_pSkinnedAnimationController->GetTrackWeight(m_ePrevAnim) < 0.8f)
		{
			m_fBlendingTime = m_pSkinnedAnimationController->GetTrackWeight(m_ePrevAnim);

			m_fAnimElapsedTime = 1.f - m_fBlendingTime;

			m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, m_fBlendingTime);

			m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fAnimElapsedTime);
			m_fBlendingTime = m_fAnimElapsedTime;
			m_fAnimElapsedTime = 0.f;
		}
		else
		{
			m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f);	// 애니메이션 3개중첩 방지
			m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 0.f);
		}
		// 이전 애니메이션의 가중치가 1보다 작으면 1로 바꾸지말고 그때부터 보간해야함
	}
	else
	{
		m_pSkinnedAnimationController->SetTrackEnable(m_ePrevAnim, false);
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 0.f);
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 1.f);	// 애니메이션 3개중첩 방지

	}
	
}

void CGolemObject::Blending_Animation(float fTimeElapsed)
{
	if (!m_bBlendingOn)
		return;

	m_fBlendingTime += fTimeElapsed * 2.f;

	if (m_fBlendingTime >= 1.f)
	{
		m_bBlendingOn = false;
		m_pSkinnedAnimationController->SetTrackEnable(m_ePrevAnim, false);
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 0.f);
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 1.f);

		return;
	}


	m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f - m_fBlendingTime);
	m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fBlendingTime);
}

void CGolemObject::SetNewRotate(XMFLOAT3 xmf3Look)
{
	XMFLOAT3 xmNormalLook = Vector3::Normalize(xmf3Look);
	m_xmVecNewRotate = XMLoadFloat3(&xmNormalLook);
	m_xmVecNowRotate = XMLoadFloat3(&xmNormalLook);

}

void CGolemObject::Check_Collision()
{
	CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();
	int id = CServerManager::GetInstance()->m_myid;
	float fDis = Vector3::Distance(pPlayer->GetPosition(), GetPosition());
	if (fDis < GOLEM_ATTACK1_DISTANCE)
	{
		// 타겟이면
		if (id == m_targetId)
		{
			pPlayer->Change_Animation(PLAYER::ANIM::TAKE_DAMAGED);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCactiBulletObject::CCactiBulletObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//TODO - 파일이름 바꾸기
	CLoadedModelInfo* pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Thorn_Projectile.bin", nullptr);
	SetChild(pModel->m_pModelRootObject, true);
	Rotate(0.f, -90.f, 0.f);

	SetScale(2.f, 2.f, 2.f);
	m_xmOOBB.Extents = Vector3::ScalarProduct(m_xmOOBB.Extents, 2.f, false);
	//SetScale(XMFLOAT3{ 0.2f, 0.2f, 0.2f });

	//TODO - 캣티가 만들 것
	/*if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_H))
	{
		CCactiBulletObject* pObj = static_cast<CCactiBulletObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"CactiBullet"));
		XMFLOAT3 xmf3Pos = m_pPlayer->GetPosition(), xmf3Target = m_pPlayer->GetPosition();
		xmf3Pos.y += 1;
		xmf3Target.z += 1;
		pObj->SetTarget(xmf3Pos, xmf3Target);
		pObj->SetActiveState(true);
	}*/
}

CCactiBulletObject::~CCactiBulletObject()
{
}

#define CACTI_BULLET_TIME 3.f
void CCactiBulletObject::Animate(float fTimeElapsed)
{
	if (!m_isActive) return;
	m_fCreateTime -= fTimeElapsed;
	if (m_fCreateTime > 0)
		return;

	m_fTime -= fTimeElapsed;
	if (m_fTime < 0)
	{
		m_isActive = false;
		return;
	}

	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos = Vector3::Add(xmf3Pos, Vector3::ScalarProduct(m_xmf3Target, fTimeElapsed*m_fSpeed, false));
	SetPosition(xmf3Pos);



	CGameObject::Animate(fTimeElapsed);
}

void CCactiBulletObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	if (!m_isActive) return;
	if (m_fCreateTime > 0)
		return;

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CCactiBulletObject::SetTarget(XMFLOAT3& xmf3Start, XMFLOAT3& xmf3Target, bool isYFix/*=true*/)
{
	m_fTime = CACTI_BULLET_TIME;

	SetPosition(xmf3Start);

	m_xmf3Target = Vector3::Subtract(xmf3Target, xmf3Start, true, isYFix);
}

CCactiObject::CCactiObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, char type)
	: CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel)
{
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, CACTI::ANIM::END, pModel);
	for (int i = 0; i < CACTI::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

	m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[CACTI::ANIM::DIE]->m_nType = ANIMATION_TYPE_ONCE;

	m_eCurAnim = CACTI::ANIM::IDLE;
	m_ePrevAnim = CACTI::ANIM::IDLE;

	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);
	m_bBlendingOn = false;
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 0.f;
	m_fBlendingTime = 0.f;

	if (type == CACTI1) {
		SetPosition(CACTI_POS_INIT1);
		SetLookAt(XMFLOAT3(CACTI_POS_AFTER1));
		m_AfterPos = CACTI_POS_AFTER1;
	}
	else {
		SetPosition(CACTI_POS_INIT2);
		SetLookAt(XMFLOAT3(CACTI_POS_AFTER2));
		m_AfterPos = CACTI_POS_AFTER2;
	}
	m_nowVerse = VERSE1;

	SetScale(2.f, 2.f, 2.f);
	m_xmOOBB.Extents = Vector3::ScalarProduct(m_xmOOBB.Extents, 2.f, false);

}

CCactiObject::~CCactiObject()
{
}

void CCactiObject::Animate(float fTimeElapsed)
{
	//Blending_Animation(fTimeElapsed);


	m_fAnimElapsedTime += fTimeElapsed;
	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		m_fAnimElapsedTime = 0.f;
		if (m_eCurAnim == CACTI::ANIM::BITE)
		{
			Change_Animation(CACTI::ANIM::WALK);
			
			m_nowVerse = VERSE2;
		}
		if (m_nowVerse == VERSE3) {
			Change_Animation(CACTI::ANIM::IDLE);

		}



	}
	
	CMonsterObject::Animate(fTimeElapsed);


	if (VERSE2 == m_nowVerse) {
		XMFLOAT3 xmf3Pos = GetPosition();
		XMFLOAT3 xmf3Look = GetLook();
		XMFLOAT3 moveSize = xmf3Look;
		moveSize.x *= fTimeElapsed * 4.f;
		moveSize.z *= fTimeElapsed * 4.f;

		XMFLOAT3 newPos = Vector3::Add(xmf3Pos, moveSize);
		SetPosition(newPos);
		xmf3Pos = GetPosition();
		float dis = Vector3::Distance(xmf3Pos, m_AfterPos);
		if (dis < 0.1f) {

			SetPosition(m_AfterPos);
			m_nowVerse = VERSE3;
			Change_Animation(CACTI::IDLE);
			Rotate(0.f, 180.f, 0.f);
			if (m_pCacti && VERSE3 == static_cast<CMonsterObject*>(m_pCacti)->m_nowVerse) {
				m_pCactus->SetActiveState(true);
			}
		}
	}

}

void CCactiObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	//if (!m_isActive) return;

	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);
}

void CCactiObject::CollsionDetection(CGameObject* pObj)
{
	OBJ_ID eObjId = pObj->m_eObjId;

	switch (eObjId)
	{
	case OBJ_SWORD:
		cout << "칼과 충돌 했다!" << endl;
		if(m_nowVerse == VERSE1)
			Change_Animation(CACTI::BITE);
		ResetAttackCoolTime();
		break;
	default:
		break;
	}
}

void CCactiObject::Change_Animation(CACTI::ANIM eNewAnim)
{
	if (m_eCurAnim == eNewAnim)
		return;

	m_ePrevAnim = m_eCurAnim;
	m_eCurAnim = eNewAnim;

	m_fAnimElapsedTime = 0.f;
	//m_fBlendingTime = 0.f;
	//m_bBlendingOn = true;
	for (int i = 0; i < CACTI::ANIM::END; i++)
	{
		//if (i == m_ePrevAnim || i == m_eCurAnim)
		//	continue;
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
		//m_pSkinnedAnimationController->SetTrackWeight(i, 0.f);
	}
	// 애니메이션 진행시간 
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();
	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);	// 다음 애니메이션 true로, 이전도 아직 true


}

void CCactiObject::Blending_Animation(float fTimeElapsed)
{
	if (!m_bBlendingOn)
		return;

	m_fBlendingTime += fTimeElapsed * 2.f;

	if (m_fBlendingTime >= 1.f)
	{
		m_bBlendingOn = false;
		m_pSkinnedAnimationController->SetTrackEnable(m_ePrevAnim, false);
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 0.f);
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 1.f);

		return;
	}


	m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f - m_fBlendingTime);
	m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fBlendingTime);
}

void CCactiObject::SetNewRotate(XMFLOAT3 xmf3Look)
{
}

void CCactiObject::AttackProcess(CACTUS::ANIM eAnim)
{

	switch (eAnim)
	{
	case CACTUS::ATTACK1: {
		Change_Animation(CACTI::ATTACK1);

		AddBullet();
		break;
	}
	case CACTUS::ATTACK2: {
		Change_Animation(CACTI::ATTACK2);
		for (int i = 0; i < 5; i++)
			AddBullet();
		break;
	}
	case CACTUS::ATTACK3:
	default:
		break;
	}
}

void CCactiObject::AddBullet()
{
	CCactiBulletObject* pObj = static_cast<CCactiBulletObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"CactiBullet"));
	CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();
	XMFLOAT3 xmf3Target = pPlayer->GetPosition();
	pObj->SetActiveState(true);

	XMFLOAT3 regenPos;
	regenPos.x = (float)(rand() % 20) * 0.05f * 2.f;
	regenPos.y = (float)(rand() % 10) * 0.05f + 0.5f;
	regenPos.z = (float)(rand() % 20) * 0.05f * 2.f;
	regenPos = Vector3::Add(GetPosition(), regenPos);
	pObj->SetPosition(regenPos);
	pObj->SetTarget(pObj->GetPosition(), xmf3Target);

	// look 벡터 설정
	pObj->SetLookAt(xmf3Target, false);
	pObj->m_fCreateTime = (float)(rand() % 5) * 0.2f;
	pObj->m_fSpeed = (float)(rand() % 10) * 0.5f + 5.f;
}

CCactusObject::CCactusObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, CGameObject* pCacti1, CGameObject* pCacti2)
	: CMonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel)
{
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, CACTUS::ANIM::END, pModel);
	for (int i = 0; i < CACTUS::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}


	m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[CACTUS::ANIM::DIE]->m_nType = ANIMATION_TYPE_ONCE;
	//m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[CACTUS::ANIM::SPAWN]->m_nType = ANIMATION_TYPE_ONCE;

	
	m_eCurAnim = CACTUS::ANIM::SPAWN;
	m_ePrevAnim = CACTUS::ANIM::SPAWN;

	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);
	m_bBlendingOn = false;
	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_eCurAnim]->GetLength();

	m_fBlendingTime = 0.f;

	//SetLookAt(XMFLOAT3(0.f, 0.f, -0.f));
	//SetLookAt(XMFLOAT3(0.f, 0.f, 0.f));
	//SetPosition(XMFLOAT3(25.0f, 0, 25.0f));

	//m_isActive = false;

	m_pCacti1 = pCacti1;
	m_pCacti2 = pCacti2;

	m_nowVerse = VERSE1;
	m_ePreAttack = CACTUS::IDLE;
	m_fAttackCoolTime = 0.f;

	SetScale(2.f, 2.f, 2.f);
	m_xmOOBB.Extents = Vector3::ScalarProduct(m_xmOOBB.Extents, 2.f, false);
}

CCactusObject::~CCactusObject()
{
}

void CCactusObject::CollsionDetection(CGameObject* pObj)
{
	OBJ_ID eObjId = pObj->m_eObjId;

	switch (eObjId)
	{
	case OBJ_SWORD:
		cout << "칼과 충돌 했다!" << endl;
		ResetAttackCoolTime();
		break;
	default:
		break;
	}
}

void CCactusObject::Animate(float fTimeElapsed)
{
	m_fAnimElapsedTime += fTimeElapsed;
	if (m_fAnimElapsedTime >= m_fAnimMaxTime)
	{
		m_fAnimElapsedTime = 0.f;
		Change_Animation(CACTUS::ANIM::IDLE);
		m_nowVerse = VERSE2;
	}
	if (m_nowVerse == VERSE2) {
		m_fAttackCoolTime += fTimeElapsed;
		if (m_fAttackCoolTime > 3.f)
		{
			m_fAttackCoolTime = 0.f;
			CACTUS::ANIM eNext = m_ePreAttack == CACTUS::ATTACK3 ? CACTUS::ATTACK1 : (CACTUS::ANIM)(m_ePreAttack + 1);
			Change_Animation(eNext);
			m_ePreAttack = eNext;
			if (eNext != CACTUS::ATTACK3) {
				static_cast<CCactiObject*>(m_pCacti1)->AttackProcess(eNext);
				static_cast<CCactiObject*>(m_pCacti2)->AttackProcess(eNext);
			}
			else {
				for (int i = 0; i < 20; i++)
				{
					AddBullet();
				}
			}


		}
	}
	

	CMonsterObject::Animate(fTimeElapsed);



}

void CCactusObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool isChangePipeline)
{
	CGameObject::Render(pd3dCommandList, pCamera, isChangePipeline);

}

void CCactusObject::Change_Animation(CACTUS::ANIM eNewAnim)
{
	if (m_eCurAnim == eNewAnim)
		return;

	m_ePrevAnim = m_eCurAnim;
	m_eCurAnim = eNewAnim;

	m_fAnimElapsedTime = 0.f;
	//m_fBlendingTime = 0.f;
	//m_bBlendingOn = true;
	for (int i = 0; i < CACTUS::ANIM::END; i++)
	{
		//if (i == m_ePrevAnim || i == m_eCurAnim)
		//	continue;
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
		//m_pSkinnedAnimationController->SetTrackWeight(i, 0.f);
	}
	// 애니메이션 진행시간 
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();
	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);	// 다음 애니메이션 true로, 이전도 아직 true

}

void CCactusObject::Blending_Animation(float fTimeElapsed)
{
}

void CCactusObject::SetNewRotate(XMFLOAT3 xmf3Look)
{
}

void CCactusObject::AddBullet()
{
	CCactiBulletObject* pObj = static_cast<CCactiBulletObject*>(CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"StandardObject", L"CactiBullet"));
	CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();
	XMFLOAT3 xmf3Target = pPlayer->GetPosition();
	pObj->SetActiveState(true);

	XMFLOAT3 regenPos;
	regenPos.x = (float)(rand() % 40) * 0.5f - 10.f;
	regenPos.y = (float)(rand() % 10) * 0.1f + 10.f;
	regenPos.z = (float)(rand() % 40) * 0.5f - 10.f;
	regenPos = Vector3::Add(xmf3Target, regenPos);
	pObj->SetPosition(regenPos);
	xmf3Target = regenPos;
	xmf3Target.y = -1.f;
	pObj->SetTarget(pObj->GetPosition(), xmf3Target, false);
	pObj->Rotate(90.f, 0.f, 0.f);
	pObj->SetScale(4.f, 4.f, 4.f);
	//pObj->m_fCreateTime = (float)(rand() % 5) * 0.2f;
	pObj->m_fSpeed = (float)(rand() % 10) * 0.5f + 4.f;
}
