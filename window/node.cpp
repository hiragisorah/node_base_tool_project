#include "node.h"
#include <algorithm>

#undef max

Seed::Node::Node(void)
	: position_(DirectX::Vector3::Zero)
	, size_(DirectX::Vector2(100.f, 40.f))
{
	this->add_input_port<Port>();
	this->add_input_port<Port>();
	this->add_input_port<Port>();
	this->add_output_port<Port>();
}

void Seed::Node::set_position(const DirectX::Vector3 & position)
{
	this->position_ = position;
}

const DirectX::Vector3 & Seed::Node::position(void)
{
	return this->position_;
}

void Seed::Node::set_size(const DirectX::Vector2 & size)
{
	this->size_ = size;
}

const DirectX::Vector2 & Seed::Node::size(void)
{
	return this->size_;
}

const unsigned int Seed::Node::input_port_cnt(void)
{
	return this->input_ports_.size();
}

const unsigned int Seed::Node::output_port_cnt(void)
{
	return this->output_ports_.size();
}

const std::unique_ptr<Seed::Port>& Seed::Node::input_port(const unsigned int & port) const
{
	return this->input_ports_[port];
}

const std::unique_ptr<Seed::Port>& Seed::Node::output_port(const unsigned int & port) const
{
	return this->output_ports_[port];
}

void Seed::Node::Update(void)
{
	this->size_.y = 40.f + std::max(this->input_ports_.size(), this->output_ports_.size()) * 40.f / 2.f;

	for (int n = 0; n < this->input_ports_.size(); ++n)
		this->input_ports_[n]->set_position({ this->position_.x + -this->size_.x + 20.f, this->position_.y + this->size_.y - 90.f + n * -40.f });
	for (int n = 0; n < this->output_ports_.size(); ++n)
		this->output_ports_[n]->set_position({ this->position_.x + +this->size_.x - 20.f, this->position_.y + this->size_.y - 90.f + n * -40.f });
}
