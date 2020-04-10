#include "stdafx.h"

#include "UserUnit.h"

//--
#include "UserManager.h"
//--

#include "Utils.h"
#include "TaskManager.h"
#include "NetworkManager.h" 
#include "TimerManager.h"
#include "RoomManager.h"

UserManager::UserManager() 
	: userThread()
	, taskCont()

	, userCont()
	, idCont()
	, lobbyUserCont()
	
	// , threadCont()

	, keyPool()
	, waitingUserPool()

	, hIOCP()

	, nicknameCont()
{
	// MAX_USER��ŭ �̸� �Ҵ� �޽��ϴ�.
	for (int i = 0; i < MAX_USER; ++i) { userCont[i] = new UserUnit(i); }
	for (int i = 0; i < MAX_USER; ++i) { keyPool.push(i); }

	// ���� �����带 �����ŵ�ϴ�.
	userThread = static_cast<std::thread>([&]() {this->UserThread(); });
}

UserManager::~UserManager()
{
	waitingUserPool.clear();

	for (auto& user : userCont)
	{
		delete user;
	}

	for (auto& socket : waitingUserPool)
	{
		closesocket(socket);
	}

	{
		_UserManagerTask task;
		while (taskCont.try_pop(task))
		{
			delete task.second;
		}
	}
}

_INLINE void UserManager::ProduceTask(const USER_MANAGER::TASK_TYPE taskType, void* const taskUnit)
{
	// TaskCont�� �ϰ� ���.
	taskCont.push({ taskType , taskUnit });
}

void UserManager::UserThread()
{
	_UserManagerTask task{};
	while (7)
	{
		using namespace USER_MANAGER;
		
		// �׽�ũ�� �ִ��� Ȯ���մϴ�.
		if (!taskCont.try_pop(task))
		{
			// using namespace std::literals;
			//std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_LOOP_TIME));

			// ������� ���, ������ �켱�������� �ڷ� �̷�ϴ�.
			std::this_thread::yield();
			continue;
		}

		// ��Ϲ��� �׽�ũ�� ó���մϴ�.
		switch (task.first)
		{
		case TASK_TYPE::USER_ACCEPT:
			AcceptProcess(static_cast<USER_MANAGER::AcceptTaskUnit*>(task.second));
			break;
		case TASK_TYPE::USER_SIGNUP:
			SignUpProcess(static_cast<USER_MANAGER::SignUpTaskUnit*>(task.second));
			break;
		case TASK_TYPE::USER_LOGIN:
			LoginProcess(static_cast<USER_MANAGER::LoginTaskUnit*>(task.second));
			break;
		case TASK_TYPE::USER_LOGOUT:
			LogoutProcess(static_cast<USER_MANAGER::LogoutTaskUnit*>(task.second));
			break;
		//case TASK_TYPE::USER_WHISPER:
		//	WhisperProcess(static_cast<USER_MANAGER::WhisperTaskUnit*>(task.second));
		//	break;
		//case TASK_TYPE::LOBBY_INFO:
		//	LobbyInfoProcess(reinterpret_cast<_Key>(task.second));
		//	break;
		case TASK_TYPE::LOBBY_CHAT:
			LobbyChatProcess(static_cast<USER_MANAGER::ChatLobbyTaskUnit*>(task.second));
			break;
		case TASK_TYPE::CHANGE_CHARACTER:
			ChangeCharacterProcess(static_cast<USER_MANAGER::ChangeCharacterTaskUnit*>(task.second));
			break;
		case TASK_TYPE::LOBBY_USER_INFO:
			JoinLobbyUserInfo(reinterpret_cast<_Key>(task.second));
			break;
		case TASK_TYPE::PUSH_OLD_KEY:
			PushOldKeyProcess(reinterpret_cast<_Key>(task.second));
			break;
		case TASK_TYPE::RANDOM_LOGIN:
			RandomLoginProcess(reinterpret_cast<_Key>(task.second));
			break;
		//case TASK_TYPE::TIMER_TO_USER_PUSH_OLD_KEY:
		//	PushOldKeyProcess(reinterpret_cast<_Key>(task.second));
		//	break;
		//case TASK_TYPE::ROOM_TO_USER_JOIN_LOBBY:
		//	JoinLobbyUser(reinterpret_cast<_Key>(task.second));
		//	break;
		//case TASK_TYPE::ROOM_TO_USER_OUT_LOBBY:
		//	OutLobbyUser(reinterpret_cast<_Key>(task.second));
		//	break;
		//case TASK_TYPE::INFO_USERS:
		//	InfoUserProcess(reinterpret_cast<_Key>(task.second));
		//	break;
		default:
			break;
		}
	}
}

