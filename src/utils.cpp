#include "utils.h"

#include <memory>

int GetPixelOffset(int x, int y, int width, int depth)
{
	return (x * depth) + (y * width * depth);
}

cv::Mat FLImageToMat(const Fl_Image* img)
{
	cv::Mat mat = cv::Mat::zeros(img->h(), img->w(), CV_8UC3);

	for (int x = 0; x < img->w(); ++x)
	{
		for (int y = 0; y < img->h(); ++y)
		{
			cv::Vec3b& cvp = mat.at<cv::Vec3b>(y, x);
					
			long index = GetPixelOffset(img, x, y);
			switch ( img->count() ) 
			{
			case 1: 
				{
					const char *buf = img->data()[0];
					switch ( img->d() ) 
					{
					case 1: 
						{
							cvp[0] = cvp[1] = cvp[2] = *(buf+index);
							break;
						}
					case 3:
						cvp[2] = *(buf+index+0);
						cvp[1] = *(buf+index+1);
						cvp[0] = *(buf+index+2);
						break;
					}
					break;
				}
			}
		}
	}
	return mat;
}

Fl_Image* MatToFLImage(const cv::Mat& mat)
{
	const int depth = 3;
	std::vector<unsigned char> newData(mat.cols * mat.rows * depth);
	unsigned char* buf = newData.data();

	for (int x = 0; x < mat.cols; ++x)
	{
		for (int y = 0; y < mat.rows; ++y)
		{
			const cv::Vec3b& cvp = mat.at<cv::Vec3b>(y, x);
					
			long index = GetPixelOffset(x, y, mat.cols, depth);

			*(buf+index+0) = cvp[2];
			*(buf+index+1) = cvp[1];
			*(buf+index+2) = cvp[0];
		}
	}

	std::unique_ptr<Fl_RGB_Image> newImage(new Fl_RGB_Image(newData.data(), mat.cols,  mat.rows, depth, 0));
	Fl_Image* rez = newImage->copy();
	return rez;
}

double Round(double val)
{
	return val + (val >= 0 ? 0.5 : -0.5);
}