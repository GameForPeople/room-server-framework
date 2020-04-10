#include "stdafx.h"

#include "IOCPServerFramework.h"

int main(int argc, char* argv[])
{
	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	IOCPServerFramework::PreTest();
	std::unique_ptr<IOCPServerFramework> roomServer = std::make_unique<IOCPServerFramework>((argc > 1) ? (atoi(argv[1])) : SERVER_LISTEN_PORT_NUMBER);
	roomServer->Run();
}