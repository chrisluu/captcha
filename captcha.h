#ifndef _CAPTCHA_HEADER
#define _CAPTCHA_HEADER
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <list>
#include <string>
#include <limits.h>

/*----------------------------------------------------------------------------*/
/** Crack captcha */
typedef struct myRect{
	cv::Point pt1;
	cv::Point pt2;
	long sum = LONG_MAX;
	int size = 0;
};

class CAPTCHA{
public:
	CAPTCHA::CAPTCHA() :pics(3), symbols(2, 0), flag(false){

	}
	~CAPTCHA(){

	}

	void crackCaptcha(cv::Mat src);

	bool isCrackSucceed();

	std::vector<cv::Mat> getPicResults();

	std::vector<int> getSymbols();

private:
	void adaptiveThreshold(const cv::Mat& src, cv::Mat& dest, int threshold);


	long computePatchValue(const cv::Mat src, cv::Point pt, int patch_width, int patch_height);


	std::string toStr(const int n);

private:
	std::vector<cv::Mat> pics;
	std::vector<int> symbols;
	bool flag;
};

/*----------------------------------------------------------------------------*/
#endif /* !CAPTCHA_HEADER */


