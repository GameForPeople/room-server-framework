#pragma once

#include "../Common/global_header.hh"

struct UserUnit;

/*
	ServerFramework
		- IOCP�� ���õ� ���� ó���ϰ� TaskManager�� Task�� �����ϴ� ServerFramework�Դϴ�.
*/
class IOCPServerFramework
{
	static constexpr int WORKER_THREAD_NUM = 4;	// IOCP ��Ŀ ������ �����Դϴ�. I/O������ ���˴ϴ�.

public:
	static void PreTest();	// static_assert, assert ������ �� �ڵ忡�� ������ �� �������� �� �κ��� üũ�ϴ� �Լ��Դϴ�.
	
	IOCPServerFramework(const unsigned short portNumber);
	~IOCPServerFramework();
	
	DISABLED_COPY(IOCPServerFramework)
	DISABLED_MOVE(IOCPServerFramework)

public:
	void Run();	// ServerFramework�� ������ �����ϴ� �Լ��Դϴ�. ���������� accept�� ȣ���մϴ�.

	void AcceptThreadFunction(); // accept�� ȣ���ϴ� �Լ��Դϴ�.
	void WorkerThreadFunction(); // worker �����尡 ����� �Լ��Դϴ�.

private:
	void InitManagers();
	void InitNetwork(const unsigned short portNumber);

	void AssembleTaskFromRecvData(UserUnit* pUserUnit, int recvSize); // ���� �����͵��� ���� Task�� 1�������� �����ϴ� �Լ��Դϴ�.

private:
	SOCKET listenSocket;	// lisetn �����Դϴ�.
	HANDLE hIOCP;	// IOCP �ڵ鰪�Դϴ�.	

	std::thread acceptThread; // accept�� ����ϴ� ������ ��ü�Դϴ�.

	std::vector<std::thread> workerThreadCont;	// worker �����带 �����ϴ� �����̳��Դϴ�.
	/*volatile*/ bool workerThreadLoopFlag;	// worker ������ ���� �����Դϴ�.
};
