#include "gui.h"

#include "appdata.h"

int main (int argc, char *argv[]) 
{
	AppData data;
	data.resultTextBuff.reset(new Fl_Text_Buffer());

	fl_register_images();
	CalibUI ui;
	ui.appData = &data;
	ui.Show(argc, argv);

	return Fl::run();
}

