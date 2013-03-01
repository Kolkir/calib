#ifndef _MEASURE_BOX_H
#define _MEASURE_BOX_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>

#include <functional>

class MeasureBox : public Fl_Box
{
public:
	typedef std::function<void (MeasureBox*)> MeasureChangeFunc;
	enum MeasurerMode
	{
		FIRST_MEASURER_POINT,
		SECOND_MEASURER_POINT
	};

	MeasureBox(int x,int y, int w, int h);
	~MeasureBox();

	virtual void draw();
	virtual int handle(int event);

	void SetMeasurerMode(MeasurerMode mode);

	void GetFirstPoint(int& x, int& y) const;
	void GetSecondPoint(int& x, int& y) const;

	int GetMeasureLength() const;

	void SetScaleValue(double val);

	void SetMeasureChangeCallback(MeasureChangeFunc func);

private:
	MeasurerMode mode;
	int x1;
	int y1;
	int x2;
	int y2;
	bool mousePressed;
	double scaleValue;
	MeasureChangeFunc onMeasureChange;
};

#endif
