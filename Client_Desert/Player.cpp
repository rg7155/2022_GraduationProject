//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "InputDev.h"
#include "Animation.h"
#include "Scene.h"
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

	m_xmVecNowRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecTmpRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecNewRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecSrc = XMVectorSet(0.f, 0.f, 1.f, 1.f);

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	SetPosition(XMFLOAT3(25.0f, 0, 25.0f));
	
	char fileName[2048];
	m_iId = *((int*)pContext);
	if (m_iId == 0)
		strcpy(fileName, "Model/Adventurer_Aland_Blue.bin");
	else
		strcpy(fileName, "Model/Adventurer_Aland_Green.bin");

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fileName, NULL);

	SetChild(pPlayerModel->m_pModelRootObject, true);

	m_pSword = FindFrame("Sword");

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
	//컴포넌트
	CreateComponent(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_eObjId = OBJ_PLAYER;
	///////////////////////////////////////////////



}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
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
	// 대기동작이나 이동중일때만 움직이기 가능
	if (m_eCurAnim != PLAYER::ANIM::IDLE && m_eCurAnim != PLAYER::ANIM::IDLE_RELAXED && m_eCurAnim != PLAYER::ANIM::RUN)
	{
		m_fLerpSpeed = 0.f;
		return;
	}
	// 속도 보간
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
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))	// 위오
	{
		xmDstVec = xmVecCamRight + xmVecCamLook;
		
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S))	// 오아
	{
		xmDstVec = xmVecCamRight - xmVecCamLook;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A))	// 아왼
	{
		xmDstVec = -xmVecCamRight - xmVecCamLook;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W))	// 왼위
	{
		xmDstVec = -xmVecCamRight + xmVecCamLook;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
	{
		xmDstVec = xmVecCamRight;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A))
	{
		xmDstVec = -xmVecCamRight;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W))
	{
		xmDstVec = xmVecCamLook;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S))
	{
		xmDstVec = -xmVecCamLook;
	}
	else
	{
			
		return;
	}

	m_xmVecNewRotate = XMVector3Normalize(xmDstVec);

	// 180도 확인
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

	// 이동
	Move(MoveByDir(fDistance * m_fLerpSpeed), bUpdateVelocity);
	m_fTempShift = fDistance * m_fLerpSpeed;

	// 현재가 RUN이면 애니메이션 바꾸지 않아도 된다.
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
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
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
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		//m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		//m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(m_xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	LerpRotate(fTimeElapsed);

	Blending_Animation(fTimeElapsed);

	//렌더링 껐다 켰다-> 공격할때만 나오게 변경
	if (IsNowAttack())
		m_pComTrail->SetRenderingTrail(true);
	else
		m_pComTrail->SetRenderingTrail(false);

	////////////////////////////////////////////
	UpdateComponent(fTimeElapsed);
	////////////////////////////////////////////

	// 바닥 이펙트
	if (m_eCurAnim == PLAYER::ANIM::SKILL1 && !m_bSkill1EffectOn && m_fAnimElapsedTime > 1.0f)
	{
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Shockwave");
		if (pObj) {
			XMFLOAT3 xmf3Pos = GetPosition();
			xmf3Pos.x += m_xmf3Look.x;
			xmf3Pos.y += 0.1f;
			xmf3Pos.z += m_xmf3Look.z;
			pObj->SetPosition(xmf3Pos);
		}
		m_bSkill1EffectOn = true;

	}
	// 애니메이션이 끝나면 이펙트 끄는 것 해야함
	
	//if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_N))
	//{
	//	CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->SetActiveObjectFromShader(L"MultiSprite", L"Explosion");
	//	if(pObj) pObj->SetPosition(GetPosition());
	//}

	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (Check_Input(fTimeElapsed))
			return;

		if (!Check_MoveInput())
		{   
			// IDLE 애니메이션이 실행되도록 하기 위해
			if (m_eCurAnim == PLAYER::ANIM::IDLE || m_eCurAnim == PLAYER::ANIM::IDLE_RELAXED)
				return;

			if (m_bBattleOn)
				Change_Animation(PLAYER::ANIM::IDLE);
			else
				Change_Animation(PLAYER::ANIM::IDLE_RELAXED);
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
	m_pComCollision->UpdateBoundingBox();

	m_pComponent[COM_TRAIL] = CTrail::Create(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pComTrail = static_cast<CTrail*>(m_pComponent[COM_TRAIL]);


}

void CPlayer::UpdateComponent(float fTimeElapsed)
{
	for (int i = 0; i < COM_END; ++i)
		if (m_pComponent[i])
			m_pComponent[i]->Update_Component(fTimeElapsed);

	if (m_pComCollision)
		m_pComCollision->UpdateBoundingBox();

	if (m_pComTrail)
	{
		XMFLOAT3 xmf3Top = Vector3::Add(m_pSword->GetPosition(), m_pSword->GetUp(), -1.f);
		m_pComTrail->AddTrail(xmf3Top, m_pSword->GetPosition()); 
	}
}

void CPlayer::CollsionDetection(CGameObject* pObj)
{
	OBJ_ID eObjId = pObj->m_eObjId;
	switch (eObjId)
	{
	case OBJ_MAP:
		////방법1. 이동한값 취소
		//XMFLOAT3 xmf3Shift = Vector3::ScalarProduct(m_xmf3PreVelocity, -1, false);
		//m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//m_pCamera->Move(xmf3Shift);
		//m_pCamera->RegenerateViewMatrix();
		//OnPrepareRender();

		//방법2. 오브젝트 밀어주기
		XMFLOAT3 xmf3ToPlayer = Vector3::Subtract(m_xmf3Position, pObj->GetPosition(), true, true);
		//xmf3ToPlayer = Vector3::ScalarProduct(xmf3ToPlayer, PLAYER_SPEED * CGameMgr::GetInstance()->m_fElapsedTime);
		xmf3ToPlayer = Vector3::ScalarProduct(xmf3ToPlayer, m_fTempShift);
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3ToPlayer);

		m_pCamera->Move(xmf3ToPlayer);
		m_pCamera->RegenerateViewMatrix();
		OnPrepareRender();
		break;
	case OBJ_END:
		break;
	}
}

