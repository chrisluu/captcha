#include "captcha.h"

void CAPTCHA::crackCaptcha(cv::Mat src)
{
	cv::Mat src_gray;
	cv::cvtColor(src, src_gray, CV_BGR2GRAY);
	std::vector<cv::Vec3f> circles;

	cv::Mat dest;
	//binary
	adaptiveThreshold(src_gray, dest, 15);
	cv::Mat afterFilter;
	afterFilter.create(dest.size(), dest.type());
	//filter
	for (int j = 1; j < dest.rows - 1; j++)
	{
		const uchar* previous = dest.ptr<const uchar>(j - 1);
		const uchar* current = dest.ptr<const uchar>(j);
		const uchar* next = dest.ptr<const uchar>(j + 1);
		uchar* out = afterFilter.ptr<uchar>(j);
		for (int i = 1; i < dest.cols - 1; i++)
		{
			if (current[i] == 0)
			{
				//fiter
				if ((previous[i - 1] == 0 && previous[i] == 0 && current[i - 1] == 0) ||
					(previous[i] == 0 && previous[i + 1] == 0 && current[i + 1] == 0) ||
					(next[i - 1] == 0 && next[i] == 0 && current[i - 1] == 0) ||
					(next[i + 1] == 0 && next[i] == 0 && current[i + 1] == 0))
				{
					out[i] = 0;
				}
				else
				{
					out[i] = 255;
				}
			}
			else
			{
				out[i] = 255;
			}
		}
	}

	//split digit
	myRect rec[5];
	//compute weight
	for (int j = 1; j < afterFilter.rows - 15; j++)
	{
		for (int i = 1; i < afterFilter.cols - 11; i++)
		{
			cv::Point pt(i, j);
			long sum = computePatchValue(afterFilter, pt, 10, 14);
			if (sum < 28560)
			{
				cv::Point pt1 = pt;
				cv::Point pt2(pt.x + 8, pt.y + 12);
				myRect rect;
				rect.pt1 = pt1;
				rect.pt2 = pt2;
				rect.sum = sum;
				int area = pt1.x + 5;
				if (area < 22 && sum < rec[0].sum)
				{
					rec[0] = rect;
					rec[0].size++;
				}
				else if (22 <= area && area < 44 && sum < rec[1].sum)
				{
					if (rec[0].size > 0)
					{
						int dis = rec[1].pt1.x - rec[0].pt1.x;
						//compare with the one before, if not enough distance,then abort it.
						if (dis > 7)
						{
							rec[1] = rect;
							rec[1].size++;
						}
					}
					else
					{
						rec[1] = rect;
						rec[1].size++;
					}
				}
				else if (44 <= area && area < 79 && sum < rec[2].sum)
				{
					if (rec[1].size > 0)
					{
						int dis = rec[2].pt1.x - rec[1].pt1.x;
						if (dis > 7)
						{
							rec[2] = rect;
							rec[2].size++;
						}
					}
					else
					{
						rec[2] = rect;
						rec[2].size++;
					}

				}
				else if (79 <= area && area < 100 && sum < rec[3].sum)
				{
					if (rec[2].size > 0)
					{
						int dis = rec[3].pt1.x - rec[2].pt1.x;
						if (dis > 7)
						{
							rec[3] = rect;
							rec[3].size++;
						}
					}
					else
					{
						rec[3] = rect;
						rec[3].size++;
					}
				}
				else if (100 <= area && area < 114 && sum < rec[4].sum)
				{
					if (rec[3].size > 0)
					{
						int dis = rec[4].pt1.x - rec[3].pt1.x;
						if (dis > 7)
						{
							rec[4] = rect;
							rec[4].size++;
						}
					}
					else
					{
						rec[4] = rect;
						rec[4].size++;
					}
				}
			}
		}
	}

	//estimate symbol
	if (rec[0].size > 0 && rec[2].size > 0 && rec[4].size > 0)
	{
		for (int j = 1; j < afterFilter.rows - 9; j++)
		{
			for (int i = 1; i < afterFilter.cols - 3; i++)
			{
				cv::Point pt(i, j);
				long firstPart = computePatchValue(dest, pt, 2, 8);
				if (firstPart == 0)
				{
					cv::Point pt1(i - 3, j + 3);
					long secondPart = computePatchValue(dest, pt1, 8, 1);
					if (secondPart == 0)
					{
						cv::Mat roi = src(cv::Range(pt.y - 2, pt.y + 8), cv::Range(pt.x - 2, pt.x + 8));
						myRect rect;
						cv::Point point(i - 3, j);
						rect.pt1 = point;
						int area = pt1.x + 5;
						if (rec[0].pt1.x <= area && area < rec[2].pt1.x)
						{
							rec[1] = rect;
							rec[1].size++;
						}
						else if (rec[2].pt1.x <= area && area < rec[4].pt1.x)
						{
							rec[3] = rect;
							rec[3].size++;
						}
					}
				}
			}
		}
	}

	//package rects and symbols
	for (int i = 0; i < 5; i++)
	{
		if (rec[0].size > 0 && rec[2].size > 0 && rec[4].size > 0)
		{
			flag = true;
			if (rec[i].size > 0)
			{
				if (i == 1)
				{
					symbols[0] = 1;
				}
				else if (i == 3)
				{
					symbols[1] = 1;
				}
				else
				{
					cv::Mat roi = src(cv::Range(rec[i].pt1.y, rec[i].pt1.y + 14), cv::Range(rec[i].pt1.x, rec[i].pt1.x + 10));
					pics[i / 2] = roi;
				}
			}
		}
	}
	
}

