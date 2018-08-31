#pragma once

#include "..\graphics\SimpleMath.h"

namespace Seed
{
	class Port
	{
	public:
		Port(void);

	private:
		DirectX::Vector2 position_;
		DirectX::Color color_;

		Port * connect_;

	public:
		void set_position(const DirectX::Vector2 & position);
		const DirectX::Vector2 & position(void) const;
		void set_color(const DirectX::Color & color);
		const DirectX::Color & color(void) const;

		Port * connect(void) const;
		void set_connect(Port * const port);
	};
}