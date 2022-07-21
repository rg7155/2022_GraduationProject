//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "InputDev.h"
#include "Animation.h"
#include "Scene.h"
#include "ServerManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	//ShowCursor(false);

	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_xmVecNowRotate = XMVectorSet(0.f, 0.f, -1.f, 1.f);
	m_xmVecTmpRotate = XMVectorSet(0.f, 0.f, -1.f, 1.f);
	m_xmVecNewRotate = XMVectorSet(0.f, 0.f, -1.f, 1.f);
	m_xmVecSrc = XMVectorSet(0.f, 0.f, 1.f, 1.f);

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;



	int id = *(int*)pContext;
	////////////////////////////////////////////////////////////
	// �ʱ� Transform
	//SetPosition(XMFLOAT3(84.f, 0.f, 96.f));

	(id == 0) ? SetPosition(Scene0_SpawnPos) : SetPosition(Scene0_SpawnPos_Duo);
	//SetPosition(Scene1_SpawnPos);
	////////////////////////////////////////////////////////////

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	

	char fileName[2048];
	if (id == 0)
		strcpy(fileName, "Model/Adventurer_Aland_Blue.bin");
	else
		strcpy(fileName, "Model/Adventurer_Aland_Green.bin");

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fileName, NULL);

	SetChild(pPlayerModel->m_pModelRootObject, true);

	m_pSword = FindFrame("Sword");
	m_pSwordTail = FindFrame("SwordTail");

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, PLAYER::ANIM::END, pPlayerModel);



	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);

	}
	m_pSkinnedAnimationController->SetTrackEnable(PLAYER::ANIM::IDLE_RELAXED, true);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	//m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.001f, _T("Sound/Footstep01.wav"));
	//m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.125f, _T("Sound/Footstep02.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	//CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	//m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);


	if (pPlayerModel) delete pPlayerModel;


	m_bBattleOn = false;
	m_eCurAnim = PLAYER::ANIM::IDLE_RELAXED;
	m_ePrevAnim = PLAYER::ANIM::IDLE_RELAXED;
	m_bBlendingOn = false;
	m_bSkill1EffectOn = false;

	m_fAnimElapsedTime = 0.f;
	m_fAnimMaxTime = 0.f;
	m_fBlendingTime = 0.f;
	///////////////////////////////////////////////
	//������Ʈ, �ؽ���
	CreateComponent(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_eObjId = OBJ_PLAYER;
	m_pReadyTex = new CTexturedObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CTexturedObject::TEXTURE_READY);

	///////////////////////////////////////////////

	//SetScale(XMFLOAT3(0.2, 1, 1.8));

	m_pSword->m_pComponent[COM_COLLISION] = CCollision::Create();

	CCollision* pCol = static_cast<CCollision*>(m_pSword->m_pComponent[COM_COLLISION]);
	pCol->m_isStaticOOBB = false;
	pCol->m_xmLocalOOBB = m_pSword->m_xmOOBB;
	pCol->m_pxmf4x4World = &m_pSword->m_xmf4x4World;
	pCol->UpdateBoundingBox();
	m_pSword->m_eObjId = OBJ_SWORD;

	m_dir = DIR_UP;
	m_isActive = true;

}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;

	if (m_pReadyTex)
	{
		m_pReadyTex->ReleaseUploadBuffers();
		m_pReadyTex->ReleaseShaderVariables();
		m_pReadyTex->Release();
	}
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nEffectsType, 33);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (abs(fDistance - 0.f) < EPSILON)
	{
		//cout << "dis�� 0�Դϴ�." << endl;
		return;

	}

	if (CGameMgr::GetInstance()->GetScene()->m_eCurScene == SCENE::SCENE_0)
		return;


	// ��⵿���̳� �̵����϶��� �����̱� ����
	if (m_eCurAnim != PLAYER::ANIM::IDLE && m_eCurAnim != PLAYER::ANIM::IDLE_RELAXED && m_eCurAnim != PLAYER::ANIM::RUN)
	{
		m_fLerpSpeed = 0.f;
		return;
	}
	// �ӵ� ����
	
	if (m_eCurAnim == PLAYER::ANIM::RUN)
	{
		m_fLerpSpeed += fDistance / PLAYER_SPEED;
		if (m_fLerpSpeed > 1.f)
			m_fLerpSpeed = 1.f;
	}
	else
	{
		m_fLerpSpeed -= (fDistance / PLAYER_SPEED) * 3.f;
		if (m_fLerpSpeed < 0.f)
			m_fLerpSpeed = 0.f;

		Move(MoveByDir(fDistance * m_fLerpSpeed), bUpdateVelocity);
	}

	XMVECTOR xmDstVec = m_xmVecSrc;

	XMVECTOR xmVecCamRight = XMLoadFloat3(&m_pCamera->GetRightVector());
	XMVECTOR xmVecCamLook = XMLoadFloat3(&m_pCamera->GetLookVector());
	xmVecCamRight = XMVectorSetY(xmVecCamRight, 0.f);
	xmVecCamLook = XMVectorSetY(xmVecCamLook, 0.f);

	if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))	// ����
	{
		xmDstVec = xmVecCamRight + xmVecCamLook;
		m_dir = DIR_UPRIGHT;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S))	// ����
	{
		xmDstVec = xmVecCamRight - xmVecCamLook;
		m_dir = DIR_DOWNRIGHT;

	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A))	// �ƿ�
	{
		xmDstVec = -xmVecCamRight - xmVecCamLook;
		m_dir = DIR_DOWNLEFT;

	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W))	// ����
	{
		xmDstVec = -xmVecCamRight + xmVecCamLook;
		m_dir = DIR_UPLEFT;

	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
	{
		xmDstVec = xmVecCamRight;
		m_dir = DIR_RIGHT;

	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A))
	{
		xmDstVec = -xmVecCamRight;
		m_dir = DIR_LEFT;

	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W))
	{
		xmDstVec = xmVecCamLook;
		m_dir = DIR_UP;

	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S))
	{
		xmDstVec = -xmVecCamLook;
		m_dir = DIR_DOWN;

	}
	else
	{
			
		return;
	}

	m_xmVecNewRotate = XMVector3Normalize(xmDstVec);

	// 180�� Ȯ��
	XMVECTOR xmVecAngle =  XMVector3AngleBetweenNormals(m_xmVecNowRotate, m_xmVecNewRotate);
	float fAngle = XMVectorGetX(xmVecAngle);

	if (XMConvertToDegrees(fAngle) >= 170)
	{
		m_xmVecNowRotate = XMVector3Normalize(m_xmVecNowRotate);
		m_xmVecTmpRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecNewRotate, 0.5f);
		m_xmVecTmpRotate = XMVector3Normalize(m_xmVecTmpRotate);

	}
	else
		m_xmVecTmpRotate = m_xmVecNewRotate;

	// �̵�
	Move(MoveByDir(fDistance * m_fLerpSpeed), bUpdateVelocity);
	m_fTempShift = fDistance * m_fLerpSpeed;

	// ���簡 RUN�̸� �ִϸ��̼� �ٲ��� �ʾƵ� �ȴ�.
	if (m_eCurAnim != PLAYER::ANIM::RUN)
		Change_Animation(PLAYER::ANIM::RUN);


}

