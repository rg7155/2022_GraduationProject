//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "InputDev.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
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
	m_xmVecNewRotate = XMVectorSet(0.f, 0.f, 1.f, 1.f);
	m_xmVecSrc = XMVectorSet(0.f, 0.f, 1.f, 1.f);

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;


	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "../Datas/Player_Blue/Adventurer_Aland_Blue.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);


	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 9, pPlayerModel);
	for (int i = 0; i < 9; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackEnable(0, true);


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
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);

	SetPosition(XMFLOAT3(10.0f, 0, 10.0f));

	if (pPlayerModel) delete pPlayerModel;


	m_bBattleOn = false;
	m_eCurAnim = ANIM::IDLE;

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
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	// 대기동작일때만 움직이기 가능
	if (m_eCurAnim != ANIM::IDLE && m_eCurAnim != ANIM::IDLE_RELAXED && m_eCurAnim != ANIM::RUN)
	{
		return;
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
		return;

	m_xmVecNewRotate = XMVector3Normalize(xmDstVec);

	// 180도 확인
	XMVECTOR xmVecAngle =  XMVector3AngleBetweenNormals(m_xmVecNowRotate, m_xmVecNewRotate);
	float fAngle = XMVectorGetX(xmVecAngle);

	if (XMConvertToDegrees(fAngle) >= 170)
	{
		m_xmVecNowRotate = XMVector3Normalize(m_xmVecNowRotate);
		m_xmVecNowRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecNewRotate, 0.5f);
	}
	// 이동
	Move(MoveByDir(fDistance), bUpdateVelocity);

	// 현재가 RUN이면 애니메이션 바꾸지 않아도 된다.
	if (m_eCurAnim != ANIM::RUN)
		Change_Animation(ANIM::RUN);

}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
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



	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		
		if (Check_Input(fTimeElapsed))
			return;

		if (!Check_MoveInput())
		{
			// IDLE 애니메이션이 실행되도록 하기 위해
			if (m_eCurAnim == ANIM::IDLE || m_eCurAnim == ANIM::IDLE_RELAXED)
				return;

			if (m_bBattleOn)
				Change_Animation(ANIM::IDLE);
			else
				Change_Animation(ANIM::IDLE_RELAXED);
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
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//if (nCameraMode == THIRD_PERSON_CAMERA) 
	CGameObject::Render(pd3dCommandList, pCamera);
}

void CPlayer::LerpRotate(float fTimeElapsed)
{
	m_xmVecNowRotate = XMVector3Normalize(m_xmVecNowRotate);
	m_xmVecNowRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecNewRotate, fTimeElapsed * 8.f);
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
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 5.0f, -6.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

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
	if (m_eCurAnim == ANIM::ATTACK1 || m_eCurAnim == ANIM::ATTACK2 || m_eCurAnim == ANIM::SKILL1 || m_eCurAnim == ANIM::SKILL2 || m_eCurAnim == ANIM::DIE || m_eCurAnim == ANIM::GET_RESOURCE)
	{
		m_fAnimElapsedTime += fTimeElapsed;
		if (m_fAnimElapsedTime >= m_fAnimMaxTime)
		{
			if(m_bBattleOn)
				Change_Animation(ANIM::IDLE);
			else
				Change_Animation(ANIM::IDLE_RELAXED);

		}
		return true;
	}
	else if (CInputDev::GetInstance()->LButtonDown())
	{
		Change_Animation(ANIM::ATTACK1);
		return true;
	}
	else if (CInputDev::GetInstance()->RButtonDown())
	{
		Change_Animation(ANIM::ATTACK2);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_1))
	{
		Change_Animation(ANIM::SKILL1);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_2))
	{
		Change_Animation(ANIM::SKILL2);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_0))
	{
		Change_Animation(ANIM::DIE);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_E))
	{
		Change_Animation(ANIM::GET_RESOURCE);
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_9))
	{
		if (m_bBattleOn)
		{
			m_bBattleOn = false;
			Change_Animation(ANIM::IDLE_RELAXED);
		}
		else
		{
			m_bBattleOn = true;
			Change_Animation(ANIM::IDLE);
		}


		return true;
	}
	return false;
}

void CPlayer::Change_Animation(ANIM eNewAnim)
{
	m_eCurAnim = eNewAnim;
	m_fAnimElapsedTime = 0.f;

	// 애니메이션 진행시간 
	m_fAnimMaxTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[eNewAnim]->GetLength();

	// 모든 애니메이션 false로
	for (int i = 0; i < ANIM::END; i++)
	{
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackPosition(eNewAnim, 0.f);
	m_pSkinnedAnimationController->SetTrackEnable(eNewAnim, true);
}

bool CPlayer::Check_MoveInput()
{
	// 이동 중이면
	if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) || CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A)
		|| CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) || CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
		return true;
	else
		return false;

	//// 이동 중이지 않으면
	//if (!CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) && !CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A)
	//	&& !CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) && !CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
	//	return false;
}