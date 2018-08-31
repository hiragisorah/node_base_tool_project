#include "node_system.h"

static DirectX::Vector2 mouse = {};

Seed::NodeSystem::NodeSystem(const std::unique_ptr<Graphics>& graphics)
	: graphics_(graphics)
	, node_tex_(0)
	, port_tex_(0)
	, nine_path_shader_(0)
	, default_shader_(0)
	, grab_node_(nullptr)
{
	this->default_shader_ = graphics->LoadShader("default.hlsl");
	this->nine_path_shader_ = graphics->LoadShader("nine_patch.hlsl");
	this->node_tex_ = graphics->LoadTexture("node-simple.png");
	this->port_tex_ = graphics->LoadTexture("node-connect-hole.png");
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

	bool on_node = false;

	if (this->grab_node_)
		(*this->grab_node_)->set_position((*this->grab_node_)->position() + (mouse - old_mouse));

	bool nodes_break = false;
	for (auto & node : this->nodes_)
	{
		node->Update();

		for (int n = 0; n < node->input_port_cnt(); ++n)
		{
			auto & port = node->input_port(n);
			auto port_pos = port->position();
			if (DirectX::Vector2::Distance(mouse, port_pos) < 10.f)
			{
				node->input_port(n)->set_color(DirectX::Color(1, 1, 1, 1));

				if (l_trigger)
					this->start_port_ = port.get();

				if (l_release && this->start_port_)
				{
					port->set_connect(this->start_port_);
					this->start_port_->set_connect(port.get());
				}

				nodes_break = true;
				break;
			}
			else
				node->input_port(n)->set_color(DirectX::Color(1, 1, 1, .5f));
		}
		for (int n = 0; n < node->output_port_cnt(); ++n)
		{
			auto & port = node->output_port(n);
			auto port_pos = port->position();
			if (DirectX::Vector2::Distance(mouse, port_pos) < 10.f)
			{
				node->output_port(n)->set_color(DirectX::Color(1, 1, 1, 1));

				if (l_trigger)
					this->start_port_ = port.get();

				if (l_release && this->start_port_)
				{
					port->set_connect(this->start_port_);
					this->start_port_->set_connect(port.get());
				}

				nodes_break = true;
				break;
			}
			else
				node->output_port(n)->set_color(DirectX::Color(1, 1, 1, .5f));
		}

		if (nodes_break) break;

		if (fabsf(mouse.x - node->position().x) < node->size().x)
			if (fabsf(mouse.y - node->position().y) < node->size().y)
			{
				on_node = true;

				if (l_trigger)
				{
					this->grab_node_ = &node;
					break;
				}
			}
	}

	if (!nodes_break && !on_node && r_trigger)
		this->AddNodes<Node>(DirectX::Vector3(mouse.x, mouse.y, 0));
	
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
	this->graphics_->SetShader(this->nine_path_shader_);
	for (auto & node : this->nodes_)
	{
		this->Draw9Patch(this->node_tex_, node->position().x, node->position().y, node->size().x, node->size().y, 0);
	}

	this->graphics_->SetShader(this->default_shader_);
	for (auto & node : this->nodes_)
	{
		for (int n = 0; n < node->input_port_cnt(); ++n)
		{ // hole-left
			auto & port = node->input_port(n);
			this->graphics_->SetColor(node->input_port(n)->color());
			auto & position = node->position();
			auto & size = node->size();
			this->graphics_->DrawTexture(this->port_tex_, port->position(), { 0.2f, 0.2f }, 0);

			if (port->connect())
			{
				auto pos = port->position();
				this->graphics_->SetShader(this->default_shader_);
				this->graphics_->SetColor(DirectX::Color(.6f, .6f, 1.f, 1.f));
				this->graphics_->SetShaderResourceFromTexture(0, { this->node_tex_ });
				this->graphics_->DrawLine({ pos.x, pos.y }, port->connect()->position());
			}
		}

		for (int n = 0; n < node->output_port_cnt(); ++n)
		{ // hole-right
			auto & port = node->output_port(n);
			this->graphics_->SetColor(node->output_port(n)->color());
			auto & position = node->position();
			auto & size = node->size();
			this->graphics_->DrawTexture(this->port_tex_, port->position(), { 0.2f, 0.2f }, 0);

			if (port->connect())
			{
				auto pos = port->position();
				this->graphics_->SetShader(this->default_shader_);
				this->graphics_->SetColor(DirectX::Color(.6f, .6f, 1.f, 1.f));
				this->graphics_->SetShaderResourceFromTexture(0, { this->node_tex_ });
				this->graphics_->DrawLine({ pos.x, pos.y }, port->connect()->position());
			}
		}
	}

	if (this->start_port_)
	{
		auto pos = this->start_port_->position();
		this->graphics_->SetShader(this->default_shader_);
		this->graphics_->SetColor(DirectX::Color(.6f, .6f, 1.f, 1.f));
		this->graphics_->SetShaderResourceFromTexture(0, { this->node_tex_ });
		this->graphics_->DrawLine({ pos.x, pos.y }, mouse);
	}
}
