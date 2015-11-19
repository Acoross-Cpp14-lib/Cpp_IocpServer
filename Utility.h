#pragma once

#include <mutex>

#ifndef _UTILITY_H_
#define _UTILITY_H_

#define NO_COPY(T) \
	T(T&) = delete;\
	T& operator=(T&) = delete;\

namespace Acoross {
		
	template <class T>
	class AutoLocker
		: protected T
	{
	public:
		template<class... Args>
		AutoLocker(Args&&... args) : T(std::forward<Args>(args)...)
		{}

		template <class F>
		auto LockedProcess(F func)
		{
			GuardLock gLock(m_lock);
			return func((T&)(*this));
		}

	private:
		std::mutex m_lock;
	};

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