bool CPlayer::IsNowAttack()
{
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 ||
		m_eCurAnim == PLAYER::ANIM::SKILL1 || m_eCurAnim == PLAYER::ANIM::SKILL2)
		return true;

	return false;
}

CS_MOVE_PACKET* CPlayer::Server_GetParentAndAnimation()
{
	// 행렬
	CS_MOVE_PACKET* _duoPlayer = new CS_MOVE_PACKET;
	_duoPlayer->xmf4x4World = m_xmf4x4ToParent;

	for (int i = 0; i < PLAYER::ANIM::END; i++)
	{
		_duoPlayer->animInfo[i].fWeight = m_pSkinnedAnimationController->GetTrackWeight(i);
		_duoPlayer->animInfo[i].bEnable = m_pSkinnedAnimationController->GetTrackEnable(i);
		_duoPlayer->animInfo[i].fPosition = m_pSkinnedAnimationController->m_fPosition[i];
	}
	_duoPlayer->eCurAnim = m_eCurAnim;
	return _duoPlayer;
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
		m_pCamera->SetOffset(XMFLOAT3(0.0f, CAM_OFFSET_Y, CAM_OFFSET_Z));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
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
	// 공격 4가지 처리

		// attack1
	if (m_eCurAnim == PLAYER::ANIM::ATTACK1 || m_eCurAnim == PLAYER::ANIM::ATTACK2 || m_eCurAnim == PLAYER::ANIM::SKILL1 
		|| m_eCurAnim == PLAYER::ANIM::SKILL2 || m_eCurAnim == PLAYER::ANIM::DIE || m_eCurAnim == PLAYER::ANIM::GET_RESOURCE || m_eCurAnim == PLAYER::ANIM::TAKE_DAMAGED)
	{
		m_fAnimElapsedTime += fTimeElapsed;
		if (m_fAnimElapsedTime >= m_fAnimMaxTime)
		{
			// 이동 중인지 확인
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
	m_ePrevAnim = m_eCurAnim;
	m_eCurAnim = eNewAnim;


	if (m_eCurAnim != PLAYER::SKILL1)
		m_bSkill1EffectOn = false;


	m_fAnimElapsedTime = 0.f;
	m_fBlendingTime = 0.f;
	m_bBlendingOn = true;

	// Prev, Cur 빼고 Enable
	for (int i = 0; i < PLAYER::ANIM::END; i++)
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
	// 이동 중이면
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