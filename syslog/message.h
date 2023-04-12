#pragma once
#include "esp_base.h"
#include "facilities.h"
#include "levels.h"

using namespace ESP_Base;

namespace ESP_Com
{
	
	namespace Syslog
	{


		struct Message
		{
			Facilities Facility = Facilities::user;
			Levels Level = Levels::Debug;
			DateTime Timestamp;
			std::string Hostname		  = "-";
			std::string Application		  = "-";
			std::string ProcessId		  = "-";
			std::string MessageId		  = "-";
			std::string StructuredData 	  = "-";
			std::string Message			  = "-";
			

			
			std::string ToString()
			{
				int priority = (int)Facility * 8 + (int)Level;
				
				std::string result;
				
				result.append("<");
				result.append(std::to_string(priority));
				result.append(">1 ");
				
				result.append(Timestamp.ToString());
				result.append(" ");
				
				result.append(Hostname);
				result.append(" ");
				
				result.append(Application);
				result.append(" ");
				
				result.append(ProcessId);
				result.append(" ");
				
				result.append(MessageId);
				result.append(" ");
				
				result.append(StructuredData);
				result.append(" ");
				
				result.append(Message);
				
				return result;
			}
		};
	}
}