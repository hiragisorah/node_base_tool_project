#include "port.h"

Seed::Port::Port(const PortType & type)
	: color_(DirectX::Color(1, 1, 1, .5f))
	, connect_(nullptr)
	, type_(type)
	, var_type_(typeid(int))
{
}

void Seed::Port::set_position(const DirectX::Vector2 & position)
{
	this->position_ = position;
}

const DirectX::Vector2 & Seed::Port::position(void) const
{
	return this->position_;
}

const DirectX::Vector2 Seed::Port::position(const DirectX::Matrix & view) const
{
	auto matrix = DirectX::Matrix::CreateTranslation(this->position_.x, this->position_.y, 0) * view;
	auto pos = matrix.Translation();
	return DirectX::Vector2(pos);
}

void Seed::Port::set_color(const DirectX::Color & color)
{
	this->color_ = color;
}

void Seed::Port::set_alpha(const float & alpha)
{
	auto color = this->color();
	color.w = alpha;
	this->set_color(color);
}

const DirectX::Color & Seed::Port::color(void) const
{
	return this->color_;
}

Seed::Port * Seed::Port::connect(void) const
{
	return this->connect_;
}

void Seed::Port::set_connect(Port * const port)
{
	this->connect_ = port;
}

const Seed::PortType & Seed::Port::type(void) const
{
	return this->type_;
}

const std::type_index & Seed::Port::var_type(void) const
{
	return this->var_type_;
}
