#include "stdafx.h"
#include "Utils.h"

_NORETURN void ERROR_UTIL::Error(const std::string_view msg)
{
	LPVOID lpMsgBuf;
	int errorCode = WSAGetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	std::cout << "[" << msg << "(" << errorCode << ")] "
		<< ((LPCTSTR)lpMsgBuf) << "\n";

	LocalFree(lpMsgBuf);

	while (true) {}
}

void ERROR_UTIL::ErrorRecv()
{
	if (int errorCode = WSAGetLastError()
		; errorCode != ERROR_IO_PENDING && errorCode != WSAENOTSOCK && errorCode != WSAECONNRESET)
	{
		Error("Recv");
	}
}

void ERROR_UTIL::ErrorSend()
{
	if (int errorCode = WSAGetLastError()
		; errorCode != ERROR_IO_PENDING && errorCode != WSAENOTSOCK && errorCode != WSAECONNRESET)
	{
		Error("Send");
	}
}

#ifdef LOG_ON
LOG_UTIL::SourceLocation::SourceLocation(const char* fileName, int fileLine, const char* functionName)
	: fileName(fileName), fileLine(fileLine), functionName(functionName), outputString()
{
	outputString = // 쓸데없이 비용이 너무 높은데..
		'['
		+ std::string(fileName)
		+ "]("
		+ std::to_string(fileLine)
		+ ") <"
		+ std::string(functionName)
		+ '>';
}

void LOG_UTIL::PrintLog(SourceLocation sourceLocation, const std::string& log = "")
{
	std::cout << sourceLocation.outputString << " " << log << std::endl;
}
#endif

std::string CONVERT_UTIL::ConvertFromUTF16ToMBCS(std::wstring utfString)
{
	// unicode string을 받아 MBCS로 변경합니다.

	int sizeBuffer = WideCharToMultiByte(CP_ACP, 0, utfString.c_str(), static_cast<int>(utfString.length()), NULL, 0, NULL, NULL);
	std::string buffer(sizeBuffer + 1, 0);
	WideCharToMultiByte(CP_ACP, 0, utfString.c_str(), static_cast<int>(utfString.length()), &buffer[0], static_cast<int>(buffer.size()), NULL, NULL);
	buffer[sizeBuffer] = '\n';

	return buffer;
}

std::wstring CONVERT_UTIL::ConvertFromMBCSToUTF16(std::string mbcsString)
{
	// MBCS를 받아 unicode string로 변경합니다.

	int sizeBuffer = MultiByteToWideChar(CP_ACP, 0, &(mbcsString.c_str()[0]), -1, NULL, 0);
	std::wstring retString(sizeBuffer, 0);
	MultiByteToWideChar(CP_ACP, 0, &(mbcsString.c_str()[0]), -1, &retString[0], sizeBuffer);

	return retString;
}