bool CAPTCHA::isCrackSucceed()
{
	return flag;
}

std::vector<cv::Mat> CAPTCHA::getPicResults()
{
	return pics;
}

std::vector<int> CAPTCHA::getSymbols()
{
	return symbols;
}

void CAPTCHA::adaptiveThreshold(const cv::Mat& src, cv::Mat& dest, int threshold)
{
	CV_Assert(src.depth() == CV_8U);  // only accept uchar type

	dest.create(src.size(), src.type());
	int nWidth = src.rows;
	int nHeight = src.cols * src.channels();
	int* intImg = NULL;
	intImg = new int[nWidth * nHeight];
	memset(intImg, 0, nWidth * nHeight * sizeof(int));
	for (int i = 0; i < nWidth; i++)
	{
		const uchar* in = src.ptr<uchar>(i);
		int sum = 0;
		for (int j = 0; j < nHeight; j++)
		{
			sum += *in++;
			if (0 == i) {
				intImg[j] = sum;
			}
			else
			{
				intImg[i * nHeight + j] = intImg[(i - 1) * nHeight + j] + sum;
			}
		}
	}
	for (int i = 0; i < nWidth; i++)
	{
		const uchar* in = src.ptr<uchar>(i);
		uchar* out = dest.ptr<uchar>(i);
		for (int j = 0; j < nHeight; j++)
		{
			long count = 0;
			long sum = 0;
			int times = 1;
			int s = nWidth;
			while (s > 2)
			{
				int x1, x2, y1, y2;
				x1 = (i - s / 2) > 0 ? (i - s / 2) : 0;
				x2 = (i + s / 2) < nWidth ? (i + s / 2) : (nWidth - 1);
				y1 = (j - s / 2) > 0 ? (j - s / 2) : 0;
				y2 = (j + s / 2) < nHeight ? (j + s / 2) : (nHeight - 1);
				count += times * times * (x2 - x1) * (y2 - y1);
				sum += times * times * (intImg[x2 * nHeight + y2] - intImg[x2 * nHeight + y1]
					- intImg[x1 * nHeight + y2] + intImg[x1 * nHeight + y1]);
				times += 2;
				s /= 2;
			}
			float ratio = 1.0 * (*in++) * count / sum;

			if (ratio <= (100.0 - threshold) / 100)
			{
				*out++ = 0;
			}
			else
			{
				*out++ = 255;
			}
		}
	}
	delete[] intImg;
}

long CAPTCHA::computePatchValue(const cv::Mat src, cv::Point pt, int patch_width, int patch_height)
{
	long sum = 0l;
	int end_width = pt.x + patch_width;
	int end_height = pt.y + patch_height;
	for (int j = pt.y; j < end_height; j++)
	{
		const uchar* data = src.ptr<uchar>(j);
		for (int i = pt.x; i < end_width; i++)
		{
			sum += data[i];
		}
	}
	return sum;
}

std::string CAPTCHA::toStr(const int n)
{

	char t[256];
	std::string s;
	sprintf(t, "%d", n);
	s = t;
	return s;
}



