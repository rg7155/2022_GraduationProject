#include "stdafx.h"
#include "Player.h"
#include "Camera.h"
#include "InputDev.h"
#include "Scene.h"

CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Projection = Matrix4x4::Identity();
	m_xmf4x4OrthoProjection = Matrix4x4::Identity();
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_nMode = 0x00;
	m_pPlayer = NULL;
	ZeroMemory(&m_xmf3Point, sizeof(m_xmf3Point));
	ZeroMemory(&m_xmf4Plane, sizeof(m_xmf4Plane));
}

CCamera::CCamera(CCamera *pCamera)
{
	if (pCamera)
	{
		*this = *pCamera;
	}
	else
	{
		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Projection = Matrix4x4::Identity();
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode = 0x00;
		m_pPlayer = NULL;
	}
}

CCamera::~CCamera()
{ 
} 

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;

	m_xmf4x4ViewPort = Matrix4x4::Identity();
	m_xmf4x4ViewPort._11 = nWidth / 2.f;
	m_xmf4x4ViewPort._22 = -nHeight / 2.f;
	m_xmf4x4ViewPort._32 = fMinZ - fMaxZ;

	m_xmf4x4ViewPort._41 = nWidth / 2.f;
	m_xmf4x4ViewPort._42 = nHeight / 2.f;
	m_xmf4x4ViewPort._43 = fMinZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);

	m_xmf4x4OrthoProjection = Matrix4x4::OrthographicLH(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f);

//	XMMATRIX xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
//	XMStoreFloat4x4(&m_xmf4x4Projection, xmmtxProjection);
}

void CCamera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void CCamera::GenerateViewMatrix()
{
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);
}

void CCamera::RegenerateViewMatrix()
{
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
}

void CCamera::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void **)&m_pcbMappedCamera);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4View;
	XMStoreFloat4x4(&xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4View, &xmf4x4View, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 xmf4x4Projection;
	XMStoreFloat4x4(&xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4Projection, &xmf4x4Projection, sizeof(XMFLOAT4X4));

	::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(XMFLOAT3));

	//m_xmf4x4OrthoProjection = Matrix4x4::OrthographicLH(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f);
	XMFLOAT4X4 xmf4x4OrthoProjection;
	XMStoreFloat4x4(&xmf4x4OrthoProjection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4OrthoProjection)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4OrthoProjection, &xmf4x4OrthoProjection, sizeof(XMFLOAT4X4));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dGpuVirtualAddress);
}

void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();
	}
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

void CCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	FrustumUpdate();
}

void CCamera::FrustumUpdate()
{
	m_xmf3Point[0] = { -1.f, 1.f, 0.f };
	m_xmf3Point[1] = { 1.f, 1.f, 0.f };
	m_xmf3Point[2] = { 1.f, -1.f, 0.f };
	m_xmf3Point[3] = { -1.f, -1.f, 0.f };

	m_xmf3Point[4] = { -1.f, 1.f, 1.f };
	m_xmf3Point[5] = { 1.f, 1.f, 1.f };
	m_xmf3Point[6] = { 1.f, -1.f, 1.f };
	m_xmf3Point[7] = { -1.f, -1.f, 1.f }; 
	
	XMFLOAT4X4	xmf4x4View, xmf4x4Proj;

	xmf4x4View = Matrix4x4::Inverse(GetViewMatrix());
	xmf4x4Proj = Matrix4x4::Inverse(GetProjectionMatrix());

	for (int i = 0; i < 8; ++i)
	{
		m_xmf3Point[i] = Vector3::TransformCoord(m_xmf3Point[i], xmf4x4Proj);
		m_xmf3Point[i] = Vector3::TransformCoord(m_xmf3Point[i], xmf4x4View);
	}

	// x+
	m_xmf4Plane[0] = GetPlane(m_xmf3Point[1], m_xmf3Point[5], m_xmf3Point[6]);
	// x-
	m_xmf4Plane[1] = GetPlane(m_xmf3Point[4], m_xmf3Point[0], m_xmf3Point[3]);
	// y+
	m_xmf4Plane[2] = GetPlane(m_xmf3Point[4], m_xmf3Point[5], m_xmf3Point[1]);
	// y-
	m_xmf4Plane[3] = GetPlane(m_xmf3Point[3], m_xmf3Point[2], m_xmf3Point[6]);
	// z+
	m_xmf4Plane[4] = GetPlane(m_xmf3Point[7], m_xmf3Point[6], m_xmf3Point[5]);
	// z-
	m_xmf4Plane[5] = GetPlane(m_xmf3Point[0], m_xmf3Point[1], m_xmf3Point[2]);
}


