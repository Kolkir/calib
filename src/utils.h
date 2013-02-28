#ifndef _UTILS_H
#define _UTILS_H

#include <opencv2/core/core.hpp>
#include <FL/Fl_Image.H>

int GetPixelOffset(int x, int y, int width, int depth);

cv::Mat FLImageToMat(const Fl_Image* img);

Fl_Image* MatToFLImage(const cv::Mat& mat);

double Round(double val);

template<class T>
int GetPixelOffset(T* img, int x, int y)
{
	return GetPixelOffset(x, y, img->w(),img->d());
}

#endif