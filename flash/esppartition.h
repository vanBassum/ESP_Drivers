#pragma once
#include <string>
#include "esp_base.h"
#include "esp_partition.h"
#include "ipartition.h"

	
struct PartitionSettings
{
	esp_partition_type_t type;
	esp_partition_subtype_t subtype;
	const char *label;
};

class Partition : public IPartition
{
	const esp_partition_t * handle = NULL;
public:
		
	bool Init(const PartitionSettings* settings) 
	{
		handle = esp_partition_find_first(settings->type, settings->subtype, settings->label);
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
	