void UserManager::RandomLoginProcess(_Key key)
{
	std::string randomString = "";
	srand((unsigned)time(NULL));
	for (int i = 0; i < 8; ++i)
	{
		randomString.insert(randomString.begin(), (char)(rand() % 26 + 97));
	}

	PrintLog(SOURCE_LOCATION, "���� �α��� ���μ����� �����մϴ�. ���� ��Ʈ���� : " + randomString);

	std::wstring randomWideString = CONVERT_UTIL::ConvertFromMBCSToUTF16(randomString);
	
	USER_MANAGER::SignUpTaskUnit* taskUnit = new USER_MANAGER::SignUpTaskUnit{ key, randomWideString,randomWideString,randomWideString };
	SignUpProcess(taskUnit);
}


void UserManager::AcceptProcess(USER_MANAGER::AcceptTaskUnit* taskUnit)
{
	_Key key;
	SOCKADDR_IN& clientAddr = taskUnit->clientAddr;

	if (!waitingUserPool.empty())
	{
		std::cout << "[ ���� ��� Ŭ���̾�Ʈ IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;
		
		PACKET_DATA::SERVER_TO_CLIENT::AcceptX packet(0); // ���� ���ʿ��ϰ� Enum ����.
		NetworkManager::GetInstance().SendPacket(taskUnit->socket, reinterpret_cast<char*>(&packet));
		waitingUserPool.emplace_back(taskUnit->socket);
		return;
	}

	if (keyPool.empty())
	{
		std::cout << "[ ���� ��� Ŭ���̾�Ʈ IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;
		
		PACKET_DATA::SERVER_TO_CLIENT::AcceptX packet(1); // ���� ���ʿ��ϰ� Enum ����.
		NetworkManager::GetInstance().SendPacket(taskUnit->socket, reinterpret_cast<char*>(&packet));
		waitingUserPool.emplace_back(taskUnit->socket);
		return;
	}

	key = keyPool.front();
	keyPool.pop();

	std::cout << "[ ���ο� Ŭ���̾�Ʈ(" << key << ") ����! IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;

	// Ű(������ �޸� �ּҷ� ����)�� ����� �Ϸ� ��Ʈ ����
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(taskUnit->socket), hIOCP, key, 0);

	// ���� ���� �ʱ�ȭ�� ���⼭ �� ������, ������ �Ұ�����

	// ���� ���� ��ü�� ���
	userCont[key]->socket = taskUnit->socket;

	// �񵿱� ������� ����.
	NetworkManager::GetInstance().SetRecvState(userCont[key]);

	// ����Ǿ����� �˸�.
	PACKET_DATA::SERVER_TO_CLIENT::AcceptO packet;
	NetworkManager::GetInstance().SendPacket(taskUnit->socket, reinterpret_cast<char*>(&packet));

	delete taskUnit;
}

