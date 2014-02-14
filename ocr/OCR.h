#ifndef _OCR_H_
#define _OCR_H_

//opencv
#include "./lib/opencv/opencv_h.h"
#include "network/IMessage.h"

class OCR
{
public:
	OCR(void);
	~OCR(void);

public:
	bool ReadOCR(std::string file, std::string& content);
	bool GenImage(std::string sSrcFile, std::string sTargetFile, int nType=1, int nThreshold=128, bool bShow=false, std::string sSrcWndName="", std::string sTargetWndName="");
	void rotate_image_90n(cv::Mat &src, cv::Mat &dst, int angle);
	IMessage* Recognition(IMessage* req);
};

#endif
