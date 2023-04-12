#pragma once
#include <string>
#include "esp_base.h"

class IPartition
{
public:
	size_t totalSize;
	size_t sectorSize;

	virtual esp_err_t EraseRange(size_t offset, size_t size) = 0;
	virtual esp_err_t Write(size_t offset, const void* src, size_t size) = 0;
	virtual esp_err_t Read(size_t offset, void* dst, size_t size) = 0;
		
};