void CPlayer::Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		//cout << "After" << xmf3Temp.x << xmf3Temp.y << xmf3Temp.z << endl;

		m_xmf3PreVelocity = m_xmf3Velocity;
	}
	else
	{
		m_xmf3PrePosition = m_xmf3Position;
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		if(m_pCamera)
			m_pCamera->Move(xmf3Shift);
		// �̵������Ƿ� ������.
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{

			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	//Nan�� �̸�
	if (isnan(GetPosition().x) != 0)
	{
		SetPosition(Scene1_SpawnPos);
		cout << "Position is Nan!" << endl;
	}

	Move(0, /*12.25f*/PLAYER_SPEED * fTimeElapsed, true);

	m_pCamera->Update(GetLook(), fTimeElapsed);

	Move(m_xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
	{
		if (CGameMgr::GetInstance()->GetScene()->m_eCurScene != SCENE::SCENE_0)
			m_pCamera->SetLookAt(m_xmf3Position);
		else
		{
			XMFLOAT3 xmf3Pos = /*Scene0_SpawnPos*/m_xmf3Position;
			xmf3Pos.y += 2.f;
			m_pCamera->SetLookAt(xmf3Pos);
		}
	}
	m_pCamera->RegenerateViewMatrix();

	m_xmf3Velocity = {0.00f,0.00f ,0.00f };
	LerpRotate(fTimeElapsed);

	Blending_Animation(fTimeElapsed);

	//������ ���� �״�-> �����Ҷ��� ������ ����
	m_pComTrail->SetRenderingTrail(IsNowAttack());

	////////////////////////////////////////////
	UpdateComponent(fTimeElapsed);

	UpdateReadyTexture(fTimeElapsed);

	//MovePosByCollision();
	////////////////////////////////////////////

	Check_CreateEffect();

	// �ִϸ��̼��� ������ ����Ʈ ���� �� �ؾ���
	
	//if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_N))
	//{
	//	CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Explosion");
	//	if(pObj) pObj->SetPosition(GetPosition());
	//}

	if (m_pSkinnedAnimationController)
	{
		//float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (Check_Input(fTimeElapsed))
			return;

		if (!Check_MoveInput())
		{   
			// IDLE �ִϸ��̼��� ����ǵ��� �ϱ� ����
			if (m_eCurAnim == PLAYER::ANIM::IDLE || m_eCurAnim == PLAYER::ANIM::IDLE_RELAXED)
				return;

			if (m_bBattleOn)
				Change_Animation(PLAYER::ANIM::IDLE);
			else
				Change_Animation(PLAYER::ANIM::IDLE_RELAXED);
		}
	}

}

void CPlayer::Check_CreateEffect()
{
	// �ٴ� ����Ʈ
	if (m_eCurAnim == PLAYER::ANIM::SKILL1 && !m_bSkill1EffectOn && m_fAnimElapsedTime > 1.0f)
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Shockwave");
		if (pObj)
		{
			XMFLOAT3 xmf3Pos = GetPosition();
			xmf3Pos.x += m_xmf3Look.x;
			xmf3Pos.y += 0.1f;
			xmf3Pos.z += m_xmf3Look.z;
			pObj->SetPosition(xmf3Pos);
			static_cast<CMultiSpriteObject*>(pObj)->SetColor(true);
		}
		m_bSkill1EffectOn = true;
	}
	else if (m_eCurAnim == PLAYER::ANIM::SKILL2 && !m_bSkill2EffectOn && m_fAnimElapsedTime > 0.5f)
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Skill2");
		if (pObj)
		{
			XMFLOAT3 xmf3Pos = GetPosition();
			//xmf3Pos.x += m_xmf3Look.x;
			xmf3Pos.y += 0.1f;
			//xmf3Pos.z += m_xmf3Look.z;
			pObj->SetPosition(xmf3Pos);
			static_cast<CMultiSpriteObject*>(pObj)->SetColor(true);
		}
		m_bSkill2EffectOn = true;
	}


	if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_3))
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Wind");
		if (pObj)
		{
			XMFLOAT3 xmf3Pos = GetPosition(), xmf3Look = GetLook();
			xmf3Look = Vector3::ScalarProduct(xmf3Look, -1.f);
			XMFLOAT3 xmf3LookAt = Vector3::Add(xmf3Pos, xmf3Look);
			xmf3Pos.y += 0.5f;

			pObj->SetPosition(xmf3Pos);
			pObj->SetLookAt(xmf3LookAt, true);

			static_cast<CMultiSpriteObject*>(pObj)->SetColor(true);
		}
	}

}

CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;


	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//cout << "Player Render" << endl;
	UpdateShaderVariables(pd3dCommandList);
	
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//if (nCameraMode == THIRD_PERSON_CAMERA) 
	CGameObject::Render(pd3dCommandList, pCamera);

	m_pComTrail->RenderTrail(pd3dCommandList, pCamera);


	//
	//XMFLOAT3 xmf3Temp = GetPosition(), xmf3Pos = { 5.f,0.f,0.f };
	//CGameObject::SetPosition(xmf3Pos);
	////OnPrepareRender();

	//CGameObject::Render(pd3dCommandList, pCamera);

	//CGameObject::SetPosition(xmf3Temp);
	////OnPrepareRender();

}

void CPlayer::LerpRotate(float fTimeElapsed)
{
	m_xmVecNowRotate = XMVector3Normalize(m_xmVecNowRotate);
	bool bNotTemp = XMVector3Equal(m_xmVecNewRotate, m_xmVecTmpRotate);
	if(bNotTemp)
		m_xmVecNowRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecNewRotate, fTimeElapsed * 10.f);
	else
		m_xmVecNowRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecTmpRotate, fTimeElapsed * 10.f);


	//m_xmVecNowRotate = XMQuaternionSlerp(m_xmVecNowRotate, m_xmVecNewRotate, fTimeElapsed * 5.f);

	XMStoreFloat3(&m_xmf3Look, m_xmVecNowRotate);

	XMVECTOR xmVecRight = XMVector3Cross(XMLoadFloat3(&m_xmf3Up), XMLoadFloat3(&m_xmf3Look));
	XMStoreFloat3(&m_xmf3Right, xmVecRight);

	
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Up = Vector3::Normalize(m_xmf3Up);
	m_xmf3Right = Vector3::Normalize(m_xmf3Right);

}

