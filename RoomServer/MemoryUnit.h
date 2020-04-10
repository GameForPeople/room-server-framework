#pragma once

#include "../Common/global_header.hh"

/*
	MEMORY_UNIT_TYPE
		- 메모리 유닛의 용도를 결정짓습니다.

	#0. 이는 메모리 유닛을 포인터로 받은 후, 어떻게 처리해야할지에 대한 플래그로 사용됩니다.
*/
enum class MEMORY_UNIT_TYPE
{
	SEND_TO_CLIENT
	, RECV_FROM_CLIENT
	// , SEND_TO_QUERY	//DB적용 및 Query 서버 활용 시 사용
	// , RECV_FROM_QUERY //DB적용 및 Query 서버 활용 시 사용
	// , TIMER_FUNCTION // Homogeneous Worker Thread 시 필요함. 현재는 Heterogeneous Manager.
};

/*
	MemoryUnit
		- IOCP에 사용되는 OVERLAPPED, WSABUF와 그 버퍼, 해당 메모리 유닛의 용도를 알수 있는 타입 등을 갖고 있습니다.

	!0.  MemoryUnit 제한 사항! (ServerFramework의 PreTest에서 해당 조건을 보장합니다.)
		- MemoryUnit의 주소와 멤버변수 overlapped의 주소는 항상 동일해야합니다. 즉 첫번째 멤버 변수는 반드시 overlapped여야 합니다.
		- 따라서 MemoryUnit은 (가상함수 사용 && 상속) 시, vptr이 낄 수 있기 때문에, 제한됩니다.
*/
struct MemoryUnit
{
public:
	OVERLAPPED overlapped;
	WSABUF wsaBuffer;

	const MEMORY_UNIT_TYPE memoryUnitType;
	char dataBuffer[DATA_BUFFER_SIZE];

public:
	MemoryUnit(const MEMORY_UNIT_TYPE);
	~MemoryUnit();
};

/*
	SendMemoryUnit
		- Send용도로만 사용되는 MemoryUnit입니다.

	!0. Send 때마다, len을 수정해야합니다. (함수에서 제한합니다.)
*/
struct SendMemoryUnit
	: public MemoryUnit
{
public:
	SOCKET toSocket;	// 전송하려는 클라이언트 소켓

public:
	SendMemoryUnit();
};


