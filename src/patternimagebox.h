#ifndef PATTERN_IMAGE_BOX
#define PATTERN_IMAGE_BOX

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Shared_Image.H>

class PatternImageBox : public Fl_Box
{
public:
	PatternImageBox(int x,int y, int w, int h);
	~PatternImageBox();

	virtual void draw();

	void SetPattern(double vertCorners, double horizCorners, double cellSize);
	void SetPageSize(int w, int h);
	void SetPointSize(double size);
	void SetStretch(bool val);
private:
	double vertCorners;
	double horizCorners;
	double cellSize;

	int pageWidth;
	int pageHeight;
	double pointSize;
	bool stretch;
};

#endif
