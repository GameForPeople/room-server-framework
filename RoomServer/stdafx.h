#pragma once

// C++
#include <iostream>
#include <numeric>

#include <string>
#include <string_view>

#include <iostream>
#include <fstream>

// C++ Multi-Thread
#include <thread>
#include <atomic>
// #include <mutex>


// C++ Container
#include <list>
#include <array>
#include <queue>
#include <map>
#include <set>
#include <unordered_set>


// MS-PPL
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>


// others
#define NDEBUG
#include <cassert> // assert

// #include <any>	// void* 대용 사용 시, 성능 체크 필요. -> 일단 사용안함. 너무 무거움


// windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "wininet.lib")
#include <WinSock2.h>

// Custom
#include "../Common/magic_enum.hpp"
#include "../Common/Spin_Lock.hpp"