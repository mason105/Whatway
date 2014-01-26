#ifndef _SYWG_H_
#define _SYWG_H_

#include "stdafx.h"
//typedef unsigned char       BYTE;
//typedef unsigned short      WORD;
//typedef unsigned long       DWORD;


#define EXTENSION_BUF_LEN 1024

#define BLOCK_TYPE_RESULT 3
#define BLOCK_TYPE_RETURN 2

#define ERR_CODE_NETWORK 1000
#define ERR_CODE_DES 1001
#define ERR_CODE_PACKAGE 1002

// 价格
typedef long	 SWISmallMoney;

// 货币
/*
typedef struct _SWIMoney            // Same as the type CURRENCY in VB
{
DWORD lo_value;
long  hi_value;
} SWIMoney;
*/
#pragma   pack(1)

typedef struct _SWIMoney    // Same as the type CURRENCY in VB
{
    DWORD lo_value;
    long  hi_value;
#ifdef WIN32
    operator double()
    {
        return 0.0001 * *((__int64*)this);
    }

    _SWIMoney& operator = (double v)
    {
        union{ __int64 i64;
        _SWIMoney  mny;
        } x;
        x.i64 = (__int64)(10000.0 * v + 0.1 * (v>=0?1:-1));
        lo_value = x.mny.lo_value;
        hi_value = x.mny.hi_value;
        return *this;
    }
#endif
    //#pragma pack(show)
}SWIMoney, FAR* PSWIMoney;


// 消息头
struct SWI_BlockHead
{
	WORD block_size;	// 整个消息块字节数
	WORD crc;			// 从下个字段block_type起 (不包括block_size和crc字段本身) 的CRC校验码
	BYTE block_type;	// 块类型  	1 – 请求         （SWI_???Request）
						//         	2 – 返回状态     （SWI_???Return）
						//          	3 – 查询结果行   （SWI_???Result）
						//          	5 – 取消查询
						//          	4 – 警报信息
						//          	5 – 网络对话
						//          	6 – 连接请求(交换密钥)
						//          	7 – 连接应答(交换密钥)
						//          	8 – 扩展请求消息
						//          	9 – 扩展应答消息
	BYTE reserved1;    	// 保留，必须置0
	WORD function_no;  	// 功能号
	long cn_id;      	// 网关连接号
	WORD reserved2;  	// 保留，内部使用（原营业部）
	WORD dest_dpt;    	// 目的营业部编号
};


struct SWI_ConnectRequest
{
	SWI_BlockHead head;    	// 消息头 block_type == 6
/*
消息头中：
block_type	6 – 连接请求(交换密钥)
		function_no  	忽略
		cn_id      	0
dest_dpt    	忽略
*/
	WORD method;     	   	// 客户端的加密方式
	char entrust_method;   	// 委托方式（见数据字典说明）
	BYTE return_cp;        	// 客户端希望的返回结构中中文字段的代码页
						     		//   	0 -- MBCS  （CP936简体中文）
						     		//   	1 -- Unicode(Java客户需要)
	BYTE network[4];      	// 客户端Novell网段号
	BYTE address[6];      	// 客户端网卡地址（Novell节点地址）
	WORD client_major_ver;	// 客户端协议接口主版本号（用于版本协商）
	WORD client_minor_ver;// 客户端协议接口次版本号（用于版本协商）
	WORD key_length;      // RSA密钥长度（对method 1, key_length=0）
};

struct SWI_ConnectResponse
{
	SWI_BlockHead  head;    	// 消息头 block_type == 7
/*
block_type	7
		function_no  	忽略
cn_id      	分配到的连接标识，客户端应缓存此连接标识，用于填充后续请求  中的相应字段
dest_dpt    	本地营业部编号
*/
    WORD return_value;		// 返回状态
	WORD method;             // 网关同意的加密方式
	char department_name[60];// 营业部名称（MBCS/Unicode）
	WORD key_length;         // DES密钥长度（key_length=8）
	BYTE des_key[8];         // DES密钥(请用“ExpressT”作为密钥des解密)
};

// 功能号901
struct SWI_ErrorMsgRequest
{
    struct SWI_BlockHead head;  // function_no==0x901, block_type==1
    long    error_no;           // 出错代码
};

struct SWI_ErrorMsgReturn
{
    struct SWI_BlockHead head;	//function_no==0x901,block_type==2
    long    return_status;      // 返回状态
    char    error_msg[40];      // 错误信息（MBCS/Unicode）
};
// 功能号901

// 功能号0x101
struct SWI_OpenAccountRequest
{
    struct SWI_BlockHead head;	    // function_no==0x101, block_type == 1
    char   account_type;     	    // 客户帐号类型
    char   account[16];      	    // 客户帐号
    char   pwd[8];           	    // 交易密码（如为操作员登陆，则为操作员密码）
    short  op_code;                 // 操作员工号
    unsigned  long   flag;          //"特殊条件"判断标志位组合,每一个二进制位对应一个"特殊条件",缺省值为0表示不判断任何"特殊条件"
    char   productno [7];		    //产品信息编号：5位电脑网络中心产品编码+2位子
    //产品编码（共7位字符）；缺省值为空。
    char   note[30];		        //备注：目前用于在客户委托时存放外围客户登陆的MAC地址 或IP 地址或电话号码等信息。
};

