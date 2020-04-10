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

		USER_ACCEPT,	// Ŭ���̾�Ʈ Accept
		USER_SIGNUP,	// Ŭ���̾�Ʈ SIGNUP �õ�
		USER_LOGIN,		// Ŭ���̾�Ʈ Login �õ�
		USER_LOGOUT,	// Ŭ���̾�Ʈ LogOut ��û

		// LOBBY ================================================

		// USER_WHISPER,

		LOBBY_INFO,		// Ŭ���̾�Ʈ Lobby ���� ��û 
		LOBBY_CHAT,		// Ŭ���̾�Ʈ Lobby ä�� ��û

		RANDOM_MATCH,	// Ŭ���̾�Ʈ Random-Matching ��û
		CANCLE_MATCH,	// Ŭ���̾�Ʈ Random-Matching ��� ��û
	};


	// accept �ϴ� ������ �ʿ� �����Դϴ�.
	struct AcceptTaskUnit
	{
		SOCKET socket;
		SOCKADDR_IN clientAddr;
	};

	// ȸ�������� ��û�ϴ� ������ �ʿ� �����Դϴ�.
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

	// �α����� ��û�ϴ� ������ �ʿ� �����Դϴ�.
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

	// �α׾ƿ��� ��û�ϴ� ������ �ʿ� �����Դϴ�.
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

	// �ӼӸ��� ��û�ϴ� ������ �ʿ� �����Դϴ�.
	//struct WhisperTaskUnit
	//{
	//	_Key key;
	//	std::string targetid;
	//	std::string chatMessage;
	//};

	// �κ� ä�ÿ� �ʿ��� �����Դϴ�.
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
		- User�� ������ ��� task�� ó���ϴ� Manager�Դϴ�.
		- �̱��� ��ü�Դϴ�.

	#0. ���� �߰������� LobbyManager�� ���ҵ� �����ϰ� �ֽ��ϴ�.
	#1. LobbyChat����(���� ���)�� ������ ��, Multi-thread�� ó���մϴ�.
	#2. 
*/
class UserManager
	: public TSingleton<UserManager>
{
	static constexpr int USER_WORKER_THREAD_NUM = 0;	// UserManager ����ϴ� ��Ŀ ������ ����

public:
	UserManager();
	~UserManager();

	DISABLED_COPY(UserManager)
	DISABLED_MOVE(UserManager)

	using _UserManagerTask = std::pair<USER_MANAGER::TASK_TYPE, void*>;
	void ProduceTask(USER_MANAGER::TASK_TYPE taskType, void* const taskUnit);	// UserManager�� Task�� ����մϴ�.
	void SetIOCPHandle(HANDLE& hIOCP); // UserManager�� �ʿ��� IOCP �ڵ��� ����ϴ� �Լ��Դϴ�.

	// ��� ���� ���� -> ���� ������!
	void GetUserPointerWithKey(UserUnit*& pUser, _Key key); // Key�� �޾� ���� ����Ʈ�� �������ݴϴ�.

private:
	void UserThread();	// UserThread�� �Ҵ��ϴ� �Լ��Դϴ�.

	void AcceptProcess(USER_MANAGER::AcceptTaskUnit* acceptTaskUnit);
	void SignUpProcess(USER_MANAGER::SignUpTaskUnit* signupTaskUnit);
	void LoginProcess(USER_MANAGER::LoginTaskUnit* loginTaskUnit);
	void LogoutProcess(USER_MANAGER::LogoutTaskUnit* logoutTaskUnit);
	// void WhisperProcess(USER_MANAGER::WhisperTaskUnit* logoutTaskUnit);
	// void LobbyInfoProcess(_Key key);
	void LobbyChatProcess(USER_MANAGER::ChatLobbyTaskUnit* logoutTaskUnit);
	void ChangeCharacterProcess(USER_MANAGER::ChangeCharacterTaskUnit* ChangeCharacterTaskUnit);

	void PushOldKeyProcess(_Key oldKey);	// �α׾ƿ��� ������ ����ϴ� Ű�� Ű Ǯ�� ����մϴ�.
	void JoinLobbyUserInfo(_Key key);
	void JoinLobbyUser(_Key key, bool isLoginCall = false);	// �ش� ������ �κ� �߰��մϴ�.
	void OutLobbyUser(_Key key); // �ش� ������ �κ񿡼� �ƿ���ŵ�ϴ�. 


	void RandomLoginProcess(_Key key);

private:
	std::thread userThread;	// ������ ��ü�Դϴ�.
	concurrency::concurrent_queue<_UserManagerTask> taskCont;	// �ٸ� ������κ��� ���� �ϰ��� ������ �����̳��Դϴ�.
	
	std::array<UserUnit*, MAX_USER> userCont;	// ���� �����̳��Դϴ�.
	std::map<std::wstring, _Key> idCont;	// id�� ���� Ű�� ȹ���Ͽ�, ���������� ã�� ���� �����̳��Դϴ�. (�α����� ������ ��ϵ˴ϴ�.)
	
	std::unordered_set<_Key> lobbyUserCont; // �κ� ������ ������ Ű�� ���� �ִ� �����γ��Դϴ�.

	//std::array<std::unordered_set<_Key>, USER_WORKER_THREAD_NUM> lobbyUserCont; // �κ� ������ ������ Ű�� ���� �ִ� �����γ��Դϴ�.
	// std::array<std::thread, USER_WORKER_THREAD_NUM> threadCont; // ������ �����̳��Դϴ�.

	std::queue<_Key> keyPool; // ��밡���� Ű�� ��� �ִ� �޸�Ǯ �Դϴ�.
	std::list<SOCKET> waitingUserPool; // ������ ������ ������ ��� �ִ� �����̳��Դϴ�. ���� ��Ȯ�ϰ� Ȱ����� ���ϰ� �ֽ��ϴ�.

	// WonSY::SPIN_LOCK::SpinLock_TTAS_BackOff<10, 10> waitingUserPoolLock;

	HANDLE hIOCP; //IOCP �ڵ��Դϴ�.

#if LOCAL_FILE_DB_MODE == TRUE
private:
	std::map<std::wstring, std::wstring> nicknameCont;	// �г����� �������� id�� ã�� ���� �����̳��Դϴ�. (�α������� ���� ������ ������ ���� �־���մϴ�.)
#endif
};

