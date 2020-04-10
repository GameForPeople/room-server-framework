#pragma once

#define LOCAL_FILE_DB_MODE TRUE

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct MemoryUnit;
struct UserUnit;
struct TaskUnit;

namespace USER_MANAGER
{
	enum class TASK_TYPE
	{
		// LOGIN ================================================

		USER_ACCEPT,	// 클라이언트 Accept
		USER_SIGNUP,	// 클라이언트 SIGNUP 시도
		USER_LOGIN,		// 클라이언트 Login 시도
		USER_LOGOUT,	// 클라이언트 LogOut 요청

		// LOBBY ================================================

		// USER_WHISPER,

		LOBBY_INFO,		// 클라이언트 Lobby 정보 요청 
		LOBBY_CHAT,		// 클라이언트 Lobby 채팅 요청

		RANDOM_MATCH,	// 클라이언트 Random-Matching 요청
		CANCLE_MATCH,	// 클라이언트 Random-Matching 취소 요청
	};


	// accept 하는 유저의 필요 정보입니다.
	struct AcceptTaskUnit
	{
		SOCKET socket;
		SOCKADDR_IN clientAddr;
	};

	// 회원가입을 요청하는 유저의 필요 정보입니다.
	struct SignUpTaskUnit
	{
		_Key key;
		std::wstring id;
		std::wstring pw;
		std::wstring nickname;

		SignUpTaskUnit(_Key key, const wstring& id, const wstring& pw, const wstring& nickname)
			: key(key)
			, id(id)
			, pw(pw)
			, nickname(nickname)
		{
			//memcpy(this->id, id, ID_SIZE);
			//memcpy(this->pw, pw, PW_SIZE);
			//memcpy(this->nickname, nickname, NICKNAME_SIZE * 2);
		}
	};

	// 로그인을 요청하는 유저의 필요 정보입니다.
	struct LoginTaskUnit
	{
		_Key key;
		std::wstring id;
		std::wstring pw;

		LoginTaskUnit(_Key key, std::wstring id, std::wstring pw)
			: key(key)
			, id(id)
			, pw(pw)
		{
			//memcpy(this->id, id, ID_SIZE);
			//memcpy(this->pw, pw, PW_SIZE);
		}
	};

	// 로그아웃을 요청하는 유저의 필요 정보입니다.
	struct LogoutTaskUnit
	{
		_Key key;
		bool isOnConnect;
	};

	struct ChangeCharacterTaskUnit
	{
		_Key key;
		_Type characterType;
	};

	// 귓속말을 요청하는 유저의 필요 정보입니다.
	//struct WhisperTaskUnit
	//{
	//	_Key key;
	//	std::string targetid;
	//	std::string chatMessage;
	//};

	// 로비 채팅에 필요한 정보입니다.
	struct ChatLobbyTaskUnit
	{
		_Key key;
		_Chat chatMessage;

		ChatLobbyTaskUnit(_Key key, _Chat chatMessage)
			: key(key)
			, chatMessage()
		{
			wmemcpy(this->chatMessage, chatMessage, CHAT_MESSAGE_LEN);
		}
	};

}

/*
	UserManager
		- User와 관련한 모든 task를 처리하는 Manager입니다.
		- 싱글턴 객체입니다.

	#0. 현재 추가적으로 LobbyManager의 역할도 수행하고 있습니다.
	#1. LobbyChat업무(높은 비용)을 수행할 때, Multi-thread로 처리합니다.
	#2. 
*/
class UserManager
	: public TSingleton<UserManager>
{
	static constexpr int USER_WORKER_THREAD_NUM = 0;	// UserManager 사용하는 워커 쓰레드 개수

public:
	UserManager();
	~UserManager();

	DISABLED_COPY(UserManager)
	DISABLED_MOVE(UserManager)

	using _UserManagerTask = std::pair<USER_MANAGER::TASK_TYPE, void*>;
	void ProduceTask(USER_MANAGER::TASK_TYPE taskType, void* const taskUnit);	// UserManager에 Task를 등록합니다.
	void SetIOCPHandle(HANDLE& hIOCP); // UserManager에 필요한 IOCP 핸들을 등록하는 함수입니다.

	// 모든 악의 원흉 -> 이제 괜찮아!
	void GetUserPointerWithKey(UserUnit*& pUser, _Key key); // Key를 받아 유저 포인트를 전달해줍니다.

private:
	void UserThread();	// UserThread에 할당하는 함수입니다.

	void AcceptProcess(USER_MANAGER::AcceptTaskUnit* acceptTaskUnit);
	void SignUpProcess(USER_MANAGER::SignUpTaskUnit* signupTaskUnit);
	void LoginProcess(USER_MANAGER::LoginTaskUnit* loginTaskUnit);
	void LogoutProcess(USER_MANAGER::LogoutTaskUnit* logoutTaskUnit);
	// void WhisperProcess(USER_MANAGER::WhisperTaskUnit* logoutTaskUnit);
	// void LobbyInfoProcess(_Key key);
	void LobbyChatProcess(USER_MANAGER::ChatLobbyTaskUnit* logoutTaskUnit);
	void ChangeCharacterProcess(USER_MANAGER::ChangeCharacterTaskUnit* ChangeCharacterTaskUnit);

	void PushOldKeyProcess(_Key oldKey);	// 로그아웃한 유저가 사용하던 키를 키 풀에 등록합니다.
	void JoinLobbyUserInfo(_Key key);
	void JoinLobbyUser(_Key key, bool isLoginCall = false);	// 해당 유저를 로비에 추가합니다.
	void OutLobbyUser(_Key key); // 해당 유저를 로비에서 아웃시킵니다. 


	void RandomLoginProcess(_Key key);

private:
	std::thread userThread;	// 쓰레드 객체입니다.
	concurrency::concurrent_queue<_UserManagerTask> taskCont;	// 다른 쓰레드로부터 받은 일감을 저장한 컨테이너입니다.
	
	std::array<UserUnit*, MAX_USER> userCont;	// 유저 컨테이너입니다.
	std::map<std::wstring, _Key> idCont;	// id를 통해 키를 획득하여, 유저정보를 찾기 위한 컨테이너입니다. (로그인한 유저만 등록됩니다.)
	
	std::unordered_set<_Key> lobbyUserCont; // 로비에 접속한 유저의 키를 갖고 있는 컨테인너입니다.

	//std::array<std::unordered_set<_Key>, USER_WORKER_THREAD_NUM> lobbyUserCont; // 로비에 접속한 유저의 키를 갖고 있는 컨테인너입니다.
	// std::array<std::thread, USER_WORKER_THREAD_NUM> threadCont; // 쓰레드 컨테이너입니다.

	std::queue<_Key> keyPool; // 사용가능한 키를 들고 있는 메모리풀 입니다.
	std::list<SOCKET> waitingUserPool; // 접속이 실패한 소켓을 들고 있는 컨테이너입니다. 현재 정확하게 활용되지 못하고 있습니다.

	// WonSY::SPIN_LOCK::SpinLock_TTAS_BackOff<10, 10> waitingUserPoolLock;

	HANDLE hIOCP; //IOCP 핸들입니다.

#if LOCAL_FILE_DB_MODE == TRUE
private:
	std::map<std::wstring, std::wstring> nicknameCont;	// 닉네임을 바탕으로 id를 찾기 위한 컨테이너입니다. (로그인하지 않은 유저의 정보도 갖고 있어야합니다.)
#endif
};

