#include "node_system.h"

static DirectX::Vector2 mouse = {};

Seed::NodeSystem::NodeSystem(const std::unique_ptr<Graphics>& graphics)
	: graphics_(graphics)
	, node_tex_(0)
	, input_port_tex_(0)
	, output_port_tex_(0)
	, nine_path_shader_(0)
	, default_shader_(0)
	, grab_node_(nullptr)
	, view_pos_(DirectX::Vector3::Zero)
	, screen_grab_(false)
{
	this->default_shader_ = graphics->LoadShader("default.hlsl");
	this->repeat_shader_ = graphics->LoadShader("repeat.hlsl");
	this->nine_path_shader_ = graphics->LoadShader("nine_patch.hlsl");
	this->bg_tex_ = graphics->LoadTexture("bg.png");
	this->node_tex_ = graphics->LoadTexture("node-simple.png");
	this->input_port_tex_ = graphics->LoadTexture("output_port.png");
	this->output_port_tex_ = graphics->LoadTexture("input_port.png");


}

void Seed::NodeSystem::Draw9Patch(const unsigned int & texture_id, const float & x, const float & y, const float & width, const float & height, const float & ang)
{
	this->graphics_->SetShader(this->nine_path_shader_);

	this->graphics_->SetShaderResourceFromTexture(0, { texture_id });

	DirectX::Vector2 size = this->graphics_->GetTextureSize(texture_id);

	DirectX::Vector2 scale = { width / size.x, height / size.y };

	this->np_cb_.geometry_.size = size;
	this->np_cb_.geometry_.scale = scale;
	this->np_cb_.border_.left = size.x / 3;
	this->np_cb_.border_.right = size.x - size.x / 3;
	this->np_cb_.border_.top = size.y / 3;
	this->np_cb_.border_.bottom = size.y - size.y / 3;

	this->graphics_->SetConstantBuffer(this->nine_path_shader_, &this->np_cb_);

	this->graphics_->DrawSquare(x, y, size.x * scale.x, size.y * scale.y, ang);
}

void Seed::NodeSystem::set_default_shader(const unsigned int & shader_id)
{
	this->default_shader_ = shader_id;
}

DirectX::Vector2 Seed::NodeSystem::GetMousePos(void)
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(static_cast<HWND>(this->graphics_->handle()), &point);
	point.x -= this->graphics_->width() * 0.5f;
	point.y -= this->graphics_->height() * 0.5f;

	return DirectX::Vector2(static_cast<float>(point.x), static_cast<float>(-point.y));
}

void Seed::NodeSystem::Update(void)
{
	mouse = GetMousePos();

	static auto old_mouse = mouse;

	static bool l_trigger = false;
	static bool r_trigger = false;

	static bool l_release = false;
	static bool r_release = false;

	{
		static bool now_frame = false;
		static bool old_frame = now_frame;

		now_frame = GetKeyState(VK_LBUTTON) & 0x80;

		l_trigger = !old_frame && now_frame;
		l_release = old_frame && !now_frame;

		old_frame = now_frame;
	}

	{
		static bool now_frame = false;
		static bool old_frame = now_frame;

		now_frame = GetKeyState(VK_RBUTTON) & 0x80;

		r_trigger = !old_frame && now_frame;
		r_release = old_frame && !now_frame;

		old_frame = now_frame;
	}

	if (l_release)
		this->screen_grab_ = false;

	if (this->screen_grab_)
		this->view_pos_ += mouse - old_mouse;

	bool on_node = false;
	bool direct_touch = true;

	auto view = DirectX::Matrix::CreateTranslation(this->view_pos_.x, this->view_pos_.y, 0);

	if (this->grab_node_)
		(*this->grab_node_)->set_position((*this->grab_node_)->position() + (mouse - old_mouse));

	bool nodes_break = false;
	for (auto & node : this->nodes_)
	{
		node->Update();

		for (int n = 0; n < node->input_port_cnt(); ++n)
		{
			auto & port = node->input_port(n);
			auto port_pos = port->position(view);

			if (port->connect())
			{
				auto & a = port_pos;
				auto & b = port->connect()->position(view);

				auto ang1 = DirectX::XMConvertToDegrees(atan2f(a.y - mouse.y, a.x - mouse.x));
				auto ang2 = DirectX::XMConvertToDegrees(atan2f(b.y - mouse.y, b.x - mouse.x) + DirectX::XM_PI);

				if (ang1 > 180.f)
					ang1 -= 360.f;
				if (ang1 < -180.f)
					ang1 += 360.f;

				if (ang2 > 180.f)
					ang2 -= 360.f;
				if (ang2 < -180.f)
					ang2 += 360.f;

				if (fabsf(ang1 - ang2) < 10.f)
					if (l_trigger)
					{
						port->connect()->set_connect(nullptr);
						port->set_connect(nullptr);
						nodes_break = true;
						direct_touch = false;
						break;
					}
			}

			if (DirectX::Vector2::Distance(mouse, port_pos) < 10.f)
			{
				node->input_port(n)->set_alpha(1);

				if (l_trigger)
					this->start_port_ = port.get();

				if (l_release && this->start_port_ && this->start_port_->type() == PortType::Output)
				{
					if (this->start_port_->var_type() == port->var_type())
					{
						port->set_connect(this->start_port_);
						this->start_port_->set_connect(port.get());
					}
				}

				direct_touch = false;
				nodes_break = true;
				break;
			}
			else
				node->input_port(n)->set_alpha(.5f);
		}
		for (int n = 0; n < node->output_port_cnt(); ++n)
		{
			auto & port = node->output_port(n);
			auto port_pos = port->position(view);
			if (DirectX::Vector2::Distance(mouse, port_pos) < 10.f)
			{
				node->output_port(n)->set_alpha(1);

				if (l_trigger)
					this->start_port_ = port.get();

				if (l_release && this->start_port_ && this->start_port_->type() == PortType::Input)
				{
					if (this->start_port_->var_type() == port->var_type())
					{
						port->set_connect(this->start_port_);
						this->start_port_->set_connect(port.get());
					}
				}

				direct_touch = false;
				nodes_break = true;
				break;
			}
			else
				node->output_port(n)->set_alpha(.5f);
		}

		if (nodes_break) break;

		if (fabsf(mouse.x - node->position(view).x) < node->size().x)
			if (fabsf(mouse.y - node->position(view).y) < node->size().y)
			{
				direct_touch = false;
				on_node = true;

				if (l_trigger)
				{
					this->grab_node_ = &node;
					break;
				}
			}
	}

	if (!nodes_break && !on_node && r_trigger)
		this->AddNodes<Node>(DirectX::Vector3(mouse.x, mouse.y, 0), view);

	if (direct_touch && l_trigger)
		this->screen_grab_ = true;

	//node->set_size(DirectX::Vector2(2.f, 2.f));

	//if (GetKeyState(VK_LBUTTON) & 0x80)
	//	for (auto & node : this->nodes_)
	//		node->set_size(DirectX::Vector2(2.f, 2.f));

	if (l_release)
	{
		this->grab_node_ = nullptr;
		this->start_port_ = nullptr;
	}

	old_mouse = mouse;
}

