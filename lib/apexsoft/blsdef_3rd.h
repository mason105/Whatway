/***********************************************************************
*         Copyright (C) 2004  福建顶点软件有限公司 All rights reserved. 
*
*文件名称：blsdef.h
*项目名称：
*摘要：    BLS定义c/c++编程用头文件。    
*
*原作者：  顶点企业级证券集中交易系统ABOSS2
*创建日期：2007.05.18
*备注：
*
*历史修改记录：
*
*序号    日期             版本                作者                         
*1.      2007.05.18       v1.0.0.1            顶点企业级证券集中交易系统ABOSS2
*    创建。
*
**********************************************************************/

#if !defined(_BLSDEF_DEFINE_H)
#define _BLSDEF_DEFINE_H

#include "fiddef.h"  //信息元定义


#define  FUN_SECU_LIST_SFBZ_ZHYW         104010  //查询帐户类业务收费标准
#define  FUN_SECU_LIST_QZXX              104027  //查询权证信息
#define  FUN_QUOTE_GETHQ                 104101  //证券行情查询
#define  FUN_SECU_LIST_ETFXX             104105  //查询ETF基本信息
#define  FUN_OFS_LIST_TAXX               105001  //查询基金公司参数
#define  FUN_OFS_LIST_JJXX               105002  //查询基金代码参数
#define  FUN_OFS_LIST_DQDESGPZ           105008  //查询定期定额申购品种
#define  FUN_CUSTOM_CHKTRDPWD            190101  //交易密码效验
#define  FUN_CUSTOM_MODI_TRDPWD          202010  //客户交易密码修改
#define  FUN_CUSTOM_MODI_FUNDPWD         202012  //客户资金密码修改
#define  FUN_WBZH_REG_ASSOCIATE_REQ      203101  //登记银证转帐对应关系申请
#define  FUN_WBZH_DESTROY_ASSOCIATE_REQ  203103  //注销银证转帐对应关系申请
#define  FUN_WBZH_TRANSFER_SAVING_ZQ     203111  //证券发起资金存入(银行转证券)
#define  FUN_WBZH_TRANSFER_TAKEOUT_ZQ    203113  //证券发起资金取出(证券转银行)
#define  FUN_WBZH_TRANSFER_SAVING_WB     203131  //处理外部发起资金转入(银转证)业务
#define  FUN_WBZH_TRANSFER_TAKEOUT_WB    203133  //处理外部发起资金转出(证转银)业务
#define  FUN_WBZH_YZT_GET_BALANCE        203201  //查询外部结算类帐户余额
#define  FUN_FUND_ACCESS                 203501  //资金存取
#define  FUN_FUND_BOOKING_ACCESS         203515  //超额存取预约
#define  FUN_SECU_MODI_GDH               204002  //股东帐户信息修改
#define  FUN_SECU_ENTRUST_TRADE          204501  //股票买卖委托
#define  FUN_SECU_ENTRUST_WITHDRAW       204502  //股票委托撤单
#define  FUN_SECU_CAL_TRADEABLEAMOUNT    204503  //可买卖数量计算
#define  FUN_SECU_BAT_ENTRUST_BUY        204509  //批量买入委托
#define  FUN_SECU_BAT_ENTRUST_SALE       204510  //批量卖出委托
#define  FUN_SECU_BAT_ENTRUST_WITHDRAW   204511  //批量撤单
#define  FUN_SECU_MIXBAT_ENTRUST_TRADE   204513  //混合批量委托买卖
#define  FUN_SECU_ETF_BOOKING_ENTRUST    204518  //ETF网下预约委托
#define  FUN_SECU_ETF_WITHDRAW_WXYYWT    204519  //ETF网下预约委托撤单
#define  FUN_OFS_JJZHKH_ZZ               205004  //基金帐户自助开户
#define  FUN_OFS_CXYYWT                  205010  //撤消基金预约委托
#define  FUN_OFS_CXDRWT                  205011  //撤消基金当日委托
#define  FUN_OFS_JJWT                    205021  //基金认购申购赎回
#define  FUN_OFS_SZFHFS                  205022  //设置分红方式
#define  FUN_OFS_JJZH                    205025  //基金转换
#define  FUN_CUSTOM_GET_CUSTINFO         302001  //查询指定客户基本信息
#define  FUN_CUSTOM_LIST_EXINFO          302005  //查询客户附加属性
#define  FUN_ACCOUNT_GETZJXXBYZJZH       303001  //按资金帐号查询资金信息
#define  FUN_ACCOUNT_LIST_ZJXXBYKHH      303002  //按客户号查询资金
#define  FUN_ACCOUNT_LIST_ZJMX           303010  //查询客户资金明细
#define  FUN_WBZH_LIST_YZZZDY            303103  //查询银证转帐对应关系
#define  FUN_WBZH_LIST_ZJJYSQ_ZQ         303111  //查询证券发起资金交易申请
#define  FUN_WBZH_LIST_ZJJYSQ_WB         303112  //查询外部发起资金交易申请
#define  FUN_SECU_LIST_GDHBYKHH          304001  //客户股东号查询
#define  FUN_SECU_GETKHHBYGDH            304003  //根据股东帐号获取客户号
#define  FUN_SECU_LIST_HOLDSTOCK         304101  //客户持仓查询
#define  FUN_SECU_LIST_ENTRUST           304103  //客户当日委托查询
#define  FUN_SECU_LIST_ETFWXRGWT         304108  //查询客户ETF网下认购委托
#define  FUN_SECU_LIST_SSCJ              304109  //客户实时成交查询
#define  FUN_SECU_LIST_FBCJ              304110  //客户分笔成交查询
#define  FUN_SECU_LIST_ETFCFG_HOLDSTOCK  304119  //查询客户ETF成份股的持仓信息
#define  FUN_OFS_LIST_JJZHBYKHH          305001  //查询基金帐户信息
#define  FUN_OFS_LIST_JJFE               305002  //查询客户基金份额
#define  FUN_OFS_LIST_JJWT               305003  //查询客户基金委托
#define  FUN_OFS_GETKHHBYJJZH            305004  //按基金帐号查询客户号
#define  FUN_ACCOUNT_LIST_ZJMXLS_KH      403201  //客户资金明细历史查询
#define  FUN_SECU_LIST_JGMXLS_KH         404201  //客户交割明细历史查询
#define  FUN_SECU_LIST_WTLS_KH           404202  //客户委托历史查询
#define  FUN_OFS_LIST_JJJGLS             405003  //查询客户历史基金交割
#endif // !defined(_BLSDEF_DEFINE_H)
