#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <array>
#include <unordered_map>
#include <DirectXMath.h>

#pragma comment(lib, "WS2_32.lib")

using namespace std;
using namespace DirectX;

void error_display(const char* msg, int err_no);


