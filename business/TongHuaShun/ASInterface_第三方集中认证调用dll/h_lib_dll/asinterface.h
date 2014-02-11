#ifndef _ASINTERFACE_API_H
#define _ASINTERFACE_API_H
/** 核新软件集中认证接入动态库 v1.0.1
 *  版权所有 @ 2009 杭州核新软件技术股份有限公司
 */

#ifdef __cplusplus
extern "C" 
{
#endif

/**动态库初始化函数
 * 参数说明： pszLinkStr  连接字符串，用作认证使用，具体填入值如何认证后台确认，暂时可以填入asinterface
 *            pszAuthAddr 需要连接的集中认证服务器地址，可以同时连接多个，以此连接，直到连接上为止
 *                       格式：ip:port|ip2:port2...  如：192.168.0.1:9901|192.168.0.2|9902
 * 返回值：  0
 * 正常情况下之需要初始化一次，如果认证地址改变，那么可以重新调用此函数，以便连接不同的认证地址
 */
long  WINAPI ASInit(const char* pszLinkStr, const char* pszAuthAddr);


/**数据请求函数
 *参数说明： pszRequest    数据请求字符串，要求从ID 35开始，最后的校验位ID 10之间的数据
 *           pszReturnData 用来存放返回数据的内存空间，返回的数据也是从ID 35开始，到ID 10结束，不包含ID8、9、10的数据
 *           cbMaxSize     指明pszReturnData最大可以存放的数据长度，
 *           nTimeOut      请求超时时间，单位是毫秒，设置为0，则一直等待
 *返回值：  返回值 >= 0 为成功，数值代表返回的数据长度
 *          < 0 失败   -1 请求数据有误
                       -2 参数pszRequest为空字符串
					   -3 请求数据超时
*/
long WINAPI ASRequest(const char* pszRequest, char* pszReturnData, int cbMaxSize, int nTimeOut);

/**退出函数
动态调用在释放动态库前请调用该函数释放资源
*/
void WINAPI ASClose();

/**退出函数2
动态调用在释放动态库前请调用该函数释放资源
*/
void WINAPI ASClose2();

#ifdef __cplusplus
}
#endif
#endif
