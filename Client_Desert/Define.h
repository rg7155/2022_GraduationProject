#ifndef Define_h__
#define Define_h__

#define FRAME_BUFFER_WIDTH				1280/*800/*640*/
#define FRAME_BUFFER_HEIGHT				720/*600/*480*/

#define RANDOM_COLOR			XMFLOAT4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX))

#define EPSILON					1.0e-10f


#define PLAYER_SPEED					10.f
#define BLENDING_TIME					0.3f
#define BLENDING_SPEED					3.f

#define ANIMATION_TYPE_ONCE				0
#define ANIMATION_TYPE_LOOP				1
#define ANIMATION_TYPE_PINGPONG			2

#define ANIMATION_CALLBACK_EPSILON		0.0165f

//Light.hlsl과 맞추기
//#define MAX_LIGHTS				4 
//#define MAX_DEPTH_TEXTURES		MAX_LIGHTS
#define MAX_LIGHTS						2 
#define MAX_MATERIALS					16 
#define MAX_DEPTH_TEXTURES		MAX_LIGHTS

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

#define _DEPTH_BUFFER_WIDTH		(FRAME_BUFFER_WIDTH * 8)
#define _DEPTH_BUFFER_HEIGHT	(FRAME_BUFFER_HEIGHT * 8)

#define DELTA_X					(1.0f / _DEPTH_BUFFER_WIDTH)
#define DELTA_Y					(1.0f / _DEPTH_BUFFER_HEIGHT)

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES

#define _PLANE_WIDTH			300
#define _PLANE_HEIGHT			300

// 카메라
#define CAM_OFFSET_Y			4.f
#define CAM_OFFSET_Z			-5.f
#define DPI						20.f


//RootParameter
#define RP_DEPTH_BUFFER		13
#define RP_TO_LIGHT			14
#define RP_FRAMEWORK_INFO	15

#define NO_COPY(CLASSNAME)							\
	private:										\
	CLASSNAME(const CLASSNAME&);					\
	CLASSNAME& operator = (const CLASSNAME&);		

#define DECLARE_SINGLETON(CLASSNAME)				\
	NO_COPY(CLASSNAME)								\
	private:										\
	static CLASSNAME*	m_pInstance;				\
	public:											\
	static CLASSNAME*	GetInstance( void );		\
	static void DestroyInstance( void );			

#define IMPLEMENT_SINGLETON(CLASSNAME)				\
	CLASSNAME*	CLASSNAME::m_pInstance = NULL;		\
	CLASSNAME*	CLASSNAME::GetInstance( void )	{	\
		if(NULL == m_pInstance) {					\
			m_pInstance = new CLASSNAME;			\
		}											\
		return m_pInstance;							\
	}												\
	void CLASSNAME::DestroyInstance( void ) {		\
		if(NULL != m_pInstance)	{					\
			delete m_pInstance;						\
			m_pInstance = NULL;						\
		}											\
	}

#define NULL_CHECK( _ptr)	\
{if( _ptr == 0){assert(false && "");return;}}

#define NULL_CHECK_RETURN( _ptr, _return)	\
{if( _ptr == 0){assert(false && "");return _return;}}

#define NULL_CHECK_MSG( _ptr, _message )		\
{if( _ptr == 0){MessageBox(NULL, _message, L"System Message",MB_OK);assert(false && "");}}

#define NULL_CHECK_RETURN_MSG( _ptr, _return, _message )	\
{if( _ptr == 0){MessageBox(NULL, _message, L"System Message",MB_OK);assert(false && "");return _return;}}

#define NULL_CHECK_ASSERT( _ptr)	\
{if( _ptr == 0){assert(false && "");}}

#define FAILED_CHECK(_hr)	if( ((HRESULT)(_hr)) < 0 )	\
{ MessageBoxW(NULL, L"Failed", L"System Error",MB_OK); assert(false && ""); return E_FAIL;}

#define FAILED_CHECK_RETURN(_hr, _return)	if( ((HRESULT)(_hr)) < 0 )		\
{ MessageBoxW(NULL, L"Failed", L"System Error",MB_OK); assert(false && ""); return _return;}

#define FAILED_CHECK_MSG( _hr, _message)	if( ((HRESULT)(_hr)) < 0 )	\
{ MessageBoxW(NULL, _message, L"System Message",MB_OK); assert(false && "");return E_FAIL;}

#define FAILED_CHECK_RETURN_MSG( _hr, _return, _message)	if( ((HRESULT)(_hr)) < 0 )	\
{ MessageBoxW(NULL, _message, L"System Message",MB_OK); assert(false && "");return _return;}

#define FAILED_CHECK_ASSERT(_hr)	if( ((HRESULT)(_hr)) < 0 )	\
{ assert(false && "");}


#endif // Define_h__