XMFLOAT3 CPlayer::MoveByDir(float fDistance)
{
	if (isnan(fDistance) != 0)
	{
		fDistance = 0.f;
	}
	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);

	return xmf3Shift;
}

void CPlayer::CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pComponent[COM_COLLISION] = CCollision::Create();

	m_pComCollision = static_cast<CCollision*>(m_pComponent[COM_COLLISION]);
	m_pComCollision->m_isStaticOOBB = false;
	if (m_pChild && m_pChild->m_isRootModelObject)
		m_pComCollision->m_xmLocalOOBB = m_pChild->m_xmOOBB;
	m_pComCollision->m_pxmf4x4World = &m_xmf4x4World;
	//m_pComCollision->m_xmf3OBBScale = { 10.f, 1.f, 10.f }; // �ٿ���ڽ� ������ Ű��
	m_pComCollision->UpdateBoundingBox();

	m_pComponent[COM_TRAIL] = CTrail::Create(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pComTrail = static_cast<CTrail*>(m_pComponent[COM_TRAIL]);
	m_pComTrail->SetColor(true);
}

void CPlayer::UpdateComponent(float fTimeElapsed)
{
	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	XMFLOAT3 xmf3Corners[8];
	if (m_pComCollision)
	{
		m_pComCollision->UpdateBoundingBox();
		m_pComCollision->m_xmOOBB.GetCorners(xmf3Corners);
	}

	if (m_pComTrail)
	{
		//XMFLOAT3 xmf3Top = Vector3::Add(m_pSword->GetPosition(), m_pSword->GetUp(), -1.f);
		//m_pComTrail->AddTrail(xmf3Top, m_pSword->GetPosition()); 
		m_pComTrail->AddTrail(m_pSwordTail->GetPosition(), m_pSword->GetPosition());
	}

	m_pSword->m_pComponent[COM_COLLISION]->Update_Component(fTimeElapsed);
	CCollision* pCol = static_cast<CCollision*>(m_pSword->m_pComponent[COM_COLLISION]);
	if (pCol)
		pCol->UpdateBoundingBox();

	// �����϶��� üũ
	if (IsNowAttack())
		pCol->m_isCollisionIgnore = false;
	else
		pCol->m_isCollisionIgnore = true;

}

