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
		DirectX::Vector3 view_pos_;
		float zoom_;
		bool screen_grab_;
		std::vector<std::unique_ptr<Node>> menu_nodes_;

	public:
		template<class _Node> const std::unique_ptr<Node> & AddNodes(const DirectX::Vector3 & position, const DirectX::Matrix & view = DirectX::Matrix::Identity)
		{
			this->nodes_.emplace_back(std::make_unique<_Node>());

			DirectX::Matrix inv;
			view.Invert(inv);

			auto matrix = DirectX::Matrix::CreateTranslation(position) * inv;
			auto pos = matrix.Translation();
			this->nodes_.back()->set_position(pos);

			return this->nodes_.back();
		}

	private:
		void Draw9Patch(const unsigned int & texture_id, const float & x, const float & y, const float & width, const float & height, const float & ang);
		void DrawLine(const DirectX::Vector2 & start, const DirectX::Vector2 & end, const DirectX::Vector2 & size, const float & vtx_cnt, const Graphics::Topology & topology);

	private:
		unsigned int bg_tex_;
		unsigned int node_tex_;
		unsigned int input_port_tex_;
		unsigned int output_port_tex_;
		unsigned int node_selector_tex_;
		unsigned int node_connector_tex_;
		unsigned int default_shader_;
		unsigned int repeat_shader_;
		unsigned int smooth_line_shader_;
		unsigned int nine_path_shader_;

	public:
		void NodeSelector(void);
		void NodeSelectorDraw(void);

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

		struct RepeatBuffer
		{
			__declspec(align(16)) struct { DirectX::Vector2 scale, scroll; } uv_;
		};

		struct SmoothLineBuffer
		{
			__declspec(align(16)) struct { DirectX::Vector2 start, end; } p_;
			__declspec(align(16)) DirectX::Vector2 size_;
			__declspec(align(16)) float vtx_cnt_;
		};

	private:
		NinePatchBuffer np_cb_;
		RepeatBuffer repeat_cb_;
		SmoothLineBuffer sl_cb_;
	};
}