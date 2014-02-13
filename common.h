#ifndef _COMMON_H_
#define _COMMON_H_

// 柜台类型
#define COUNTER_TYPE_UNKNOWN 0
#define COUNTER_TYPE_HS_T2 1
#define COUNTER_TYPE_HS_COM 2
#define COUNTER_TYPE_JZ_WIN 3
#define COUNTER_TYPE_JZ_LINUX 4
#define COUNTER_TYPE_DINGDIAN 5
#define COUNTER_TYPE_JSD 6
#define COUNTER_TYPE_XINYI 7


// 业务类型
//<!-- 业务类型：0.通用, 1.证券交易, 2.融资融券, 3.网上开户 4.账户系统, 5.统一认证， 6.期权, 7.验签-->
#define BUSI_TYPE_ALL 0
#define BUSI_TYPE_STOCK 1
#define BUSI_TYPE_CREDIT 2
#define BUSI_TYPE_REGISTER 3
#define BUSI_TYPE_ACCOUNT 4
#define BUSI_TYPE_AUTH 5
#define BUSI_TYPE_OPTION 6
#define BUSI_TYPE_VERIFY 7


typedef struct func
{
	bool isQuery;
	bool hasResultRet;
}FUNCTION_DESC;



#endif
