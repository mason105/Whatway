#include "stdafx.h"
//#define BOOST_FILESYSTEM_VERSION 2
// mongodb用的还是老版本，只支持2，未来会自带boost1.49

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
//压缩
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
//uuid
#include <boost/uuid/uuid.hpp>  
#include <boost/uuid/uuid_generators.hpp>  
#include <boost/uuid/uuid_io.hpp>  


#include "trade_server_ssl.h"
#include "common.h"



#include "./config/configmanager.h"



//#include <mongo/client/dbclient.h>
//#include <mongo/client/connpool.h>



//#include "trade.pb.h"




#include "./encrypt/mybotan.h"
#include <Magick++.h>
#include "./captcha/captchamanager.h"


#include <unicode/putil.h>
#include <unicode/ucnv.h>

#include "./business/szkingdom/tradebusiness.h"
#include "./business/hundsun_t2/tradebusinesst2.h"
#include "./business/hundsun_com/TradeBusinessComm.h"
#include "./business/apexsoft/TradeBusinessDingDian.h"
#include "./business/apexsoft/DingDian.h"



#include "./output/FileLog.h"



trade_server_ssl::trade_server_ssl():
req_worker_(recvq_, boost::bind(&trade_server_ssl::process_msg, this, _1), gConfigManager::instance().m_nSslWorkerThreadPool)
	,resp_worker_(sendq_, boost::bind(&trade_server_ssl::send_msg, this, _1), gConfigManager::instance().m_nSslSendThreadPool)
	
{
}

bool trade_server_ssl::send_msg(ssl_response_ptr resp)
{
	// 压缩

	if (resp->msg_body.size() <= gConfigManager::instance().zlib)
	{
		//不压缩
	}
	else
	{
		// 压缩
		//gFileLog::instance().Log("开始压缩");

		std::vector<char> compressed;
		boost::iostreams::filtering_streambuf<boost::iostreams::output> compress_out;
		compress_out.push(boost::iostreams::zlib_compressor());
		compress_out.push(boost::iostreams::back_inserter(compressed));
		boost::iostreams::copy(boost::make_iterator_range(resp->msg_body), compress_out);

		int bodysize = compressed.size();
		resp->set_body_size(bodysize);
		memcpy(&(resp->msg_body.front()), &compressed[0], compressed.size());

		resp->encode_header(resp->header.msgtype(), resp->header.errcode(), 1);

		//gFileLog::instance().Log("结束压缩");
	}

	resp->get_session()->write(resp);

	return true;
}




void trade_server_ssl::start()
{
	req_worker_.start();
	resp_worker_.start();
}

void trade_server_ssl::stop()
{
	req_worker_.stop();
	resp_worker_.stop();
}

trade_server_ssl::req_queue_type& trade_server_ssl::recv_queue()
{
	return recvq_;
}

bool trade_server_ssl::process_msg(ssl_request_ptr& req)
{
	switch(req->header.msgtype())
	{
	case quote::PkgHeader::REQ_TRADE:
		trade(req);
		break;
	case quote::PkgHeader::REQ_CAPTCHA:
		captcha(req);
		break;
	case quote::PkgHeader::REQ_OCR:
		OCR(req);
		break;

	default:
		error(req);
		break;
	}
	
	req->destroy();
	return true;
}

