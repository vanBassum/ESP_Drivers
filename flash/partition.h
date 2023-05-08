#pragma once
#include <string>
#include "esp_base.h"
#include "esp_partition.h"

class Partition
{
public:
	size_t totalSize;
	size_t sectorSize;
	
	const esp_partition_t * handle = NULL;
	bool Init(const char* label, const esp_partition_type_t type, const esp_partition_subtype_t subtype) 
	{
		handle = esp_partition_find_first(type, subtype, label);
		if (handle == NULL)
			return false;
			
		totalSize = handle->size;
		sectorSize = handle->erase_size;
		return true;
	}
		
	esp_err_t EraseRange(size_t offset, size_t size)
	{
		return esp_partition_erase_range(handle, offset, size);
	}
		
	esp_err_t Read(size_t offset, void* dst, size_t size)
	{
		return esp_partition_read(handle, offset, dst, size);
	}
		
	esp_err_t Write(size_t offset, const void* src, size_t size)
	{
		return esp_partition_write(handle, offset, src, size);
	}
};
	