void UserManager::SignUpProcess(USER_MANAGER::SignUpTaskUnit* taskUnit)
{
#if LOCAL_FILE_DB_MODE == TRUE
	
	if (auto nicknameIter = nicknameCont.find(taskUnit->nickname)
		; nicknameIter != nicknameCont.end())
	{
		// �̹� �ش� �г����� ������ ������.
		PACKET_DATA::SERVER_TO_CLIENT::SignUpX packet(0); // ���� ���ʿ��ϰ� Enum ����.
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		PrintLog(SOURCE_LOCATION, "ȸ������ ���� - �ش� �г����� ������ ������.");
	}
	else
	{
		int testValue = -1;
		std::string mbcsId = CONVERT_UTIL::ConvertFromUTF16ToMBCS(taskUnit->id);
		
		// ���� ���� - ���߿� �� ���������
		mbcsId.pop_back();

		{
			// �ش� ���̵��� ������ �ִ��� Ȯ���ϱ� ����, �߰�
			string fileName = "UserData/" + mbcsId + ".ud";

			// ���� ����
			std::ifstream inFile(fileName, std::ios::in);

			// ���� �б�.
			inFile >> testValue;
		}

		if (testValue == -1)
		{
			PrintLog(SOURCE_LOCATION, "ȸ������ ����!");

			// �ش� �г����� ������ �������� �ʾ�, ���� ���� ����.
			nicknameCont.emplace(taskUnit->nickname, taskUnit->id);
			
			// File I/O ==============================================================================
			// ���� ID�� ���� �̸� ����. count�� 0����, �г����� �� ���� �ʿ�.
			// =======================================================================================
			string fileName = "UserData/" + mbcsId + ".ud";

			// ���� ���� ���� ����
			{
				std::ofstream outFile(fileName, std::ios::out);

				outFile
					<< " " << 1	// level
					<< " " << 0	// money
					<< " " << 0	// winCount
					<< " " << 0	// loseCount
					<< " " << 1	// pickedCharacterIndex
					<< " " << CONVERT_UTIL::ConvertFromUTF16ToMBCS(taskUnit->nickname)	// nickName
					<< " " << CONVERT_UTIL::ConvertFromUTF16ToMBCS(taskUnit->pw)	// nickName
					<< std::endl;
			}

			// ���� ���� ���������� �˸�.
			PACKET_DATA::SERVER_TO_CLIENT::SignUpO packet; // ���� ���ʿ��ϰ� Enum ����.
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

			// ���������� �α��� �ٷ� ����.
			LoginProcess(new USER_MANAGER::LoginTaskUnit({ taskUnit->key, taskUnit->id, taskUnit->pw }));
		}
		else
		{
			PrintLog(SOURCE_LOCATION, "ȸ������ ����, �ش� ���̵��� ������ �̹� ������.");

			// �̹� �ش� ���̵��� ������ ������.
			PACKET_DATA::SERVER_TO_CLIENT::SignUpX packet(1); 
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		}
	}
#endif
	delete taskUnit;
}

