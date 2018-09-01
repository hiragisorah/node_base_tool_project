#include "window.h"
#include "input.h"
#include <node_system.h>
#include <qtimeline.h>

static float wheel_x = 0;
static float wheel_y = 0;

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
		wheel_y = 0;
	}

	this->graphics_->Present(Seed::Graphics::FrameRate::Unlimited);
}

void window::wheelEvent(QWheelEvent * e)
{
	wheel_y = e->angleDelta().y() > 0 ? 0.1f : -0.1f;
}

const float & Seed::Input::GetWheelX(void)
{
	return wheel_x;
}

const float & Seed::Input::GetWheelY(void)
{
	return wheel_y;
}