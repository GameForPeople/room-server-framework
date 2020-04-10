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
	// MAX_USER만큼 미리 할당 받습니다.
	for (int i = 0; i < MAX_USER; ++i) { userCont[i] = new UserUnit(i); }
	for (int i = 0; i < MAX_USER; ++i) { keyPool.push(i); }

	// 유저 쓰레드를 실행시킵니다.
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
	// TaskCont에 일감 등록.
	taskCont.push({ taskType , taskUnit });
}

void UserManager::UserThread()
{
	_UserManagerTask task{};
	while (7)
	{
		using namespace USER_MANAGER;
		
		// 테스크가 있는지 확인합니다.
		if (!taskCont.try_pop(task))
		{
			// using namespace std::literals;
			//std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_LOOP_TIME));

			// 비어있을 경우, 쓰레드 우선순위에서 뒤로 미룹니다.
			std::this_thread::yield();
			continue;
		}

		// 등록받은 테스크를 처리합니다.
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

	PrintLog(SOURCE_LOCATION, "랜덤 로그인 프로세스를 시작합니다. 랜덤 스트링은 : " + randomString);

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
		std::cout << "[ 접속 대기 클라이언트 IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;
		
		PACKET_DATA::SERVER_TO_CLIENT::AcceptX packet(0); // 굳이 불필요하게 Enum 안함.
		NetworkManager::GetInstance().SendPacket(taskUnit->socket, reinterpret_cast<char*>(&packet));
		waitingUserPool.emplace_back(taskUnit->socket);
		return;
	}

	if (keyPool.empty())
	{
		std::cout << "[ 접속 대기 클라이언트 IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;
		
		PACKET_DATA::SERVER_TO_CLIENT::AcceptX packet(1); // 굳이 불필요하게 Enum 안함.
		NetworkManager::GetInstance().SendPacket(taskUnit->socket, reinterpret_cast<char*>(&packet));
		waitingUserPool.emplace_back(taskUnit->socket);
		return;
	}

	key = keyPool.front();
	keyPool.pop();

	std::cout << "[ 새로운 클라이언트(" << key << ") 접속! IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;

	// 키(오버랩 메모리 주소로 변경)와 입출력 완료 포트 연결
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(taskUnit->socket), hIOCP, key, 0);

	// 소켓 유저 초기화를 여기서 할 것인지, 나갈때 할것인지

	// 소켓 유저 객체에 등록
	userCont[key]->socket = taskUnit->socket;

	// 비동기 입출력의 시작.
	NetworkManager::GetInstance().SetRecvState(userCont[key]);

	// 연결되었음을 알림.
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
		// 이미 해당 닉네임의 유저가 존재함.
		PACKET_DATA::SERVER_TO_CLIENT::SignUpX packet(0); // 굳이 불필요하게 Enum 안함.
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		PrintLog(SOURCE_LOCATION, "회원가입 실패 - 해당 닉네임의 유저가 존재함.");
	}
	else
	{
		int testValue = -1;
		std::string mbcsId = CONVERT_UTIL::ConvertFromUTF16ToMBCS(taskUnit->id);
		
		// 공백 제거 - 나중에 더 살펴봐야함
		mbcsId.pop_back();

		{
			// 해당 아이디의 유저가 있는지 확인하기 위해, 추가
			string fileName = "UserData/" + mbcsId + ".ud";

			// 파일 오픈
			std::ifstream inFile(fileName, std::ios::in);

			// 파일 읽기.
			inFile >> testValue;
		}

		if (testValue == -1)
		{
			PrintLog(SOURCE_LOCATION, "회원가입 성공!");

			// 해당 닉네임의 유저가 존재하지 않아, 계정 생성 성공.
			nicknameCont.emplace(taskUnit->nickname, taskUnit->id);
			
			// File I/O ==============================================================================
			// 관련 ID로 파일 이름 생성. count는 0으로, 닉네임은 꼭 저장 필요.
			// =======================================================================================
			string fileName = "UserData/" + mbcsId + ".ud";

			// 파일 쓰기 모드로 오픈
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

			// 계정 생성 성공했음을 알림.
			PACKET_DATA::SERVER_TO_CLIENT::SignUpO packet; // 굳이 불필요하게 Enum 안함.
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

			// 동기적으로 로그인 바로 실행.
			LoginProcess(new USER_MANAGER::LoginTaskUnit({ taskUnit->key, taskUnit->id, taskUnit->pw }));
		}
		else
		{
			PrintLog(SOURCE_LOCATION, "회원가입 실패, 해당 아이디의 유저가 이미 존재함.");

			// 이미 해당 아이디의 유저가 존재함.
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
		// 로그인 실패했음을 전송함.
		PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(0);
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

		PrintLog(SOURCE_LOCATION, "로그인 실패 - 이미 로그인한 계정이, 또 로그인을 시도함.");
	}
	else if (auto iter = idCont.find(taskUnit->id)
		; iter != idCont.end())
	{
		// 로그인 실패했음을 전송함.
		PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(1);
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		
		PrintLog(SOURCE_LOCATION, "로그인 실패 - 이미 로그인한 아이디로 로그인을 시도함.");
	}
	else
	{
		std::string mbcsId = CONVERT_UTIL::ConvertFromUTF16ToMBCS(taskUnit->id);
		
		// 공백 제거 - 나중에 더 살펴봐야함
		mbcsId.pop_back();

		// 변수 준비
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

			// 파일 오픈
			std::ifstream inFile(fileName, std::ios::in);

			// 파일 읽기
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
			// 해당 ID 미존재
			PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(2);
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

			PrintLog(SOURCE_LOCATION, "로그인 실패 - 해당 아이디가 존재하지 않음.");
		}
		//else if (wpwBuffer != taskUnit->pw)
		//{
		//	//	std::wcout << L" pwBuffer : " << wpwBuffer << L" // Size : "<< wpwBuffer.size() << std::endl;
			//	std::wcout << L" taskUnit->pw : " << taskUnit->pw << L" // Size : " << taskUnit->pw.size() << std::endl;
		//
		//	// PW 틀림.
		//	PACKET_DATA::SERVER_TO_CLIENT::LoginX packet(3);
		//	NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
		//
		//	PrintLog(SOURCE_LOCATION, "로그인 실패 - 비밀번호 다름.");
		//}
		else
		{
			// id 및 관련 자료 등록
			userCont[taskUnit->key]->userManagerUnit.id = taskUnit->id;
			userCont[taskUnit->key]->userManagerUnit.nickname = CONVERT_UTIL::ConvertFromMBCSToUTF16(strBuffer);
			userCont[taskUnit->key]->pw = CONVERT_UTIL::ConvertFromMBCSToUTF16(pwBuffer);

			userCont[taskUnit->key]->level = level;
			userCont[taskUnit->key]->money = money;
			userCont[taskUnit->key]->winCount = winCount;
			userCont[taskUnit->key]->loseCount = loseCount;
			userCont[taskUnit->key]->pickedCharacterIndex = pickedCharacterIndex;

			// 아이디 컨테이너에 등록
			idCont.insert({ taskUnit->id , taskUnit->key });

			// 로비유저에 등록.
			JoinLobbyUser(taskUnit->key, true);

			// PrintLog(SOURCE_LOCATION, "로그인 성공 ID : " + taskUnit->id + "(" + std::to_string(taskUnit->key) +")");

			// 로그인 성공했음을 전송함.
			PACKET_DATA::SERVER_TO_CLIENT::LoginO packet(level, money, winCount, loseCount, pickedCharacterIndex, 
				CONVERT_UTIL::ConvertFromMBCSToUTF16(strBuffer));
			NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));

			PrintLog(SOURCE_LOCATION, "로그인 성공!! ID " + mbcsId);
		}
	}
