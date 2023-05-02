#include "ntp.h"

Event<struct timeval*> NTP::OnSync; 
bool NTP::NtpReceived = false;

bool NTP::Init(const std::string host)
{
	//setenv("TZ", timezone.c_str(), 1);
		//tzset();
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, host.c_str());
	sntp_init();
	sntp_set_time_sync_notification_cb(OnSyncCallback);
	return true;
}

void NTP::OnSyncCallback(struct timeval* tv)
{
	NtpReceived = true;
	OnSync.Invoke(tv);
}

void NTP::SetSystemTime(DateTime* value)
{
	struct timeval now;
	value->Get(&now);
	settimeofday(&now, NULL);
}

void NTP::SetTimeZone(std::string timeZone)
{
	setenv("TZ", timeZone.c_str(), 1);
	tzset();
}

bool NTP::Received()
{
	return NtpReceived;
}