void UserManager::LoginProcess(USER_MANAGER::LoginTaskUnit* taskUnit)
{
#if LOCAL_FILE_DB_MODE == TRUE
	if (userCont[taskUnit->key]->userManagerUnit.isLogin == true)
	{
		// �α��� ���������� ������.
		PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(0);
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

		PrintLog(SOURCE_LOCATION, "�α��� ���� - �̹� �α����� ������, �� �α����� �õ���.");
	}
	else if (auto iter = idCont.find(taskUnit->id)
		; iter != idCont.end())
	{
		// �α��� ���������� ������.
		PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(1);
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		
		PrintLog(SOURCE_LOCATION, "�α��� ���� - �̹� �α����� ���̵�� �α����� �õ���.");
	}
	else
	{
		std::string mbcsId = CONVERT_UTIL::ConvertFromUTF16ToMBCS(taskUnit->id);
		
		// ���� ���� - ���߿� �� ���������
		mbcsId.pop_back();

		// ���� �غ�
		//_Level level{ -1 };
		int level{ -1 };
		_Money money{ 0 };
		_Count winCount{ 0 };
		_Count loseCount{ 0 };
		_Count pickedCharacterIndex{ 0 };
		std::string strBuffer{};
		std::string pwBuffer{};

		{
			// File I/O ==============================================================================
			string fileName = "UserData/" + mbcsId + ".ud";

			// ���� ����
			std::ifstream inFile(fileName, std::ios::in);

			// ���� �б�
			inFile
				>> level
				>> money
				>> winCount
				>> loseCount
				>> pickedCharacterIndex
				>> strBuffer
				>> pwBuffer
				;

			// =======================================================================================
		}

		std::wstring wpwBuffer = CONVERT_UTIL::ConvertFromMBCSToUTF16(pwBuffer);
		wpwBuffer.pop_back();

		if (level == -1)
		{
			// �ش� ID ������
			PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(2);
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

			PrintLog(SOURCE_LOCATION, "�α��� ���� - �ش� ���̵� �������� ����.");
		}
		//else if (wpwBuffer != taskUnit->pw)
		//{
		//	//	std::wcout << L" pwBuffer : " << wpwBuffer << L" // Size : "<< wpwBuffer.size() << std::endl;
			//	std::wcout << L" taskUnit->pw : " << taskUnit->pw << L" // Size : " << taskUnit->pw.size() << std::endl;
		//
		//	// PW Ʋ��.
		//	PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(3);
		//	NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		//
		//	PrintLog(SOURCE_LOCATION, "�α��� ���� - ��й�ȣ �ٸ�.");
		//}
		else
		{
			// id �� ���� �ڷ� ���
			userCont[taskUnit->key]->userManagerUnit.id = taskUnit->id;
			userCont[taskUnit->key]->userManagerUnit.nickname = CONVERT_UTIL::ConvertFromMBCSToUTF16(strBuffer);
			userCont[taskUnit->key]->pw = CONVERT_UTIL::ConvertFromMBCSToUTF16(pwBuffer);

			userCont[taskUnit->key]->level = level;
			userCont[taskUnit->key]->money = money;
			userCont[taskUnit->key]->winCount = winCount;
			userCont[taskUnit->key]->loseCount = loseCount;
			userCont[taskUnit->key]->pickedCharacterIndex = pickedCharacterIndex;

			// ���̵� �����̳ʿ� ���
			idCont.insert({ taskUnit->id , taskUnit->key });

			// �κ������� ���.
			JoinLobbyUser(taskUnit->key, true);

			// PrintLog(SOURCE_LOCATION, "�α��� ���� ID : " + taskUnit->id + "(" + std::to_string(taskUnit->key) +")");

			// �α��� ���������� ������.
			PACKET_DATA::SERVER_TO_CLIENT::LoginO packet(level, money, winCount, loseCount, pickedCharacterIndex, 
				CONVERT_UTIL::ConvertFromMBCSToUTF16(strBuffer));
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

			PrintLog(SOURCE_LOCATION, "�α��� ����!! ID " + mbcsId);
		}
	}
#endif
	delete taskUnit;
}