void CPlayer::CollsionDetection(CGameObject* pObj, XMFLOAT3* xmf3Line)
{
	OBJ_ID eObjId = pObj->m_eObjId;
	switch (eObjId)
	{
	case OBJ_MAP:
	{
		////���1. �̵��Ѱ� ���
		//XMFLOAT3 xmf3Shift = Vector3::ScalarProduct(m_xmf3PreVelocity, -1, false);
		//m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//m_pCamera->Move(xmf3Shift);
		//m_pCamera->RegenerateViewMatrix();
		//OnPrepareRender();

		////���2. �浿�� ������Ʈ �������� �о��ֱ�
		//XMFLOAT3 xmf3ToPlayer = Vector3::Subtract(m_xmf3PrePosition, pObj->GetPosition(), true, true);
		//xmf3ToPlayer = Vector3::ScalarProduct(xmf3ToPlayer, m_fTempShift);
		//m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3ToPlayer);

		//m_pCamera->Move(xmf3ToPlayer);
		//m_pCamera->RegenerateViewMatrix();
		//OnPrepareRender();

		////���3. �浹�� ������ �������� �������� �о��ֱ�
		//vector<XMFLOAT3> vec = { xmf3Line[0], xmf3Line[1] };
		//m_vecLine.emplace_back(vec);


		XMFLOAT3 xmf3Temp(0, 0, 0), xmf3Normal(0, 0, 0);
		XMFLOAT3 xmf3Point0 = xmf3Line[0], xmf3Point1 = xmf3Line[1]; //���� ���� ���� �̻���? no �̻��ϰ� ���޵�
		xmf3Temp = Vector3::Subtract(xmf3Point0, xmf3Point1, true, false);
		xmf3Normal.z = -xmf3Temp.x;
		xmf3Normal.x = xmf3Temp.z;


		//XMFLOAT3 xmf3Reflect = Vector3::Add(m_xmf3Look, Vector3::ScalarProduct(xmf3Normal, 2 * Vector3::DotProduct(Vector3::ScalarProduct(m_xmf3Look, -1.f), xmf3Normal)));
		XMFLOAT3 xmf3Pos = GetPosition();
		float x0 = xmf3Pos.x, y0 = xmf3Pos.z;
		float x1 = xmf3Point0.x, y1 = xmf3Point0.z;
		float x2 = xmf3Point1.x, y2 = xmf3Point1.z;
		//������ �� ���� �Ÿ�
		float fDis = abs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1)) / (float)sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
		//fDis -= m_pComCollision->m_xmOOBB.Extents.z;
		float fShift = m_pComCollision->m_xmOOBB.Extents.z - fDis;
		if (fShift < 0.f) fShift = 0.f;
		//cout << fShift << endl;
		XMFLOAT3 xmf3Dir = xmf3Normal/*xmf3Reflect*/;
		xmf3Dir = Vector3::ScalarProduct(xmf3Dir, fShift/*m_fTempShift*/, false);
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Dir);

		m_pCamera->Move(xmf3Dir);
		m_pCamera->RegenerateViewMatrix();
		OnPrepareRender();

		cout << xmf3Normal.x << "," << xmf3Normal.z << endl;
		//cout << "Col" << endl;

	}
		break;
	case OBJ_BULLET:
		SetDamaged();
		break;
	case OBJ_END:
		break;
	}
}

