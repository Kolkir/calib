#include "ImageBox.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Shared_Image.H>

#include <algorithm>

ImageBox::ImageBox(int x,int y, int w, int h)
	: Fl_Box(x, y, w, h)
	, stretch(false)
	, imagePoints(nullptr)
{
	this->box(FL_DOWN_FRAME);
	this->color(45);
	this->align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
}

ImageBox::~ImageBox()
{
}

void ImageBox::SetStretch(bool val)
{
	this->stretch = val;
}

void ImageBox::SetImagePoints(const ImagePoints* imagePoints)
{
	this->imagePoints = imagePoints;
}

void ImageBox::draw()
{
	double sw = 1;
	double sh = 1;

	if (this->stretch)
	{
		int scrollBarSize = 15;
		sw = static_cast<double>(this->parent()->w() - scrollBarSize) / static_cast<double>(this->w());
		sh = static_cast<double>(this->parent()->h() - scrollBarSize) / static_cast<double>(this->h());

		this->size(this->parent()->w() - scrollBarSize, this->parent()->h() - scrollBarSize);
	}
	else
	{
		if (this->image() != nullptr)
		{
			this->size(this->image()->w(), this->image()->h());
		}
	}

	this->Fl_Box::draw();

	if (this->stretch && this->image() != nullptr)
	{
		fl_rectf(this->x(), this->y(),this->w(), this->h(),0xff,0xff,0xff);
		Fl_Image* newImg = this->image()->copy(this->w(), this->h());

		fl_rectf(this->x(), 
				this->y(),
				this->w(), 
				this->h(), 
				0xff, 0xff, 0xff);

		newImg->draw(this->x(), this->y());
		Fl_Shared_Image* simg = dynamic_cast<Fl_Shared_Image*>(newImg);
		if (simg != nullptr)
		{
			simg->release();
		}
		else
		{
			delete newImg;
		}
	}

	if (this->imagePoints != nullptr)
	{
		fl_color(255,0,0);

		std::for_each(this->imagePoints->begin(), this->imagePoints->end(),
			[&](const cv::Point2f& point)
		{
			fl_begin_polygon();
			fl_circle(this->x() + static_cast<int>(point.x * sw), 
					this->y() + static_cast<int>(point.y * sh), 3);
			fl_end_polygon();
		});
	}
}

