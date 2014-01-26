/*
 * 顶点中间服务平台外围接入头文件 (C++语言标准接口文件)
 * FixApi.h   2007年4月 创建
 * 与FixApi.lib FixApi.dll一并提供。
 * 适合ABOSS2.0外围接入。 
*/

#if !defined(AFX_FIXAPI_H__VER2001__INCLUDED_)
#define AFX_FIXAPI_H__VER2001__INCLUDED_

#ifdef USE_FIXAPI_CLASS
    #include <FixApi/FixConn.h>
#endif

#ifndef FIX_API_EXPORT
    #define FIX_API_EXPORT  __declspec(dllexport)        
#endif

//连接句柄
#define HANDLE_CONN     long

//会话句柄
#define HANDLE_SESSION  long

//服务句柄
#define HANDLE_SVC      long

/*  函数名称: Fix_Initialize
              第三方接口库初始化。只有调用一次就可以，不能多次调用。
    返回数据: 
              返回True表示初始化成功；False表示失败。
*/
    BOOL FIX_API_EXPORT Fix_Initialize();
    BOOL FIX_API_EXPORT Fix_SetAppInfo( const char *pszAppName, const char *pszVer );
/*  函数名称: Fix_Uninitialize
              第三方接口库卸载。只有调用一次就可以，不能多次调用。
    返回数据: 
              返回True表示初始化成功；False表示失败。
*/
    BOOL FIX_API_EXPORT Fix_Uninitialize();

/*
  函数名称: Fix_SetDefaultInfo
            设置每个业务请求包缺省的头信息
  参数说明: 
            pszUser     -- [in] 系统要求的柜员代码(八位的字符串)。
            pszWTFS     -- [in] 委托方式。
            pszFBDM     -- [in] 发生营业部的代码信息(四位的字符串)
            pszDestFBDM -- [in] 目标营业部的代码信息(四位的字符串)

  返回数据: 
            系统返回类型为HANDLE_CONN的连接句柄。如果连接失败则返回0; 成功不为0;
*/
    FIX_API_EXPORT BOOL Fix_SetDefaultInfo( const char *pszUser, const char *pszWTFS, const char *pszFBDM, const char *pszDestFBDM );

/*
 *	函数名称:Fix_SetDefaultQYBM
    设置缺省的区域代码
 */
    FIX_API_EXPORT BOOL Fix_SetDefaultQYBM( const char *pszQYBM );
/*
  函数名称: Fix_Connect
            连接到顶点中间件服务器。
  参数说明: 
            pszAddr  -- [in] 为要连接的服务器地址; 格式为: "ip地址@端口/tcp"
            pszUser  -- [in]  通信用户名称; 由客户提供。
            pszPwd   -- [in] 通信用户的密码; 由客户提供。
            nTimeOut -- [in] 连接超时的时间。单位为秒。

  返回数据: 
            系统返回类型为HANDLE_CONN的连接句柄。如果连接失败则返回0; 成功不为0;
*/
    FIX_API_EXPORT HANDLE_CONN  Fix_Connect( const char *pszAddr, const char *pszUser, const char *pszPwd, long nTimeOut );
    FIX_API_EXPORT BOOL Fix_AddBackupSvrAddr( const char *pszAddr );
/*
  函数名称: Fix_Close
            与顶点中间件连接关闭。
  参数说明: 
            conn -- [in] 类型为HANDLE_CONN的连接句柄。该句柄由Fix_Connect生成的。
  返回数据: 
            返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_Close( HANDLE_CONN conn );

/*
  函数名称: Fix_AllocateSession
            根据一个连接句柄分配一个会话句柄，用于数据交互。
            由用户自己在应用层通过调用Fix_AllocateSession分配出来的对象必须由用户自己调用Fix_ReleaseSession来释放。
            反之则不必要。原则上遵循“谁分配谁释放”。这样就可以确保内存不泄露。
  参数说明: 
            conn -- [in] 类型为HANDLE_CONN的连接句柄。该句柄由Fix_Connect生成的。
  返回数据: 
            返回值类型为HANDLE_SESSION的会话对象；如果对象值为空表示对象分配失败。否则表示成功。
*/
    FIX_API_EXPORT HANDLE_SESSION Fix_AllocateSession( HANDLE_CONN conn );

