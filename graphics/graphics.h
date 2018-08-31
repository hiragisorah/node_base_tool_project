#pragma once

//*
// @lang English
// @file graphics.h
// @brief Game Graphics Base Class to use create dynamic link library. All variables hidden with pImpl.
// @author Flauzino Vinicius
// @date August 27, 2018
//*

//*
// @lang ���{��
// @file graphics.h
// @brief DLL�쐬�ɗp����Q�[���O���t�B�b�N�X�̃x�[�X�N���X�B���ׂẴ����o�[�ϐ���pImpl�ɂ���ĉB������Ă���B
// @author �t���E�W�m ���B�j�V�E�X
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
		// @brief �`��Ώۂƕ`��̈�̎w��
		// @param (handle) �����̐���
		// @param (width) �����̐���
		// @param (height) �����̐���
		// @detail �O���t�B�b�N�X�̕`��ΏۃE�B���h�E�̃n���h���ƕ`��̈�̑傫�������m��
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