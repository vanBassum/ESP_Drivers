#pragma once
#include <string>
#include "esp_base.h"
#include "esp_partition.h"

class IPartition
{
public:
	size_t sectorSize;
	size_t totalSize;
	
	virtual bool Read(void* dst, size_t offset, size_t length) = 0;
	virtual bool Write(const void* src, size_t offset, size_t length) = 0;
	virtual bool Erase(size_t offset, size_t length) = 0;
};

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
		
	bool Erase(size_t offset, size_t length) override
	{
		esp_err_t result = esp_partition_erase_range(handle, offset, length);
		if (result != ESP_OK)
			ESP_LOGE("Partition", "Erase error %d => %s", result, esp_err_to_name(result));
		return result == ESP_OK;
	}
		
	bool Read(void* dst, size_t offset, size_t length) override
	{
		esp_err_t result = esp_partition_read(handle, offset, dst, length);
		if (result != ESP_OK)
			ESP_LOGE("Partition", "Read error %d => %s", result, esp_err_to_name(result));
		return result == ESP_OK;
	}
		
	bool Write(const void* src, size_t offset, size_t length) override
	{
		esp_err_t result = esp_partition_write(handle, offset, src, length);
		if (result != ESP_OK)
			ESP_LOGE("Partition", "Write error %d => %s", result, esp_err_to_name(result));
		return result == ESP_OK;
	}
};
	

class PartitionRange : public IPartition
{
	IPartition* parent;
	size_t start;
public:
	bool Init(IPartition* parent, const size_t start, const size_t length)
	{
		this->start = start;
		this->parent = parent;
		this->sectorSize = parent->sectorSize;
		this->totalSize = length;
		return true;
	}
	
	
	bool Read(void* dst, size_t offset, size_t length)
	{
		if (offset + start + length > parent->totalSize) 
		{
			ESP_LOGE("PartitionRange", "Read Out of range: %d + %d + %d > %d", offset, start, length, parent->totalSize);
			return false;
		}
		return parent->Read(dst, offset + start, length);
	}
	bool Write(const void* src, size_t offset, size_t length)
	{
		if (offset + start + length > parent->totalSize)	
		{
			ESP_LOGE("PartitionRange", "Write Out of range: %d + %d + %d > %d", offset, start, length, parent->totalSize);
			return false;
		}
		
		return parent->Write(src, offset + start, length);
	}
	bool Erase(size_t offset, size_t length)
	{
		if (offset + start + length > parent->totalSize)	
		{
			ESP_LOGE("PartitionRange", "Erase Out of range: %d + %d + %d > %d", offset, start, length, parent->totalSize);
			return false;
		}
		return parent->Erase(offset + start, length);
	}
};