void trade_server_ssl::trade(ssl_request_ptr& req)
{
	int msgtype = req->header.msgtype();
	if (msgtype != quote::PkgHeader::REQ_TRADE )
	{
		return;
//		resp->set_body_size(0);
//		resp->encode_header(quote::PkgHeader::RES_TRADE, EC_REQ_INVALID);
//		sendq_.push(resp);


	}
	//ssl_response_ptr resp = new ssl_message(req->get_session());
	ssl_response_ptr resp = boost::factory<ssl_response_ptr>()(req->get_session());




	std::string request_utf8(req->msg_body.begin(), req->msg_body.end());

//	std::string temp="原始请求:";
//	temp += request_utf8;
//	gFileLog::instance().Log(temp);

	std::string response;
	
	UErrorCode errcode = U_ZERO_ERROR;
	char dest[8192];
	memset(dest, 0x00, sizeof(dest));
	int ret = ucnv_convert("gbk", "utf8", dest, sizeof(dest), request_utf8.c_str(), -1, &errcode);
	std::string request_gbk = dest;

	Trade::TradeLog logMsg;
	// 请求包中的接收时间
	std::string recvtime = boost::gregorian::to_iso_extended_string(req->RecvTime.date()) + " " + boost::posix_time::to_simple_string(req->RecvTime.time_of_day());
	logMsg.set_recvtime(recvtime);

	std::string sysNo;
	std::string busiType;

	GetSysNoAndBusiType(request_gbk, sysNo, busiType);

	std::string msg = "系统编号: " + sysNo + "业务类型: " + busiType;
	gFileLog::instance().Log(msg);

	CounterType ct = gConfigManager::instance().GetCounterType(sysNo, busiType);

	if (ct == CT_JZ_WIN)
	{
		TradeBusiness * trade = new TradeBusiness();
		trade->Process(request_gbk, response, logMsg);
		delete trade;
	}
	else if (ct == CT_HS_T2)
	{
		TradeBusinessT2 * trade = new TradeBusinessT2();
		trade->Process(request_gbk, response, logMsg);
		delete trade;
	}
	else if (ct == CT_DINGDIAN)
	{
		// 延迟初始化函数指针，只允许初始化一次
		
		g_DingDian.Init();

		TradeBusinessDingDian * trade = new TradeBusinessDingDian();
		trade->Process(request_gbk, response, logMsg);
		delete trade;
	}
	else if (ct == CT_JSD)
	{
//		TradeBusinessAGC * trade = new TradeBusinessAGC();
//		trade->Process(request_gbk, response, logMsg);
//		delete trade;
	}
	else if (ct == CT_XINYI)
	{
//		TradeBusinessXinyi * trade = new TradeBusinessXinyi();
//		trade->Process(request_gbk, response, logMsg);
//		delete trade;
	}
	else
	{
		std::string SOH = "\x01";

		response = "1";
		response += SOH;
		response += "3";
		response += SOH;

		response += "cssweb_code";
		response += SOH;
		response += "cssweb_msg";
		response += SOH;
		response += "cssweb_gwInfo";
		response += SOH;

		response += "1001";
		response += SOH;
		response += "自定义参数，没有找到对应的柜台类型";
		response += SOH;
		response += "0.0.0.0:5000";
		response += SOH;
	
		gFileLog::instance().Log("ssl 没有找到对应的柜台类型");
	}

	// 业务处理日志
	resp->logMsg = logMsg;

	int msglen = response.length();
	resp->set_body_size(msglen);
	memcpy(&(resp->msg_body.front()), response.c_str(), response.length());
	resp->encode_header(quote::PkgHeader::RES_TRADE, EC_SUCCESS);
	sendq_.push(resp);
}

