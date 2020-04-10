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
	// IOCP GQCS 오버랩 적법한 형변환을 위해, 해당 조건을 테스트합니다.
	// 메모리 유닛이 필드 최상단에 존재해야합니다.

	UserUnit tempUserUnit(0);
	assert((unsigned long)(&tempUserUnit) == (unsigned long)(&(tempUserUnit.memoryUnit.overlapped)) && "UserUnit 구조체의 필드에서, MemoryUnit(Overlap)이 최상단에 생성되지 않아 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다.");

	SendMemoryUnit tempSendUnit;
	assert((unsigned long)(&tempSendUnit) == (unsigned long)(&(tempSendUnit.overlapped)) && "SendMemoryUnit 구조체의 필드에서, MemoryUnit(Overlap)이 최상단에 생성되지 않아 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다.");
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
	// 이 코드 문제 심각함.
	workerThreadLoopFlag = false;
	{
		using namespace std::chrono_literals;
		for (int i = 10; i != 0; --i)
		{
			std::cout << i << " 초 후, 서버가 종료됩니다.\n";
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
#pragma region [ 윈속 초기화 ]
	if (WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		ERROR_UTIL::Error("WSAStartup()");
	}
#pragma endregion

#pragma region [ 입출력 완료 포트 생성 ]
	if (hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)
		; hIOCP == NULL)
	{
		ERROR_UTIL::Error("Create_IOCompletionPort()");
	}
#pragma endregion

#pragma region [ 워커 쓰레드 생성 ]
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

	std::cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << std::endl;
	std::cout << "  Server	               " << std::endl;
	std::cout << "                                     " << VERSION << std::endl;
	std::cout << "                                                        " << std::endl;
	std::cout << "  IP ADDRESS  : " << /*serverAddr.sin_addr.s_addr*/ ip << "                         " << std::endl;
	std::cout << "  PORT NUMBER : " << portNumber << "                                 " << std::endl;
	std::cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << std::endl << std::endl;

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

	// 공간 무시.1초 미만.
	std::cout << "[MAIN] Accept On!" << std::endl;
	std::cout << "[MAIN] Server Run!" << std::endl;
	// Join 호출의 적절한 위치 확인 필요.

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

		// Accept 시, 유저 매니저에게 accept했다고 알려줍니다.
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

		// 비 정상 처리 여부를 확인합니다. 오류 시 client 종료 처리를 시행합니다.
		if (retVal == 0 || cbTransferred == 0)
		{
			PrintLog(SOURCE_LOCATION, "retVal이 0이거나, cbTransferred가 0입니다.");

			// 유저 매니저에게 로그아웃 처리를 요청합니다.
			UserManager::GetInstance().ProduceTask(USER_MANAGER::TASK_TYPE::USER_LOGOUT,
				new USER_MANAGER::LogoutTaskUnit
				{
					static_cast<_Key>(key & (std::numeric_limits<_Key>::max)())
					, false
				});
			continue;
		}

		// 해당 GQCS에서 받은 메모리를 바탕으로, 처리해야할 일을 확인합니다.
		switch (reinterpret_cast<MemoryUnit*>(pMemoryUnit)->memoryUnitType)
		{
		case MEMORY_UNIT_TYPE::SEND_TO_CLIENT:
			// PrintLog(SOURCE_LOCATION, "Send 메모리 유닛 회수");
			NetworkManager::GetInstance().PushSendMemoryUnit(reinterpret_cast<SendMemoryUnit*>(pMemoryUnit));
			break;
		case MEMORY_UNIT_TYPE::RECV_FROM_CLIENT:
			// PrintLog(SOURCE_LOCATION, "Recv 요청 받음");
			AssembleTaskFromRecvData(reinterpret_cast<UserUnit*>(pMemoryUnit), cbTransferred);
			NetworkManager::GetInstance().SetRecvState(reinterpret_cast<UserUnit*>(pMemoryUnit));
			break;
		default:
			PrintLog(SOURCE_LOCATION, "잘못된 메모리 유닛 타입 처리 요구됨 : " 
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
		// 기존에 적재한 메모리가 없다면, 받은 데이터 0번지의 정보를 요청한다.
		if (packetSize == 0) packetSize = static_cast<int>(pBuf[0]);

		// 처리해야하는 패킷 사이즈 중에서, 이전에 이미 처리한 패킷 사이즈를 빼준다.
		int required = (packetSize)-(taskManagerUnit.loadedSize);

		// 패킷을 완성할 수 있을 때 (요청해야할 사이즈보다, 남은 사이즈가 크거나 같을 때)
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
		// 패킷을 완성할 수 없을 때
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
		// 한번에 여러 바이트를 땡겨오더라도, 1바이트 씩 처리합니다.
		--recvSize;
		//std::cout << "받은 문자는 : " << pBuf[0] << std::endl;

		if (pBuf[0] == DEFINED_ASCII::CARIAGE_RETURN || pBuf[0] == DEFINED_ASCII::BACK_SPACE)
		{
			// 텔넷에서 전송되지만, 처리할 필요 없는 문자입니다.
		}
		else if (pBuf[0] == DEFINED_ASCII::LINE_FEED)
		{
			// 엔터키를 입력받았습니다.

			if (taskManagerUnit.loadedSize != 0)
			{
				if (taskManagerUnit.loadedSize - 1 >= DATA_BUFFER_SIZE) 
				{ 
					PrintLog(SOURCE_LOCATION, "Recv버퍼의 사이즈보다, 더 큰 데이터를 한번에 처리하려합니다."); 
					taskManagerUnit.loadedSize = 0; 
					break;
				}

				// 스트링 분할을 위해 임이적으로 값을 넣어줍니다.
				taskManagerUnit.loadedBuffer[taskManagerUnit.loadedSize] = DEFINED_ASCII::SPACE;
				taskManagerUnit.loadedBuffer[taskManagerUnit.loadedSize + 1] = DEFINED_ASCII::ZERO;
				
				//----------------------------------------------------
				TaskManager::GetInstance().AssembleTask(pUserUnit, taskManagerUnit.loadedBuffer);
				//----------------------------------------------------
				
				// 로딩한 데이터를 0으로 변경해줍니다.
				//pUserUnit->taskUnit = nullptr;
				taskManagerUnit.loadedSize = 0;
			}
		}
		else
		{
			if (taskManagerUnit.loadedSize - 1 >= DATA_BUFFER_SIZE)
			{
				PrintLog(SOURCE_LOCATION, "Recv버퍼의 사이즈보다, 더 큰 데이터를 한번에 처리하려합니다.");
				taskManagerUnit.loadedSize = 0;
				break;
			}

			// 글자를 복사합니다.
			taskManagerUnit.loadedBuffer[taskManagerUnit.loadedSize] = pBuf[0];
			++(taskManagerUnit.loadedSize);
		}

		pBuf = pBuf + 1;
	}
}
*/
