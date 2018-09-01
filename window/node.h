#pragma once

#include <memory>
#include <vector>

#include "..\graphics\SimpleMath.h"
#include "port.h"

namespace Seed
{
	class Node
	{
	public:
		Node(void);
		virtual ~Node(void);

	private:
		DirectX::Vector3 position_;
		DirectX::Vector2 size_;

	public:
		void set_position(const DirectX::Vector3 & position);
		const DirectX::Vector3 & position(void) const;
		const DirectX::Vector3 position(const DirectX::Matrix & view) const;
		void set_size(const DirectX::Vector2 & size);
		const DirectX::Vector2 & size(void);

	private:
		std::vector<std::unique_ptr<Port>> input_ports_;
		std::vector<std::unique_ptr<Port>> output_ports_;

	public:
		const unsigned int input_port_cnt(void);
		const unsigned int output_port_cnt(void);
		const std::unique_ptr<Port> & input_port(const unsigned int & port) const;
		const std::unique_ptr<Port> & output_port(const unsigned int & port) const;
		template<class _Port> void add_input_port(void)
		{
			this->input_ports_.emplace_back(std::make_unique<_Port>(PortType::Input));
		}
		template<class _Port> void add_output_port(void)
		{
			this->output_ports_.emplace_back(std::make_unique<_Port>(PortType::Output));
		}

	public:
		void Update(void);

	public:
		void operator=(const Node & node)
		{
			this->input_ports_.resize(0);

			for (auto & port : node.input_ports_)
			{
				this->add_input_port<TPort<int>>();
				*this->input_ports_.back() = *port;
			}

			this->output_ports_.resize(0);

			for (auto & port : node.output_ports_)
			{
				this->add_output_port<TPort<int>>();
				*this->output_ports_.back() = *port;
			}

			//this->output_ports_.resize(node.output_ports_.size());

			//for (int n = 0; n < this->input_ports_.size(); ++n)
			//	*this->input_ports_[n] = *node.input_ports_[n];
			//for (int n = 0; n < this->output_ports_.size(); ++n)
			//	*this->output_ports_[n] = *node.output_ports_[n];
		}
	};
}