void trade_server_ssl::captcha(ssl_request_ptr& req)
{
	int msgtype = req->header.msgtype();
	if (msgtype != quote::PkgHeader::REQ_CAPTCHA)
	{
		return;
	}

	ssl_response_ptr resp = boost::factory<ssl_response_ptr>()(req->get_session());

	std::string text = captcha::instance().GetCaptcha();
	int captcha = boost::lexical_cast<int>(text);
	OutputDebugString("captcha ");
	OutputDebugString(text.c_str());
	OutputDebugString("\n");
/*
	std::string backGround = "xc:#CCCCCC";
	std::string buttonSize = "118x41";
	// Button background texture
	std::string buttonTexture = "granite:";
	// Button text
	// Button text color
	std::string textColor = "red";
	// Font point size
	int fontPointSize = 24;
	Magick::Image captcha;
	// Set button size
	captcha.size( buttonSize);
	// Read background image
	captcha.read( backGround);
	// Set background to buttonTexture
	Magick::Image backgroundTexture( buttonTexture );
	captcha.texture( backgroundTexture );
	// Add some text
	captcha.fillColor( textColor );
	captcha.fontPointsize( fontPointSize );
	captcha.annotate( text, Magick::CenterGravity );
	// Write to BLOB in JPEG format
	Magick::Blob blob;
	captcha.magick("JPEG"); // Set JPEG output format
	captcha.write( &blob ); 
*/


		std::string jpeg_b64;

		std::string image_jpeg;
		std::string image_type = "jpg";
		m_Captcha.VerificationCodeToImage(text,image_jpeg, image_type);

		//g_MyBotan.Base64Encoder((const unsigned char*)blob.data(), blob.length(), jpeg_b64);

		g_MyBotan.Base64Encoder((const unsigned char*)image_jpeg.c_str(), image_jpeg.length(), jpeg_b64);
		
		int msglen = jpeg_b64.length();

		
		resp->set_body_size(msglen + text.length());
		
		
		// 返回captcha text
		memcpy(&(resp->msg_body.front()), text.c_str(), text.length());

		memcpy(&(resp->msg_body.front()) + text.length(), jpeg_b64.c_str(), jpeg_b64.length());
	
		resp->encode_header(quote::PkgHeader::RES_CAPTCHA, EC_SUCCESS);
		
		sendq_.push(resp);

}

