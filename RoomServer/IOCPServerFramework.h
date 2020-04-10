#pragma once

#include "../Common/global_header.hh"

struct UserUnit;

/*
	ServerFramework
		- IOCP와 관련된 일을 처리하고 TaskManager에 Task를 전달하는 ServerFramework입니다.
*/
class IOCPServerFramework
{
	static constexpr int WORKER_THREAD_NUM = 4;	// IOCP 워커 쓰레드 개수입니다. I/O용으로 사용됩니다.

public:
	static void PreTest();	// static_assert, assert 등으로 이 코드에서 구현상 꼭 지켜져야 할 부분을 체크하는 함수입니다.
	
	IOCPServerFramework(const unsigned short portNumber);
	~IOCPServerFramework();
	
	DISABLED_COPY(IOCPServerFramework)
	DISABLED_MOVE(IOCPServerFramework)

public:
	void Run();	// ServerFramework가 실제로 시작하는 함수입니다. 내부적으로 accept를 호출합니다.

	void AcceptThreadFunction(); // accept를 호출하는 함수입니다.
	void WorkerThreadFunction(); // worker 쓰레드가 사용할 함수입니다.

private:
	void InitManagers();
	void InitNetwork(const unsigned short portNumber);

	void AssembleTaskFromRecvData(UserUnit* pUserUnit, int recvSize); // 받은 데이터들을 통해 Task를 1차적으로 조립하는 함수입니다.

private:
	SOCKET listenSocket;	// lisetn 소켓입니다.
	HANDLE hIOCP;	// IOCP 핸들값입니다.	

	std::thread acceptThread; // accept를 담당하는 쓰레드 객체입니다.

	std::vector<std::thread> workerThreadCont;	// worker 쓰레드를 관리하는 컨테이너입니다.
	/*volatile*/ bool workerThreadLoopFlag;	// worker 쓰레드 종료 조건입니다.
};
