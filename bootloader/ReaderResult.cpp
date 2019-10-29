#include "ReaderResult.h"

namespace Firmware
{
	ReaderResult::ReaderResult(OperationType type)
	{
		data = malloc(type);
	}
	ReaderResult::~ReaderResult()
	{
		free(data);
		
	}
}