void CPlayer::MovePosByCollision()//�浹�� ������ ����� ���� �Ѱ��� �о�� �̵�
{
	if (m_vecLine.empty()) return;

	vector<float> vecDis;
	for (auto& iter : m_vecLine)
	{
		XMFLOAT3 xmf3Line[2] = {iter[0], iter[1]};

		//XMFLOAT3 xmf3Point0 = xmf3Line[0], xmf3Point1 = xmf3Line[1]; //���� ���� ���� �̻���? no �̻��ϰ� ���޵�

		//XMFLOAT3 xmf3Reflect = Vector3::Add(m_xmf3Look, Vector3::ScalarProduct(xmf3Normal, 2 * Vector3::DotProduct(Vector3::ScalarProduct(m_xmf3Look, -1.f), xmf3Normal)));
		XMFLOAT3 xmf3Pos = GetPosition();
		float x0 = xmf3Pos.x, y0 = xmf3Pos.z;
		float x1 = xmf3Line[0].x, y1 = xmf3Line[0].z;
		float x2 = xmf3Line[1].x, y2 = xmf3Line[1].z;
		//������ �� ���� �Ÿ�
		float fDis = abs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1)) / (float)sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
		vecDis.emplace_back(fDis);
	}

	int iMinIndex = int(min_element(vecDis.begin(), vecDis.end()) - vecDis.begin());
	XMFLOAT3 xmf3Line[2] = { m_vecLine[iMinIndex][0],  m_vecLine[iMinIndex][1] };

	XMFLOAT3 xmf3Temp(0, 0, 0), xmf3Normal(0, 0, 0);
	xmf3Temp = Vector3::Subtract(xmf3Line[0], xmf3Line[1], true, false);
	xmf3Normal.z = -xmf3Temp.x;
	xmf3Normal.x = xmf3Temp.z;

	float fShift = m_pComCollision->m_xmOOBB.Extents.z - vecDis[iMinIndex];
	if (fShift < 0.f) fShift = 0.f;

	XMFLOAT3 xmf3Dir = xmf3Normal/*xmf3Reflect*/;
	xmf3Dir = Vector3::ScalarProduct(xmf3Dir, fShift/*m_fTempShift*/, false);
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Dir);

	m_pCamera->Move(xmf3Dir);
	m_pCamera->RegenerateViewMatrix();
	OnPrepareRender();

	//cout << xmf3Normal.x << "," << xmf3Normal.y << "," << xmf3Normal.z<< "," << endl;
	//cout << xmf3Line[1].x << "," << xmf3Line[1].y << "," << xmf3Line[1].z << "," << endl;

	m_vecLine.clear();
}

void CPlayer::HitEffectOn()
{
	CUIObject* pEffect = static_cast<CUIObject*>(CGameMgr::GetInstance()->GetScene()->m_pUIObjectShader->GetObjectList(L"UI_Hit_Effect").front());
	if (!pEffect) return;

	pEffect->m_isHit = true;
}

bool CPlayer::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}

void CPlayer::SetDamaged()
{
	Change_Animation(PLAYER::TAKE_DAMAGED);
}

void CPlayer::Set_object_anim(object_anim* _object_anim)
{
	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		_object_anim[i].fWeight = m_pSkinnedAnimationController->GetTrackWeight(i);
		_object_anim[i].bEnable = m_pSkinnedAnimationController->GetTrackEnable(i);
		_object_anim[i].fPosition = m_pSkinnedAnimationController->m_fPosition[i];
	}
}

void CPlayer::ClickedReadyButton()
{
	m_isReadyToggle = !m_isReadyToggle;
	CServerManager::GetInstance()->m_queue_send_packet.push(CS_READY);
}

void CPlayer::UpdateReadyTexture(float fTimeElapsed)
{
	if (!m_pReadyTex || !m_isReadyToggle) return;

	m_pReadyTex->Animate(fTimeElapsed);

	CScene* pScene = CGameMgr::GetInstance()->GetScene();
	pScene->AddAlphaObjectToList(m_pReadyTex);

	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos.y += 2.f;
	m_pReadyTex->SetPosition(xmf3Pos);
}


CCamera* CPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		//SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(10.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		//m_pCamera->SetOffset(XMFLOAT3(0.0f, CAM_OFFSET_Y, CAM_OFFSET_Z));
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.f, -4.f)); //�κ� ȭ�� ������
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}

	//if (CGameMgr::GetInstance()->GetScene()->m_eCurScene != SCENE::SCENE_0)
		m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	//else
	//	m_pCamera->SetPosition(Vector3::Add(Scene0_SpawnPos, m_pCamera->GetOffset()));

	//Update(fTimeElapsed);

	return(m_pCamera);
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{

}

void CPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{

}

