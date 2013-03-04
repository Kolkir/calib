#include "gui.h"
#include "appdata.h"

#include <FL/Fl_File_Chooser.H>

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

		if (ui->appData->cameraMatrix.rows != 2 &&
			ui->appData->cameraMatrix.cols != 2)
		{
			fl_alert("There is no calibration data");
		}
		else
		{
			double fx = ui->appData->cameraMatrix.at<double>(0, 0);
			double fy = ui->appData->cameraMatrix.at<double>(1, 1);

			if (sensorHeight != 0 && sensorWidth != 0 && focalLength == 0)
			{
				focalLength = ((fx * sensorWidth) + (fy *sensorHeight)) / 2;
				ui->focalLengthInput->value(focalLength);
			}
			else if (sensorHeight == 0 && sensorWidth == 0 && focalLength != 0)
			{
				sensorHeight = focalLength / fy;
				sensorWidth = focalLength / fx;
				ui->sensorHeightInput->value(sensorHeight);
				ui->sensorWidthInput->value(sensorWidth);
			}
			else
			{
				fl_alert("You have to enter a focal length or a sensor size!");
			}
		}
	}
}

