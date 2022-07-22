#pragma once

#include "Object.h"
#include "Camera.h"
#include "InputDev.h"

const static XMFLOAT3 Scene0_SpawnPos = { 40.f, 0.f, 60.f };
const static XMFLOAT3 Scene1_SpawnPos = { 25.f, 0.f, 25.f };
const static XMFLOAT3 Scene2_SpawnPos = { 10.f, 0.f, 15.f };

const static XMFLOAT3 Scene0_SpawnPos_Duo = { 42.f, 0.f, 60.f };
const static XMFLOAT3 Scene1_SpawnPos_Duo = { 27.f, 0.f, 25.f };
const static XMFLOAT3 Scene2_SpawnPos_Duo = { 12.f, 0.f, 15.f };
class CUIObject;
class CPlayer : public CGameObject
{
//private:
//	enum ANIM {
//		IDLE_RELAXED = 0, RUN = 1, ATTACK1 = 2, ATTACK2 = 3, SKILL1 = 4,
//		SKILL2 = 5, IDLE = 6, GET_RESOURCE = 7, DIE = 8, END = 9,
//	};

protected:	
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3PrePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3PreVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;
	float						m_fTempShift = 0.f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera						*m_pCamera = NULL;
	CGameObject					*m_pSword = NULL;
	CGameObject					*m_pSwordTail = NULL;
public:
	CGameObject* GetSword() { return m_pSword; }
public:
	CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext);
	~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) 
	{ 
		Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); 
	}

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);
	
	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

private:
	void LerpRotate(float fTimeElapsed);
	void Blending_Animation(float fTimeElapsed);
	XMFLOAT3 MoveByDir(float fDistance);

private:
	bool Check_Input(float fTimeElapsed);
	bool Check_MoveInput();
	void Check_CreateEffect();
public:
	void Change_Animation(PLAYER::ANIM eNewAnim);

private:
	virtual void	CreateComponent(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual void	UpdateComponent(float fTimeElapsed);

public:
	virtual void	CollsionDetection(CGameObject* pObj, XMFLOAT3* xmf3Line) override;

public:
	bool IsNowAttack();
	void SetDamaged();

private:
	XMVECTOR m_xmVecNowRotate;
	XMVECTOR m_xmVecTmpRotate;
	XMVECTOR m_xmVecNewRotate;

	XMVECTOR m_xmVecSrc;

public:
	PLAYER::ANIM GetCurAnim() { return m_eCurAnim; }
	void Set_object_anim(object_anim* _object_anim);
protected:
	PLAYER::ANIM	m_ePrevAnim;			// 이전 애니메이션
	PLAYER::ANIM	m_eCurAnim;				// 현재 애니메이션
	float	m_fBlendingTime;		// 블렌딩 시간
	float	m_fAnimMaxTime;			// 현재 애니메이션의 진행 시간
	float	m_fAnimElapsedTime;		// 현재 애니메이션의 흐른 시간
	float	m_fLerpSpeed;
	bool	m_bLerpSpeedOn;

private:
	bool	m_bBattleOn;
	bool	m_bBlendingOn;
	bool	m_bSkill1EffectOn;
	bool	m_bSkill2EffectOn;

public:
	CCollision* m_pComCollision = nullptr;
	CTrail* m_pComTrail = nullptr;
public:
	char	m_dir;

public:
	void	ClickedReadyButton(); 
	bool	m_isReadyToggle = false;
	CUIObject* m_pSkillICon[2] = { nullptr };
private:
	void UpdateReadyTexture(float fTimeElapsed);
	void MovePosByCollision();
	void HitEffectOn();

	CGameObject*	m_pReadyTex = nullptr;

	vector<vector<XMFLOAT3>> m_vecLine;
};


class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

