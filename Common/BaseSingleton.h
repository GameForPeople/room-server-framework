#pragma once

inline namespace WONSY_BASE
{
	template <typename T>
	class TSingleton
	{
	public:
		__forceinline static T& GetInstance()
		{
			static T instance;
			return instance;
		}
	protected:
		TSingleton() {}
		~TSingleton() {}
	public:
		TSingleton(TSingleton const&) = delete;
		TSingleton& operator=(TSingleton const&) = delete;
	};
};