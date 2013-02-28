#ifndef IMAGE_BOX
#define IMAGE_BOX

#include <FL/Fl.H>
#include <FL/Fl_Box.H>

#include <vector>
#include <opencv2/core/core.hpp>


class ImageBox : public Fl_Box
{
	typedef std::vector<cv::Point2f> ImagePoints;
public:
	ImageBox(int x,int y, int w, int h);
	~ImageBox();

	virtual void draw();

	void SetStretch(bool val);
	void SetImagePoints(const ImagePoints* imagePoints);
private:
	bool stretch;
	const ImagePoints* imagePoints;
};

#endif
