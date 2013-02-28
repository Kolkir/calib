#include "gui.h"
#include "utils.h"
#include "appdata.h"

#include <FL/Fl_File_Chooser.H>

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
				std::map<std::string, int>::iterator i = ui->appData->imagesPointsMap.find(fileName);
				if (i != ui->appData->imagesPointsMap.end())
				{
					ui->calibImageBox->SetImagePoints(&ui->appData->imagesPoints[i->second]);
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
			if (ui->appData->distCoeffs.rows == 5)
			{
				Fl::wait();
				double k1 = ui->appData->distCoeffs.at<double>(0, 0);
				double k2 = ui->appData->distCoeffs.at<double>(1, 0);
				double p1 = ui->appData->distCoeffs.at<double>(2, 0);
				double p2 = ui->appData->distCoeffs.at<double>(3, 0);
				double k3 = ui->appData->distCoeffs.at<double>(4, 0);

				double fx = ui->appData->cameraMatrix.at<double>(0, 0);
				double fy = ui->appData->cameraMatrix.at<double>(1, 1);
				double cx = ui->appData->cameraMatrix.at<double>(0, 2);
				double cy = ui->appData->cameraMatrix.at<double>(1, 2);

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
		ui->patternImageScroll->redraw();
	}
}