void Seed::NodeSystem::Draw(void)
{
	auto view = DirectX::Matrix::CreateTranslation(this->view_pos_.x, this->view_pos_.y, 0);

	auto sw = static_cast<float>(this->graphics_->width());
	auto sh = static_cast<float>(this->graphics_->height());

	{ // bg
		auto size = this->graphics_->GetTextureSize(this->bg_tex_);

		auto scale = this->repeat_cb_.uv_.scale = { sw / size.x, sh / size.y };
		this->repeat_cb_.uv_.scroll = { -this->view_pos_.x / size.x / 2, this->view_pos_.y / size.y / 2 };

		this->graphics_->SetView(DirectX::Matrix::Identity);
		this->graphics_->SetShader(this->repeat_shader_);
		this->graphics_->SetConstantBuffer(this->repeat_shader_, &this->repeat_cb_);
		this->graphics_->SetColor(DirectX::Color(1, 1, 1, 1));
		this->graphics_->DrawTexture(this->bg_tex_, { 0,0 }, scale, 0);
	}

	this->graphics_->SetShader(this->nine_path_shader_);
	for (auto & node : this->nodes_)
	{
		this->graphics_->SetView(view);
		this->graphics_->SetColor({ 1, 1, 1, 0.7f });
		this->Draw9Patch(this->node_tex_, node->position().x, node->position().y, node->size().x, node->size().y, 0);
	}

	this->graphics_->SetShader(this->default_shader_);
	for (auto & node : this->nodes_)
	{
		for (int n = 0; n < node->input_port_cnt(); ++n)
		{ // hole-left
			auto & port = node->input_port(n);
			this->graphics_->SetColor(node->input_port(n)->color());
			auto & size = node->size();
			this->graphics_->SetView(view);
			this->graphics_->DrawTexture(this->input_port_tex_, port->position(), { 0.1f, 0.1f }, 0);

			if (port->connect())
			{
				this->graphics_->SetShader(this->default_shader_);

				auto & a = port->position(view);
				auto & b = port->connect()->position(view);

				auto ang1 = DirectX::XMConvertToDegrees(atan2f(a.y - mouse.y, a.x - mouse.x));
				auto ang2 = DirectX::XMConvertToDegrees(atan2f(b.y - mouse.y, b.x - mouse.x) + DirectX::XM_PI);

				if (ang1 > 180.f)
					ang1 -= 360.f;
				if (ang1 < -180.f)
					ang1 += 360.f;

				if (ang2 > 180.f)
					ang2 -= 360.f;
				if (ang2 < -180.f)
					ang2 += 360.f;

				if (fabsf(ang1 - ang2) < 10.f)
					this->graphics_->SetColor(DirectX::Color(.2f, .2f, .2f, 1.f));
				else
					this->graphics_->SetColor(DirectX::Color(.2f, .2f, .2f, .8f));

				this->graphics_->SetShaderResourceFromTexture(0, { this->node_tex_ });
				this->graphics_->SetView(DirectX::Matrix::Identity);
				this->graphics_->DrawLine({ a.x, a.y }, port->connect()->position(view));
			}
		}

		for (int n = 0; n < node->output_port_cnt(); ++n)
		{ // hole-right
			auto & port = node->output_port(n);
			this->graphics_->SetColor(node->output_port(n)->color());
			auto & position = node->position();
			auto & size = node->size();
			this->graphics_->SetView(view);
			this->graphics_->DrawTexture(this->output_port_tex_, port->position(), { 0.1f, 0.1f }, 0);
		}
	}

	if (this->start_port_)
	{
		auto pos = this->start_port_->position(view);
		this->graphics_->SetShader(this->default_shader_);
		this->graphics_->SetColor(DirectX::Color(.2f, .2f, .2f, .8f));
		this->graphics_->SetShaderResourceFromTexture(0, { this->node_tex_ });
		this->graphics_->SetView(DirectX::Matrix::Identity);
		this->graphics_->DrawLine({ pos.x, pos.y }, mouse);
	}
}
