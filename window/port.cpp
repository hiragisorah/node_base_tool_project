#include "port.h"

Seed::Port::Port(void)
	: color_(DirectX::Color(1, 1, 1, .5f))
	, connect_(nullptr)
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

void Seed::Port::set_color(const DirectX::Color & color)
{
	this->color_ = color;
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