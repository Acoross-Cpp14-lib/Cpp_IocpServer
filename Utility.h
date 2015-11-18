#pragma once

#include <mutex>

#ifndef _UTILITY_H_
#define _UTILITY_H_

#define NO_COPY(T) \
	T(T&) = delete;\
	T(T&&) = delete;\
	T& operator=(T&) = delete;\
	T& operator=(T&&) = delete;

namespace Acoross {
	class CLog
	{
	public:
		void Add(const wchar_t* format)
		{
			wprintf_s(format);
		}

		template <class... Args>
		void Add(const wchar_t* format, Args&&... args)
		{
			wprintf_s(format, std::forward<Args>(args)...);
		}

		void Add(const char* format)
		{
			printf_s(format);
		}

		template <class... Args>
		void Add(const char* format, Args&&... args)
		{
			printf_s(format, std::forward<Args>(args)...);
		}
	};

	extern CLog Log;

	class GuardLock
	{
	public:
		GuardLock(std::mutex& Lock)
		{
			pLock = &Lock;
			pLock->lock();
		}
		~GuardLock()
		{
			pLock->unlock();
		}
	private:
		std::mutex* pLock;
	};
}//Acoross

#endif //_UTILITY_H_