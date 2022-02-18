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

CPlayer::CPlayer()
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

	m_xmVecNowRotate = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	m_xmVecNewRotate = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	float fRotateAngle = 0.f;
	//if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
	//if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
	if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) &&	
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))	// 위오
	{
		fRotateAngle = 45.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S))	// 오아
	{
		fRotateAngle = 135.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A))	// 아왼
	{
		fRotateAngle = -135.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A) &&
		CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W))	// 왼위
	{
		fRotateAngle = -45.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
	{
		fRotateAngle = 90.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A))
	{
		fRotateAngle = -90.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W))
	{
		fRotateAngle = 0.f;
	}
	else if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S))
	{
		// if(nowAngle < 0) fRotateAngle = -180.f;
		if(XMVectorGetY(m_xmVecNowRotate) < 0.f)
			fRotateAngle = -180.f;
		else
			fRotateAngle = 180.f;


	}
	else
		return;

	Move(MoveByDir(fRotateAngle, fDistance), bUpdateVelocity);
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);

		//맵 충돌
		if (CheckCollision(OBJ_ID::OBJ_MAP))
		{
			XMFLOAT3 xmf3TempShift = xmf3Shift;
			xmf3TempShift = Vector3::ScalarProduct(xmf3TempShift, -1.5, false);
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3TempShift);
		}
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

	//UpdateBoundingBox();
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
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

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//if (nCameraMode == THIRD_PERSON_CAMERA) 
		CGameObject::Render(pd3dCommandList, pCamera);
}

void CPlayer::LerpRotate(float fTimeElapsed)
{

	// 2. Lerp와 행렬 적용 프레임마다 진행
	float fPrevAngle = XMVectorGetY(m_xmVecNowRotate);
	m_xmVecNowRotate = XMVectorLerp(m_xmVecNowRotate, m_xmVecNewRotate, fTimeElapsed*5.f);

	//// 보간된 회전값이 범위를 벗어날 경우
	//if (XMVectorGetY(m_xmVecNowRotate) > 360.0f)
	//	XMVectorSetY(m_xmVecNowRotate, XMVectorGetY(m_xmVecNowRotate) - 360.f);
	//if (XMVectorGetY(m_xmVecNowRotate) < 0.0f)
	//	XMVectorSetY(m_xmVecNowRotate, XMVectorGetY(m_xmVecNowdwRotate) + 360.f);
	float fRotateAngle = XMVectorGetY(m_xmVecNowRotate) - fPrevAngle;

	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fRotateAngle));
	m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

}

XMFLOAT3 CPlayer::MoveByDir(float fAngle, float fDistance)
{
	// if(nowAngle < 0) fRotateAngle = -180.f;
	if (fAngle > 0.f && int(XMVectorGetY(m_xmVecNowRotate)) == -180)
	{
		m_xmVecNowRotate = XMVectorSetY(m_xmVecNowRotate, 180.f);
	}
	else if (fAngle < 0.f && int(XMVectorGetY(m_xmVecNowRotate)) == 180)
	{
		m_xmVecNowRotate = XMVectorSetY(m_xmVecNowRotate, -180.f);
	}

	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

	m_xmVecNewRotate = XMVectorSetY(m_xmVecNewRotate, fAngle);
	xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
	
	return xmf3Shift;
}

bool CPlayer::CheckCollision(OBJ_ID eObjId)
{
	//CollisionMgr에서 충돌판정을 하고 충돌 했으면 그 오브젝트의 충돌함수를 불러서 eID로 switch문 돌리기
	switch (eObjId)
	{
	case OBJ_MAP:
	{
		CMapObjectsShader* pMapObjectShader = CGameMgr::GetInstance()->GetScene()->m_pMapObjectShader;

		XMFLOAT3 xmf3TempPos = Vector3::Add(m_xmf3Position, m_xmf3Velocity);

		for (int i = 0; i < pMapObjectShader->m_nObjects; ++i)
		{
			CMapObject* pMapObject = static_cast<CMapObject*>(pMapObjectShader->m_ppObjects[i]);
			if (!pMapObject->m_isCollisionIgnore)
			{
				if (m_pComCollision->Check_Collision_AfterMove(pMapObject->m_pComCollision->m_xmOOBB, xmf3TempPos, m_xmf4x4World))
					return true;
			}
		}
		break;
	}
	case OBJ_END:
		break;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CLoadedModelInfo *pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "../Datas/Player_Blue/Adventurer_Aland_Blue.bin", NULL);
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
	//CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();
	//m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);s

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	//SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);

	SetPosition(XMFLOAT3(10.0f, 0, 10.0f));

	if (pPlayerModel) delete pPlayerModel;

	//컴포넌트
	CreateComponent();
}

CTerrainPlayer::~CTerrainPlayer()
{
}

void CTerrainPlayer::CreateComponent()
{
	m_pComponent[COM_COLLISION] = CCollision::Create();

	m_pComCollision = static_cast<CCollision*>(m_pComponent[COM_COLLISION]);
	m_pComCollision->m_isStaticOOBB = false;
	if (m_pChild && m_pChild->m_isRootModelObject)
		m_pComCollision->m_xmLocalOOBB = m_pChild->m_xmOOBB;
	m_pComCollision->m_pxmf4x4World = &m_xmf4x4World;
	m_pComCollision->UpdateBoundingBox();
}
 
CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
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
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, -6.0f));
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

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{

}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{

}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	// 다른 동작 중일 때는 움직이지 않음
	if (!m_eCurAnim == ANIM::IDLE && !m_eCurAnim == ANIM::IDLE_RELAXED)
		return;

	if (Check_MoveInput())
	{
		Change_Animation(ANIM::RUN);
		CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
	}

}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	
	CPlayer::Update(fTimeElapsed);


	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		// GET_RESOURCE, 공격 중에는 다른 동작으로 바뀌지 않음
		if (!m_eCurAnim == ANIM::IDLE && !m_eCurAnim == ANIM::IDLE_RELAXED)
			return;

		// 공격키 확인
		if (Check_Attack(fTimeElapsed))
			return;

		// 상호작용키 확인
		if (Check_GetResource(fTimeElapsed))
			return;

		if(!Check_MoveInput())
		{
			Change_Animation(ANIM::IDLE_RELAXED);
		}
	}
}



bool CTerrainPlayer::Check_GetResource(float fTimeElapsed)
{
	if (m_eCurAnim == ANIM::GET_RESOURCE)
	{
		m_fAnimElapsedTime += fTimeElapsed;
		if (m_fAnimElapsedTime >= m_fAnimMaxTime)
		{
			Change_Animation(ANIM::IDLE_RELAXED);
		}
		return true;
	}
	else if (CInputDev::GetInstance()->KeyDown(DIKEYBOARD_E))
	{
		Change_Animation(ANIM::GET_RESOURCE);
		return true;
	}

	return false;
}

bool CTerrainPlayer::Check_Attack(float fTimeElapsed)
{
	// 공격 4가지 처리

	// attack1
	if (m_eCurAnim == ANIM::ATTACK1)
	{
		m_fAnimElapsedTime += fTimeElapsed;
		if (m_fAnimElapsedTime >= m_fAnimMaxTime)
		{
			Change_Animation(ANIM::IDLE);
		}
		return true;
	}
	else if (CInputDev::GetInstance()->LButtonDown())
	{
		Change_Animation(ANIM::ATTACK1);
		return true;
	}
	// attack2
	// skill1
	// skill2

	return false;
}

void CTerrainPlayer::Change_Animation(ANIM eNewAnim)
{
	switch (eNewAnim)
	{
	case IDLE_RELAXED:
		m_eCurAnim = ANIM::IDLE_RELAXED;
		m_fAnimMaxTime = 0.f;
		m_fAnimElapsedTime = 0.f;
		m_pSkinnedAnimationController->SetTrackEnable(IDLE_RELAXED, true);
		m_pSkinnedAnimationController->SetTrackEnable(RUN, false);
		m_pSkinnedAnimationController->SetTrackEnable(GET_RESOURCE, false);
		m_pSkinnedAnimationController->SetTrackPosition(RUN, 0.0f);
		m_pSkinnedAnimationController->SetTrackEnable(ATTACK1, false);

		break;
	case RUN:
		m_eCurAnim = ANIM::RUN;
		m_fAnimMaxTime = 2.5f;
		m_fAnimElapsedTime = 0.f;
		m_pSkinnedAnimationController->SetTrackEnable(IDLE_RELAXED, false);
		m_pSkinnedAnimationController->SetTrackEnable(RUN, true);
		m_pSkinnedAnimationController->SetTrackEnable(GET_RESOURCE, false);
		m_pSkinnedAnimationController->SetTrackEnable(ATTACK1, false);

		break;
	case ATTACK1:
		m_eCurAnim = ANIM::ATTACK1;
		m_fAnimMaxTime = 1.5f;
		m_fAnimElapsedTime = 0.f;
		m_pSkinnedAnimationController->SetTrackEnable(IDLE_RELAXED, false);
		m_pSkinnedAnimationController->SetTrackEnable(RUN, false);
		m_pSkinnedAnimationController->SetTrackEnable(GET_RESOURCE, false);
		m_pSkinnedAnimationController->SetTrackEnable(ATTACK1, true);
		m_pSkinnedAnimationController->SetTrackPosition(ATTACK1, 0.f);

		break;
	case ATTACK2:
		m_eCurAnim = ANIM::ATTACK2;
		m_fAnimMaxTime = 2.5f;
		m_fAnimElapsedTime = 0.f;
		break;
	case SKILL1:
		m_eCurAnim = ANIM::SKILL1;
		m_fAnimMaxTime = 2.5f;
		m_fAnimElapsedTime = 0.f;
		break;
	case SKILL2:
		m_eCurAnim = ANIM::SKILL2;
		m_fAnimMaxTime = 2.5f;
		m_fAnimElapsedTime = 0.f;
		break;
	case GET_RESOURCE:
		m_eCurAnim = ANIM::GET_RESOURCE;
		m_fAnimMaxTime = 1.833333f;
		m_fAnimElapsedTime = 0.f;
		m_pSkinnedAnimationController->SetTrackEnable(IDLE_RELAXED, false);
		m_pSkinnedAnimationController->SetTrackEnable(RUN, false);
		m_pSkinnedAnimationController->SetTrackEnable(ATTACK1, false);
		m_pSkinnedAnimationController->SetTrackEnable(GET_RESOURCE, true);
		m_pSkinnedAnimationController->SetTrackPosition(RUN, 0.0f);
		m_pSkinnedAnimationController->SetTrackPosition(GET_RESOURCE, 0.f);
	
		break;
	case IDLE:
		m_eCurAnim = ANIM::IDLE;
		m_fAnimMaxTime = 2.5f;
		m_fAnimElapsedTime = 0.f;
		break;
	case DIE:
		m_eCurAnim = ANIM::DIE;
		m_fAnimMaxTime = 2.5f;
		m_fAnimElapsedTime = 0.f;
		break;
	default:
		break;
	}
}

bool CTerrainPlayer::Check_MoveInput()
{
	// 이동 중이면
	if (CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) || CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A)
		|| CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) || CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
		return true;

	// 이동 중이지 않으면
	if (!CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_W) && !CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_A)
		&& !CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_S) && !CInputDev::GetInstance()->KeyPressing(DIKEYBOARD_D))
		return false;
}