

#include "CAPTCHA.h"
#include <io.h>


/** @function main */
int main(int argc, char** argv)
{
	_finddata_t fileDir;//a structure,which can be used to save file info

	std::string dirName = "D:/imgtest/verificationCode/img/";
	std::string saveDir = "D:/imgtest/verificationCode/result/";
	std::string cannotOperate = "D:/imageTest/affineTest/cannotDo/";
	char* dir = "D:/imgtest/verificationCode/img/*.*";//save path
	long lfDir;//save path HWND
	//find the first file£¬and send it to fileDir
	//return a unique search HWND£¬which represent the file info
	int count = 0;
	if ((lfDir = _findfirst(dir, &fileDir)) == -1l)
	{
		printf("No file is found\n");
	}
	else
	{
		while (_findnext(lfDir, &fileDir) == 0)
		{//search next file using HWND and send it to fileDir
			//printf("%s\n", fileDir.name);//obtain filename 
			//printf("%d\n", fileDir.size);//obtain file size 
			std::string name = fileDir.name;
			if (name.length() < 3)
				continue;

			std::string fileName = name.substr(0, name.find_last_of("."));
			std::string suffix = name.substr(name.find_last_of("."), name.length());

			//Load an image
			cv::Mat src = cv::imread(dirName + name, CV_LOAD_IMAGE_COLOR);
			if (!src.data) { printf("read image error£¬make sure image exist£¡ \n"); return false; }
			//record time 
			double t = (double)cv::getTickCount();
			CAPTCHA* captcha = new CAPTCHA();
			captcha->crackCaptcha(src);
			if (captcha->isCrackSucceed())
			{
				std::vector<cv::Mat> pics = captcha->getPicResults();
				std::vector<int> symbols = captcha->getSymbols();
				//write result to disk
				cv::imwrite(saveDir + fileName + "(pics0)" + suffix, pics[0]);
				cv::imwrite(saveDir + fileName + "(pics1)" + suffix, pics[1]);
				cv::imwrite(saveDir + fileName + "(pics2)" + suffix, pics[2]);
				std::cout << symbols[0] << "   " << symbols[1] << std::endl;
			}

			//record time
			t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
			std::cout << "time cunsuming: " << t << "  fileName" << "   " << fileName << std::endl;
		}
	}
	_findclose(lfDir);//close all files in the directory
	return 0;
}