#pragma once

#include "../Common/global_header.hh"

/*
	MEMORY_UNIT_TYPE
		- �޸� ������ �뵵�� ���������ϴ�.

	#0. �̴� �޸� ������ �����ͷ� ���� ��, ��� ó���ؾ������� ���� �÷��׷� ���˴ϴ�.
*/
enum class MEMORY_UNIT_TYPE
{
	SEND_TO_CLIENT
	, RECV_FROM_CLIENT
	// , SEND_TO_QUERY	//DB���� �� Query ���� Ȱ�� �� ���
	// , RECV_FROM_QUERY //DB���� �� Query ���� Ȱ�� �� ���
	// , TIMER_FUNCTION // Homogeneous Worker Thread �� �ʿ���. ����� Heterogeneous Manager.
};

/*
	MemoryUnit
		- IOCP�� ���Ǵ� OVERLAPPED, WSABUF�� �� ����, �ش� �޸� ������ �뵵�� �˼� �ִ� Ÿ�� ���� ���� �ֽ��ϴ�.

	!0.  MemoryUnit ���� ����! (ServerFramework�� PreTest���� �ش� ������ �����մϴ�.)
		- MemoryUnit�� �ּҿ� ������� overlapped�� �ּҴ� �׻� �����ؾ��մϴ�. �� ù��° ��� ������ �ݵ�� overlapped���� �մϴ�.
		- ���� MemoryUnit�� (�����Լ� ��� && ���) ��, vptr�� �� �� �ֱ� ������, ���ѵ˴ϴ�.
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
		- Send�뵵�θ� ���Ǵ� MemoryUnit�Դϴ�.

	!0. Send ������, len�� �����ؾ��մϴ�. (�Լ����� �����մϴ�.)
*/
struct SendMemoryUnit
	: public MemoryUnit
{
public:
	SOCKET toSocket;	// �����Ϸ��� Ŭ���̾�Ʈ ����

public:
	SendMemoryUnit();
};


