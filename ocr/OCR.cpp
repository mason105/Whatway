#include "stdafx.h"
#include "OCR.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
//uuid
#include <boost/uuid/uuid.hpp>  
#include <boost/uuid/uuid_generators.hpp>  
#include <boost/uuid/uuid_io.hpp>  

#include <unicode/putil.h>
#include <unicode/ucnv.h>

#include "./output/FileLog.h"
#include "config/ConfigManager.h"

#include "./network/ssl/pkgheader.pb.h"
#include "network/ssl/ssl_message.h"


OCR::OCR(void)
{
}


OCR::~OCR(void)
{
}

IMessage* OCR::Recognition(IMessage* req)
{
	quote::PkgHeader msgHeader;
	msgHeader.ParseFromArray(req->GetMsgHeader().data(), req->GetMsgHeaderSize());
	
	
	

	// 为了身份证识别特殊处理
	// 1代表姓名，2代表地址，3代表身份证号码，4代表有效期
	int nImageType = msgHeader.errcode();

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
	
	std::string uuid = boost::lexical_cast<std::string>(u);

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

	int nPkgBodySize = req->GetMsgContentSize();
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
	outfile.write(req->GetMsgContent().data(), nPkgBodySize);
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

	
	IMessage * resp = new ssl_message;

	quote::PkgHeader pbHeader;
	pbHeader.set_zip(false);
	pbHeader.set_bodysize(length);
	
	pbHeader.set_ver(1);
	pbHeader.set_enc(false);
		
	pbHeader.set_more(false);
	pbHeader.set_msgtype(quote::PkgHeader::RES_OCR);
	pbHeader.set_errcode(0);
		
		
	bool bTmp = pbHeader.SerializeToArray(&(resp->m_MsgHeader.front()), pbHeader.ByteSize());

	// 设置消息内容
	resp->SetMsgContent(ocr);

	// 设置会话
	resp->SetSession(req->GetSession());

	return resp;
}

/*
用opencv处理图像文件
*/
bool OCR::GenImage(std::string sSrcFile, std::string sTargetFile, int nType, int nThreshold, bool bShow, std::string sSrcWndName, std::string sTargetWndName)
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

/*
读ocr引擎识别后输出的文本文件
*/
bool OCR::ReadOCR(std::string file, std::string& content)
{
	std::ifstream is(file, std::ifstream::binary);
	if (!is.is_open())
	{
		return false;
	}

	is.seekg (0, is.end);
    size_t length = is.tellg();
    is.seekg (0, is.beg);

	char * buffer = new char [length+1];
	memset(buffer, 0, length+1);

	is.read(buffer,length);
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
void OCR::rotate_image_90n(cv::Mat &src, cv::Mat &dst, int angle)
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
