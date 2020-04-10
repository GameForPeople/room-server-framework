#pragma once

#include "../Common/global_header.hh"
#include "MemoryUnit.h"

enum class USER_STATE
{
	NO_LOGIN
};

// Framework에서 public하게 접근 가능한 데이터입니다.
class FrameworkAccessibleField
{
	friend class IOCPServerFramework;

private:
	int loadedSize = 0; // 현재까지 적재한 데이터의 사이즈입니다. <- 이거 굳이 인트? 
	char loadedBuffer[DATA_BUFFER_SIZE]{}; // 받은 데이터를 바탕으로 적재된 데이터입니다.
};

// UserManagerUnit에서 public하게 접근 가능한 데이터입니다.
class UserManagerAccessibleField
{
	friend class UserManager;

private:
	std::wstring id{};		// 각 유저의 ID입니다.
	std::wstring nickname{};	// 각 유저의 NickName입니다.
	std::wstring pw{};			// 각 유저의 Pw입니다.

	_Level level = -1;
	_Money money = 0;
	_Count winCount = 0;
	_Count loseCount = 0;

	bool isLogin = false;

public:
	_INLINE std::wstring GetID() noexcept { return id; };
	_INLINE std::wstring GetNickname() noexcept { return nickname; };
	_INLINE bool GetIsLogin() noexcept { return isLogin; };
};

// RoomManagerUnit에서 public하게 접근 가능한 데이터입니다.
struct RoomUnit;
class MatchingManagerAccessibleField
{
	friend struct RoomUnit;

	RoomUnit* pRoom = nullptr;	// 방에 접근 가능 여부를 판단하는 포인터입니다.

public:
	_INLINE RoomUnit* const GetRoom() noexcept { return pRoom; }
};

/*
	UserUnit
		- 각각의 User와 관련된 자료, 정보들을 담아놓는 객체입니다.
*/
struct UserUnit
{
	MemoryUnit memoryUnit;	// IOCP에 필수적으로 필요한 변수입니다.

	FrameworkAccessibleField frameworkAccessibleField;
	UserManagerAccessibleField userManagerAccessibleField;
	MatchingManagerAccessibleField matchingManagerAccessibleField;

	const _Key key; // 각 유저 식별 고유값(컨테이너 인덱스)입니다. 상수보장
	SOCKET socket; // 동기화에 어느정도 안전이 보장된 소켓입니다.

	// USER_STATE userState;
public:
	UserUnit(const _Key key);
};

