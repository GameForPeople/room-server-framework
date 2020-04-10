#include "stdafx.h"

#include "MemoryUnit.h"
#include "UserUnit.h"
#include "NetworkManager.h"

#include "Utils.h"

NetworkManager::NetworkManager()
	: sendMemoryPool()
{
	// Send에 사용할 메모리를 미리 할당합니다.
	for (int i = 0; i < SEND_MEMORY_POOL_SIZE; ++i)
	{
		sendMemoryPool.push(new SendMemoryUnit);
	}

#if MULTI_SEND_THREAD_MODE == TRUE
	// Send Thread Array의 각 thread를 활성화합니다.
	for (auto& thread : sendThreadCont)
	{
		thread = static_cast<std::thread>([&]() { this->SendThread(); });
	}
#endif
}

NetworkManager::~NetworkManager()
{
	SendMemoryUnit* memoryUnit{ nullptr };
	while (sendMemoryPool.try_pop(memoryUnit))
	{
		delete memoryUnit;
	}

#if MULTI_SEND_THREAD_MODE == TRUE
	while (sendMemoryCont.try_pop(memoryUnit))
	{
		delete memoryUnit;
	}
#endif
}

#if MULTI_SEND_THREAD_MODE == TRUE
void NetworkManager::AddSendTask(SOCKET toSocket, std::string packetData)
{
	// Send Memory Pool에서 메모리 하나를 뽑아옵니다.
	auto sendMemoryUnit = PopSendMemoryUnit();
	sendMemoryUnit->toSocket = toSocket;
	
	// 오버랩 구조체를 초기화하고, 데이터를 카피합니다.
	ZeroMemory(&(sendMemoryUnit->overlapped), sizeof(sendMemoryUnit->overlapped));
	memcpy(sendMemoryUnit->dataBuffer, packetData.c_str(), packetData.length());
	sendMemoryUnit->wsaBuffer.len = static_cast<ULONG>(packetData.length());

	// Send Thread에게 Send Task를 할당합니다.
	sendMemoryCont.push(sendMemoryUnit);
}
#endif

void NetworkManager::SendPacket(SOCKET toSocket, char* packetData)
{
	// Send Memory Pool에서 메모리 하나를 뽑아옵니다.
	auto sendMemoryUnit = PopSendMemoryUnit();
	sendMemoryUnit->toSocket = toSocket;

	// 오버랩 구조체를 초기화하고, 데이터를 카피합니다.
	ZeroMemory(&(sendMemoryUnit->overlapped), sizeof(sendMemoryUnit->overlapped));
	memcpy(sendMemoryUnit->dataBuffer, packetData, static_cast<unsigned char>(static_cast<unsigned char>(packetData[0])));
	sendMemoryUnit->wsaBuffer.len = static_cast<ULONG>(static_cast<unsigned char>(packetData[0]));

	// 실제 Send를 직접 때립니다.
	if (SOCKET_ERROR == WSASend(sendMemoryUnit->toSocket, &(sendMemoryUnit->wsaBuffer), 1, NULL, 0, &(sendMemoryUnit->overlapped), NULL))
	{
		ERROR_UTIL::ErrorSend();
	}
}

SendMemoryUnit* NetworkManager::PopSendMemoryUnit()
{
	SendMemoryUnit* retMemoryUnit;

	while (!sendMemoryPool.try_pop(retMemoryUnit))
	{
		asyncAllocateMemoryThread = static_cast<std::thread>([&]()
		{
			std::cout << "Send Memory Unit를 추가로 할당합니다. 서버 네트워크 부하로 인한 송신 지연이 의심됩니다. \n";
			for (int i = 0; i < ADD_SEND_MEMORY_POOL_SIZE; ++i)
			{
				retMemoryUnit = new SendMemoryUnit();
				sendMemoryPool.push(retMemoryUnit);
			}
		});
	}
	return retMemoryUnit;
}

void NetworkManager::SetRecvState(UserUnit* pUser)
{
	DWORD flag{};
	
	ZeroMemory(&(pUser->memoryUnit.overlapped), sizeof(pUser->memoryUnit.overlapped));

	if (SOCKET_ERROR == WSARecv(pUser->socket, &(pUser->memoryUnit.wsaBuffer), 1, NULL, &flag /* NULL*/, &(pUser->memoryUnit.overlapped), NULL))
	{
		ERROR_UTIL::ErrorRecv();
	}
}

void NetworkManager::PushSendMemoryUnit(SendMemoryUnit* const memoryUnit)
{
	sendMemoryPool.push(memoryUnit);
}

#if MULTI_SEND_THREAD_MODE == TRUE
void NetworkManager::SendThread()
{
	SendMemoryUnit* sendMemoryUnit;

	while (7)
	{
		while(sendMemoryCont.try_pop(sendMemoryUnit))
		{
			if (SOCKET_ERROR == WSASend(sendMemoryUnit->toSocket, &(sendMemoryUnit->wsaBuffer), 1, NULL, 0, &(sendMemoryUnit->overlapped), NULL))
			{
				ERROR_UTIL::ErrorSend();
			}
		}
		std::this_thread::yield();
	}
}
#endif
