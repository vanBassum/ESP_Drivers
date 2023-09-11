#include "ntp.h"

Event<void, struct timeval*> NTP::OnSync; 
bool NTP::NtpReceived = false;

bool NTP::Init(const std::string& host, const TimeSpan timeSpan)
{
	//setenv("TZ", timezone.c_str(), 1);
		//tzset();
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, host.c_str());
	sntp_set_sync_interval(timeSpan.GetTotalMiliSeconds());
	sntp_init();
	sntp_set_time_sync_notification_cb(OnSyncCallback);
	return true;
}

void NTP::OnSyncCallback(struct timeval* tv)
{
	NtpReceived = true;
	OnSync.Invoke(NULL, tv);
}

void NTP::SetSystemTime(DateTime* value)
{
	//struct timezone tz;
	struct timeval now;
	//gettimeofday(&now, &tz);	//Just to get the timezone
	value->GetAsUTC(&now);
	settimeofday(&now, NULL);
}

void NTP::SetTimeZone(const std::string& timeZone)
{
	setenv("TZ", timeZone.c_str(), 1);
	tzset();
}

bool NTP::Received()
{
	return NtpReceived;
}