/*
  函数名称: Fix_ReleaseSession
            释放会话句柄。
  参数说明: 
            sess -- [in] 类型为HANDLE_SESSION的会话对象。
  返回数据: 
            返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_ReleaseSession( HANDLE_SESSION sess );

/*
  函数名称: Fix_SetTimeOut
            设置会话交互的超时时间。
  参数说明: 
            conn    -- [in] 类型为HANDLE_CONN的连接句柄。该句柄由Fix_Connect生成的。
            timeout -- [in] 业务应答超时时间；单位为秒，系统默认为30秒。
  返回数据: 
            返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetTimeOut( HANDLE_SESSION sess, long timeout );

/*
   函数名称: Fix_SetWTFS
             设置会话的委托方式.(必须在Fix_CreateHead函数之前调用)
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             val  -- [in] 字符串类型。用于表示客户的接入方式；
                          比如电话委托，磁卡委托，互联网委托等。
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetWTFS( HANDLE_SESSION sess, const char *val );

/*
   函数名称: Fix_SetFBDM
             设置会话的发生营业部代码.(必须在Fix_CreateHead函数之前调用)
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             val  -- [in] 字符串类型。用于表示客户业务发生的营业代码,必须是四位的营业部代码;
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetFBDM( HANDLE_SESSION sess, const char *val );

/*
   函数名称: Fix_SetFBDM
             设置会话的发生营业部代码.(必须在Fix_CreateHead函数之前调用)
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             val  -- [in] 字符串类型。用于表示客户业务到达的目标的营业代码,必须是四位的营业部代码;
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetDestFBDM( HANDLE_SESSION sess, const char *val );

/*
   函数名称: Fix_SetNode
             设置会话的业务发生的节点信息.(必须在Fix_CreateHead函数之前调用)
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             val  -- [in] 字符串类型。一般是客户网卡信息或IP地址。
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetNode( HANDLE_SESSION sess, const char *val );

/*
   函数名称: Fix_SetGYDM
            设置会话的业务发生的柜员代码信息.(必须在Fix_CreateHead函数之前调用)
   参数说明: sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             val  -- [in] 字符串类型。对一些柜台特殊业务，需要用到柜员信息进行认证。
   返回数据: 返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetGYDM( HANDLE_SESSION sess, const char *val );

/*
   函数名称: Fix_CreateHead
             设置会话的业务功能号.具体的功能号的定义请参照【第三方接入业务接口文档】。
   参数说明: 
             sess  -- [in]  类型为HANDLE_SESSION的会话句柄。
             nFunc -- [in]  整型。即为系统提供的业务功能号。
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_CreateHead( HANDLE_SESSION sess, long nFunc );
    BOOL FIX_API_EXPORT Fix_CreateHead( HANDLE_SESSION sess, const char *pszFunc );
    BOOL FIX_API_EXPORT Fix_CreateReq( HANDLE_SESSION sess, long nFunc );
/*
   函数名称: Fix_SetItem
             设置该会话要发送给中间件的业务的请求域数据 或服务应答域数据。
             注:( 服务应答域数据只是针对一些中间件业务需要第三处理机处理的情况下使用。
                  而且是要求通过注册服务实现; 一般的情况下不使用该方式接入。)
   参数说明: 
             sess -- [in]  类型为HANDLE_SESSION的会话句柄。
             id   -- [in]  请求域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             val  -- [in]  字符串类型;对应于id的业务数据。
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetItem( HANDLE_SESSION sess, long id, const char *val );
    BOOL FIX_API_EXPORT Fix_SetString( HANDLE_SESSION sess, long id, const char *val );
    BOOL FIX_API_EXPORT Fix_SetLong( HANDLE_SESSION sess, long id, long val );
    BOOL FIX_API_EXPORT Fix_SetDouble( HANDLE_SESSION sess, long id, double val );

/*
   函数名称: Fix_SetItem
             设置该会话要发送给中间件的业务的请求域数据 或服务应答域数据。  
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             id   -- [in] 请求域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             val  -- [in] 整型;对应于id的业务数据。
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetItem( HANDLE_SESSION sess, long id, long val );

/*
   函数名称: Fix_SetItem
             设置该会话要发送给中间件的业务的请求域数据 或服务应答域数据。  
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
             id   -- [in] 请求域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             val  -- [in] 浮点类型;对应于id的业务数据。
   返回数据: 
             返回值为True表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_SetItem( HANDLE_SESSION sess, long id, double val );

/*
   函数名称: Fix_Run
             提交业务数据，并等待全部的业务结果返回。  
   参数说明: 
             sess -- [in]  类型为HANDLE_SESSION的会话句柄。

   返回数据: 
             返回值为True表示服务业务处理成功; FALSE表示失败,这个失败是表示业务通信上的失败；
             失败后，可以通过Fix_GetCode取出错误码(必定是一个负数)。通过Fix_GetErrMsg取出错误的信息。
*/
    BOOL FIX_API_EXPORT Fix_Run( HANDLE_SESSION sess );

