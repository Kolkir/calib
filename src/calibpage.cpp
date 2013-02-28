#include "gui.h"

#include "utils.h"
#include "appdata.h"

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Shared_Image.H>

#include <memory>

void OnCalibSave(class Fl_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		Fl_File_Chooser fc(".", "XML (*.xml)", Fl_File_Chooser::SINGLE | Fl_File_Chooser::CREATE, "Save calibration results");
		fc.show();
		while(fc.shown())
		{ 
			Fl::wait(); 
		}
		if (fc.value() != nullptr)
		{
			cv::FileStorage fs(fc.value(), cv::FileStorage::WRITE);
			fs << "camera_matrix" << ui->appData->cameraMatrix;
			fs << "distortion_coefficients" << ui->appData->distCoeffs;
		}
	}
}

static void calcChessboardCorners(const cv::Size& boardSize, double squareSize, std::vector<cv::Point3f>& corners)
{
	corners.resize(0);
	for( int i = 0; i < boardSize.height; i++ )
	{
		for( int j = 0; j < boardSize.width; j++ )
		{
			corners.push_back(cv::Point3f(float(j*squareSize), float(i*squareSize), 0));
		}
	}
}

bool runCalibration(const std::vector<std::vector<cv::Point2f> >& imagePoints,
					cv::Size imageSize, cv::Size boardSize,
					double squareSize, 
					cv::Mat& cameraMatrix, cv::Mat& distCoeffs)
{
	cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	std::vector<std::vector<cv::Point3f> > objectPoints(1);
	calcChessboardCorners(boardSize, squareSize, objectPoints[0]);

	objectPoints.resize(imagePoints.size(),objectPoints[0]);

	std::vector<cv::Mat> rvecs, tvecs;

	double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
					distCoeffs, rvecs, tvecs, CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

	bool ok = cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs);

	return ok;
}

void OnStartCalib(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->calibImageBox->SetImagePoints(nullptr);
		cv::Size imageSize;
		cv::Size boardSize(static_cast<int>(ui->vertCornersInput->value()),
							static_cast<int>(ui->horizCornersInput->value()));
		
		ui->appData->imagesPoints.clear();
		ui->appData->imagesPointsMap.clear();
		ui->calibTextResult->buffer(ui->appData->resultTextBuff.get());

		ui->appData->resultTextBuff->text("Calibration in progress ...");
		ui->calibTextResult->redraw();
		Fl::wait();

		for (int i = 1; i <= ui->calibImageBrowser->size(); ++i)
		{
			Fl::wait();
			const char* fileName = ui->calibImageBrowser->text(i);
			Fl_Shared_Image *img = Fl_Shared_Image::get(fileName);
			cv::Mat mat = FLImageToMat(img);

			imageSize = mat.size();

			std::vector<cv::Point2f> pointbuf;
			bool found = cv::findChessboardCorners(mat, boardSize, pointbuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

			Fl::wait();
			cv::Mat matGray;
			cv::cvtColor(mat, matGray, CV_BGR2GRAY);
			if(found)
			{
				cv::cornerSubPix(matGray, pointbuf, cv::Size(11,11),
							cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
				ui->appData->imagesPoints.push_back(pointbuf);
				ui->appData->imagesPointsMap.insert(std::make_pair(fileName, ui->appData->imagesPoints.size() - 1));
			}
			img->release();
		}

		if (!ui->appData->imagesPoints.empty())
		{
			double squareSize = ui->cellSizeInput->value();
			bool ok = runCalibration(ui->appData->imagesPoints, imageSize, boardSize, squareSize,
									ui->appData->cameraMatrix, ui->appData->distCoeffs);
			if (ok)
			{
				std::stringstream buf;
				double pixelSize = 0;
				buf << "Focal length X (pixel rel. units) : " << ui->appData->cameraMatrix.at<double>(0,0) << "\n";
				buf << "Focal length Y (pixel rel. units) : " << ui->appData->cameraMatrix.at<double>(1,1) << "\n";
				buf << "Principal point x : " << ui->appData->cameraMatrix.at<double>(0,2) << "\n";
				buf << "Principal point y : " << ui->appData->cameraMatrix.at<double>(1,2) << "\n";

				buf << "K1 : " << ui->appData->distCoeffs.at<double>(0, 0) << "\n";
				buf << "K2 : " << ui->appData->distCoeffs.at<double>(1, 0) << "\n";
				buf << "K3 : " << ui->appData->distCoeffs.at<double>(4, 0) << "\n";
				buf << "P1 : " << ui->appData->distCoeffs.at<double>(2, 0) << "\n";
				buf << "P2 : " << ui->appData->distCoeffs.at<double>(3, 0) << "\n";

				ui->appData->resultTextBuff->text(buf.str().c_str());
				OnCalibImageSelected(ui->calibImageBrowser, nullptr);
				return;
			}
		}
	}
	ui->appData->resultTextBuff->text("Calibration failed!");
}

void OnCalibPrev(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->prev();
	}
}

void OnCalibNext(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->next();
	}
}