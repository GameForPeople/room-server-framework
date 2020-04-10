#include "stdafx.h"

#include "MemoryUnit.h"

MemoryUnit::MemoryUnit(const MEMORY_UNIT_TYPE memoryUnitType)
	: overlapped()
	, wsaBuffer()
	, memoryUnitType(memoryUnitType)
	, dataBuffer("\0")
{
	if (memoryUnitType == MEMORY_UNIT_TYPE::RECV_FROM_CLIENT) //|| memoryUnitType == MEMORY_UNIT_TYPE::RECV_FROM_QUERY)
	{
		wsaBuffer.len = DATA_BUFFER_SIZE;
	}

	wsaBuffer.buf = dataBuffer;
}

MemoryUnit::~MemoryUnit()
{
	//	delete[] dataBuffer;
}

SendMemoryUnit::SendMemoryUnit()
	: MemoryUnit(MEMORY_UNIT_TYPE::SEND_TO_CLIENT)
	, toSocket()
{
}
