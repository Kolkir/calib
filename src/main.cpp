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
std::unique_ptr<Fl_Text_Buffer> resultTextBuff;
std::vector<std::vector<cv::Point2f> > imagesPoints;
std::map<std::string, int> imagesPointsMap;

namespace
{

int GetPixelOffset(int x, int y, int width, int depth)
{
	return (x * depth) + (y * width * depth);
}

template<class T>
int GetPixelOffset(T* img, int x, int y)
{
	return GetPixelOffset(x, y, img->w(),img->d());
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

}

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
				std::map<std::string, int>::iterator i = imagesPointsMap.find(fileName);
				if (i != imagesPointsMap.end())
				{
					ui->calibImageBox->SetImagePoints(&imagesPoints[i->second]);
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

void OnUndistort(class Fl_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		Fl_Image* img = ui->calibImageBox->image();
		
		if (img != nullptr)
		{
			if (distCoeffs.rows == 5)
			{
				Fl::wait();
				double k1 = distCoeffs.at<double>(0, 0);
				double k2 = distCoeffs.at<double>(1, 0);
				double p1 = distCoeffs.at<double>(2, 0);
				double p2 = distCoeffs.at<double>(3, 0);
				double k3 = distCoeffs.at<double>(4, 0);

				double fx = cameraMatrix.at<double>(0, 0);
				double fy = cameraMatrix.at<double>(1, 1);
				double cx = cameraMatrix.at<double>(0, 2);
				double cy = cameraMatrix.at<double>(1, 2);

				std::vector<unsigned char> newData(img->w() * img->h() * img->d());

				const unsigned char* src = reinterpret_cast<const unsigned char*>(img->data()[0]);
				unsigned char* dst = newData.data();

				for (int x = 0; x < img->w(); ++x)
				{
					for (int y = 0; y < img->h(); ++y)
					{
						double xi = (x - cx) / fx;
						double yi = (y - cy) / fy;
						
						double r2 = xi * xi + yi * yi;
						double x2 = xi*xi, y2 = yi*yi;
						double _2xy = 2*xi*yi;
						double kr = 1 + ((k3 * r2 + k2) * r2 + k1) * r2;

						double xc = xi * kr + p1 *_2xy + p2 * (r2 + 2 * x2);
						double yc = yi * kr + p1 * (r2 + 2 * y2) + p2 * _2xy;
						
						int xcc = static_cast<int>(Round((xc * fx) + cx));
						int ycc = static_cast<int>(Round((yc * fy) + cy));

						if (xcc < img->w() && xcc >= 0 &&
							ycc < img->h() && ycc >= 0)
						{
							const unsigned char* s = src + GetPixelOffset(img, xcc, ycc);
							unsigned char*       d = dst + GetPixelOffset(img, x, y);

							for (int i = 0; i < img->d(); ++i)
							{
								*(d + i) = *(s + i);
							}
						}
					}
				}

				std::unique_ptr<Fl_RGB_Image> image(new Fl_RGB_Image(newData.data(), img->w(), img->h(), img->d(), 0));
				Fl_Image* newImage = image->copy();

				ui->calibImageBox->image(newImage);
				ui->calibImageBox->redraw();
		
				Fl_Shared_Image* simg = dynamic_cast<Fl_Shared_Image*>(img);
				if (simg == nullptr)
				{
					delete img;
				}
				else
				{
					simg->release();
				}
			}
		}
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
		
		imagesPoints.clear();
		imagesPointsMap.clear();
		ui->calibTextResult->buffer(resultTextBuff.get());

		resultTextBuff->text("Calibration in progress ...");
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
				imagesPoints.push_back(pointbuf);
				imagesPointsMap.insert(std::make_pair(fileName, imagesPoints.size() - 1));
			}
			img->release();
		}

		if (!imagesPoints.empty())
		{
			double squareSize = ui->cellSizeInput->value();
			bool ok = runCalibration(imagesPoints, imageSize, boardSize, squareSize,
									cameraMatrix, distCoeffs);
			if (ok)
			{
				std::stringstream buf;
				double pixelSize = 0;
				buf << "Focal length X (pixel rel. units) : " << cameraMatrix.at<double>(0,0) << "\n";
				buf << "Focal length Y (pixel rel. units) : " << cameraMatrix.at<double>(1,1) << "\n";
				buf << "Principal point x : " << cameraMatrix.at<double>(0,2) << "\n";
				buf << "Principal point y : " << cameraMatrix.at<double>(1,2) << "\n";

				buf << "K1 : " << distCoeffs.at<double>(0, 0) << "\n";
				buf << "K2 : " << distCoeffs.at<double>(1, 0) << "\n";
				buf << "K3 : " << distCoeffs.at<double>(4, 0) << "\n";
				buf << "P1 : " << distCoeffs.at<double>(2, 0) << "\n";
				buf << "P2 : " << distCoeffs.at<double>(3, 0) << "\n";

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

