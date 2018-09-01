#pragma once

#include "node.h"

class NodeB : public Seed::Node
{
public:
	NodeB(void)
	{
		this->add_input_port<Seed::TPort<int>>();

		this->add_output_port<Seed::TPort<double>>();
		this->add_output_port<Seed::TPort<std::string>>();
		this->add_output_port<Seed::TPort<double>>();
	}
};