/*
   函数名称: Fix_Cancel
             直接放弃等待业务的应答数据。(在同步方式下使用)
   参数说明: 
             sess -- [in]  类型为HANDLE_SESSION的会话句柄。

   返回数据: 
             返回值为TRUE表示成功; FALSE表示失败。
*/
    BOOL FIX_API_EXPORT Fix_Cancel( HANDLE_SESSION sess );


/*
   函数名称: Fix_AsyncRun
             异步提交业务数据，不等待待全部的业务结果返回。
             和函数Fix_IsReplyed配合使用.
   参数说明: 
             sess -- [in]  类型为HANDLE_SESSION的会话句柄。

   返回数据: 
             返回值为True表示服务业务处理成功; FALSE表示失败,这个失败是表示业务通信上的失败；
             失败后，可以通过Fix_GetCode取出错误码(必定是一个负数)。通过Fix_GetErrMsg取出错误的信息。
*/
    BOOL FIX_API_EXPORT Fix_AsyncRun( HANDLE_SESSION sess );

/*
   函数名称: Fix_IsReplyed
             异步提交业务数据后，检查数据是否全部返回。
   参数说明: 
             sess -- [in]  类型为HANDLE_SESSION的会话句柄。
             nMSec [in/option] 指定等待的时间，默认不等待。单位为毫秒。

   返回数据: 
             返回值为True表示应答数据全部返回; FALSE表示需要继续等待。
*/
    BOOL FIX_API_EXPORT Fix_IsReplyed( HANDLE_SESSION sess, long nMSec=-1 );

/*
   函数名称: Fix_GetCode
             读取业务提交失败的错误代码。  
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回负整数。
*/
    long FIX_API_EXPORT Fix_GetCode( HANDLE_SESSION sess );

/*
   函数名称: Fix_GetErrMsg
             读取业务提交失败的错误信息。  
   参数说明: 
             sess   -- [in] 类型为HANDLE_SESSION的会话句柄。
             out    -- [in/out] 用于输出错误信息的字符串.
             outlen -- [in] 参数out缓冲区的大小.
   返回数据: 
             返回错误信息字符串。
*/
    char FIX_API_EXPORT *Fix_GetErrMsg( HANDLE_SESSION sess, char *out, int outlen );

/*
   函数名称: Fix_GetCount
             读取中间件返回的业务应答数据的行数。  
   参数说明: 
             sess -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回行数量。
*/
    long FIX_API_EXPORT Fix_GetCount( HANDLE_SESSION sess );