void trade_server_ssl::OCR(ssl_request_ptr& req)
{
	int msgtype = req->header.msgtype();
	if (msgtype != quote::PkgHeader::REQ_OCR)
	{
		return;
	}

	ssl_response_ptr resp = boost::factory<ssl_response_ptr>()(req->get_session());

	// 为了身份证识别特殊处理
	// 1代表姓名，2代表地址，3代表身份证号码，4代表有效期
	int nImageType = req->header.errcode();

	// 根据当前日期创建目录
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	std::string dir = boost::gregorian::to_iso_extended_string(now.date());
	dir = gConfigManager::instance().OCROutput + "\\" + dir;
	boost::filesystem::path p(dir);
	if (!boost::filesystem::exists(p))
	{
		boost::filesystem::create_directories(p);
	}
	
	// 根据当前时间+uuid创建子目录
	std::string sTime =  boost::posix_time::to_simple_string(now.time_of_day());
	boost::algorithm::replace_all(sTime, ":", "-");

	boost::uuids::basic_random_generator<boost::mt19937> gen;
	boost::uuids::uuid u = gen();
	std::string uuid = boost::to_string(u);

	std::string subdir;
	if (nImageType == 1)
	{
		subdir = dir + "\\name" + "_" + sTime + "_" + uuid;
	}
	if (nImageType == 2)
	{
		subdir = dir + "\\address" + "_" + sTime + "_" + uuid;
	}
	if (nImageType == 3)
	{
		subdir = dir + "\\idno" + "_" + sTime + "_" + uuid;
	}
	if (nImageType == 4)
	{
		subdir = dir + "\\expire" + "_" + sTime + "_" + uuid;
	}

	boost::filesystem::path p2(subdir);
	if (!boost::filesystem::exists(p2))
	{
		boost::filesystem::create_directories(p2);
	}

	int nPkgBodySize = req->msg_body.size();
	TRACE("消息内容大小%d\n", nPkgBodySize);

	// 保存身份证正面照
	std::string sSrcFile;
	std::string sTargetFile;
	
	std::string sTxtFile;
	
	int nAlgorithm;
	int nThreshold;

	if (nImageType == 1)
	{
		sSrcFile = subdir + "\\name.jpg";
		sTargetFile = subdir + "\\name_out.jpg";
		sTxtFile = subdir + "\\name_out";
		nAlgorithm = gConfigManager::instance().m_nNameAlgorithm;
		nThreshold = gConfigManager::instance().m_nNameThreshold;
	}

	if (nImageType == 2)
	{
		sSrcFile = subdir + "\\address.jpg";
		sTargetFile = subdir + "\\address_out.jpg";
		sTxtFile = subdir + "\\address_out";
		nAlgorithm = gConfigManager::instance().m_nAddressAlgorithm;
		nThreshold = gConfigManager::instance().m_nAddressThreshold;
	}

	if (nImageType == 3)
	{
		sSrcFile = subdir + "\\idno.jpg";
		sTargetFile = subdir + "\\idno_out.jpg";
		sTxtFile = subdir + "\\idno_out";
		nAlgorithm = gConfigManager::instance().m_nIDNoAlgorithm;
		nThreshold = gConfigManager::instance().m_nIDNoThreshold;
	}

	if (nImageType == 4)
	{
		sSrcFile = subdir + "\\expire.jpg";
		sTargetFile = subdir + "\\expire_out.jpg";
		sTxtFile = subdir + "\\expire_out";
		nAlgorithm = gConfigManager::instance().m_nExpireAlgorithm;
		nThreshold = gConfigManager::instance().m_nExpireThreshold;
	}

	std::ofstream outfile (sSrcFile, std::ofstream::binary);
	outfile.write(&(req->msg_body.front()), nPkgBodySize);
	outfile.close();

	// 开始图像处理
	GenImage(sSrcFile, sTargetFile, nAlgorithm, nThreshold);
	
	

	// 开始调用tesseract-ocr
	std::string sOCRExe = gConfigManager::instance().OCRPath + "\\tesseract.exe";
	std::string sOCRPath = gConfigManager::instance().OCRPath;
	std::string sConfigFile = gConfigManager::instance().m_sPath + "\\ocr_config.txt";

	std::string sParam = sTargetFile; // jpg

	
	
	sParam += " " + sTxtFile;

	if (nImageType == 1 || nImageType == 2)
	{
		sParam += " -l chi_sim "; // 最后有空格
	}

	if (nImageType == 3)
	{
		sParam += " -l id "; // 最后有空格
	}

	if (nImageType == 4)
	{
		sParam += " -l expire "; // 最后有空格
	}

	sParam += sConfigFile;

	SHELLEXECUTEINFO sei; 
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO)); 
	
	sei.cbSize = sizeof(SHELLEXECUTEINFO); 
	sei.fMask = SEE_MASK_NOCLOSEPROCESS; 
	sei.lpVerb = _T("open"); 
	sei.lpFile = _T(sOCRExe.c_str()); 
	sei.lpDirectory = _T(sOCRPath.c_str()); 
	sei.lpParameters = _T(sParam.c_str());
	sei.nShow = SW_HIDE; 
	ShellExecuteEx(&sei); 
	//ShellExecute(NULL, "Open", sOCRExe.c_str(), sParam.c_str(), sOCRPath.c_str(), SW_HIDE);
	WaitForSingleObject(sei.hProcess, INFINITE); 
	CloseHandle(sei.hProcess);  
	
	std::string OCRFile = sTxtFile + ".txt";

	std::string ocr = "";
	ReadOCR(OCRFile, ocr);

	if (ocr.empty())
		ocr = "图像解析失败";

	int length = ocr.length();
	TRACE("ocr返回长度%d\n", length);

	

	resp->set_body_size(length);

	memcpy(&(resp->msg_body.front()), ocr.c_str(), length);
	
	resp->encode_header(quote::PkgHeader::RES_OCR, EC_SUCCESS);
		
	sendq_.push(resp);
	
}

bool trade_server_ssl::GenImage(std::string sSrcFile, std::string sTargetFile, int nType, int nThreshold, bool bShow, std::string sSrcWndName, std::string sTargetWndName)
{
	cv::Mat imgSrc;
	imgSrc = cv::imread(sSrcFile);
	if (!imgSrc.data)
	{
		return false;
	}

	rotate_image_90n(imgSrc, imgSrc, -90);

	if (bShow)
	{
		cv::namedWindow(sSrcWndName);
		cv::imshow(sSrcWndName, imgSrc);
	}

	//灰度化
	cv::Mat imgGray;
	cv::cvtColor(imgSrc, imgGray, CV_BGR2GRAY);

	cv::Mat imgOut;

	if (nType == 0)
	{
		cv::threshold(imgGray, imgOut, nThreshold, 255, cv::THRESH_BINARY);
	}

	if (nType == 1)
	{
		// OTSU
		cv::threshold(imgGray, imgOut, nThreshold, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);
	}

	if (bShow)
	{
		cv::namedWindow(sTargetWndName);
		cv::imshow(sTargetWndName, imgOut);
	}

	// 生成文件
	return cv::imwrite(sTargetFile, imgOut);	
	
}

