#include "gui.h"

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Shared_Image.H>

#include <memory>

const double point_size = 1. / 72.;

cv::Mat cameraMatrix, distCoeffs;
std::vector<cv::Mat> rvecs, tvecs;
std::vector<float> reprojErrs;
double totalAvgErr = 0;
std::unique_ptr<Fl_Text_Buffer> resultTextBuff;
std::vector<std::vector<cv::Point2f> > imagesPoints;

int main (int argc, char *argv[]) 
{
	resultTextBuff.reset(new Fl_Text_Buffer());

	fl_register_images();
	CalibUI ui;
	ui.Show(argc, argv);

	return Fl::run();
}

void OnPatternChanged(class Fl_Value_Input* i, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(i->window()->user_data());
	if (ui != nullptr)
	{
		ui->patternImageBox->SetPattern(ui->vertCornersInput->value(), 
										ui->horizCornersInput->value(),
										ui->cellSizeInput->value());
		ui->patternImageBox->redraw();
	}
}

void OnPrintPattern(class Fl_Button* b, void* )
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		std::unique_ptr<Fl_Printer> p(new Fl_Printer());
		if (!p->start_job(1)) 
		{
			if (!p->start_page())
			{
				int width(0), height(0);
				p->printable_rect(&width, &height);

				int left(0), top(0), right(0), bottom(0);
				p->margins (&left, &top, &right, &bottom);
				
				ui->patternImageBox->SetPointSize(point_size);
				ui->patternImageBox->SetPageSize(width, height);
				p->print_widget(ui->patternImageBox);

				p->end_page();
			}
			p->end_job();
		}
		ui->patternImageBox->redraw();
	}
}

void OnFitToPrinter(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		std::unique_ptr<Fl_Printer> p(new Fl_Printer());
		if (!p->start_job(1)) 
		{
			int width(0), height(0);
			p->printable_rect(&width, &height);

			int left(0), top(0), right(0), bottom(0);
			p->margins (&left, &top, &right, &bottom);
				
			ui->patternImageBox->SetPointSize(point_size);
			ui->patternImageBox->SetPageSize(width, height);

			p->end_job();
		}
		ui->patternImageBox->redraw();
	}
}

void OnPatternNext(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->next();
	}
}

void OnCalibImagesPrev(class Fl_Button* b, void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->prev();
	}
}

void OnCalibImagesNext(class Fl_Button* b, void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->next();
	}
}

void OnAddCalibImage(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		Fl_File_Chooser fc(".", "JPG (*.{jpg,jpeg})\tPNG (*.png)\tBMP (*.bmp)", Fl_File_Chooser::MULTI, "Select image");
		fc.show();
		while(fc.shown())
		{ 
			Fl::wait(); 
		}
		if (fc.value() != nullptr)
		{
			if (fc.count() > 1)
			{
				for (int i = 1; i < fc.count(); ++i)
				{
					char relative[FL_PATH_MAX];
					fl_filename_relative(relative, sizeof(relative), fc.value(i));
					ui->calibImageBrowser->add(relative);
				}
			}
			else
			{
				char relative[FL_PATH_MAX];
				fl_filename_relative(relative, sizeof(relative), fc.value());
				ui->calibImageBrowser->add(relative);
			}

			ui->calibImageBrowser->select(ui->calibImageBrowser->size());
			OnCalibImageSelected(ui->calibImageBrowser, nullptr);
		}
	}
}

void OnDelCalibImage(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		int i = ui->calibImageBrowser->value();
		ui->calibImageBrowser->remove(i);
		Fl_Image* img = ui->calibImageBox->image();
		Fl_Shared_Image* simg = dynamic_cast<Fl_Shared_Image*>(img);
		if (simg == nullptr)
		{
			delete img;
		}
		else
		{
			simg->release();
		}
		ui->calibImageBox->image(nullptr);
		ui->calibImageScroll->redraw();

		ui->calibImageBrowser->select(ui->calibImageBrowser->size());
		OnCalibImageSelected(ui->calibImageBrowser, nullptr);
	}
}

void OnCalibImageSelected(class Fl_File_Browser* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		Fl_Image* img = ui->calibImageBox->image();
		Fl_Shared_Image* simg = dynamic_cast<Fl_Shared_Image*>(img);
		if (simg == nullptr)
		{
			delete img;
		}
		else
		{
			simg->release();
		}
		int selection = b->value();
		const char* fileName = b->text(selection);
		if (fileName != nullptr)
		{
			simg =  Fl_Shared_Image::get(fileName);
			if (simg != nullptr)
			{
				ui->calibImageBox->image(simg);
				ui->calibImageBox->size(simg->w(), simg->h());
				ui->calibImageBox->redraw();
				if (static_cast<size_t>(selection - 1) < imagesPoints.size())
				{
					ui->calibImageBox->SetImagePoints(&imagesPoints[selection - 1]);
				}
				else
				{
					ui->calibImageBox->SetImagePoints(nullptr);
				}
			}
			else
			{
				fl_alert("Can't load an image!");
			}
		}
		ui->calibImageScroll->redraw();
	}
}