#endif
	delete taskUnit;
}

void UserManager::LogoutProcess(USER_MANAGER::LogoutTaskUnit* taskUnit)
{
	// 로비에서 삭제.
	//OutLobbyUser(taskUnit->key);

	// id Cont에서 먼저 삭제.
	//idCont.erase(userCont[taskUnit->key]->userManagerUnit.id);

	// 방에 접속중이거나, 방에 접속할 예정일 경우 나가야함.
	//if (userCont[taskUnit->key]->userState == USER_STATE::IN_ROOM
	//	|| userCont[taskUnit->key]->userState == USER_STATE::WAIT_IN_ROOM)
		
	//{
	//	ROOM_MANAGER::OutRoomTaskUnit* roomOutTaskUnit
	//		= new ROOM_MANAGER::OutRoomTaskUnit{ userCont[taskUnit->key] };
	//	RoomManager::GetInstance().ProduceTask(TASK_TYPE::EXIT_ROOM, roomOutTaskUnit);
	//}

	// 로그인 여부를 False로 변경해주고.
	userCont[taskUnit->key]->userManagerUnit.isLogin = false;

	// 방이 nullptr일 경우, 대기열에 있는지 의심하고, 대기열 헤제를 요청합니다.
	if (RoomUnit* pRoomUnit = userCont[taskUnit->key]->roomManagerUnit.GetRoom(); 
		pRoomUnit == nullptr)
	{
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::CANCLE_MATCH, reinterpret_cast<void*>(taskUnit->key));
	}
	else
	{
		// 복사한 방의 정보가 nullptr가 아닐 때, 이에 대한 접근은 vaild가 보장되어야 한다.
		// 접근 직접하지 않고, 방에서 나갈 때, 처리하도록 변경
		return;
	}

	// LogOut 됬음을 메세지 보냄.
	if (taskUnit->isOnConnect)
	{
		PACKET_DATA::SERVER_TO_CLIENT::Logout packet;
		NetworkManager::GetInstance().SendPacket(userCont[taskUnit->key]->socket, reinterpret_cast<char*>(&packet));
	}
	
	// 정보를 저장함.
	{
		std::string mbcsId = CONVERT_UTIL::ConvertFromUTF16ToMBCS(userCont[taskUnit->key]->userManagerUnit.id);
		mbcsId.pop_back();
		string fileName = "UserData/" + mbcsId + ".ud";

		// 파일 쓰기 모드로 오픈
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


	// 소켓 닫기, 정리 키 반납은 바로하지 않고, 일정 시간 뒤에 함.
	TimerManager::GetInstance().AddTimerEvent(TIMER_TYPE::PUSH_OLD_KEY, TASK_TYPE::PUSH_OLD_KEY, taskUnit->key, nullptr, TIME::PUSH_OLD_KEY);

	// ip, port 탐색
	int size;
	struct sockaddr_in clientAddr;
	size = sizeof(clientAddr);
	memset(&clientAddr, 0x00, sizeof(clientAddr));
	getpeername(userCont[taskUnit->key]->socket, (struct sockaddr*) & clientAddr, &size);

	std::cout << "[ 클라이언트(" << taskUnit->key << ") 종료! IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;

	delete taskUnit;
}