/*
   函数名称: Fix_GetItem
             1、从中间件的返回结果集的指定行中读取业务应答数据。
             2、或者是读取中间件请求的业务请求。
   参数说明: 
             sess   -- [in] 类型为HANDLE_SESSION的会话句柄。
             id     -- [in] 指定域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             out    -- [in/out] 用于输出业务信息的字符串.
             outlen -- [in] 指定out缓冲区的大小.
             row    -- [in][option] 指定返回数据结果集中的行数(行数索引值以0开始).如果不设置这个值，
                       表示读取第一行的数据。
   返回数据: 
             返回指定行的的字符串数据。
*/
    char FIX_API_EXPORT *Fix_GetItem( HANDLE_SESSION sess, long id, char *out, int outlen, long row = -1 );
    
/*
   函数名称: Fix_GetLong
             1、从中间件的返回结果集的指定行中读取业务应答数据。
             2、或者是读取中间件请求的业务请求。
   参数说明: 
             sess   -- [in] 类型为HANDLE_SESSION的会话句柄。
             id     -- [in] 指定域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             row    -- [in][option] 指定返回数据结果集中的行数(行数索引值以0开始).如果不设置这个值，
                       表示读取第一行的数据。
   返回数据: 
             返回指定行的整型数据。
*/
    long FIX_API_EXPORT Fix_GetLong( HANDLE_SESSION sess, long id, long row = -1 );
  
/*
   函数名称: Fix_GetDouble
             1、从中间件的返回结果集的指定行中读取业务应答数据。
             2、或者是读取中间件请求的业务请求。
   参数说明: 
             sess   -- [in] 类型为HANDLE_SESSION的会话句柄。
             id     -- [in] 指定域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             row    -- [in][option] 指定返回数据结果集中的行数(行数索引值以0开始).如果不设置这个值，
                       表示读取第一行的数据。
   返回数据: 
             返回指定行的浮点数据。
*/
    double FIX_API_EXPORT Fix_GetDouble( HANDLE_SESSION sess, long id, long row = -1 );

/*
   函数名称: Fix_HaveItem
             1、检查中间件的返回结果集的指定行中是否含有指定域。
             2、或者检查中间件请求的业务数据是否含有指定域。
   参数说明: 
             sess   -- [in] 类型为HANDLE_SESSION的会话句柄。
             id     -- [in] 指定域的tag值；具体的定义值请参考【第三方接入业务接口文档】。
             row    -- [in][option] 指定返回数据结果集中的行数(行数索引值以0开始).如果不设置这个值，
                       表示读取第一行的数据。
   返回数据: 
             返回True表示,含有指定的域数据；否则没有。
*/
    BOOL FIX_API_EXPORT Fix_HaveItem( HANDLE_SESSION sess, long id, long row = -1 );

/*
   函数名称: Fix_UploadFile
             文件上传服务.
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
             pszName     -- [in] 指定上传文件的简称。
                                 (文件简称由后台系统提供，所以该参数一般作为可配置的选项)
             pszFileName -- [in] 本地要上传的文件路径。                       
   返回数据: 
             返回True表示,文件上传成功；否则表示失败。
             失败原因可以通过 Fix_ErrMsg函数,读取失败的原因。
*/
    BOOL FIX_API_EXPORT Fix_UploadFile(  HANDLE_SESSION sess, const char *pszName, const char *pszFileName );

/*
   函数名称: Fix_DownloadFile
             文件下传服务.
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
             pszName     -- [in] 指定下载文件的简称。
                                (文件简称由后台系统提供，所以该参数一般作为可配置的选项)
             pszFileName -- [in] 要下载到本地的文件路径。
   返回数据: 
             返回True表示,文件下载成功；否则表示失败。
             失败原因可以通过 Fix_ErrMsg函数,读取失败的原因。
*/
    BOOL FIX_API_EXPORT Fix_DownloadFile(  HANDLE_SESSION sess, const char *pszName, const char *pszFileName );



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
/*  当第三方需要接受中间件发起的业务请求时，可利用以下函数进行数据的交互。
*/