bool CPlayer::Check_Input(float fTimeElapsed)
{
	if (CGameMgr::GetInstance()->GetScene()->m_eCurScene == SCENE::SCENE_0)
		return false;

	// ���� 4���� ó��

		// attack1
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 || m_eCurAnim == PLAYER::ANIM::SKILL1 
		|| m_eCurAnim == PLAYER::ANIM::SKILL2 || m_eCurAnim == PLAYER::ANIM::DIE || m_eCurAnim == PLAYER::ANIM::GET_RESOURCE || m_eCurAnim == PLAYER::ANIM::TAKE_DAMAGED)
	{
		m_fAnimElapsedTime += fTimeElapsed;
		if (m_fAnimElapsedTime >= m_fAnimMaxTime)
		{
			// �̵� ������ Ȯ��
			if(Check_MoveInput())
				Change_Animation(PLAYER::ANIM::RUN);
			else if (m_bBattleOn)
				Change_Animation(PLAYER::ANIM::IDLE);
			else
				Change_Animation(PLAYER::ANIM::IDLE_RELAXED);

		}
		return true;
	}
	if (CInputDev::GetInstance()->LButtonDown())
	{
		Change_Animation(PLAYER::ANIM::ATTACK1);
		return true;
	}
	else if (CInputDev::GetInstance()->RButtonDown())
	{
		Change_Animation(PLAYER::ANIM::ATTACK2);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_1))
	{
		Change_Animation(PLAYER::ANIM::SKILL1);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_2))
	{
		Change_Animation(PLAYER::ANIM::SKILL2);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_0))
	{
		Change_Animation(PLAYER::ANIM::TAKE_DAMAGED);
		SetPosition(XMFLOAT3(54.f, 0.f, 50.f));
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_E))
	{
		Change_Animation(PLAYER::ANIM::GET_RESOURCE);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_9))
	{
		if (m_bBattleOn)
		{
			m_bBattleOn = false;
			Change_Animation(PLAYER::ANIM::IDLE_RELAXED);
		}
		else
		{
			m_bBattleOn = true;
			Change_Animation(PLAYER::ANIM::IDLE);
		}


		return true;
	}
	return false;

}


void CPlayer::Change_Animation(PLAYER::ANIM eNewAnim)
{
	if (m_eCurAnim == eNewAnim)
		return;

	m_ePrevAnim = m_eCurAnim;
	m_eCurAnim = eNewAnim;


	if (m_eCurAnim != PLAYER::SKILL1)
		m_bSkill1EffectOn = false;
	if (m_eCurAnim != PLAYER::SKILL2)
		m_bSkill2EffectOn = false;

	m_fAnimElapsedTime = 0.f;
	m_fBlendingTime = 0.f;
	m_bBlendingOn = true;

	// Prev, Cur ���� Enable
	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		if (i == m_ePrevAnim || i == m_eCurAnim)
			continue;
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pSkinnedAnimationController->SetTrackWeight(i, 0.f);
	}

	// �ִϸ��̼� ����ð� 
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();


	m_pSkinnedAnimationController->SetTrackPosition(m_eCurAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(m_eCurAnim, true);	// ���� �ִϸ��̼� true��, ������ ���� true

	////	// 1 2 3 ������ �ִϸ��̼� ����ȴ��ϸ�. 1,2 ���� �߿� 3���� �ٲ�� 2�� �������� 3�� 1-2�Ǻ��������� ���õǾ�� ��
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
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f);	// �ִϸ��̼� 3����ø ����
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 0.f);
	}
	// ���� �ִϸ��̼��� ����ġ�� 1���� ������ 1�� �ٲ������� �׶����� �����ؾ���
}

void CPlayer::Blending_Animation(float fTimeElapsed)
{
	if (!m_bBlendingOn)
		return;

	m_fBlendingTime += fTimeElapsed * BLENDING_SPEED;

	if (m_fBlendingTime >= 1.f)
	{
		m_bBlendingOn = false;
		m_pSkinnedAnimationController->SetTrackEnable(m_ePrevAnim, false);
		m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 0.f);
		m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, 1.f);

		return;
	}
	

	m_pSkinnedAnimationController->SetTrackWeight(m_ePrevAnim, 1.f-m_fBlendingTime);
	m_pSkinnedAnimationController->SetTrackWeight(m_eCurAnim, m_fBlendingTime);
}


bool CPlayer::Check_MoveInput()
{
	// �̵� ���̸�
	if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) || CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A)
		|| CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) || CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
		return true;
	else
		return false;

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	_TCHAR* pWavName = (_TCHAR*)pCallbackData;
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
	PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
	PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}