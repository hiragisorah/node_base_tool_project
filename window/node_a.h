#pragma once

#include "node.h"

class NodeA : public Seed::Node
{
public:
	NodeA(void)
	{
		this->add_input_port<Seed::TPort<float>>();
		this->add_input_port<Seed::TPort<std::string>>();

		this->add_output_port<Seed::TPort<int>>();
	}
};