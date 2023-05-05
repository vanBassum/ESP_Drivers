#pragma once
#include "partition.h"
#include "nvs_flash.h"
#include "esp_log.h"

class NVS
{
public:
	static bool InitDefaultPartition()
	{
		esp_err_t ret = nvs_flash_init();
		if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}
		return ret == ESP_OK;
	}
};


class NVSPartition
{
	nvs_handle_t handle;
		
public:
	bool Init(const std::string& partitionLabel, const std::string& nvsNamespace)											
	{ 
		esp_err_t err = nvs_flash_init_partition(partitionLabel.c_str());
		if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
			// NVS partition was truncated and needs to be erased
			// Retry nvs_flash_init
			ESP_ERROR_CHECK(nvs_flash_erase());
			err = nvs_flash_init_partition(partitionLabel.c_str());
		}
		if (err == ESP_OK)
			err = nvs_open_from_partition(partitionLabel.c_str(), nvsNamespace.c_str(), NVS_READWRITE, &handle);
		return err == ESP_OK; 
	}
	
	bool SetI8(const std::string& key, int8_t value)                            { return nvs_set_i8(handle, key.c_str(), value) == ESP_OK; }
	bool SetU8(const std::string& key, uint8_t value)                           { return nvs_set_u8(handle, key.c_str(), value) == ESP_OK; }
	bool SetI16(const std::string& key, int16_t value)                          { return nvs_set_i16(handle, key.c_str(), value) == ESP_OK; }
	bool SetU16(const std::string& key, uint16_t value)                         { return nvs_set_u16(handle, key.c_str(), value) == ESP_OK; }
	bool SetI32(const std::string& key, int32_t value)                          { return nvs_set_i32(handle, key.c_str(), value) == ESP_OK; }
	bool SetU32(const std::string& key, uint32_t value)                         { return nvs_set_u32(handle, key.c_str(), value) == ESP_OK; }
	bool SetI64(const std::string& key, int64_t value)                          { return nvs_set_i64(handle, key.c_str(), value) == ESP_OK; }
	bool SetU64(const std::string& key, uint64_t value)                         { return nvs_set_u64(handle, key.c_str(), value) == ESP_OK; }
	bool SetStr(const std::string& key, const std::string& value)				{ return nvs_set_str(handle, key.c_str(), value.c_str()) == ESP_OK; }
	bool SetBlob(const std::string& key, const void* value, size_t length)		{ return nvs_set_blob(handle, key.c_str(), value, length) == ESP_OK; }
	bool GetI8(const std::string& key, int8_t* out_value)                       { return nvs_get_i8(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetU8(const std::string& key, uint8_t* out_value)                      { return nvs_get_u8(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetI16(const std::string& key, int16_t* out_value)                     { return nvs_get_i16(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetU16(const std::string& key, uint16_t* out_value)                    { return nvs_get_u16(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetI32(const std::string& key, int32_t* out_value)                     { return nvs_get_i32(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetU32(const std::string& key, uint32_t* out_value)                    { return nvs_get_u32(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetI64(const std::string& key, int64_t* out_value)                     { return nvs_get_i64(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetU64(const std::string& key, uint64_t* out_value)                    { return nvs_get_u64(handle, key.c_str(), out_value) == ESP_OK; }
	bool GetBlob(const std::string& key, void* out_value, size_t* length)		{ return nvs_get_blob(handle, key.c_str(), out_value, length) == ESP_OK; }
	bool EraseKey(const std::string& key)                                       { return nvs_erase_key(handle, key.c_str()) == ESP_OK; }
	bool EraseAll()                                                             { return nvs_erase_all(handle) == ESP_OK; }
	bool Commit()                                                               { return nvs_commit(handle) == ESP_OK; }
	void Close()                                                                { nvs_close(handle); }
	
	bool GetStr(const std::string& key, std::string* out_value)
	{ 
		size_t length;
		if (nvs_get_str(handle, key.c_str(), NULL, &length) == ESP_OK)
		{
			char temp[length];
			if (nvs_get_str(handle, key.c_str(), temp, &length) == ESP_OK)
			{
				*out_value = std::string(temp);
				return true;
			}
		}
		return false;
	}
};

