#include "stdafx.h"
#include "Utils.h"

#include "UserUnit.h" 

#include "IOCPServerFramework.h"

#include "MemoryUnit.h"

#include "NetworkManager.h"
#include "TimerManager.h"
#include "TaskManager.h"
#include "UserManager.h"
#include "RoomManager.h"

void IOCPServerFramework::PreTest()
{
	// IOCP GQCS ������ ������ ����ȯ�� ����, �ش� ������ �׽�Ʈ�մϴ�.
	// �޸� ������ �ʵ� �ֻ�ܿ� �����ؾ��մϴ�.

	UserUnit tempUserUnit(0);
	assert((unsigned long)(&tempUserUnit) == (unsigned long)(&(tempUserUnit.memoryUnit.overlapped)) && "UserUnit ����ü�� �ʵ忡��, MemoryUnit(Overlap)�� �ֻ�ܿ� �������� �ʾ� ���������� ����� ������ �� �ֽ��ϴ�. ���� ������ �����Ͽ����ϴ�.");

	SendMemoryUnit tempSendUnit;
	assert((unsigned long)(&tempSendUnit) == (unsigned long)(&(tempSendUnit.overlapped)) && "SendMemoryUnit ����ü�� �ʵ忡��, MemoryUnit(Overlap)�� �ֻ�ܿ� �������� �ʾ� ���������� ����� ������ �� �ֽ��ϴ�. ���� ������ �����Ͽ����ϴ�.");
}

IOCPServerFramework::IOCPServerFramework(const unsigned short portNumber)
	: listenSocket()
	, hIOCP(nullptr)

	, acceptThread()

	, workerThreadCont()
	, workerThreadLoopFlag(true)
{
	InitManagers();
	InitNetwork(portNumber);
	
	setlocale(LC_ALL, "korean");
	UserManager::GetInstance().SetIOCPHandle(hIOCP);
}

IOCPServerFramework::~IOCPServerFramework()
{
	// �� �ڵ� ���� �ɰ���.
	workerThreadLoopFlag = false;
	{
		using namespace std::chrono_literals;
		for (int i = 10; i != 0; --i)
		{
			std::cout << i << " �� ��, ������ ����˴ϴ�.\n";
			std::this_thread::sleep_for(1s);
			// system("cls");
		}
	}

	closesocket(listenSocket);
	WSACleanup();
}

void IOCPServerFramework::InitManagers()
{
	UserManager::GetInstance();
	TimerManager::GetInstance();
	NetworkManager::GetInstance();
	TaskManager::GetInstance();
}

void IOCPServerFramework::InitNetwork(const unsigned short portNumber)
{
#pragma region [ Init Network ]
#pragma region [ ���� �ʱ�ȭ ]
	if (WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		ERROR_UTIL::Error("WSAStartup()");
	}
#pragma endregion

#pragma region [ ����� �Ϸ� ��Ʈ ���� ]
	if (hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)
		; hIOCP == NULL)
	{
		ERROR_UTIL::Error("Create_IOCompletionPort()");
	}
#pragma endregion

#pragma region [ ��Ŀ ������ ���� ]
	workerThreadCont.reserve(WORKER_THREAD_NUM);
	for (int i = 0; i < /* (int)si.dwNumberOfProcessors * 2 */ WORKER_THREAD_NUM; ++i)
	{
		workerThreadCont.emplace_back([&]()
		{
			this->WorkerThreadFunction();
		});
	}
#pragma endregion

#pragma region [ socket() ]
	if (listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)
		; listenSocket == INVALID_SOCKET)
	{
		ERROR_UTIL::Error("socket()");
	}
#pragma endregion

#pragma region [ Bind() ]
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(portNumber); // htons(SERVER_LISTEN_PORT_NUMBER);
	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		ERROR_UTIL::Error("bind()");
	}
#pragma endregion

#pragma region [Print ServerUI]
	PHOSTENT host;
	char name[255];
	char* ip{};

	if (gethostname(name, sizeof(name)) != 0)
	{
		ERROR_UTIL::Error("gethostname()");
	}
	if ((host = gethostbyname(name)) != NULL)
	{
		ip = inet_ntoa(*(struct in_addr*) * host->h_addr_list);
	}

	std::cout << "�������������������������������" << std::endl;
	std::cout << "  Server	               " << std::endl;
	std::cout << "                                     " << VERSION << std::endl;
	std::cout << "                                                        " << std::endl;
	std::cout << "  IP ADDRESS  : " << /*serverAddr.sin_addr.s_addr*/ ip << "                         " << std::endl;
	std::cout << "  PORT NUMBER : " << portNumber << "                                 " << std::endl;
	std::cout << "�������������������������������" << std::endl << std::endl;

	std::cout << "[MAIN] Init Network!" << std::endl;
