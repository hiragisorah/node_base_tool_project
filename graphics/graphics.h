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
		// @brief �`��Ώۂƕ`��̈�̎w��
		// @param (handle) �����̐���
		// @param (width) �����̐���
		// @param (height) �����̐���
		// @detail �O���t�B�b�N�X�̕`��ΏۃE�B���h�E�̃n���h���ƕ`��̈�̑傫�������m��
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