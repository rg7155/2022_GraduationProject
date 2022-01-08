#include "InputDev.h"

IMPLEMENT_SINGLETON(CInputDev)

CInputDev::CInputDev(void)
{

}

CInputDev::~CInputDev(void)
{
	//Free();
}

HRESULT CInputDev::Ready_InputDev(HINSTANCE hInst, HWND hWnd)
{

	// DInput 컴객체를 생성하는 함수
	FAILED_CHECK_RETURN(DirectInput8Create(hInst,
											DIRECTINPUT_VERSION,
											IID_IDirectInput8,
											(void**)&m_pInputSDK,
											NULL), E_FAIL);

	// 키보드 객체 생성
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr), E_FAIL);

	// 생성된 키보드 객체의 대한 정보를 컴 객체에게 전달하는 함수
	m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard);

	// 장치에 대한 독점권을 설정해주는 함수, (클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수)
	//m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	m_pKeyBoard->Acquire();


	// 마우스 객체 생성
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr), E_FAIL);

	// 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
	m_pMouse->SetDataFormat(&c_dfDIMouse2);

	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	//m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	m_pMouse->Acquire();

	ZeroMemory(m_KeyboardInfo, sizeof(KEYINFO) * 256);
	ZeroMemory(m_MouseInfo, sizeof(KEYINFO) * 3);

	return S_OK;
}

void CInputDev::Set_InputDev(void)
{
	HRESULT	result = m_pKeyBoard->GetDeviceState(256, m_byKeyState);

	for (size_t i = 0; i < 256; i++)
	{
		if (m_byKeyState[i] & 0x80)
		{
			if (!m_KeyboardInfo[i].bKeyDown && !m_KeyboardInfo[i].bKeyPressing)
			{
				m_KeyboardInfo[i].bKeyDown = true;
				m_KeyboardInfo[i].bKeyPressing = true;
			}
			else
			{
				m_KeyboardInfo[i].bKeyDown = false;
			}
		}

		else if (m_KeyboardInfo[i].bKeyDown || m_KeyboardInfo[i].bKeyPressing)
		{
			m_KeyboardInfo[i].bKeyDown = false;
			m_KeyboardInfo[i].bKeyPressing = false;
			m_KeyboardInfo[i].bKeyUp = true;
		}

		else if (m_KeyboardInfo[i].bKeyUp)
			m_KeyboardInfo[i].bKeyUp = false;
	}


	result =  m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);

	for (size_t i = 0; i < 3; i++)
	{
		if (m_tMouseState.rgbButtons[i] & 0x80)
		{
			if (!m_MouseInfo[i].bKeyDown && !m_MouseInfo[i].bKeyPressing)
			{
				m_MouseInfo[i].bKeyDown = true;
				m_MouseInfo[i].bKeyPressing = true;
			}
			else
			{
				m_MouseInfo[i].bKeyDown = false;
			}
		}

		else if (m_MouseInfo[i].bKeyDown || m_MouseInfo[i].bKeyPressing)
		{
			m_MouseInfo[i].bKeyDown = false;
			m_MouseInfo[i].bKeyPressing = false;
			m_MouseInfo[i].bKeyUp = true;
		}

		else if (m_MouseInfo[i].bKeyUp)
			m_MouseInfo[i].bKeyUp = false;
	}
}

bool CInputDev::KeyUp(const unsigned char& cKey)
{
	return m_KeyboardInfo[cKey].bKeyUp;
}

bool CInputDev::KeyDown(const unsigned char& cKey)
{
	return m_KeyboardInfo[cKey].bKeyDown;
}

bool CInputDev::KeyPressing(const unsigned char& cKey)
{
	return m_KeyboardInfo[cKey].bKeyPressing;
}


bool CInputDev::LButtonUp()
{
	return m_MouseInfo[0].bKeyUp;
}

bool CInputDev::LButtonDown()
{
	return m_MouseInfo[0].bKeyDown;
}

bool CInputDev::LButtonPressing()
{
	return m_MouseInfo[0].bKeyPressing;
}

bool CInputDev::RButtonUp()
{
	return m_MouseInfo[1].bKeyUp;
}

bool CInputDev::RButtonDown()
{
	return m_MouseInfo[1].bKeyDown;
}

bool CInputDev::RButtonPressing()
{
	return m_MouseInfo[1].bKeyPressing;
}



bool CInputDev::WheelButtonUp()
{
	return m_MouseInfo[2].bKeyUp;
}

bool CInputDev::WheelButtonDown()
{
	return m_MouseInfo[2].bKeyDown;
}

bool CInputDev::WheelButtonPressing()
{
	return m_MouseInfo[2].bKeyPressing;
}

void CInputDev::Free(void)
{
	if (m_pKeyBoard) delete m_pKeyBoard;
	if (m_pMouse) delete m_pMouse;
	if (m_pInputSDK) delete m_pInputSDK;


	//Safe_Release(m_pKeyBoard);
	//Safe_Release(m_pMouse);
	//Safe_Release(m_pInputSDK);
}

