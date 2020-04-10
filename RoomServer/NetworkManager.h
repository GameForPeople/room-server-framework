#pragma once

#define MULTI_SEND_THREAD_MODE FALSE	// ��Ʈ��ũ �Ŵ������� SendThread�� ����, Send�� ó���Ұ���.

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct MemoryUnit;
struct UserUnit;

/*
	NetworkManager
		- IOCP ���� �Լ�, ��Ŀ�����带 ������ ��Ʈ��ũ ���� �Լ����� ó���ϴ� Manager�Դϴ�.
		- �̱��� ��ü�Դϴ�.
	 
	#0. Multi-producer�κ����� SendTask�� PPL Queue�� ���� �ְ�, n���� multi-thread(multi-consumer)�� ���� Send ��û�� ó���մϴ�.
*/
class NetworkManager
	: public TSingleton<NetworkManager>
{
	static constexpr int SEND_MEMORY_POOL_SIZE = 1000000;	// Init�ܰ迡��, Send Memory Pool�� �־��� �޸� ���� �����Դϴ�.
	static constexpr int ADD_SEND_MEMORY_POOL_SIZE = 500000; // ��Ÿ�� ��, Send Memory Pool���� ���� �޸𸮰� ���� ���, �߰��� Send Memory Pool�� ���� �޸� ���� �����Դϴ�.

public:
	NetworkManager();
	~NetworkManager();

	DISABLED_COPY(NetworkManager)	// ���� ������ �����մϴ�.
	DISABLED_MOVE(NetworkManager)	// �̵� ������ �����մϴ�.

	void SetRecvState(UserUnit* pUser);	// ���� ���� �������� ���Ͽ� ���Ͽ� Recv���¸� �����մϴ�.
	void PushSendMemoryUnit(SendMemoryUnit* const memoryUnit); // ����� Send MemoryUnit�� �ݳ��޾� Send Memory Pool�� ����ֽ��ϴ�.

private:
	SendMemoryUnit* PopSendMemoryUnit();	// ������ SendPacket �Լ� ��, sendMemoryPool���� �޸𸮸� ������ ����մϴ�. 

	concurrency::concurrent_queue<SendMemoryUnit*> sendMemoryPool;  // ������ �ʴ� SendMemoryUnit�� ����ִ� �޸� Ǯ�Դϴ�.

#if MULTI_SEND_THREAD_MODE == TRUE
public:
	void AddSendTask(SOCKET toSocket, std::string packetData); // �ٸ� �Ŵ����������� Send ��û�� �޾� Send Task Queue�� ����մϴ�.
private:
	static constexpr int SEND_THREAD_NUM = 3; // NetworkManager���� ���۽�ų Send Thread �����Դϴ�.
	
	void SendThread();	// ������ ���� �� ȣ���ϴ� �Լ��Դϴ�.

	std::array<std::thread, SEND_THREAD_NUM> sendThreadCont; // thread�� �����ϴ� �����̳��Դϴ�.
	concurrency::concurrent_queue<SendMemoryUnit*> sendMemoryCont;  // �ش� Manager�� �Ҵ�Ǵ� task �����̳��Դϴ�.
#else
public:
	void SendPacket(SOCKET toSocket, char* packetData);

private:
	std::thread asyncAllocateMemoryThread;
#endif
};