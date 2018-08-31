#pragma once

#include "..\graphics\SimpleMath.h"
#include <typeindex>
#include <unordered_map>

namespace Seed
{
	enum class PortType
	{
		Input,
		Output
	};

	class Port
	{
	public:
		Port(const PortType & type);

	protected:
		DirectX::Vector2 position_;
		DirectX::Color color_;

		Port * connect_;

		PortType type_;
		std::type_index var_type_;

	public:
		void set_position(const DirectX::Vector2 & position);
		const DirectX::Vector2 & position(void) const;
		const DirectX::Vector2 position(const DirectX::Matrix & view) const;
		void set_color(const DirectX::Color & color);
		void set_alpha(const float & alpha);
		const DirectX::Color & color(void) const;

		Port * connect(void) const;
		void set_connect(Port * const port);

		const PortType & type(void) const;
		const std::type_index & var_type(void) const;
	};

	template<class _Type>
	class TPort : public Port
	{
	public:
		TPort(const PortType & type)
			: Port(type)
		{
			DirectX::Color color = {};
			this->var_type_ = typeid(_Type);

			std::unordered_map<std::type_index, DirectX::Color> colors;

			colors[typeid(int)] = DirectX::Color(0, 1, 0, 1);
			colors[typeid(float)] = DirectX::Color(0, 1, 1, 1);
			colors[typeid(double)] = DirectX::Color(1, 0, 0, 1);
			colors[typeid(DirectX::Color)] = DirectX::Color(1, 0, 1, 1);
			colors[typeid(DirectX::Vector2)] = DirectX::Color(1, 1, 0, 1);
			colors[typeid(DirectX::Matrix)] = DirectX::Color(0, 0, .5f, 1);
			colors[typeid(std::string)] = DirectX::Color(0, 0, .5f, 1);

			this->set_color(colors[this->var_type_]);
		}
	};
}