void UserManager::ChangeCharacterProcess(USER_MANAGER::ChangeCharacterTaskUnit* taskUnit)
{
	PrintLog(SOURCE_LOCATION, "캐릭터를 " + std::to_string(taskUnit->characterType) + "에서 " + std::to_string(userCont[taskUnit->key]->pickedCharacterIndex) + " 로 변경하였습니다." );
	userCont[taskUnit->key]->pickedCharacterIndex = taskUnit->characterType;
	delete taskUnit;
}

//void UserManager::WhisperProcess(USER_MANAGER::WhisperTaskUnit* taskUnit)
//{
//	if (taskUnit->targetid == userCont[taskUnit->key]->userManagerUnit.id)
//	{
//		// 귓속말 실패 -> 자기가 자기한테 귓속말
//		NetworkManager::GetInstance().SendPacket(
//			userCont[taskUnit->key]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_FAIL_YOUANDME
//		);
//	}
//	else if (auto iter = idCont.find(taskUnit->targetid)
//		; iter != idCont.end())
//	{
//		//타겟 유저에게 귓속말 전송
//		NetworkManager::GetInstance().SendPacket(
//			userCont[iter->second]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_TRUE
//			+ userCont[taskUnit->key]->userManagerUnit.id
//			+ " : "
//			+ taskUnit->chatMessage
//			+ "\n\r \n\r "
//		);
//
//		//귓속말 성공했음을 전송
//		NetworkManager::GetInstance().SendPacket(
//			userCont[taskUnit->key]->socket
//			, MESSAGE_SERVER_TO_CLIENT::WHISEPER_SUCCESS
//		);
//	}
//	else
//	{
//		// 귓속말 실패 -> 그런 ID 미접속 중임.
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
//	// 10명까지의 정보를 확인하기 위해, 순회합니다.
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
//	// 관련 정보를 전송합니다.
//	NetworkManager::GetInstance().SendPacket(userCont[key]->socket, reinterpret_cast<char*>(&packet));
//}

