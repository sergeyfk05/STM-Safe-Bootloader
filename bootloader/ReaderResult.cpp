#include "ReaderResult.h"

namespace Firmware
{
	ReaderResult::ReaderResult(OperationType type)
	{
		sizeBuffer = type;
		data = malloc(type);
	}
}