#pragma endregion
#pragma endregion
}

void IOCPServerFramework::Run()
{
#pragma region [ Listen() ]
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		ERROR_UTIL::Error("listen()");
	}
	std::cout << "[MAIN] Listen On!" << std::endl;
#pragma endregion

	acceptThread = static_cast<std::thread>([&]() {this->AcceptThreadFunction(); });

	// ���� ����.1�� �̸�.
	std::cout << "[MAIN] Accept On!" << std::endl;
	std::cout << "[MAIN] Server Run!" << std::endl;
	// Join ȣ���� ������ ��ġ Ȯ�� �ʿ�.

	acceptThread.join();
	for (auto& thread : workerThreadCont) thread.join();
}

void IOCPServerFramework::AcceptThreadFunction()
{
	SOCKET acceptedSocket{};
	SOCKADDR_IN clientAddr{};
	int addrLen{};

	while (7)
	{
		//accept
		addrLen = sizeof(clientAddr);
		if (acceptedSocket = WSAAccept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen, NULL, NULL);
			acceptedSocket == INVALID_SOCKET) 
		{
			ERROR_UTIL::Error("accept()");
		}

		// Accept ��, ���� �Ŵ������� accept�ߴٰ� �˷��ݴϴ�.
		UserManager::GetInstance().ProduceTask(USER_MANAGER::TASK_TYPE::USER_ACCEPT, 
			new USER_MANAGER::AcceptTaskUnit
			{
				acceptedSocket
				, clientAddr
			}
		);
	}
}

void IOCPServerFramework::WorkerThreadFunction()
{
	int retVal{};
	DWORD cbTransferred{};
	unsigned long long key{};
	MemoryUnit* pMemoryUnit{nullptr};

	while (workerThreadLoopFlag)
	{
		retVal = GetQueuedCompletionStatus
		(
			hIOCP
			, &cbTransferred
			, &key // &pMemoryUnit
			, reinterpret_cast<LPOVERLAPPED*>(&pMemoryUnit)
			, INFINITE
		);

		// PrintLog(SOURCE_LOCATION, "GetQueuedCompletionStatus - KEY : " + std::to_string(key) + "\n");

		// �� ���� ó�� ���θ� Ȯ���մϴ�. ���� �� client ���� ó���� �����մϴ�.
		if (retVal == 0 || cbTransferred == 0)
		{
			PrintLog(SOURCE_LOCATION, "retVal�� 0�̰ų�, cbTransferred�� 0�Դϴ�.");

			// ���� �Ŵ������� �α׾ƿ� ó���� ��û�մϴ�.
			UserManager::GetInstance().ProduceTask(USER_MANAGER::TASK_TYPE::USER_LOGOUT,
				new USER_MANAGER::LogoutTaskUnit
				{
					static_cast<_Key>(key & (std::numeric_limits<_Key>::max)())
					, false
				});
			continue;
		}

		// �ش� GQCS���� ���� �޸𸮸� ��������, ó���ؾ��� ���� Ȯ���մϴ�.
		switch (reinterpret_cast<MemoryUnit*>(pMemoryUnit)->memoryUnitType)
		{
		case MEMORY_UNIT_TYPE::SEND_TO_CLIENT:
			// PrintLog(SOURCE_LOCATION, "Send �޸� ���� ȸ��");
			NetworkManager::GetInstance().PushSendMemoryUnit(reinterpret_cast<SendMemoryUnit*>(pMemoryUnit));
			break;
		case MEMORY_UNIT_TYPE::RECV_FROM_CLIENT:
			// PrintLog(SOURCE_LOCATION, "Recv ��û ����");
			AssembleTaskFromRecvData(reinterpret_cast<UserUnit*>(pMemoryUnit), cbTransferred);
			NetworkManager::GetInstance().SetRecvState(reinterpret_cast<UserUnit*>(pMemoryUnit));
			break;
		default:
			PrintLog(SOURCE_LOCATION, "�߸��� �޸� ���� Ÿ�� ó�� �䱸�� : " 
				+ static_cast<int>(reinterpret_cast<MemoryUnit*>(pMemoryUnit)->memoryUnitType));
			break;
		}
	}
}

