#include "PatternImageBox.h"

#include <FL/fl_draw.H>

#include <algorithm>

PatternImageBox::PatternImageBox(int x,int y, int w, int h)
	: Fl_Box(x, y, w, h)
	, vertCorners(6)
	, horizCorners(7)
	, cellSize(30)
	, pageWidth(560)
	, pageHeight(807)
	, pointSize(1./72.)
	, stretch(false)
{
	this->box(FL_DOWN_FRAME);
	this->color(45);
	this->align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	this->size(this->pageWidth, this->pageHeight);
}

PatternImageBox::~PatternImageBox()
{
}

void PatternImageBox::SetPattern(double vertCorners, double horizCorners, double cellSize)
{
	this->vertCorners = vertCorners;
	this->horizCorners = horizCorners;
	this->cellSize = cellSize;
}

void PatternImageBox::SetPageSize(int w, int h)
{
	this->pageWidth = w;
	this->pageHeight = h;
	this->size(this->pageWidth, this->pageHeight);
}

void PatternImageBox::SetPointSize(double size)
{
	this->pointSize = size;
}

void PatternImageBox::SetStretch(bool val)
{
	this->stretch = val;
}

void PatternImageBox::draw()
{
	this->Fl_Box::draw();

	double sw = 1;
	double sh = 1;
	if (this->stretch)
	{
		sw = static_cast<double>(this->parent()->w()) / static_cast<double>(this->w());
		sh = static_cast<double>(this->parent()->h()) / static_cast<double>(this->h());
	}

	fl_rectf(this->x(), 
			this->y(),
			static_cast<int>(this->w() * sw), 
			static_cast<int>(this->h() * sh), 
			0xff, 0xff, 0xff);

	int cellSizePix = static_cast<int>((this->cellSize * 1 / 25.4) / this->pointSize);

	int maxCells = static_cast<int>(std::max(this->horizCorners, this->vertCorners));
	if (maxCells % 2 > 0)
	{
		maxCells = maxCells / 2 + 1;
	}
	else 
	{
		maxCells = maxCells / 2;
	}
	int minCells = static_cast<int>(std::min(this->horizCorners, this->vertCorners));
	if (minCells % 2 > 0)
	{
		minCells = minCells / 2 + 1;
	}
	else 
	{
		minCells = minCells / 2;
	}

	if (this->w() > this->h())
	{
		int cellsW = (maxCells * 2) * cellSizePix;
		int cellsH = (minCells + maxCells) * cellSizePix;
		int xOffset = (this->w() - cellsW) / 2;
		int yOffset = (this->h() - cellsH) / 2;

		for (int y = 0; y < minCells + maxCells; ++y)
		{
			for (int x = 0; x < maxCells; ++x)
			{
				int startX = this->x() + static_cast<int>(xOffset * sw);
				int startY = this->y() + static_cast<int>(yOffset * sh);
				if (y % 2 > 0)
				{
					startX += static_cast<int>(cellSizePix * sw);
				}

				fl_rectf(startX +  static_cast<int>(x * cellSizePix * 2 * sw), 
						startY +  static_cast<int>(y * cellSizePix * sh), 
						static_cast<int>(cellSizePix * sw), 
						static_cast<int>(cellSizePix * sh), 
						0x00, 0x00,0x00);
			}
		}
	}
	else
	{
		int cellsH = (maxCells * 2) * cellSizePix;
		int cellsW = (minCells + maxCells) * cellSizePix;
		int xOffset = (this->w() - cellsW) / 2;
		int yOffset = (this->h() - cellsH) / 2;

		for (int x = 0; x < minCells + minCells + 1; ++x)
		{
			for (int y = 0; y < maxCells; ++y)
			{
				int startX = this->x() + static_cast<int>(xOffset * sw);
				int startY = this->y() + static_cast<int>(yOffset * sh);
				if (x % 2 > 0)
				{
					startY += static_cast<int>(cellSizePix * sh);
				}

				fl_rectf(startX +  static_cast<int>(x * cellSizePix * sw), 
						startY +  static_cast<int>(y * cellSizePix * 2 * sh), 
						static_cast<int>(cellSizePix * sw), 
						static_cast<int>(cellSizePix * sh), 
						0x00, 0x00,0x00);
			}
		}
	}
}

