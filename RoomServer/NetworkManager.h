#pragma once

#define MULTI_SEND_THREAD_MODE FALSE	// 네트워크 매니저에서 SendThread를 돌려, Send를 처리할건지.

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct MemoryUnit;
struct UserUnit;

/*
	NetworkManager
		- IOCP 관련 함수, 워커쓰레드를 제외한 네트워크 관련 함수들을 처리하는 Manager입니다.
		- 싱글턴 객체입니다.
	 
	#0. Multi-producer로부터의 SendTask를 PPL Queue에 집어 넣고, n개의 multi-thread(multi-consumer)를 통해 Send 요청을 처리합니다.
*/
class NetworkManager
	: public TSingleton<NetworkManager>
{
	static constexpr int SEND_MEMORY_POOL_SIZE = 1000000;	// Init단계에서, Send Memory Pool에 넣어줄 메모리 유닛 개수입니다.
	static constexpr int ADD_SEND_MEMORY_POOL_SIZE = 500000; // 런타임 중, Send Memory Pool에서 꺼낼 메모리가 없는 경우, 추가로 Send Memory Pool에 넣을 메모리 유닛 개수입니다.

public:
	NetworkManager();
	~NetworkManager();

	DISABLED_COPY(NetworkManager)	// 복사 생성을 제한합니다.
	DISABLED_MOVE(NetworkManager)	// 이동 생성을 제한합니다.

	void SetRecvState(UserUnit* pUser);	// 받은 유저 포인터의 소켓에 대하여 Recv상태를 적용합니다.
	void PushSendMemoryUnit(SendMemoryUnit* const memoryUnit); // 사용한 Send MemoryUnit을 반납받아 Send Memory Pool에 집어넣습니다.

private:
	SendMemoryUnit* PopSendMemoryUnit();	// 내부의 SendPacket 함수 중, sendMemoryPool에서 메모리를 꺼낼때 사용합니다. 

	concurrency::concurrent_queue<SendMemoryUnit*> sendMemoryPool;  // 사용되지 않는 SendMemoryUnit을 들고있는 메모리 풀입니다.

#if MULTI_SEND_THREAD_MODE == TRUE
public:
	void AddSendTask(SOCKET toSocket, std::string packetData); // 다른 매니저쓰레드의 Send 요청을 받아 Send Task Queue에 등록합니다.
private:
	static constexpr int SEND_THREAD_NUM = 3; // NetworkManager에서 동작시킬 Send Thread 개수입니다.
	
	void SendThread();	// 쓰레드 생성 시 호출하는 함수입니다.

	std::array<std::thread, SEND_THREAD_NUM> sendThreadCont; // thread를 관리하는 컨테이너입니다.
	concurrency::concurrent_queue<SendMemoryUnit*> sendMemoryCont;  // 해당 Manager에 할당되는 task 컨테이너입니다.
#else
public:
	void SendPacket(SOCKET toSocket, char* packetData);

private:
	std::thread asyncAllocateMemoryThread;
#endif
};