#ifndef ERROR_CODE_MSG_H
#define ERROR_CODE_MSG_H

#include <map>
#include <string>
#include <boost/thread/detail/singleton.hpp>

#define CONNECT_COUNTER_ERROR 1000
#define CONNECT_ALL_COUNTER_ERROR 1001
#define PARAM_ERROR 1002
#define MSG_HEADER_ERROR 1003
#define CONFIG_ERROR 1004
#define BUSI_CRYPT_ERROR 1005

class Error
{
private:
	std::map<int, std::string> err;

public:
	void init()
	{
		err[CONNECT_COUNTER_ERROR] = "连接柜台失败";
		err[CONNECT_ALL_COUNTER_ERROR] = "连接所有柜台失败";
		err[PARAM_ERROR] = "参数错误";
		err[MSG_HEADER_ERROR] = "消息头错误";
		err[CONFIG_ERROR] = "配置文件错误";
		err[BUSI_CRYPT_ERROR] = "业务层加解密错误";
	}

	std::string GetErrMsg(int errCode)
	{
		return err[errCode];
	}
};
typedef boost::detail::thread::singleton<Error> gError;

#endif
