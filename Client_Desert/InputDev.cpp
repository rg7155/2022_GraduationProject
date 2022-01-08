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

	// DInput �İ�ü�� �����ϴ� �Լ�
	FAILED_CHECK_RETURN(DirectInput8Create(hInst,
											DIRECTINPUT_VERSION,
											IID_IDirectInput8,
											(void**)&m_pInputSDK,
											NULL), E_FAIL);

	// Ű���� ��ü ����
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr), E_FAIL);

	// ������ Ű���� ��ü�� ���� ������ �� ��ü���� �����ϴ� �Լ�
	m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard);

	// ��ġ�� ���� �������� �������ִ� �Լ�, (Ŭ���̾�Ʈ�� ���ִ� ���¿��� Ű �Է��� ������ ������ �����ϴ� �Լ�)
	//m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// ��ġ�� ���� access ������ �޾ƿ��� �Լ�
	m_pKeyBoard->Acquire();


	// ���콺 ��ü ����
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr), E_FAIL);

	// ������ ���콺 ��ü�� ���� ������ �� ��ü���� �����ϴ� �Լ�
	m_pMouse->SetDataFormat(&c_dfDIMouse2);

	// ��ġ�� ���� �������� �������ִ� �Լ�, Ŭ���̾�Ʈ�� ���ִ� ���¿��� Ű �Է��� ������ ������ �����ϴ� �Լ�
	//m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// ��ġ�� ���� access ������ �޾ƿ��� �Լ�
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

