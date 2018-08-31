#pragma once

#include <memory>
#include <vector>

#include "..\graphics\graphics.h"
#include "node.h"

namespace Seed
{
	class NodeSystem
	{
	public:
		NodeSystem(const std::unique_ptr<Graphics> & graphics);

	private:
		const std::unique_ptr<Graphics> & graphics_;

	private:
		std::vector<std::unique_ptr<Node>> nodes_;
		std::unique_ptr<Node> * grab_node_;
		Port * start_port_;

	public:
		template<class _Node> void AddNodes(const DirectX::Vector3 & position)
		{
			this->nodes_.emplace_back(std::make_unique<_Node>());
			this->nodes_.back()->set_position(position);
		}

	private:
		void Draw9Patch(const unsigned int & texture_id, const float & x, const float & y, const float & width, const float & height, const float & ang);

	private:
		unsigned int node_tex_;
		unsigned int port_tex_;
		unsigned int nine_path_shader_;
		unsigned int default_shader_;

	public:
		void set_default_shader(const unsigned int & shader_id);

	public:
		DirectX::Vector2 GetMousePos(void);

	public:
		void Update(void);
		void Draw(void);

	private:
		struct NinePatchBuffer
		{
			__declspec(align(16)) struct { DirectX::Vector2 size, scale; } geometry_;
			__declspec(align(16)) struct { float left, top, right, bottom; } border_;
		};

	private:
		NinePatchBuffer np_cb_;
	};
}