struct SWI_OpenAccountReturn
{
    struct SWI_BlockHead head; 	//  function_no==0x101
    long    return_status;      //  返回状态
    BYTE    flag;				//  (新扩充字段)大集中网关标志，0—否（默认）；1—是
    //  当客户号登录返回1时,适用于【大集中约定】
	char    last_login_date[9];  // 上次登录的日期，格式为：YYYYMMDD
	char    last_login_time[9];  // 上次登录的时间，格式为：HH:MM:SS
	char    last_full_note[60];  // 上次的登陆信息，参见注意事项5的说明
	char    reserved_auth_info[20];    // 客户预留验证信息
};
// 功能号0x101


// 功能号0x111
struct SWI_AccountLoginRequest
{
    struct SWI_BlockHead head;	// function_no==0x111, block_type == 1
    char   account_type;     	// 客户帐号类型（见数据字典说明）
    char   account[16];      	// 客户帐号
    char   pwd[8];           	// 交易密码
    unsigned  long   flag;      //"特殊条件"判断标志位组合,每一个二进制位对应一个"特殊条件",缺省值为0表示不判断任何"特殊条件"
    char   productno [7];		//产品信息编号：5位电脑网络中心产品编码+2位子
    //产品编码（共7位字符）；缺省值为空。
    char   note[30];	        //备注：目前用于在客户委托时存放外围客户登陆的MAC地址 或IP 地址或电话号码等信息。

};

struct SWI_AccountLoginResult
{
    struct SWI_BlockHead head;     	// function==0x111,block_type==3
    WORD   row_no;          	    // 记录号，0xFFFF表示记录集结束
    long   bankbook_number;         // 资金帐号
    char   account_type;  			// 帐号类型（见数据字典, 不含资金帐号‘0’）
    char   security_account[16];	// 证券帐号
};


struct SWI_AccountLoginReturn
{
    struct SWI_BlockHead head;		// function_no = 0x111,block_type==2
    long   return_status;     		// 返回状态
    long    bankbook_number;     	// 资金帐号
    BYTE    account_status;      	// 帐号状态（见数据字典说明）
    char    name[20];            	// 客户姓名（MBCS/Unicode）
    char    id[19];      			// 身份证号
    BYTE    card_version;          	//磁卡版本号
    char    customer_flag[16];      //客户个性化信息标志
    char	Cust_flag[32];		    //客户权限标志（字段中如出现R——允许融资融券）
    char	Cust_risk_type;		    //客户风险评级类别（见数据字典说明）
	short   depart_number;			//营业部编码（4位）
	char    last_login_date[9];     // 上次登录的日期，格式为：YYYYMMDD
	char    last_login_time[9];     // 上次登录的时间，格式为：HH:MM:SS
	char    last_full_note[60];     // 上次的登陆信息，参见0x101注意事项5的说明
	char    reserved_auth_info[20];     // 客户预留验证信息
};
// 功能号0x111


struct SWI_AddOTCEntrustRequest
{
    struct SWI_BlockHead head;	// function_no==0x2601, block_type == 1
    
    char    account[16];      // 登录客户资金账号
	char    bs_type;          // 买卖类别
                          // ‘1’ ----- 买入
	// ‘2’ ----- 卖出
							  // ‘7’ ----- 认购
	// ‘c’ ----- 申购
	// ‘d’ ----- 赎回
	char    ta_code[8];       // 登记机构代码
		char    security_code[7]; // 产品代码
		SWIMoney	   fund_amount;  //委托数量(支持开放式基金小数份额)
		SWISmallMoney price;      // 委托价格（SWISmallMoney见数据字典说明，下同）
	short   effect_days;      // 有效天数
	SWIMoney   apply_amount;  //申请金额 
	BYTE    mark;             //是否连续赎回 1为连续赎回,0为非连续 确省为0
	char 	MarketOrder_type[2]; // 价格类型 默认为空
	//‘11’——  成交申报
	//‘12’——  意向申报
	char 	match_type;     //  意向约定
								  //  ‘0’: 可部分成交
	//  ‘1’: 不可部分成交
	long 	contract_no;	    //约定号（价格类型为‘11’时需要输入）
	char    contact_name[30];  //联系人名称
	char 	contact_tel [30];   //联系方式
	char    Entrust_sign[10];   //交易签名(缺省为’’)


};

struct SWI_AddOTCEntrustReturn
{
	struct SWI_BlockHead head;
	long return_status;   	      	//  返回状态
	long entrust_sn;      			//  委托编号
};


#pragma   pack()

#endif
