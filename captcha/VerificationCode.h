#pragma once 
#include <string> 
#include <vector>   
using namespace std; 


class VerificationCode
{ 
public: 
	struct CmdInfo
	{ 
		CmdInfo();
		string m_ImageType;//图片类型
		unsigned int m_Width;//图片宽度
		unsigned int m_Heigh;//图片高度
		string m_XcColor;//图片背景色
		string m_Font;//字体
		unsigned int m_Pointsize;//大小
		string m_FontColor;//字体颜色
	};
	//修改验证码图片属性
	void SetCmdInfo(CmdInfo in_CmdInfo);
	// 输入验证码,获取图片
	string VerificationCodeToImage(const string& in_VerificationCode	//输入验证码
									,string& out_ImageBinaryData		//输出二进制图片数据
									,string& in_ImageType);				//输入验证码图片类型
private: 
	CmdInfo m_CmdInfo; //
	// 输入验证码
	string GetCmd(string in_VerificationCode);
	//
	void SplitCmd(string in_CmdStr,vector<string>& out_SplitStringVector);
	//生成语句时用引号代理空格,表示字符串是一体的,在分解完Convert命令后,还原空格

	//长 宽  背景色
	string GetSizeXc(unsigned int in_Width,unsigned int in_Heigh,string in_XcColor);
	//注释        
	string GetAnnotate(string in_Font	//字体
						, unsigned int in_Pointsize  //字体大小
						, string in_Color		//颜色
						,unsigned int in_X_		//角度
						,unsigned int in_Y_		//角度
						,unsigned int in_X		//X坐标
						,unsigned int in_Y		//Y坐标
						,string in_Value);		//显示字符
	//画随机线
	string GetDrawLine(); 
	//获取随机数
	int GetRand(int in_Min,int in_Max); 
	//生成图片// outImageData:增加的参数,获取二进制数据,string& outImageData
	static int ConvertMain(int argc,char **argv,string& outImageData); 
};