#include "window.h"
#include <QtWidgets/QApplication>

#include <map>

class T
{

};

std::map<int, T*> maps;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	window w;

	return a.exec();
}
