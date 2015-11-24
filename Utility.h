#ifndef _UTILITY_H_
#define _UTILITY_H_

#pragma once

#include <mutex>

#define NO_COPY(T) \
	T(T&) = delete;\
	T& operator=(T&) = delete;\

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


	template<class T>
	class locker
	{
		class locked;

	public:
		locker(T&& Obj)
			: m_Obj(std::move(Obj))
		{
		}

		locker(locker&& rhs)
			: m_Obj(std::move(rhs.m_Obj)), lock(std::move(rhs.lock))
		{
		}

		locked operator->()
		{
			return Locked(&m_Obj, &lock);
		}

	private:
		T m_Obj;
		std::mutex lock;

		class locked
		{
		public:
			locked(T* pObj, std::mutex* pLock)
				: m_pObj(pObj), m_pLock(pLock)
			{
				m_pLock->lock();
			}

			~locked()
			{
				m_pLock->unlock();
			}

			T* operator->()
			{
				return m_pObj;
			}

		private:
			T* m_pObj;
			std::mutex* m_pLock;
		};
	};

	template<class T, class... Args>
	locker<T> make_locker(Args&&... args)
	{
		return locker<T>(T(std::forward<Args>(args)...));
	}

}//Acoross

#endif //_UTILITY_H_