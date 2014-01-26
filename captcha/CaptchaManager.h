#ifndef _CAPTCHA_MANAGER_
#define _CAPTCHA_MANAGER_

#include <map>
#include <boost/random.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <boost/thread/detail/singleton.hpp>

// 散列映射
#include <boost/unordered_map.hpp>

#include <boost/thread.hpp>


class CaptchaManager
{
public:
	// 产生4位随机数
	std::string GetCaptcha();
	bool VerifyCaptcha(std::string captcha);

	template<typename Rng>
	std::string rand_bytes(int buf_len);
	

private:
	boost::unordered_map<std::string, time_t> captcha_map;

	boost::thread * thrd;

public:
	void ClearExpireCaptcha();
	void StartClearExpireCaptchaThread();
	void StopClearExpireCaptchaThread();
};

typedef boost::detail::thread::singleton<CaptchaManager> captcha;

#endif
