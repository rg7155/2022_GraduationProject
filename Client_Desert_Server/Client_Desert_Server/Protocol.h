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

#define DISCONNECT -99.f
#include "../../Client_Desert/Enum.h"

// �������� ����
#pragma pack(push, 1) // ��ü ���α׷��� ������ ��ġ�� �ʵ���

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type; 
	float x, y, z;
	char direction;
	unsigned int client_time;
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
	unsigned char size;
	char type;
	char id;
	float x, y, z;
	short race;		// ���� �Ǻ���
	int hp, hpmax;
};

struct SC_REMOVE_OBJECT_PACKET
{
	unsigned char size;
	char type;
	short race;		// ���� �Ǻ���
	short id;
};

struct SC_MOVE_OBJECT_PACKET
{
	unsigned char size;
	char type;
	short id;
	short race;		// ���� �Ǻ���
	float x, y, z;
	char direction;
	unsigned int client_time;
};

struct SC_STAT_CHANGE_PACKET {
	unsigned char size;
	char	type;
	short	race;		// ���� �Ǻ���
	int		id;
	int		hp, hpmax;
	char	anim;
};
#pragma pack (pop)
