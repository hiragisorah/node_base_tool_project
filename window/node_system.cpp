#include "node_system.h"
#include "input.h"
#include "node_a.h"
#include "node_b.h"

#include <algorithm>

static DirectX::Matrix view = DirectX::Matrix::Identity;

static DirectX::Vector2 mouse = {};
static auto old_mouse = mouse;

static bool l_trigger = false;
static bool r_trigger = false;

static bool l_release = false;
static bool r_release = false;

static bool direct_touch = true;

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
	, zoom_(1)
{
	this->default_shader_ = graphics->LoadShader("default.hlsl");
	this->repeat_shader_ = graphics->LoadShader("repeat.hlsl");
	this->smooth_line_shader_ = graphics->LoadShader("smooth_line.hlsl");
	this->nine_path_shader_ = graphics->LoadShader("nine_patch.hlsl");
	this->bg_tex_ = graphics->LoadTexture("bg.png");
	this->node_tex_ = graphics->LoadTexture("node_simple.png");
	this->input_port_tex_ = graphics->LoadTexture("output_port.png");
	this->output_port_tex_ = graphics->LoadTexture("input_port.png");
	this->node_selector_tex_ = graphics->LoadTexture("node_selector.png");
	this->node_connector_tex_ = graphics->LoadTexture("node_connector.png");

	this->menu_nodes_.emplace_back(std::make_unique<NodeA>());
	this->menu_nodes_.emplace_back(std::make_unique<NodeB>());
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

void Seed::NodeSystem::DrawLine(const DirectX::Vector2 & start, const DirectX::Vector2 & end, const DirectX::Vector2 & size, const float & vtx_cnt, const Graphics::Topology & topology = Graphics::Topology::LineStrip)
{
	this->sl_cb_.p_.start = start;
	this->sl_cb_.p_.end = end;
	this->sl_cb_.vtx_cnt_ = vtx_cnt;
	this->sl_cb_.size_ = size;

	this->graphics_->SetConstantBuffer(this->smooth_line_shader_, &this->sl_cb_);

	this->graphics_->DrawVertices(topology, vtx_cnt);
}

static auto node_selector = false;
static DirectX::Vector2 selector_position = {};

void Seed::NodeSystem::NodeSelector(void)
{
	if (r_trigger && direct_touch)
	{
		node_selector = true;
		selector_position = mouse;
	}

	if (l_trigger && node_selector)
	{
		auto size = this->graphics_->GetTextureSize(this->node_selector_tex_);
		for (int n = 0; n < this->menu_nodes_.size(); ++n)
		{
			auto pos = selector_position - DirectX::Vector2(0.f, size.y + 25.f) * n;
			if (fabsf(mouse.x - pos.x) < size.x && fabsf(mouse.y - pos.y) < size.y)
			{
				auto node = this->AddNodes<Node>({selector_position.x, selector_position.y, 0}).get();
				*node = *this->menu_nodes_[n];
			}
		}
	}

	if (l_trigger) node_selector = false;
}

void Seed::NodeSystem::NodeSelectorDraw(void)
{
	if (node_selector)
	{
		this->graphics_->SetShader(this->default_shader_);
		this->graphics_->SetView(DirectX::XMMatrixIdentity());
		auto size = this->graphics_->GetTextureSize(this->node_selector_tex_);
		for (int n = 0; n < this->menu_nodes_.size(); ++n)
		{
			auto pos = selector_position - DirectX::Vector2(0.f, size.y + 25.f) * n;

			if (fabsf(mouse.x - pos.x) < size.x && fabsf(mouse.y - pos.y) < size.y)
				this->graphics_->SetColor(DirectX::Color(0.2f, .6f, 1.f, 1));
			else
				this->graphics_->SetColor(DirectX::Color(0, 0, 0, 1));

			this->graphics_->DrawTexture(this->node_selector_tex_, pos, { 1, 1 }, 0);
		}
	}
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

#undef min
#undef max

void Seed::NodeSystem::Update(void)
{
	mouse = GetMousePos();

	direct_touch = true;
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

	{
		this->zoom_ = std::max(std::min(this->zoom_ + Input::GetWheelY(), 5.f), 0.3f);
	}

	if (l_release)
		this->screen_grab_ = false;

	if (this->screen_grab_)
		this->view_pos_ += DirectX::Vector3(mouse.x - old_mouse.x, mouse.y - old_mouse.y, 0) / this->zoom_;

	bool on_node = false;

	view = DirectX::Matrix::CreateTranslation(this->view_pos_) * DirectX::Matrix::CreateScale(this->zoom_);

	if (this->grab_node_)
	{
		DirectX::Vector2 movement = (mouse - old_mouse) / this->zoom_;
		(*this->grab_node_)->set_position((*this->grab_node_)->position() + movement);
	}

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

				if (fabsf(ang1 - ang2) < 10.f * this->zoom_)
					if (l_trigger)
					{
						port->connect()->set_connect(nullptr);
						port->set_connect(nullptr);
						nodes_break = true;
						direct_touch = false;
						break;
					}
			}

			if (DirectX::Vector2::Distance(mouse, port_pos) < 10.f * this->zoom_)
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
			if (DirectX::Vector2::Distance(mouse, port_pos) < 10.f * this->zoom_)
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

		if (fabsf(mouse.x - node->position(view).x) < node->size().x * this->zoom_)
			if (fabsf(mouse.y - node->position(view).y) < node->size().y * this->zoom_)
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

	if (direct_touch && l_trigger)
		this->screen_grab_ = true;

	this->NodeSelector();

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
	view = DirectX::Matrix::CreateTranslation(this->view_pos_) * DirectX::Matrix::CreateScale(this->zoom_);

	auto sw = static_cast<float>(this->graphics_->width());
	auto sh = static_cast<float>(this->graphics_->height());

	DirectX::Matrix v;
	view.Invert(v);
	DirectX::Matrix mouse_m = DirectX::Matrix::CreateTranslation(mouse.x, mouse.y, 0) * v;
	auto w_mouse = mouse_m.Translation();

	this->graphics_->SetView(DirectX::XMMatrixIdentity());
	{ // bg
		DirectX::Vector2 size = this->graphics_->GetTextureSize(this->bg_tex_) * 0.1f * this->zoom_;

		auto scale = this->repeat_cb_.uv_.scale = DirectX::Vector2(sw / size.x, sh / size.y) * .5f;
		auto view_pos = view.Translation();
		this->repeat_cb_.uv_.scroll = { -view_pos.x / size.x / 2, view_pos.y / size.y / 2 };

		this->graphics_->SetShader(this->repeat_shader_);
		this->graphics_->SetConstantBuffer(this->repeat_shader_, &this->repeat_cb_);
		this->graphics_->SetColor(DirectX::Color(1, 1, 1, 1));
		this->graphics_->DrawTexture(this->bg_tex_, { 0,0 }, scale * 0.1f * this->zoom_, 0);
	}
	this->graphics_->SetView(view);

	this->graphics_->SetShader(this->nine_path_shader_);
	for (auto & node : this->nodes_)
	{
		node->Update();
		this->graphics_->SetColor({ 1, 1, 1, 0.7f });
		this->Draw9Patch(this->node_tex_, node->position().x, node->position().y, node->size().x, node->size().y, 0);
	}

	for (auto & node : this->nodes_)
	{
		this->graphics_->SetShader(this->smooth_line_shader_);

		for (int n = 0; n < node->input_port_cnt(); ++n)
		{
			auto & port = node->input_port(n);
			auto & size = node->size();

			if (port->connect())
			{
				auto & a = port->position();
				auto & b = port->connect()->position();

				auto ang1 = DirectX::XMConvertToDegrees(atan2f(a.y - mouse.y, a.x - w_mouse.x));
				auto ang2 = DirectX::XMConvertToDegrees(atan2f(b.y - mouse.y, b.x - w_mouse.x) + DirectX::XM_PI);

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

				this->graphics_->SetWorld(DirectX::Matrix::Identity);
				this->graphics_->SetColor(port->color());
				this->graphics_->SetShaderResourceFromTexture(0, { this->node_connector_tex_ });
				this->DrawLine({ a.x, a.y }, port->connect()->position(), { 5.f, 5.f }, 20.f);
				//this->graphics_->DrawLine({ a.x, a.y }, port->connect()->position());
			}
		}

		this->graphics_->SetShader(this->default_shader_);

		for (int n = 0; n < node->input_port_cnt(); ++n)
		{ // hole-left
			auto & port = node->input_port(n);
			this->graphics_->SetColor(node->input_port(n)->color());
			auto & size = node->size();
			this->graphics_->DrawTexture(this->input_port_tex_, port->position(), { 0.1f, 0.1f }, 0);
		}

		for (int n = 0; n < node->output_port_cnt(); ++n)
		{ // hole-right
			auto & port = node->output_port(n);
			this->graphics_->SetColor(node->output_port(n)->color());
			this->graphics_->DrawTexture(this->output_port_tex_, port->position(), { 0.1f, 0.1f }, 0);
		}
	}

	if (this->start_port_)
	{
		this->graphics_->SetShader(this->smooth_line_shader_);
		auto pos = this->start_port_->position();
		this->graphics_->SetColor(this->start_port_->color());
		this->graphics_->SetShaderResourceFromTexture(0, { this->node_connector_tex_ });
		this->graphics_->SetWorld(DirectX::Matrix::Identity);
		this->DrawLine({ pos.x, pos.y }, { w_mouse.x, w_mouse.y }, { 5.f, 5.f }, 20.f, Graphics::Topology::LineList);
		//this->graphics_->DrawLine({ pos.x, pos.y }, { w_mouse.x, w_mouse.y });
	}

	this->NodeSelectorDraw();
}
