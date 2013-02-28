#include "gui.h"

#include <FL/Fl_Printer.H>

namespace
{
	const double point_size = 1. / 72.;
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