void OnCalibSave(class Fl_Button *,void *)
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
		fs << "camera_matrix" << cameraMatrix;
		fs << "distortion_coefficients" << distCoeffs;

		fs << "avg_reprojection_error" << totalAvgErr;
		if( !reprojErrs.empty() )
		fs << "per_view_reprojection_errors" << cv::Mat(reprojErrs);

		if( !rvecs.empty() && !tvecs.empty() )
		{
			CV_Assert(rvecs[0].type() == tvecs[0].type());
			cv::Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
			for( int i = 0; i < (int)rvecs.size(); i++ )
			{
				cv::Mat r = bigmat(cv::Range(i, i+1), cv::Range(0,3));
				cv::Mat t = bigmat(cv::Range(i, i+1), cv::Range(3,6));

				CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
				CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
				//*.t() is MatExpr (not Mat) so we can use assignment operator
				r = rvecs[i].t();
				t = tvecs[i].t();
			}
			cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
			fs << "extrinsic_parameters" << bigmat;
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

static double computeReprojectionErrors(const std::vector<std::vector<cv::Point3f> >& objectPoints,
										const std::vector<std::vector<cv::Point2f> >& imagePoints,
										const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs,
										const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs,
										std::vector<float>& perViewErrors)
{
	std::vector<cv::Point2f> imagePoints2;
	int i, totalPoints = 0;
	double totalErr = 0, err;
	perViewErrors.resize(objectPoints.size());

	for( i = 0; i < (int)objectPoints.size(); i++ )
	{
		cv::projectPoints(cv::Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
		err = cv::norm(cv::Mat(imagePoints[i]), cv::Mat(imagePoints2), CV_L2);
		int n = (int)objectPoints[i].size();
		perViewErrors[i] = (float)std::sqrt(err*err/n);
		totalErr += err*err;
		totalPoints += n;
	}

	return std::sqrt(totalErr/totalPoints);
}

bool runCalibration(const std::vector<std::vector<cv::Point2f> >& imagePoints,
					cv::Size imageSize, cv::Size boardSize,
					double squareSize, 
					cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
					std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs,
					std::vector<float>& reprojErrs,
					double& totalAvgErr)
{
	cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	std::vector<std::vector<cv::Point3f> > objectPoints(1);
	calcChessboardCorners(boardSize, squareSize, objectPoints[0]);

	objectPoints.resize(imagePoints.size(),objectPoints[0]);

	double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
					distCoeffs, rvecs, tvecs, CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

	bool ok = cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs);

	totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
				rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

	return ok;
}

void OnStartCalib(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->calibImageBox->SetImagePoints(nullptr);
		cv::Size imageSize;
		cv::Size boardSize(static_cast<int>(ui->horizCornersInput->value()), 
							static_cast<int>(ui->vertCornersInput->value()));
		
		imagesPoints.clear();
		ui->calibTextResult->buffer(resultTextBuff.get());

		resultTextBuff->text("Calibration in progress ...");
		ui->calibTextResult->redraw();
		Fl::wait();

		for (int i = 1; i <= ui->calibImageBrowser->size(); ++i)
		{
			const char* fileName = ui->calibImageBrowser->text(i);
			Fl_Shared_Image *img = Fl_Shared_Image::get(fileName);
			cv::Mat mat = cv::Mat::zeros(img->w(), img->h(), CV_8UC3);

			
			for (int x = 0; x < img->w(); ++x)
			{
				for (int y = 0; y < img->h(); ++y)
				{
					cv::Vec3b& cvp = mat.at<cv::Vec3b>(x,y);
					
					long index = (y * img->w() * img->d()) + (x * img->d());
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

			imageSize = mat.size();

			std::vector<cv::Point2f> pointbuf;
			bool found = cv::findChessboardCorners(mat, boardSize, pointbuf,
				CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

			cv::Mat matGray;
			cv::cvtColor(mat, matGray, CV_BGR2GRAY);
			if(found)
			{
				cv::cornerSubPix(matGray, pointbuf, cv::Size(11,11),
							cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
				imagesPoints.push_back(pointbuf);
			}
			img->release();
		}

		if (!imagesPoints.empty())
		{
			double squareSize = ui->cellSizeInput->value();
			bool ok = runCalibration(imagesPoints, imageSize, boardSize, squareSize,
									cameraMatrix, distCoeffs,
									rvecs, tvecs, reprojErrs, totalAvgErr);
			if (ok)
			{
				std::stringstream buf;
				double pixelSize = 0;
				buf << "Focal length X (pixel rel. units) : " << cameraMatrix.at<double>(0,0) << "\n";
				buf << "Focal length Y (pixel rel. units) : " << cameraMatrix.at<double>(1,1) << "\n";
				buf << "Principal point x : " << cameraMatrix.at<double>(0,2) << "\n";
				buf << "Principal point y : " << cameraMatrix.at<double>(1,2) << "\n";
				resultTextBuff->text(buf.str().c_str());
				OnCalibImageSelected(ui->calibImageBrowser, nullptr);
				return;
			}
		}
	}
	resultTextBuff->text("Calibration failed!");
}

void OnCalibPrev(class Fl_Button* b, void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->prev();
	}
}

void OnStretchImages(class Fl_Check_Button* c,void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(c->window()->user_data());
	if (ui != nullptr)
	{
		ui->calibImageBox->SetStretch(c->value() > 0);
		ui->calibImageScroll->redraw();
	}
}

void OnRealSizeView(class Fl_Check_Button* c,void*)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(c->window()->user_data());
	if (ui != nullptr)
	{
		ui->patternImageBox->SetStretch(c->value() < 1);
		ui->imageScroll->redraw();
	}
}