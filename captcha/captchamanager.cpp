#include "captchamanager.h"

#include <boost/random.hpp>
#include <boost/typeof/typeof.hpp>
#include <afx.h>

#include "./config/ConfigManager.h"


std::string CaptchaManager::GetCaptcha()
{
	
	std::string captcha = rand_bytes<boost::random::mt19937>(4);
	
	
	return captcha;
}

bool CaptchaManager::VerifyCaptcha(std::string captcha)
{
	// 比对成功，要删除
	// 比对失败，也要删除
	if (captcha.length() < 4)
		return false;

	

	boost::unordered_map<std::string, time_t>::iterator iter = captcha_map.find(captcha);  
	if (iter != captcha_map.end())
	{
		//captcha_map.erase(iter);
		return true;
	}

	return false;
}


void CaptchaManager::StartClearExpireCaptchaThread()
{
	//thrd = new boost::thread( boost::bind(&CaptchaManager::ClearExpireCaptcha, &captcha::instance()) );
}

void CaptchaManager::StopClearExpireCaptchaThread()
{
//	if (thrd != NULL)
//		delete thrd;
}

void CaptchaManager::ClearExpireCaptcha()
{
	while(1)
	{
		

		time_t now = time(NULL);

		BOOST_AUTO(it, captcha_map.begin());
		for (it; it != captcha_map.end(); )
		{
				OutputDebugString("clear expire captcha ... ");
				OutputDebugString(it->first.c_str());
				OutputDebugString("\n");

				if ( (now - it->second) >= gConfigManager::instance().captcha_timeout) // 5s
			{
				OutputDebugString("erase...\n");
				//it = captcha_map.erase(it);
				it = captcha_map.erase(it);

			}

			if (it != captcha_map.end())
				it++;
		}


		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
}
