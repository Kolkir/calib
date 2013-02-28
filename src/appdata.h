#ifndef _APP_DATA_H
#define _APP_DATA_H

#include <FL/Fl.H>
#include <opencv2/core/core.hpp>

#include <string>
#include <map>
#include <vector>
#include <memory>

class AppData
{
public:
	cv::Mat cameraMatrix, distCoeffs;
	std::unique_ptr<Fl_Text_Buffer> resultTextBuff;
	std::vector<std::vector<cv::Point2f> > imagesPoints;
	std::map<std::string, int> imagesPointsMap;
};

#endif
