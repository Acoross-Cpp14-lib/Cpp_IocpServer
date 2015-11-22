#ifndef _ACOROSS_MEMORY_MEMORYMANAGER_H
#define _ACOROSS_MEMORY_MEMORYMANAGER_H

#pragma once

#include <new>

namespace Acoross {
	namespace Memory
	{
		// memory manager test
		class MemoryManager
		{
		public:
			MemoryManager(MemoryManager&) = delete;
			MemoryManager& operator=(MemoryManager&) = delete;

			static MemoryManager& Inst()
			{
				static MemoryManager inst;

				return inst;
			}

			void* malloc(size_t _Size)
			{
				return ::malloc(_Size);
			}

			void free(void* _Block)
			{
				::free(_Block);
			}

		private:
			MemoryManager() = default;
		};
	}
}

void* operator new(size_t _Size);
void operator delete(void* _Block);

#endif //_ACOROSS_MEMORY_MEMORYMANAGER_H