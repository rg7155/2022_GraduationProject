#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

constexpr int SERVER_PORT = 4000;
constexpr int BUFSIZE = 512;
constexpr int NAME_SIZE = 20;
constexpr int MAX_USER = 10;

constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_OBJECT = 3;
constexpr char SC_REMOVE_OBJECT = 4;
constexpr char SC_MOVE_OBJECT = 5;
constexpr char SC_STAT_CHANGE = 6;


// dir
constexpr char DIR_UPRIGHT = 1;
constexpr char DIR_DOWNRIGHT = 2;
constexpr char DIR_DOWNLEFT = 3;
constexpr char DIR_UPLEFT = 4;
constexpr char DIR_RIGHT = 5;
constexpr char DIR_LEFT = 6;
constexpr char DIR_UP = 7;
constexpr char DIR_DOWN = 8;


#define DISCONNECT -99.f

#include "../../Client_Desert/Enum.h"

// 프로토콜 정의
#pragma pack(push, 1) // 전체 프로그램에 영향을 미치지 않도록

struct object_anim
{
	float	fPosition;
	float	fWeight;
	bool	bEnable;
};

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
};

struct CS_MOVE_PACKET
{
	unsigned char	size;
	char			type; 
	XMFLOAT4X4		xmf4x4World;
	object_anim		animInfo[10];
	int				eCurAnim;
	unsigned int	client_time;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
	int		skill;
};

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	char id;
	float x, y, z;
};

struct SC_ADD_OBJECT_PACKET
{
	unsigned char	size;
	char			type;
	char			id;
	XMFLOAT4X4		xmf4x4World;
	object_anim		animInfo[10];
	int				eCurAnim;
	short			race;		// 몬스터 판별용
	int				hp, hpmax;
};

struct SC_REMOVE_OBJECT_PACKET
{
	unsigned char size;
	char type;
	short race;		// 몬스터 판별용
	short id;
};

struct SC_MOVE_OBJECT_PACKET
{
	unsigned char	size;
	char			type;
	short			id;
	short			race;		// 몬스터 판별용
	XMFLOAT4X4		xmf4x4World;
	object_anim		animInfo[10];
	int				eCurAnim;
	unsigned int	client_time;
};

struct SC_STAT_CHANGE_PACKET {
	unsigned char size;
	char	type;
	short	race;		// 몬스터 판별용
	int		id;
	int		hp, hpmax;
	char	anim;
};
#pragma pack (pop)
