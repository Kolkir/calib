#include "gui.h"
#include "appdata.h"

#include <FL/Fl_File_Chooser.H>

void OnMeasureChange(MeasureBox* b)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->measurePixelLength->value(b->GetMeasureLength());
	}
}

void UpdateMeasureImage(CalibUI* ui)
{
	Fl_Image* img = ui->appData->measureImage.get();
	if (img != nullptr)
	{
		Fl_Image* newImage = img->copy(static_cast<int>(img->w() * ui->appData->measureImageZoomValue), 
										static_cast<int>(img->h() * ui->appData->measureImageZoomValue));

		if (newImage != nullptr)
		{
			Fl_Image* oldImg = ui->mesureImageBox->image();
			ui->mesureImageBox->image(newImage);
			if (oldImg != nullptr)
			{
				delete oldImg;
			}
			ui->mesureImageBox->size(newImage->w(), newImage->h());
			ui->mesureImageBox->SetMeasureChangeCallback(OnMeasureChange);
			ui->mainWindow->redraw();
		}
	}
}

void OnLoadMeasureImage(class Fl_Button *b,void *)
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
			if (fc.count() == 1)
			{
				char relative[FL_PATH_MAX];
				fl_filename_relative(relative, sizeof(relative), fc.value());
				Fl_Shared_Image* img = Fl_Shared_Image::get(relative);
				ui->appData->measureImage.reset(img->copy());
				UpdateMeasureImage(ui);
			}
		}
	}
}

void OnMeasurePrev(class Fl_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->wizard->prev();
	}
}

void OnMeasureCalculate(class Fl_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		double sensorHeight = ui->sensorHeightInput->value();
		double sensorWidth = ui->sensorWidthInput->value();
		double focalLength = ui->focalLengthInput->value();
		double pixelLength = ui->measurePixelLength->value();
		double realLength = ui->measureRealLength->value();
		double distToObj = ui->distToObjInput->value();

		if (pixelLength == 0 || realLength == 0 || distToObj == 0)
		{
			fl_alert("You have to enter pixel length, real length and distance to the object");
		}
		else
		{
			if (sensorHeight != 0 && sensorWidth != 0 && focalLength == 0)
			{
				double avSize = (sensorHeight + sensorWidth) / 2;

				
			}
			else if (sensorHeight == 0 && sensorWidth == 0 && focalLength != 0)
			{
				double magnification = focalLength / (distToObj - focalLength);
				double imgLenMM = realLength * magnification;
				double pixelPerMM = pixelLength / imgLenMM;
				ui->sensorHeightInput->value(ui->appData->measureImage->h() * pixelPerMM);
				ui->sensorWidthInput->value(ui->appData->measureImage->w() * pixelPerMM);
			}
			else
			{
				fl_alert("You have to enter focal length or sensor size either!");
			}
		}
	}
}

void OnMeasureZoomOut(class Fl_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->appData->measureImageZoomValue -= 0.05;
		if (ui->appData->measureImageZoomValue < 0)
		{
			ui->appData->measureImageZoomValue = 0.1;
		}
		ui->mesureImageBox->SetScaleValue(ui->appData->measureImageZoomValue);
		UpdateMeasureImage(ui);
	}
}

void OnMeasureZoomIn(class Fl_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		ui->appData->measureImageZoomValue += 0.05;
		ui->mesureImageBox->SetScaleValue(ui->appData->measureImageZoomValue);
		UpdateMeasureImage(ui);
	}
}

void OnChangeMeasureMode(class Fl_Check_Button *b,void *)
{
	CalibUI* ui = reinterpret_cast<CalibUI*>(b->window()->user_data());
	if (ui != nullptr)
	{
		if (b->value() == 1)
		{
			ui->mesureImageBox->SetMeasurerMode(MeasureBox::FIRST_MEASURER_POINT);
		}
		else
		{
			ui->mesureImageBox->SetMeasurerMode(MeasureBox::SECOND_MEASURER_POINT);
		}
	}
}

