#pragma once

#include "../Common/global_header.hh"
#include "MemoryUnit.h"

enum class USER_STATE
{
	NO_LOGIN
};

// Framework���� public�ϰ� ���� ������ �������Դϴ�.
class FrameworkAccessibleField
{
	friend class IOCPServerFramework;

private:
	int loadedSize = 0; // ������� ������ �������� �������Դϴ�. <- �̰� ���� ��Ʈ? 
	char loadedBuffer[DATA_BUFFER_SIZE]{}; // ���� �����͸� �������� ����� �������Դϴ�.
};

// UserManagerUnit���� public�ϰ� ���� ������ �������Դϴ�.
class UserManagerAccessibleField
{
	friend class UserManager;

private:
	std::wstring id{};		// �� ������ ID�Դϴ�.
	std::wstring nickname{};	// �� ������ NickName�Դϴ�.
	std::wstring pw{};			// �� ������ Pw�Դϴ�.

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

// RoomManagerUnit���� public�ϰ� ���� ������ �������Դϴ�.
struct RoomUnit;
class MatchingManagerAccessibleField
{
	friend struct RoomUnit;

	RoomUnit* pRoom = nullptr;	// �濡 ���� ���� ���θ� �Ǵ��ϴ� �������Դϴ�.

public:
	_INLINE RoomUnit* const GetRoom() noexcept { return pRoom; }
};

/*
	UserUnit
		- ������ User�� ���õ� �ڷ�, �������� ��Ƴ��� ��ü�Դϴ�.
*/
struct UserUnit
{
	MemoryUnit memoryUnit;	// IOCP�� �ʼ������� �ʿ��� �����Դϴ�.

	FrameworkAccessibleField frameworkAccessibleField;
	UserManagerAccessibleField userManagerAccessibleField;
	MatchingManagerAccessibleField matchingManagerAccessibleField;

	const _Key key; // �� ���� �ĺ� ������(�����̳� �ε���)�Դϴ�. �������
	SOCKET socket; // ����ȭ�� ������� ������ ����� �����Դϴ�.

	// USER_STATE userState;
public:
	UserUnit(const _Key key);
};

