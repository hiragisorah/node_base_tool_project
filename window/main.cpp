#include "window.h"
#include <QtWidgets/QApplication>
#include "..\graphics\graphics.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	window w;
	
	Seed::Graphics graphics;
	
	auto handle = w.centralWidget()->winId();
	auto width = w.centralWidget()->width();
	auto height = w.centralWidget()->height();

	graphics.ConnectToWindow(&handle, width, height);

	auto bb = graphics.LoadRenderTargetBackBuffer(width, height);
	auto dsv = graphics.LoadDepthsStencil(width, height);

	graphics.ClearRenderTarget({ bb });
	graphics.ClearDepthStencil({ dsv });

	graphics.SetRenderTarget({ bb }, dsv);
	graphics.Present(Seed::Graphics::GraphicsFrameRate::VSync);
	
	w.show();

	return a.exec();
}
