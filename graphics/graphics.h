#pragma once

//*
// @lang English
// @file graphics.h
// @brief Game Graphics Base Class to use create dynamic link library. All variables hidden with pImpl.
// @author Flauzino Vinicius
// @date August 27, 2018
//*

//*
// @lang 日本語
// @file graphics.h
// @brief DLL作成に用いるゲームグラフィックスのベースクラス。すべてのメンバー変数はpImplによって隠蔽されている。
// @author フラウジノ ヴィニシウス
// @date 2018/08/27
//*

#include <memory>
#include <vector>
#include <typeindex>

#include "SimpleMath.h"

namespace Seed
{
	class Graphics
	{
	public:
		enum class FrameRate
		{
			Unlimited,
			VSync
		};
		enum class Topology
		{
			PointList,
			LineList,
			LineStrip,
			TriangleList,
			TriangleStrip,
		};
		struct NinePatchBuffer
		{
			__declspec(align(16)) struct { DirectX::Vector2 size, scale; } geometry_;
			__declspec(align(16)) struct { float left, top, right, bottom; } border_;
		};

	public:
		Graphics(void);
		Graphics(Graphics const&) = delete;
		Graphics& operator= (Graphics const&) = delete;

		virtual ~Graphics(void);

	public:
		void Initialize(void);
		//*
		// @fn
		// 
		// @brief 描画対象と描画領域の指定
		// @param (handle) 引数の説明
		// @param (width) 引数の説明
		// @param (height) 引数の説明
		// @detail グラフィックスの描画対象ウィンドウのハンドルと描画領域の大きさをを確定
		//*
		void ConnectToWindow(void * handle, const unsigned int & width, const unsigned int & height);
		
	public:
		void * const handle(void);
		const unsigned int & width(void);
		const unsigned int & height(void);

	public:
		void ClearRenderTarget(const std::vector<unsigned int> & render_target_id_s);
		void ClearDepthStencil(const std::vector<unsigned int> & depth_stencil_id_s);
		void SetRenderTargetAndDepthStencil(const std::vector<unsigned int> & render_target_id_s, const unsigned int & depth_stencil_id);
		void SetViewPort(const std::vector<unsigned int> & view_port_id_s);
		void SetShaderResourceFromRenderTarget(const unsigned int & start_slot, const std::vector<unsigned int> & render_target_id_s);
		void SetShaderResourceFromTexture(const unsigned int & start_slot, const std::vector<unsigned int> & texture_id_s);
		void SetShader(const unsigned int & shader_id);
		void SetConstantBuffer(const unsigned int & shader_id, void * constant_buffer);
		void SetWorld(const DirectX::Matrix & world);
		void SetView(const DirectX::Matrix & view);
		void SetProjection(const DirectX::Matrix & projection);
		void SetColor(const DirectX::Color & color);
		void Present(const FrameRate & frame_rate);

	public:
		const unsigned int LoadRenderTargetBackBuffer(const unsigned int & width, const unsigned int & height);
		const unsigned int LoadRenderTargetR32(const unsigned int & width, const unsigned int & height);
		const unsigned int LoadRenderTargetRGBA32(const unsigned int & width, const unsigned int & height);
		const unsigned int LoadRenderTargetRGBA128(const unsigned int & width, const unsigned int & height);

		const unsigned int LoadDepthsStencil(const unsigned int & width, const unsigned int & height);

		const unsigned int LoadViewPort(const unsigned int & width, const unsigned int & height);

		const unsigned int LoadShader(const std::string & file_name);
		const unsigned int LoadTexture(const std::string & file_name);
		const unsigned int LoadMesh(const std::string & file_name);

		void UnloadRenderTarget(const unsigned int & render_target_id);
		void UnloadDepthsStencil(const unsigned int & depth_stencil_id);
		void UnloadViewPort(const unsigned int & view_port_id);
		void UnloadShader(const unsigned int & shader_id);
		void UnloadTexture(const unsigned int & texture_id);
		void UnloadMesh(const unsigned int & mesh_id);

		const DirectX::Vector2 GetTextureSize(const unsigned int & texture_id) const;

	public:
		void DrawVertices(const Topology & topology, const unsigned int & vertex_cnt);
		void DrawLine(const DirectX::Vector2 & start, const DirectX::Vector2 & end);
		void DrawSquare(const float & x, const float & y, const float & width, const float & height, const float & ang);
		void DrawTexture(const unsigned int & texture_id, const DirectX::Vector2 & position, const DirectX::Vector2 & scale, const float & ang);
		void DrawTextureFixed(const unsigned int & texture_id, const DirectX::Vector2 & position, const DirectX::Vector2 & size, const float & ang);

	public:
		class Impl;

	private:

		std::unique_ptr<Impl> impl_;
	};
}