#include "window.h"
#include <node_system.h>

window::window(QWidget *parent)
	: QMainWindow(parent)
	, graphics_(std::make_unique<Seed::Graphics>())
{
	ui.setupUi(this);

	this->show();

	this->timer_ = new QTimer(this);
	connect(this->timer_, SIGNAL(timeout()), this, SLOT(Update()));
	this->timer_->start(1000 / 60);

	auto & graphics = this->graphics_;

	auto & widget = this->ui.centralWidget;

	auto handle = reinterpret_cast<void*>(widget->winId());
	auto width = widget->width();
	auto height = widget->height();

	graphics->ConnectToWindow(handle, width, height);

	this->bb_ = graphics->LoadRenderTargetBackBuffer(width, height);
	this->dsv_ = graphics->LoadDepthsStencil(width, height);
	this->vp_ = graphics->LoadViewPort(width, height);
	this->sh_default_ = graphics->LoadShader("default.hlsl");
	this->sh_9patch_ = graphics->LoadShader("nine_patch.hlsl");
	this->window_tex_ = graphics->LoadTexture("node-window-2.png");
	this->tex_node_hole_ = graphics->LoadTexture("node-connect-hole.png");
	this->tex_none_ = graphics->LoadTexture("none.png");
}

#undef max

void window::Update(void)
{
	{ // Last Pass
		this->graphics_->ClearRenderTarget({ this->bb_ });
		//this->graphics_->ClearDepthStencil({ this->dsv_ });
		this->graphics_->SetRenderTargetAndDepthStencil({ this->bb_ }, -1);
		this->graphics_->SetViewPort({ this->vp_ });

		static Seed::NodeSystem node_system(this->graphics_);

		static int xxxx = 0;

		node_system.Update();
		node_system.Draw();

		{ // node
			//static int hole_left_num = 2;
			//static int hole_right_num = 2;

			//if (GetKeyState('W') & 0x800)
			//	if (hole_right_num > 1)
			//		hole_right_num--;

			//if (GetKeyState('S') & 0x800)
			//	hole_right_num++;


			//if (GetKeyState('A') & 0x800)
			//	if (hole_left_num > 1)
			//		hole_left_num--;

			//if (GetKeyState('D') & 0x800)
			//	hole_left_num++;

			//const int hole_move = 45.f;

			//auto width = 100.f;
			//auto height = 70.f + std::max(hole_right_num, hole_left_num) * hole_move / 2.f;
			//
			//auto hole_start = height - 90;
			//
			//for (int n = 0; n < hole_right_num; ++n)
			//{ // hole-right
			//	this->graphics_->SetShader(this->sh_default_);
			//	this->graphics_->SetColor(DirectX::Color(.8f, .8f, .8f, 1));
			//	this->graphics_->DrawTexture(this->tex_node_hole_, { +width / 1.f, hole_start + n * -hole_move }, { .5f,.5f }, 0);
			//}

			//for (int n = 0; n < hole_left_num; ++n)
			//{ // hole-right
			//	this->graphics_->SetShader(this->sh_default_);
			//	this->graphics_->SetColor(DirectX::Color(.8f, .8f, .8f, 1));
			//	this->graphics_->DrawTexture(this->tex_node_hole_, { -width / 1.f, hole_start + n * -hole_move }, { .5f,.5f }, 0);
			//}

			//{ // main
			//	this->Draw9Patch(this->window_tex_, 0, 0, width, height, 0);
			//	this->graphics_->SetShader(this->sh_default_);
			//}
		}
	}

	this->graphics_->Present(Seed::Graphics::FrameRate::Unlimited);
}