void UserManager::JoinLobbyUserInfo(_Key key)
{
	auto& user = userCont[key];
	PACKET_DATA::SERVER_TO_CLIENT::JoinLobbyInfo packet(user->level, user->money, user->winCount, user->loseCount, user->pickedCharacterIndex);

	// 로비에 들어왔을 때 관련 데이터 전송해줌.
	NetworkManager::GetInstance().SendPacket(user->socket, reinterpret_cast<char*>(&packet));
}

void UserManager::LobbyChatProcess(USER_MANAGER::ChatLobbyTaskUnit* taskUnit)
{
	// 싱글 쓰레드로 처리하기에는 높은 비용이라 생각되어, 일시적을 쓰레드 만들어서 처리.
	//if constexpr (USER_WORKER_THREAD_NUM > 1)
	//{
	//	PACKET_DATA::SERVER_TO_CLIENT::LobbyChat packet(
	//		userCont[taskUnit->key]->userManagerUnit.nickname.c_str(),
	//		taskUnit->chatMessage
	//	);
	//
	//	// 로비에 접속한 모든 유저들에게 멀티쓰레드로 메세지를 전송합니다.
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
	// 싱글 쓰레드 로비 채팅 전송 코드입니다.

	PrintLog(SOURCE_LOCATION, "로비 채팅을 입력합니다.");

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
	PrintLog(SOURCE_LOCATION, "PushOldKeyProcess가 실행되었습니다.");

	// File I/O ==============================================================================
	// SAVE!
	// =======================================================================================

	// 로그아웃한 유저를 일정 시간 이후에 초기화합니다.
	userCont[oldKey]->userManagerUnit.isLogin = false;

	// 로비에서 삭제.
	OutLobbyUser(oldKey);

	// id Cont에서 먼저 삭제.
	idCont.erase(userCont[oldKey]->userManagerUnit.id);

	//userCont[oldKey]->userState = USER_STATE::NO_LOGIN;
	userCont[oldKey]->taskManagerUnit.Unsafe_SetLoadedSize();
	// userCont[oldKey]->roomIndex = -1;

	// 사용한 키를 반납합니다.
	closesocket(userCont[oldKey]->socket);
	keyPool.push(oldKey);
}

void UserManager::JoinLobbyUser(_Key key, bool isLoginCall)
{
	// 로비에 접속합니다.
	if (isLoginCall == true) 
	{
		userCont[key]->userManagerUnit.isLogin = true;
		// 로그인 씐에서 호출한 태스크입니다.
		//if (!ATOMIC_UTIL::T_CAS(&(userCont[key]->userState), USER_STATE::WAIT_IN_USER, USER_STATE::IN_LOBBY))
		//{
		//	PrintLog(SOURCE_LOCATION, "로그인에서 로비 접속 실패, 논리 오류 발생");
		//	std::cout << magic_enum::enum_name<USER_STATE>(userCont[key]->userState) << "\n";
		//}
	}
	else 
	{
		// 방 씐에서 호출한 태스크입니다.
		//if (!ATOMIC_UTIL::T_CAS(&(userCont[key]->userState), USER_STATE::ROOM_TO_LOBBY, USER_STATE::IN_LOBBY))
		//{
		//	PrintLog(SOURCE_LOCATION, "방에서 로비로의 복귀 실패. 논리 오류 발생 : " + std::to_string((int)userCont[key]->userState.load()));
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
	assert(IOCPHandle == nullptr || "IOCP 핸들로 nullptr이 호출되었습니다. Server 생성자 확인해주세요.\n");
	hIOCP = IOCPHandle;
}

void UserManager::GetUserPointerWithKey(UserUnit*& pUser, _Key key)
{
	// 해당 공유 메모리로 접근해도, 내부적으로는 제한되어 있습니다.
	pUser = userCont[key];
}