void IOCPServerFramework::AssembleTaskFromRecvData(UserUnit* pUserUnit, int recvSize)
{
	char* pBuf = pUserUnit->memoryUnit.dataBuffer;
	TaskManagerUnit& taskManagerUnit = pUserUnit->taskManagerUnit;

	int packetSize{ 0 };
	
	if (taskManagerUnit.loadedSize > 0) { packetSize = pBuf[0]; }

	while (recvSize > 0)
	{
		// ������ ������ �޸𸮰� ���ٸ�, ���� ������ 0������ ������ ��û�Ѵ�.
		if (packetSize == 0) packetSize = static_cast<int>(pBuf[0]);

		// ó���ؾ��ϴ� ��Ŷ ������ �߿���, ������ �̹� ó���� ��Ŷ ����� ���ش�.
		int required = (packetSize)-(taskManagerUnit.loadedSize);

		// ��Ŷ�� �ϼ��� �� ���� �� (��û�ؾ��� �������, ���� ����� ũ�ų� ���� ��)
		if (recvSize >= required)
		{
			memcpy(taskManagerUnit.loadedBuffer + taskManagerUnit.loadedSize, pBuf, required);
			
			//-------------------------------------------------------------------------------
			TaskManager::GetInstance().ProduceTask(pUserUnit);
			//TaskManager::GetInstance().ProduceTask(TaskManager::TASK_PROCESS_CASE::MAIN_ZONE, pUserUnit->taskUnit);
			//-------------------------------------------------------------------------------

			taskManagerUnit.loadedSize = 0;
			recvSize -= required;
			pBuf += required;
			packetSize = 0;
		}
		// ��Ŷ�� �ϼ��� �� ���� ��
		else
		{
			memcpy(taskManagerUnit.loadedBuffer + taskManagerUnit.loadedSize, pBuf, recvSize);
			taskManagerUnit.loadedSize += recvSize;
			break;
		}
	}
}

/*
void ServerFramework::LEGACY_AssembleTaskFromRecvData(UserUnit* pUserUnit, int recvSize)
{
	auto& taskManagerUnit = pUserUnit->taskManagerUnit;
	char* pBuf = pUserUnit->memoryUnit.dataBuffer;

	while (recvSize > 0)
	{
		// �ѹ��� ���� ����Ʈ�� ���ܿ�����, 1����Ʈ �� ó���մϴ�.
		--recvSize;
		//std::cout << "���� ���ڴ� : " << pBuf[0] << std::endl;

		if (pBuf[0] == DEFINED_ASCII::CARIAGE_RETURN || pBuf[0] == DEFINED_ASCII::BACK_SPACE)
		{
			// �ڳݿ��� ���۵�����, ó���� �ʿ� ���� �����Դϴ�.
		}
		else if (pBuf[0] == DEFINED_ASCII::LINE_FEED)
		{
			// ����Ű�� �Է¹޾ҽ��ϴ�.

			if (taskManagerUnit.loadedSize != 0)
			{
				if (taskManagerUnit.loadedSize - 1 >= DATA_BUFFER_SIZE) 
				{ 
					PrintLog(SOURCE_LOCATION, "Recv������ �������, �� ū �����͸� �ѹ��� ó���Ϸ��մϴ�."); 
					taskManagerUnit.loadedSize = 0; 
					break;
				}

				// ��Ʈ�� ������ ���� ���������� ���� �־��ݴϴ�.
				taskManagerUnit.loadedBuffer[taskManagerUnit.loadedSize] = DEFINED_ASCII::SPACE;
				taskManagerUnit.loadedBuffer[taskManagerUnit.loadedSize + 1] = DEFINED_ASCII::ZERO;
				
				//----------------------------------------------------
				TaskManager::GetInstance().AssembleTask(pUserUnit, taskManagerUnit.loadedBuffer);
				//----------------------------------------------------
				
				// �ε��� �����͸� 0���� �������ݴϴ�.
				//pUserUnit->taskUnit = nullptr;
				taskManagerUnit.loadedSize = 0;
			}
		}
		else
		{
			if (taskManagerUnit.loadedSize - 1 >= DATA_BUFFER_SIZE)
			{
				PrintLog(SOURCE_LOCATION, "Recv������ �������, �� ū �����͸� �ѹ��� ó���Ϸ��մϴ�.");
				taskManagerUnit.loadedSize = 0;
				break;
			}

			// ���ڸ� �����մϴ�.
			taskManagerUnit.loadedBuffer[taskManagerUnit.loadedSize] = pBuf[0];
			++(taskManagerUnit.loadedSize);
		}

		pBuf = pBuf + 1;
	}
}
*/