/*
   函数名称: Fix_RegisterSvc
             服务注册函数，用于处理中间件发起的业务请求。
   参数说明: 
             conn         -- [in] 类型为HANDLE_CONN的连接句柄。
             func         -- [in] 整型；指定的服务名称。
             name         -- [in] 字符串类型；表示该服务的名称。
             pFunc        -- [in] 指定的回调数据指针。
                                  回调函数的传参格式: bool CallBack( HANDLE_CONN conn, HANDLE_SVC svc, HANDLE_SESSION sess );
             pszCondition -- [in] 指定服务限制条件。比如银行发起的业务，要求指定只是做某个银行的业务。
                                  或只是处理的指定区域中心的的业务。
   返回数据: 
             返回一个相对一个连接句柄下唯一的服务句柄。
             如果返回值为NULL,表示服务注册失败; 成功不为NULL。
*/
    HANDLE_SVC FIX_API_EXPORT Fix_RegisterSvc( HANDLE_CONN conn, long func, const char *name, void *pFunc, const char *pszCondition );

/*
   函数名称: Fix_UnRegisterSvc
             服务注消函数。
   参数说明: 
             conn         -- [in] 类型为HANDLE_CONN的连接句柄。
             svc          -- [in] 指定该连接已经注册的服务号码。
   返回数据: 
             返回True表示成功；否则表示失败。
*/

    BOOL FIX_API_EXPORT Fix_UnRegisterSvc( HANDLE_CONN conn, HANDLE_SVC svc );

/*
   函数名称: Fix_CreateAnswer
             创建服务应答数据;用于单行的数据应答。
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回True表示成功；否则表示失败。
*/

    BOOL FIX_API_EXPORT Fix_CreateAnswer( HANDLE_SESSION sess );

/*
   函数名称: Fix_CreateAnswerMul
             创建服务应答数据;用于多行的数据应答。必须和Fix_CreateNextAnswer、Fix_CreateEndMark配合使用。
             具体可以参照Sample2的代码。
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回True表示成功；否则表示失败。
*/
    BOOL FIX_API_EXPORT Fix_CreateAnswerMul( HANDLE_SESSION sess );

/*
   函数名称: Fix_CreateNextAnswer
             创建服务应答数据;在多行的数据应答。创建下一条的应答数据。
             具体可以参照Sample2的代码。
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回True表示成功；否则表示失败。
*/
    BOOL FIX_API_EXPORT Fix_CreateNextAnswer( HANDLE_SESSION sess );

/*
   函数名称: Fix_CreateEndMark
             创建服务应答数据;在多行的数据应答。创建结束应答数据。
             具体可以参照Sample2的代码。
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回True表示成功；否则表示失败。
*/
    BOOL FIX_API_EXPORT Fix_CreateEndMark( HANDLE_SESSION sess );

/*
   函数名称: Fix_Write
             发出应答数据。
             具体可以参照Sample2的代码。
   参数说明: 
             sess        -- [in] 类型为HANDLE_SESSION的会话句柄。
   返回数据: 
             返回True表示成功；否则表示失败。
*/
    BOOL FIX_API_EXPORT Fix_Write( HANDLE_SESSION sess );

//简单加密函数(用户密码加密)
    char FIX_API_EXPORT *Fix_Encode( char *pData );

/*
   函数名称: Fix_GetToken
             取出对于的令牌字符串(可选、与后台系统配置有关)
*/

    char FIX_API_EXPORT *Fix_GetToken( HANDLE_SESSION sess, char *out, int outlen );
/*
   函数名称: Fix_SetToken
             设置会话的令牌(可选、与后台系统配置有关)
*/
    BOOL FIX_API_EXPORT Fix_SetToken( HANDLE_SESSION sess, const char *val );


#endif
