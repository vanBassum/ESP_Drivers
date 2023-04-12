#pragma once


namespace ESP_Com
{
	
	namespace Syslog
	{
		enum class Facilities 
		{
			kern = 0,    
            user = 1,    
            mail = 2,    
            daemon = 3,  
            auth = 4,    
            syslog = 5,  
            lpr  = 6,    
            news = 7,    
            uucp = 8,    
            cron = 9,    
            authpriv = 10,    
            ftp = 11,    		
            ntp = 12,    		
            security = 13,    
            console = 14,    
            solaris_cron = 15,
            local0 = 16,    	
            local1 = 17,    	
            local2 = 18,    	
            local3 = 19,    	
            local4 = 20,    	
            local5 = 21,    	
            local6 = 22,    	
            local7 = 23,  
		};
	}
}