XMFLOAT4 CCamera::GetPlane(XMFLOAT3& xmf3Pos1, XMFLOAT3& xmf3Pos2, XMFLOAT3& xmf3Pos3)
{
	XMFLOAT4 xmf4Plane;
	XMStoreFloat4(&xmf4Plane, (XMPlaneFromPoints(XMLoadFloat3(&xmf3Pos1), XMLoadFloat3(&xmf3Pos2), XMLoadFloat3(&xmf3Pos3))));
	return  xmf4Plane;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSpaceShipCamera

CSpaceShipCamera::CSpaceShipCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate(float x, float y, float z)
{
	if (m_pPlayer && (x != 0.0f))
	{
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Right), XMConvertToRadians(x));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFirstPersonCamera

CFirstPersonCamera::CFirstPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CThirdPersonCamera

CThirdPersonCamera::CThirdPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	
	if (m_isStartFocusing)
	{
		m_fTime += fTimeElapsed;

		SetLookAtPosition(m_xmf3TargetPos);

		XMFLOAT3 xmf3OffsetPos = Vector3::Add(m_xmf3TargetPos, m_xmf3TargetLookingOffset);
		if (m_fTime <= 1.f)
			m_xmf3Position = Vector3::Lerp(m_xmf3PrePos, xmf3OffsetPos, m_fTime);
		
	}
	else
		RotateByMouse(xmf3LookAt);

	if (m_isShaking) CameraShaking(fTimeElapsed);

	
	CCamera::Update(xmf3LookAt, fTimeElapsed);
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, m_pPlayer->GetUpVector());
	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}

void CThirdPersonCamera::RotateByMouse(XMFLOAT3& xmf3LookAt)
{
	if (CGameMgr::GetInstance()->GetScene()->m_eCurScene == SCENE::SCENE_0)
		return;

	// 마우스 회전 
	// 위아래
	long dwMouseMove = 0;
	dwMouseMove = CInputDev::GetInstance()->Get_DIMouseMove(DIMS_Y);
	//if (dwMouseMove = CInputDev::GetInstance()->Get_DIMouseMove(DIMS_Y))
	{
		// 카메라 right 기준
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(dwMouseMove / DPI));
		XMFLOAT3 xmf3NewOffSet = Vector3::TransformNormal(m_xmf3Offset, xmmtxRotate);

		//상하 회전 제한 브랜치
		if (xmf3NewOffSet.y > 1.f && xmf3NewOffSet.y < CAM_OFFSET_Y + CAM_OFFSET_Y*0.4f)
		{
			m_xmf3Offset = xmf3NewOffSet;
		}

		if (m_pPlayer)
		{
			XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();

			XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
			XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
			XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
			float fLength = Vector3::Length(xmf3Direction);
			xmf3Direction = Vector3::Normalize(xmf3Direction);

			if (fLength > 0)
			{
				m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fLength);
				//SetLookAt(xmf3LookAt);
			}
		}

	}

	dwMouseMove = CInputDev::GetInstance()->Get_DIMouseMove(DIMS_X);
	// 좌우
	//if (dwMouseMove = CInputDev::GetInstance()->Get_DIMouseMove(DIMS_X))
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(dwMouseMove / DPI));
		m_xmf3Offset = Vector3::TransformNormal(m_xmf3Offset, xmmtxRotate);

		if (m_pPlayer)
		{
			XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();

			XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
			XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
			XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
			float fLength = Vector3::Length(xmf3Direction);
			xmf3Direction = Vector3::Normalize(xmf3Direction);

			if (fLength > 0)
			{
				m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fLength);
				//SetLookAt(xmf3LookAt);

			}
		}

	}
}

void CThirdPersonCamera::SetFocusOnTarget(bool isFocus, XMFLOAT3 xmf3Target, XMFLOAT3 xmf3TargetOffset)
{
	m_xmf3TargetPos = xmf3Target;
	m_xmf3TargetLookingOffset = xmf3TargetOffset;
	m_isStartFocusing = isFocus;
	if (isFocus)
	{
		m_xmf3PrePos = m_xmf3Position;
		//m_fTime += fTimeElapsed;

		//Vector3::Lerp(m_xmf3Position, m_xmf3TargetPos, 0.5f);

	}
	else
	{

	}
}

void CThirdPersonCamera::CameraShaking(float fTimeElapsed)
{
	m_fShakeTime += fTimeElapsed * 5.f;
	float fValue = sinf(m_fShakeTime * 10.f) * powf(0.5f, m_fShakeTime);
	//cout << "Cam Shake Val-" << fValue << endl;

	XMFLOAT3 xmf3Dir;
	if (m_isSero) 
		xmf3Dir = { 0.f, 1.f, 0.f };
	else
	{
		XMFLOAT3 xmf3Right = m_xmf3Right;
		xmf3Right.y = 0.f;
		xmf3Dir = Vector3::Normalize(xmf3Right);
		//cout << xmf3Dir.x << "," << xmf3Dir.z << endl;
	}
	xmf3Dir = Vector3::ScalarProduct(xmf3Dir, fValue * 2.f, false);
	Move(xmf3Dir);
	 
	//XMFLOAT3 xmf3Pos = GetPosition();s
	//xmf3Pos.y += fValue;
	//SetPosition(xmf3Pos);

	//m_xmf3Offset.y += fValue;
	if (m_fShakeTime > 10.f)
	{
		m_isShaking = false;
		//m_xmf3Offset = { 0.f, CAM_OFFSET_Y, CAM_OFFSET_Z };
	}

	//XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), m_xmf3Offset);
	//SetPosition(xmf3Position);
}

void CThirdPersonCamera::DoShaking(bool isSero)
{
	m_isSero = isSero;
	m_isShaking = true;
	m_fShakeTime = 0.f;
}
