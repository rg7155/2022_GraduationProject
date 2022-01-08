#ifndef InputDev_h__
#define InputDev_h__

#include "stdafx.h"

class CInputDev
{
	DECLARE_SINGLETON(CInputDev)

	typedef struct _tagKeyInfo
	{
		bool	bKeyDown;
		bool	bKeyPressing;
		bool	bKeyUp;

		_tagKeyInfo() :
			bKeyDown(false),
			bKeyPressing(false),
			bKeyUp(false)
		{
		}
	}KEYINFO;

private:
	CInputDev(void);
	virtual ~CInputDev(void);

public:
	char	Get_DIKeyState(char byKeyID) { return m_byKeyState[byKeyID]; }

	char	Get_DIMouseState(MOUSEKEYSTATE eMouse)
	{ return m_tMouseState.rgbButtons[eMouse]; }
	
	long	Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
	{
		return *(((long*)&m_tMouseState) + eMouseState);
	}


public:
	HRESULT Ready_InputDev(HINSTANCE hInst, HWND hWnd);
	void	Set_InputDev(void);

public:
	// 키보드
	bool KeyUp(const unsigned char& cKey);
	bool KeyDown(const unsigned char& cKey);
	bool KeyPressing(const unsigned char& cKey);

	// 마우스
	bool LButtonUp();
	bool LButtonDown();
	bool LButtonPressing();

	bool RButtonUp();
	bool RButtonDown();
	bool RButtonPressing();

	bool WheelButtonUp();
	bool WheelButtonDown();
	bool WheelButtonPressing();

private:
	LPDIRECTINPUT8			m_pInputSDK = nullptr;

private:
	LPDIRECTINPUTDEVICE8	m_pKeyBoard = nullptr;
	LPDIRECTINPUTDEVICE8	m_pMouse = nullptr;

private:
	char					m_byKeyState[256];
	DIMOUSESTATE2			m_tMouseState;

	KEYINFO					m_KeyboardInfo[256];
	KEYINFO					m_MouseInfo[3];


public:
	virtual void	Free(void);

};

#endif // InputDev_h__
