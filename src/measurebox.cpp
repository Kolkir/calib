#include "measurebox.h"

#include <FL/fl_draw.H>

MeasureBox::MeasureBox(int x,int y, int w, int h)
	: Fl_Box(x, y, w, h)
	, x1(-1)
	, y1(-1)
	, x2(-1)
	, y2(-1)
	, mode(FIRST_MEASURER_POINT)
	, mousePressed(false)
	, scaleValue(1)
{
	this->box(FL_NO_BOX);
	this->align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
}

MeasureBox::~MeasureBox()
{
}

void MeasureBox::SetMeasurerMode(MeasurerMode mode)
{
	this->mode = mode;
}

void MeasureBox::GetFirstPoint(int& x, int& y) const
{
	x = this->x1;
	y = this->y1;
}

void MeasureBox::GetSecondPoint(int& x, int& y) const
{
	x = this->x2;
	y = this->y2;
}

int MeasureBox::GetMeasureLength() const
{
	return static_cast<int>(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + 0.0));
}

void MeasureBox::SetScaleValue(double val)
{
	this->scaleValue = val;
}

void MeasureBox::SetMeasureChangeCallback(MeasureChangeFunc func)
{
	this->onMeasureChange = func;
}

void MeasureBox::draw()
{
	this->Fl_Box::draw();

	fl_color(255,255,0);
	fl_line_style(FL_SOLID, 2);
	fl_line(static_cast<int>(this->x() + this->x1 * this->scaleValue), 
			static_cast<int>(this->y() + this->y1 * this->scaleValue),
			static_cast<int>(this->x() + this->x2 * this->scaleValue), 
			static_cast<int>(this->y() + this->y2 * this->scaleValue));
}

int MeasureBox::handle(int event)
{
	switch(event)
	{
	case FL_PUSH:
		if (!this->mousePressed && Fl::event_button() == FL_LEFT_MOUSE)
		{
			this->mousePressed = true;
			Fl::focus(this);
			return 1;
		}
		break;
	case FL_RELEASE:
		if (this->mousePressed && Fl::event_button() == FL_LEFT_MOUSE)
		{
			if (this->mode == FIRST_MEASURER_POINT)
			{
				this->x1 = static_cast<int>((Fl::event_x() - this->x()) / this->scaleValue);
				this->y1 = static_cast<int>((Fl::event_y() - this->y()) / this->scaleValue);
			}
			else if (this->mode == SECOND_MEASURER_POINT)
			{
				this->x2 = static_cast<int>((Fl::event_x() - this->x()) / this->scaleValue);
				this->y2 = static_cast<int>((Fl::event_y() - this->y()) / this->scaleValue);
			}
			if (this->onMeasureChange)
			{
				this->onMeasureChange(this);
				this->redraw();
			}
		}
		this->mousePressed = false;
		return 1;
	}
	return Fl_Box::handle(event);
}