void UserManager::LogoutProcess(USER_MANAGER::LogoutTaskUnit* taskUnit)
{
	// �κ񿡼� ����.
	//OutLobbyUser(taskUnit->key);

	// id Cont���� ���� ����.
	//idCont.erase(userCont[taskUnit->key]->userManagerUnit.id);

	// �濡 �������̰ų�, �濡 ������ ������ ��� ��������.
	//if (userCont[taskUnit->key]->userState == USER_STATE::IN_ROOM
	//	|| userCont[taskUnit->key]->userState == USER_STATE::WAIT_IN_ROOM)
		
	//{
	//	ROOM_MANAGER::OutRoomTaskUnit* roomOutTaskUnit
	//		= new ROOM_MANAGER::OutRoomTaskUnit{ userCont[taskUnit->key] };
	//	RoomManager::GetInstance().ProduceTask(TASK_TYPE::EXIT_ROOM, roomOutTaskUnit);
	//}

	// �α��� ���θ� False�� �������ְ�.
	userCont[taskUnit->key]->userManagerUnit.isLogin = false;

	// ���� nullptr�� ���, ��⿭�� �ִ��� �ǽ��ϰ�, ��⿭ ������ ��û�մϴ�.
	if (RoomUnit* pRoomUnit = userCont[taskUnit->key]->roomManagerUnit.GetRoom(); 
		pRoomUnit == nullptr)
	{
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::CANCLE_MATCH, reinterpret_cast<void*>(taskUnit->key));
	}
	else
	{
		// ������ ���� ������ nullptr�� �ƴ� ��, �̿� ���� ������ vaild�� ����Ǿ�� �Ѵ�.
		// ���� �������� �ʰ�, �濡�� ���� ��, ó���ϵ��� ����
		return;
	}

	// LogOut ������ �޼��� ����.
	if (taskUnit->isOnConnect)
	{
		PACKET_DATA::SERVER_TO_CLIENT::Logout packet;
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
	}
	
	// ������ ������.
	{
		std::string mbcsId = CONVERT_UTIL::ConvertFromUTF16ToMBCS(userCont[taskUnit->key]->userManagerUnit.id);
		mbcsId.pop_back();
		string fileName = "UserData/" + mbcsId + ".ud";

		// ���� ���� ���� ����
		{
			std::ofstream outFile(fileName, std::ios::out);

			outFile
				<< " " << (int)(userCont[taskUnit->key]->level)	// level
				<< " " << userCont[taskUnit->key]->money	// money
				<< " " << userCont[taskUnit->key]->winCount	// winCount
				<< " " << userCont[taskUnit->key]->loseCount	// loseCount
				<< " " << (int)(userCont[taskUnit->key]->pickedCharacterIndex)	// pickedCharacterIndex
				<< " " << CONVERT_UTIL::ConvertFromUTF16ToMBCS(userCont[taskUnit->key]->userManagerUnit.nickname)	// nickName
				<< " " << CONVERT_UTIL::ConvertFromUTF16ToMBCS(userCont[taskUnit->key]->pw)	// nickName
				<< std::endl;
		}
	}


	// ���� �ݱ�, ���� Ű �ݳ��� �ٷ����� �ʰ�, ���� �ð� �ڿ� ��.
	TimerManager::GetInstance().AddTimerEvent(TIMER_TYPE::PUSH_OLD_KEY, TASK_TYPE::PUSH_OLD_KEY, taskUnit->key, nullptr, TIME::PUSH_OLD_KEY);

	// ip, port Ž��
	int size;
	struct sockaddr_in clientAddr;
	size = sizeof(clientAddr);
	memset(&clientAddr, 0x00, sizeof(clientAddr));
	getpeername(userCont[taskUnit->key]->socket, (struct sockaddr*) & clientAddr, &size);

	std::cout << "[ Ŭ���̾�Ʈ(" << taskUnit->key << ") ����! IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;

	delete taskUnit;
}

void UserManager::ChangeCharacterProcess(USER_MANAGER::ChangeCharacterTaskUnit* taskUnit)
{
	PrintLog(SOURCE_LOCATION, "ĳ���͸� " + std::to_string(taskUnit->characterType) + "���� " + std::to_string(userCont[taskUnit->key]->pickedCharacterIndex) + " �� �����Ͽ����ϴ�." );
	userCont[taskUnit->key]->pickedCharacterIndex = taskUnit->characterType;
	delete taskUnit;
}

