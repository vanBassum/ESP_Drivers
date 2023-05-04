#pragma once
#include <string>
#include "esp_base.h"
#include "esp_partition.h"
#include "ipartition.h"

class Partition : public IPartition
{
	
public:
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
		
	virtual esp_err_t EraseRange(size_t offset, size_t size) override
	{
		return esp_partition_erase_range(handle, offset, size);
	}
		
	virtual esp_err_t Read(size_t offset, void* dst, size_t size) override
	{
		return esp_partition_read(handle, offset, dst, size);
	}
		
	virtual esp_err_t Write(size_t offset, const void* src, size_t size) override
	{
		return esp_partition_write(handle, offset, src, size);
	}
};
	