#include "../Common/global_header.hh"

#define LOG_ON

namespace ERROR_UTIL
{
	_NORETURN void Error(const std::string_view msg);
	/*_NORETURN*/ void ErrorRecv();
	/*_NORETURN*/ void ErrorSend();
};

inline namespace LOG_UTIL
{
#ifdef LOG_ON
#define __FUNCTION_NAME__ __FUNCTION__
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define SOURCE_LOCATION {__FILENAME__, __LINE__, __FUNCTION_NAME__}

	struct SourceLocation
	{
		const char* fileName;
		int fileLine;
		const char* functionName;
		//std::wstring outputString;
		std::string outputString;

		SourceLocation(const char* fileName, int fileLine, const char* functionName);
	};

	void PrintLog(SourceLocation sourceLocation, const std::string& log);
#else
#define SOURCE_LOCATION 0
#define PrintLog(x, y) {}
#endif
}

namespace ATOMIC_UTIL
{
	template <class TYPE> _INLINE auto T_CAS(std::atomic<TYPE>* addr, TYPE oldValue, TYPE newValue) noexcept -> bool
	{
		return atomic_compare_exchange_strong(addr, &oldValue, newValue);
	};

	template <class TYPE> _INLINE auto T_CAS(volatile TYPE* addr, TYPE oldValue, TYPE newValue) noexcept -> bool
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<TYPE>*>(addr), &oldValue, newValue);
	};
}

namespace CONVERT_UTIL
{
	std::string ConvertFromUTF16ToMBCS(std::wstring utfString);
	std::wstring ConvertFromMBCSToUTF16(std::string mbcsString);
}