//void UserManager::WhisperProcess(USER_MANAGER::WhisperTaskUnit* taskUnit)
//{
//	if (taskUnit->targetid == userCont[taskUnit->key]->userManagerUnit.id)
//	{
//		// �ӼӸ� ���� -> �ڱⰡ �ڱ����� �ӼӸ�
//		NetworkManager::GetInstance().SendPacket(
//			userCont[taskUnit->key]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_FAIL_YOUANDME
//		);
//	}
//	else if (auto iter = idCont.find(taskUnit->targetid)
//		; iter != idCont.end())
//	{
//		//Ÿ�� �������� �ӼӸ� ����
//		NetworkManager::GetInstance().SendPacket(
//			userCont[iter->second]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_TRUE
//			+ userCont[taskUnit->key]->userManagerUnit.id
//			+ " : "
//			+ taskUnit->chatMessage
//			+ "\n\r \n\r "
//		);
//
//		//�ӼӸ� ���������� ����
//		NetworkManager::GetInstance().SendPacket(
//			userCont[taskUnit->key]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_SUCCESS
//		);
//	}
//	else
//	{
//		// �ӼӸ� ���� -> �׷� ID ������ ����.
//		NetworkManager::GetInstance().SendPacket(
//			userCont[taskUnit->key]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_FAIL_NOID
//		);
//	}
//	delete taskUnit;
//}

//void UserManager::LobbyInfoProcess(_Key key)
//{
//	PACKET_DATA::SERVER_TO_CLIENT::LobbyInfo packet;
//
//	packet.lobbyUser = 0;
//	for (const auto& cont : lobbyUserCont)
//	{
//		//packet.lobbyUser += cont.size();
//	}
//
//	packet.allUser = idCont.size();
//
//	// 10������� ������ Ȯ���ϱ� ����, ��ȸ�մϴ�.
//	int loopCount = 0;
//	bool isBreak = false;
//	for (const auto& cont : lobbyUserCont)
//	{
//		//for (const auto& userKey : cont)
//		//{
//		//	++loopCount;
//		//
//		//	wmemcpy(packet.nicknames[loopCount], 
//		//		userCont[userKey]->userManagerUnit.GetNickname().c_str(),
//		//		userCont[userKey]->userManagerUnit.GetNickname().length());
//		//}
//	}
//
//	// ���� ������ �����մϴ�.
//	NetworkManager::GetInstance().SendPacket(userCont[key]->socket, reinterpret_cast<char*>(&packet));
//}

void UserManager::JoinLobbyUserInfo(_Key key)
{
	auto& user = userCont[key];
	PACKET_DATA::SERVER_TO_CLIENT::JoinLobbyInfo packet(user->level, user->money, user->winCount, user->loseCount, user->pickedCharacterIndex);

	// �κ� ������ �� ���� ������ ��������.
	NetworkManager::GetInstance().SendPacket(user->socket, reinterpret_cast<char*>(&packet));
}

void UserManager::LobbyChatProcess(USER_MANAGER::ChatLobbyTaskUnit* taskUnit)
{
	// �̱� ������� ó���ϱ⿡�� ���� ����̶� �����Ǿ�, �Ͻ����� ������ ���� ó��.
	//if constexpr (USER_WORKER_THREAD_NUM > 1)
	//{
	//	PACKET_DATA::SERVER_TO_CLIENT::LobbyChat packet(
	//		userCont[taskUnit->key]->userManagerUnit.nickname.c_str(),
	//		taskUnit->chatMessage
	//	);
	//
	//	// �κ� ������ ��� �����鿡�� ��Ƽ������� �޼����� �����մϴ�.
	//	for (int index = 0
	//		; index < USER_WORKER_THREAD_NUM
	//		; index++)
	//	{
	//		threadCont[index] = static_cast<std::thread>([&, index]()->auto
	//		{
	//			for (const auto lobbyUserKey : lobbyUserCont[index])
	//			{
	//				NetworkManager::GetInstance().SendPacket(userCont[lobbyUserKey]->socket, reinterpret_cast<char*>(&packet));
	//			}
	//		});
	//	}
	//
	//	for (auto& thread : threadCont)
	//	{
	//		thread.join();
	//	}
	//}
	//else
	//{
	// �̱� ������ �κ� ä�� ���� �ڵ��Դϴ�.

	PrintLog(SOURCE_LOCATION, "�κ� ä���� �Է��մϴ�.");

	PACKET_DATA::SERVER_TO_CLIENT::LobbyChat packet(
		userCont[taskUnit->key]->userManagerUnit.nickname.c_str(),
		taskUnit->chatMessage
	);

	for (const auto lobbyUserKey : lobbyUserCont)
	{
		NetworkManager::GetInstance().SendPacket(userCont[lobbyUserKey]->socket, reinterpret_cast<char*>(&packet));
	}

	delete taskUnit;
}

