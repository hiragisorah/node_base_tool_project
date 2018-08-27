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
		enum class GraphicsFrameRate
		{
			VSync,
			Unlimited
		};

	public:
		Graphics(void);
		Graphics(Graphics const&) = delete;
		Graphics& operator= (Graphics const&) = delete;

		virtual ~Graphics(void);

	public:
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
		void ClearRenderTarget(const std::vector<unsigned int> & render_target_id_s);
		void ClearDepthStencil(const std::vector<unsigned int> & depth_stencil_id_s);
		void SetRenderTarget(const std::vector<unsigned int> & render_target_id_s, const unsigned int & depth_stencil_id);
		void SetShaderResourceFromRenderTarget(const unsigned int & start_slot, const std::vector<unsigned int> & render_target_id_s);
		void SetShaderResourceFromTexture(const unsigned int & start_slot, const std::vector<unsigned int> & texture_id_s);
		void SetShader(const unsigned int & shader_id);
		void SetConstantBuffer(void * constant_buffer);
		void SetWorld(DirectX::Matrix & world);
		void SetView(DirectX::Matrix & world);
		void SetProjection(DirectX::Matrix & world);
		void Present(const GraphicsFrameRate & frame_rate);

	public:
		const unsigned int LoadRenderTargetBackBuffer(const unsigned int & width, const unsigned int & height);
		const unsigned int LoadRenderTargetR32(const unsigned int & width, const unsigned int & height);
		const unsigned int LoadRenderTargetRGBA32(const unsigned int & width, const unsigned int & height);
		const unsigned int LoadRenderTargetRGBA128(const unsigned int & width, const unsigned int & height);

		const unsigned int LoadDepthsStencil(const unsigned int & width, const unsigned int & height);

		void UnloadRenderTarget(const unsigned int & render_target_id);
		void UnloadDepthsStencil(const unsigned int & depth_stencil_id);

	public:
		void DrawSquare(const float & x, const float & y, const float & width, const float & height);

	private:
		class Impl;

		std::unique_ptr<Impl> impl_;
	};
}