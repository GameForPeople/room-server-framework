#include "stdafx.h"

#include "MemoryUnit.h"
#include "UserUnit.h"
#include "NetworkManager.h"

#include "Utils.h"

NetworkManager::NetworkManager()
	: sendMemoryPool()
{
	// Send�� ����� �޸𸮸� �̸� �Ҵ��մϴ�.
	for (int i = 0; i < SEND_MEMORY_POOL_SIZE; ++i)
	{
		sendMemoryPool.push(new SendMemoryUnit);
	}

#if MULTI_SEND_THREAD_MODE == TRUE
	// Send Thread Array�� �� thread�� Ȱ��ȭ�մϴ�.
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
	// Send Memory Pool���� �޸� �ϳ��� �̾ƿɴϴ�.
	auto sendMemoryUnit = PopSendMemoryUnit();
	sendMemoryUnit->toSocket = toSocket;
	
	// ������ ����ü�� �ʱ�ȭ�ϰ�, �����͸� ī���մϴ�.
	ZeroMemory(&(sendMemoryUnit->overlapped), sizeof(sendMemoryUnit->overlapped));
	memcpy(sendMemoryUnit->dataBuffer, packetData.c_str(), packetData.length());
	sendMemoryUnit->wsaBuffer.len = static_cast<ULONG>(packetData.length());

	// Send Thread���� Send Task�� �Ҵ��մϴ�.
	sendMemoryCont.push(sendMemoryUnit);
}
#endif

void NetworkManager::SendPacket(SOCKET toSocket, char* packetData)
{
	// Send Memory Pool���� �޸� �ϳ��� �̾ƿɴϴ�.
	auto sendMemoryUnit = PopSendMemoryUnit();
	sendMemoryUnit->toSocket = toSocket;

	// ������ ����ü�� �ʱ�ȭ�ϰ�, �����͸� ī���մϴ�.
	ZeroMemory(&(sendMemoryUnit->overlapped), sizeof(sendMemoryUnit->overlapped));
	memcpy(sendMemoryUnit->dataBuffer, packetData, static_cast<unsigned char>(static_cast<unsigned char>(packetData[0])));
	sendMemoryUnit->wsaBuffer.len = static_cast<ULONG>(static_cast<unsigned char>(packetData[0]));

	// ���� Send�� ���� �����ϴ�.
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
			std::cout << "Send Memory Unit�� �߰��� �Ҵ��մϴ�. ���� ��Ʈ��ũ ���Ϸ� ���� �۽� ������ �ǽɵ˴ϴ�. \n";
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
