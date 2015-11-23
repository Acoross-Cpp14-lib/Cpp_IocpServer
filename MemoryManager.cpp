#include "MemoryManager.h"

void* operator new(size_t _Size)
{
	//printf("overload new\n");
	return Acoross::Memory::MemoryManager::Inst().malloc(_Size);
}

void operator delete(void* _Block)
{
	//printf("overload delete\n");
	Acoross::Memory::MemoryManager::Inst().free(_Block);
}