bool trade_server_ssl::ReadOCR(std::string file, std::string& content)
{
	std::ifstream is(file, std::ifstream::binary);
	if (!is.is_open())
	{
		return false;
	}

	is.seekg (0, is.end);
    int length = is.tellg();
    is.seekg (0, is.beg);

	char * buffer = new char [length+1];
	memset(buffer, 0, length+1);

	is.read (buffer,length);
	buffer[length] = '\0';

	gFileLog::instance().Log(buffer);


	UErrorCode errcode = U_ZERO_ERROR;
	char dest[8192];
	memset(dest, 0x00, sizeof(dest));
	int ret = ucnv_convert("gbk", "utf8", dest, sizeof(dest), buffer, -1, &errcode);
	
	gFileLog::instance().Log(dest);



	content = dest;
	delete[] buffer;

	is.close();

	boost::replace_all(content, "\n", "");
	boost::replace_all(content, "?", "");


	return true;
}
/*
 *@brief rotate image by factor of 90 degrees
 *
 *@param source : input image
 *@param dst : output image
 *@param angle : factor of 90, even it is not factor of 90, the angle
 * will be mapped to the range of [-360, 360].
 * {angle = 90n; n = {-4, -3, -2, -1, 0, 1, 2, 3, 4} }
 * if angle bigger than 360 or smaller than -360, the angle will
 * be map to -360 ~ 360.
 * mapping rule is : angle = ((angle / 90) % 4) * 90;
 *
 * ex : 89 will map to 0, 98 to 90, 179 to 90, 270 to 3, 360 to 0.
 *
 */
void trade_server_ssl::rotate_image_90n(cv::Mat &src, cv::Mat &dst, int angle)
{   
   if(src.data != dst.data){
       src.copyTo(dst);
   }

   angle = ((angle / 90) % 4) * 90;

   //0 : flip vertical; 1 flip horizontal
   bool const flip_horizontal_or_vertical = angle > 0 ? 1 : 0;
   int const number = std::abs(angle / 90);     
   TRACE("number = %d\n", number);

   for(int i = 0; i != number; ++i){
       cv::transpose(dst, dst);
       cv::flip(dst, dst, flip_horizontal_or_vertical);
   }
}

void trade_server_ssl::error(ssl_request_ptr& req)
{
	ssl_response_ptr resp = boost::factory<ssl_response_ptr>()(req->get_session());
	resp->set_body_size(0);
	//应该再定义一个类型
	resp->encode_header(quote::PkgHeader::RES_TRADE, EC_REQ_INVALID);

	sendq_.push(resp);
}

bool trade_server_ssl::GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType)
{
	

	std::vector<std::string> keyvalues;
	std::string SOH = "\x01";
	boost::split(keyvalues, request, boost::is_any_of(SOH)); // 注意需要通过配置文件配置

	// 全部转成小写
	//取的时候也全部改成小写

	for (std::vector<std::string>::iterator it = keyvalues.begin(); it != keyvalues.end(); it++)
	{
		std::string keyvalue = *it;


		if (keyvalue.empty())
			break;

		std::vector<std::string> kv;
		boost::split(kv, keyvalue, boost::is_any_of("="));

		std::string key = "";
		if (!kv[0].empty())
			key = kv[0];

		std::string value = "";
		if (!kv[1].empty())
			value = kv[1];


		if (key == "cssweb_sysNo")
			sysNo = value;

		if (key == "cssweb_busiType")
			busiType = value;

	}

	if (sysNo.empty() || busiType.empty())
		return false;

	return true;
}