void UserManager::PushOldKeyProcess(_Key oldKey)
{
	PrintLog(SOURCE_LOCATION, "PushOldKeyProcess�� ����Ǿ����ϴ�.");

	// File I/O ==============================================================================
	// SAVE!
	// =======================================================================================

	// �α׾ƿ��� ������ ���� �ð� ���Ŀ� �ʱ�ȭ�մϴ�.
	userCont[oldKey]->userManagerUnit.isLogin = false;

	// �κ񿡼� ����.
	OutLobbyUser(oldKey);

	// id Cont���� ���� ����.
	idCont.erase(userCont[oldKey]->userManagerUnit.id);

	//userCont[oldKey]->userState = USER_STATE::NO_LOGIN;
	userCont[oldKey]->taskManagerUnit.Unsafe_SetLoadedSize();
	// userCont[oldKey]->roomIndex = -1;

	// ����� Ű�� �ݳ��մϴ�.
	closesocket(userCont[oldKey]->socket);
	keyPool.push(oldKey);
}

void UserManager::JoinLobbyUser(_Key key, bool isLoginCall)
{
	// �κ� �����մϴ�.
	if (isLoginCall == true) 
	{
		userCont[key]->userManagerUnit.isLogin = true;
		// �α��� ������ ȣ���� �½�ũ�Դϴ�.
		//if (!ATOMIC_UTIL::T_CAS(&(userCont[key]->userState), USER_STATE::WAIT_IN_USER, USER_STATE::IN_LOBBY))
		//{
		//	PrintLog(SOURCE_LOCATION, "�α��ο��� �κ� ���� ����, �� ���� �߻�");
		//	std::cout << magic_enum::enum_name<USER_STATE>(userCont[key]->userState) << "\n";
		//}
	}
	else 
	{
		// �� ������ ȣ���� �½�ũ�Դϴ�.
		//if (!ATOMIC_UTIL::T_CAS(&(userCont[key]->userState), USER_STATE::ROOM_TO_LOBBY, USER_STATE::IN_LOBBY))
		//{
		//	PrintLog(SOURCE_LOCATION, "�濡�� �κ���� ���� ����. �� ���� �߻� : " + std::to_string((int)userCont[key]->userState.load()));
		//}
	}

	//lobbyUserCont[key % USER_WORKER_THREAD_NUM].emplace(key);
	lobbyUserCont.emplace(key);
}

void UserManager::OutLobbyUser(_Key key)
{
	// lobbyUserCont[key % USER_WORKER_THREAD_NUM].erase(key);
	lobbyUserCont.emplace(key);
}

void UserManager::SetIOCPHandle(HANDLE& IOCPHandle)
{
	assert(IOCPHandle == nullptr || "IOCP �ڵ�� nullptr�� ȣ��Ǿ����ϴ�. Server ������ Ȯ�����ּ���.\n");
	hIOCP = IOCPHandle;
}

void UserManager::GetUserPointerWithKey(UserUnit*& pUser, _Key key)
{
	// �ش� ���� �޸𸮷� �����ص�, ���������δ� ���ѵǾ� �ֽ��ϴ�.
	pUser = userCont[key];
}
