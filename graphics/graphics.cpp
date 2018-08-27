#include "graphics.h"

#include <d3d11.h>
#include <wrl/client.h>

#include "resource_pool.h"

class Seed::Graphics::Impl
{
	friend class Seed::Graphics;

	struct RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
	};

	struct DepthStencil
	{
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv_;
	};

public:
	Impl(void);

private:
	void * handle_;
	unsigned int width_;
	unsigned int height_;

private:
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;

private:
	ResourcePool<RenderTarget> render_targets_;
	ResourcePool<DepthStencil> depth_stencils_;

private:
	void Initialize(void);
};

Seed::Graphics::Graphics(void)
	: impl_(std::make_unique<Graphics::Impl>())
{
}

Seed::Graphics::~Graphics(void)
{
}

void Seed::Graphics::ConnectToWindow(void * handle, const unsigned int & width, const unsigned int & height)
{
	this->impl_->handle_ = handle;
	this->impl_->width_ = width;
	this->impl_->height_ = height;

	this->impl_->Initialize();
}

void Seed::Graphics::ClearRenderTarget(const std::vector<unsigned int>& render_target_id_s)
{
	float color[4] = { .2f,.4f,.8f,1.f };
	for (auto & rt : render_target_id_s)
		this->impl_->device_context_->ClearRenderTargetView(this->impl_->render_targets_.Get(rt)->rtv_.Get(), color);
}

void Seed::Graphics::ClearDepthStencil(const std::vector<unsigned int>& depth_stencil_id_s)
{
	for (auto & ds : depth_stencil_id_s)
		this->impl_->device_context_->ClearDepthStencilView(this->impl_->depth_stencils_.Get(ds)->dsv_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Seed::Graphics::SetRenderTarget(const std::vector<unsigned int>& render_target_id_s, const unsigned int & depth_stencil_id)
{
	auto & self = this->impl_;

	std::vector<ID3D11RenderTargetView*> rtvs_;

	for (auto & rt : render_target_id_s)
		rtvs_.emplace_back(self->render_targets_.Get(rt)->rtv_.Get());

	auto dsv = self->depth_stencils_.Get(depth_stencil_id)->dsv_.Get();

	self->device_context_->OMSetRenderTargets(static_cast<unsigned int>(rtvs_.size()), rtvs_.data(), dsv);
}

void Seed::Graphics::SetShaderResourceFromRenderTarget(const unsigned int & start_slot, const std::vector<unsigned int>& render_target_id_s)
{
}

void Seed::Graphics::SetShaderResourceFromTexture(const unsigned int & start_slot, const std::vector<unsigned int>& texture_id_s)
{
}

void Seed::Graphics::SetShader(const unsigned int & shader_id)
{
}

void Seed::Graphics::SetConstantBuffer(void * constant_buffer)
{
}

void Seed::Graphics::SetWorld(DirectX::Matrix & world)
{
}

void Seed::Graphics::SetView(DirectX::Matrix & world)
{
}

void Seed::Graphics::SetProjection(DirectX::Matrix & world)
{
}

void Seed::Graphics::Present(const GraphicsFrameRate & frame_rate)
{
}

const unsigned int Seed::Graphics::LoadRenderTargetBackBuffer(const unsigned int & width, const unsigned int & height)
{
	auto & self = this->impl_;

	auto render_target = std::make_unique<Impl::RenderTarget>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex_2d;

		self->swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&tex_2d);

		device->CreateRenderTargetView(tex_2d.Get(), nullptr, render_target->rtv_.GetAddressOf());
	}

	return self->render_targets_.Load(render_target);
}

const unsigned int Seed::Graphics::LoadRenderTargetR32(const unsigned int & width, const unsigned int & height)
{
	return 0;
}

const unsigned int Seed::Graphics::LoadRenderTargetRGBA32(const unsigned int & width, const unsigned int & height)
{
	return 0;
}

const unsigned int Seed::Graphics::LoadRenderTargetRGBA128(const unsigned int & width, const unsigned int & height)
{
	return 0;
}

const unsigned int Seed::Graphics::LoadDepthsStencil(const unsigned int & width, const unsigned int & height)
{
	auto & self = this->impl_;

	auto depth_stencil = std::make_unique<Impl::DepthStencil>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex_2d;

	//深度マップテクスチャをレンダーターゲットにする際のデプスステンシルビュー用のテクスチャーを作成
	D3D11_TEXTURE2D_DESC tex_desc = {};
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_D32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	device->CreateTexture2D(&tex_desc, nullptr, tex_2d.GetAddressOf());
	device->CreateDepthStencilView(tex_2d.Get(), nullptr, depth_stencil->dsv_.GetAddressOf());

	return self->depth_stencils_.Load(depth_stencil);
}

void Seed::Graphics::UnloadRenderTarget(const unsigned int & render_target_id)
{
	this->impl_->render_targets_.Unload(render_target_id);
}

void Seed::Graphics::UnloadDepthsStencil(const unsigned int & depth_stencil_id)
{
	this->impl_->depth_stencils_.Unload(depth_stencil_id);
}

void Seed::Graphics::DrawSquare(const float & x, const float & y, const float & width, const float & height)
{
}

Seed::Graphics::Impl::Impl(void)
{
}

void Seed::Graphics::Impl::Initialize(void)
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;

	// デバイスとスワップチェーンの作成 
	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = this->width_;
	sd.BufferDesc.Height = this->height_;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = static_cast<HWND>(this->handle_);
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	D3D_FEATURE_LEVEL feature_levels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL * feature_level = nullptr;

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		0, &feature_levels, 1, D3D11_SDK_VERSION, &sd, this->swap_chain_.GetAddressOf(), device.GetAddressOf(),
		feature_level, &this->device_context_);
}
