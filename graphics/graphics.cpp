#include "graphics.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "DirectXTex.h"
#include "resource_pool.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma comment(lib, "dxguid.lib")

#if defined(DEBUG) || defined(_DEBUG)
static constexpr DWORD SHADER_FLAGS = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#else
static constexpr DWORD SHADER_FLAGS = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

namespace Seed
{
	static constexpr const char * k_texture_dir = "../data/texture/";
	static constexpr const char * k_shader_dir = "../data/shader/";
	static constexpr const char * k_mesh_dir = "../data/mesh/";

	static constexpr const D3D11_PRIMITIVE_TOPOLOGY k_topology_s[] =
	{
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	};
}

class Seed::Graphics::Impl
{
	friend class Seed::Graphics;
	
public:
	struct RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
	};

	struct DepthStencil
	{
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv_;
	};

	struct ViewPort
	{
		D3D11_VIEWPORT vp_ = {};
	};

	struct Texture
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;

		unsigned int width_ = 0;
		unsigned int height_ = 0;
	};

	struct Shader
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> gs_;
		Microsoft::WRL::ComPtr<ID3D11HullShader> hs_;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> ds_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> ps_;

		Microsoft::WRL::ComPtr<ID3D11Buffer> cb_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> il_;
	};

	struct Mesh
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;

		unsigned int vertex_cnt_ = 0;
		unsigned int index_cnt_ = 0;
	};

	struct MainBuffer
	{
		DirectX::Matrix world_ = DirectX::Matrix::Identity;
		DirectX::Matrix view_ = DirectX::Matrix::Identity;
		DirectX::Matrix projection_ = DirectX::Matrix::Identity;
		DirectX::XMFLOAT2A view_port_ = { 0, 0 };
		DirectX::Color color_;
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
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb_;

private:
	ResourcePool<RenderTarget> render_targets_;
	ResourcePool<DepthStencil> depth_stencils_;
	ResourcePool<ViewPort> view_ports_;
	ResourcePool<Shader> shaders_;
	ResourcePool<Texture> textures_;
	ResourcePool<Mesh> meshes_;

private:
	MainBuffer main_buffer_;
};

Seed::Graphics::Impl::Impl(void)
	: handle_(nullptr)
	, width_(0)
	, height_(0)
{

}

Seed::Graphics::Graphics(void)
	: impl_(std::make_unique<Graphics::Impl>())
{
}

Seed::Graphics::~Graphics(void)
{
}

void Seed::Graphics::Initialize(void)
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;

	auto & self = this->impl_;

	// デバイスとスワップチェーンの作成 
	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = self->width_;
	sd.BufferDesc.Height = self->height_;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = static_cast<HWND>(self->handle_);
	sd.SampleDesc.Count = 2;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	D3D_FEATURE_LEVEL feature_levels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL * feature_level = nullptr;

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		0, &feature_levels, 1, D3D11_SDK_VERSION, &sd, self->swap_chain_.GetAddressOf(), device.GetAddressOf(),
		feature_level, &self->device_context_);

	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof(Impl::MainBuffer);
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED(device->CreateBuffer(&bd, nullptr, self->cb_.GetAddressOf())))
	{
	}
		//std::cout << "コンスタントバッファーの作成に失敗しました。" << std::endl;

	self->device_context_->UpdateSubresource(self->cb_.Get(), 0, nullptr, &self->main_buffer_, 0, 0);
	self->device_context_->VSSetConstantBuffers(1, 1, self->cb_.GetAddressOf());
	self->device_context_->GSSetConstantBuffers(1, 1, self->cb_.GetAddressOf());
	self->device_context_->HSSetConstantBuffers(1, 1, self->cb_.GetAddressOf());
	self->device_context_->DSSetConstantBuffers(1, 1, self->cb_.GetAddressOf());
	self->device_context_->PSSetConstantBuffers(1, 1, self->cb_.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state;

	D3D11_BLEND_DESC blend_desc = {};

	blend_desc.AlphaToCoverageEnable = false;
	blend_desc.IndependentBlendEnable = false;

	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blend_desc, blend_state.GetAddressOf());

	self->device_context_->OMSetBlendState(blend_state.Get(), nullptr, 0xffffffff);

	{
		D3D11_RASTERIZER_DESC desc = {};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rs;

		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		desc.FillMode = D3D11_FILL_SOLID;

		device->CreateRasterizerState(&desc, rs.GetAddressOf());
		this->impl_->device_context_->RSSetState(rs.Get());
	}
}

void * const Seed::Graphics::handle(void)
{
	return this->impl_->handle_;
}

const unsigned int & Seed::Graphics::width(void)
{
	return this->impl_->width_;
}

const unsigned int & Seed::Graphics::height(void)
{
	return this->impl_->height_;
}

void Seed::Graphics::ConnectToWindow(void * handle, const unsigned int & width, const unsigned int & height)
{
	this->impl_->handle_ = handle;
	this->impl_->width_ = width;
	this->impl_->height_ = height;

	this->impl_->main_buffer_.view_port_ = { static_cast<float>(width), static_cast<float>(height) };

	this->Initialize();
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

void Seed::Graphics::SetRenderTargetAndDepthStencil(const std::vector<unsigned int>& render_target_id_s, const unsigned int & depth_stencil_id)
{
	auto & self = this->impl_;

	std::vector<ID3D11RenderTargetView*> rtvs_;

	for (auto & rt : render_target_id_s)
		rtvs_.emplace_back(self->render_targets_.Get(rt)->rtv_.Get());

	ID3D11DepthStencilView * dsv = nullptr;

	if (depth_stencil_id != -1)
		dsv = self->depth_stencils_.Get(depth_stencil_id)->dsv_.Get();

	self->device_context_->OMSetRenderTargets(static_cast<unsigned int>(rtvs_.size()), rtvs_.data(), dsv);
}

void Seed::Graphics::SetViewPort(const std::vector<unsigned int> & view_port_id_s)
{
	auto & self = this->impl_;

	std::vector<D3D11_VIEWPORT> view_ports;

	for (auto & vp : view_port_id_s)
		view_ports.emplace_back(self->view_ports_.Get(vp)->vp_);

	self->device_context_->RSSetViewports(static_cast<unsigned int>(view_ports.size()), view_ports.data());
}

void Seed::Graphics::SetShaderResourceFromRenderTarget(const unsigned int & start_slot, const std::vector<unsigned int>& render_target_id_s)
{
	auto & self = this->impl_;

	std::vector<ID3D11ShaderResourceView*> srvs_;

	for (auto & rt : render_target_id_s)
		srvs_.emplace_back(self->render_targets_.Get(rt)->srv_.Get());

	self->device_context_->PSSetShaderResources(start_slot, static_cast<int>(render_target_id_s.size()), srvs_.data());
}

void Seed::Graphics::SetShaderResourceFromTexture(const unsigned int & start_slot, const std::vector<unsigned int>& texture_id_s)
{
	auto & self = this->impl_;

	std::vector<ID3D11ShaderResourceView*> srvs_;

	for (auto & tex : texture_id_s)
		srvs_.emplace_back(self->textures_.Get(tex)->srv_.Get());

	self->device_context_->PSSetShaderResources(start_slot, static_cast<int>(texture_id_s.size()), srvs_.data());
}

void Seed::Graphics::SetShader(const unsigned int & shader_id)
{
	auto & self = this->impl_;

	auto & shader = self->shaders_.Get(shader_id);

	self->device_context_->IASetInputLayout(shader->il_.Get());

	self->device_context_->VSSetShader(shader->vs_.Get(), nullptr, 0);
	self->device_context_->GSSetShader(shader->gs_.Get(), nullptr, 0);
	self->device_context_->HSSetShader(shader->hs_.Get(), nullptr, 0);
	self->device_context_->DSSetShader(shader->ds_.Get(), nullptr, 0);
	self->device_context_->PSSetShader(shader->ps_.Get(), nullptr, 0);

	self->device_context_->VSSetConstantBuffers(0, 1, shader->cb_.GetAddressOf());
	self->device_context_->GSSetConstantBuffers(0, 1, shader->cb_.GetAddressOf());
	self->device_context_->HSSetConstantBuffers(0, 1, shader->cb_.GetAddressOf());
	self->device_context_->DSSetConstantBuffers(0, 1, shader->cb_.GetAddressOf());
	self->device_context_->PSSetConstantBuffers(0, 1, shader->cb_.GetAddressOf());
}

void Seed::Graphics::SetConstantBuffer(const unsigned int & shader_id, void * constant_buffer)
{
	auto & self = this->impl_;

	auto & shader = self->shaders_.Get(shader_id);

	self->device_context_->UpdateSubresource(shader->cb_.Get(), 0, nullptr, constant_buffer, 0, 0);
}

void Seed::Graphics::SetWorld(const DirectX::Matrix & world)
{
	auto & self = this->impl_;

	self->main_buffer_.world_ = world;

	self->device_context_->UpdateSubresource(self->cb_.Get(), 0, nullptr, &self->main_buffer_, 0, 0);
}

void Seed::Graphics::SetView(const DirectX::Matrix & view)
{
	auto & self = this->impl_;

	self->main_buffer_.view_ = view;

	self->device_context_->UpdateSubresource(self->cb_.Get(), 0, nullptr, &self->main_buffer_, 0, 0);
}

void Seed::Graphics::SetProjection(const DirectX::Matrix & projection)
{
	auto & self = this->impl_;

	self->main_buffer_.projection_ = projection;

	self->device_context_->UpdateSubresource(self->cb_.Get(), 0, nullptr, &self->main_buffer_, 0, 0);
}

void Seed::Graphics::SetColor(const DirectX::Color & color)
{
	auto & self = this->impl_;

	self->main_buffer_.color_ = color;

	self->device_context_->UpdateSubresource(self->cb_.Get(), 0, nullptr, &self->main_buffer_, 0, 0);
}

void Seed::Graphics::Present(const FrameRate & frame_rate)
{
	this->impl_->swap_chain_->Present(static_cast<unsigned int>(frame_rate), 0);
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
	auto & self = this->impl_;

	auto render_target = std::make_unique<Impl::RenderTarget>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex_2d;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	device->CreateTexture2D(&desc, nullptr, tex_2d.GetAddressOf());

	RTVDesc.Format = desc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(tex_2d.Get(), &RTVDesc, render_target->rtv_.GetAddressOf());

	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(tex_2d.Get(), &SRVDesc, render_target->srv_.GetAddressOf());

	return self->render_targets_.Load(render_target);
}

const unsigned int Seed::Graphics::LoadRenderTargetRGBA32(const unsigned int & width, const unsigned int & height)
{
	auto & self = this->impl_;

	auto render_target = std::make_unique<Impl::RenderTarget>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex_2d;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	device->CreateTexture2D(&desc, nullptr, tex_2d.GetAddressOf());

	RTVDesc.Format = desc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(tex_2d.Get(), &RTVDesc, render_target->rtv_.GetAddressOf());

	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(tex_2d.Get(), &SRVDesc, render_target->srv_.GetAddressOf());

	return self->render_targets_.Load(render_target);
}

const unsigned int Seed::Graphics::LoadRenderTargetRGBA128(const unsigned int & width, const unsigned int & height)
{
	auto & self = this->impl_;

	auto render_target = std::make_unique<Impl::RenderTarget>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex_2d;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	device->CreateTexture2D(&desc, nullptr, tex_2d.GetAddressOf());

	RTVDesc.Format = desc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(tex_2d.Get(), &RTVDesc, render_target->rtv_.GetAddressOf());

	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(tex_2d.Get(), &SRVDesc, render_target->srv_.GetAddressOf());

	return self->render_targets_.Load(render_target);
}

const unsigned int Seed::Graphics::LoadDepthsStencil(const unsigned int & width, const unsigned int & height)
{
	auto & self = this->impl_;

	auto depth_stencil = std::make_unique<Impl::DepthStencil>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex_2d;

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

const unsigned int Seed::Graphics::LoadViewPort(const unsigned int & width, const unsigned int & height)
{
	auto & self = this->impl_;

	auto view_port = std::make_unique<Impl::ViewPort>();

	view_port->vp_.Width = static_cast<float>(width);
	view_port->vp_.Height = static_cast<float>(height);
	view_port->vp_.MaxDepth = 1.f;

	return self->view_ports_.Load(view_port);
}

static DXGI_FORMAT GetDxgiFormat(D3D_REGISTER_COMPONENT_TYPE type, BYTE mask)
{
	if (mask == 0x0F)
	{
		// xyzw
		switch (type)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case D3D_REGISTER_COMPONENT_UINT32:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		}
	}

	if (mask == 0x07)
	{
		// xyz
		switch (type)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case D3D_REGISTER_COMPONENT_UINT32:
			return DXGI_FORMAT_R32G32B32_UINT;
		}
	}

	if (mask == 0x3)
	{
		// xy
		switch (type)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			return DXGI_FORMAT_R32G32_FLOAT;
		case D3D_REGISTER_COMPONENT_UINT32:
			return DXGI_FORMAT_R32G32_UINT;
		}
	}

	if (mask == 0x1)
	{
		// x
		switch (type)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			return DXGI_FORMAT_R32_FLOAT;
		case D3D_REGISTER_COMPONENT_UINT32:
			return DXGI_FORMAT_R32_UINT;
		}
	}

	return DXGI_FORMAT_UNKNOWN;
}

static void CreateConstantBufferFromShader(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> & device_context, std::unique_ptr<Seed::Graphics::Impl::Shader> & shader, ID3DBlob * blob)
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;

	device_context->GetDevice(device.GetAddressOf());

	ID3D11ShaderReflection * reflector = nullptr;
	D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

	D3D11_SHADER_DESC shader_desc;
	reflector->GetDesc(&shader_desc);

	auto cb = reflector->GetConstantBufferByName("unique");

	int size = 0;
	D3D11_SHADER_BUFFER_DESC desc = {};
	cb->GetDesc(&desc);

	for (unsigned int j = 0; desc.Name != nullptr && j < desc.Variables; ++j)
	{
		auto v = cb->GetVariableByIndex(j);
		D3D11_SHADER_VARIABLE_DESC vdesc;
		v->GetDesc(&vdesc);
		if (vdesc.Size % 16)
			size += vdesc.Size + 16 - (vdesc.Size % 16);
		else
			size += vdesc.Size;
	}

	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = size;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED(device->CreateBuffer(&bd, nullptr, shader->cb_.GetAddressOf())))
	{
	}
	//std::cout << "コンスタントバッファーの作成に失敗しました。" << std::endl;
}

static void CreateInputLayoutFromShader(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> & device_context, std::unique_ptr<Seed::Graphics::Impl::Shader> & shader, ID3DBlob * blob)
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;

	device_context->GetDevice(device.GetAddressOf());

	ID3D11ShaderReflection * reflector = nullptr;
	D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

	D3D11_SHADER_DESC shader_desc;
	reflector->GetDesc(&shader_desc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> element;
	for (unsigned int i = 0; i < shader_desc.InputParameters; ++i) {
		D3D11_SIGNATURE_PARAMETER_DESC sigdesc;
		reflector->GetInputParameterDesc(i, &sigdesc);

		auto format = GetDxgiFormat(sigdesc.ComponentType, sigdesc.Mask);

		D3D11_INPUT_ELEMENT_DESC eledesc =
		{
			sigdesc.SemanticName
			, sigdesc.SemanticIndex
			, format
			, 0
			, D3D11_APPEND_ALIGNED_ELEMENT
			, D3D11_INPUT_PER_VERTEX_DATA
			, 0
		};

		element.emplace_back(eledesc);
	}

	if (!element.empty())
		if (FAILED(device->CreateInputLayout(&element[0], static_cast<unsigned int>(element.size()),
			blob->GetBufferPointer(), blob->GetBufferSize(), shader->il_.GetAddressOf())))
		{ }
			//std::cout << "インプットレイアウトの作成に失敗しました。" << std::endl;
}

const unsigned int Seed::Graphics::LoadShader(const std::string & file_name)
{
	auto file_path = k_shader_dir + file_name;

	auto & self = this->impl_;

	auto shader = std::make_unique<Impl::Shader>();

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	self->device_context_->GetDevice(device.GetAddressOf());

	ID3DBlob * blob = nullptr;
	ID3DBlob * error = nullptr;

	if (FAILED(D3DCompileFromFile(std::wstring(file_path.begin(), file_path.end()).c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", SHADER_FLAGS, 0, &blob, &error)))
	{
		if (error)
		{
			//char * err = (char*)error->GetBufferPointer();
			//if (error != nullptr)
			//	std::cout << __FUNCTION__ << "::" << err << std::endl;
			//else
			//	std::cout << __FUNCTION__ << "::シェーダーの読み込みに失敗しました。" << std::endl;
		}
	}
	else
	{
		device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader->vs_.GetAddressOf());
		CreateInputLayoutFromShader(self->device_context_, shader, blob);
		CreateConstantBufferFromShader(self->device_context_, shader, blob);
	}

	if (SUCCEEDED(D3DCompileFromFile(std::wstring(file_path.begin(), file_path.end()).c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "GS", "gs_5_0", SHADER_FLAGS, 0, &blob, &error)))
	{
		device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader->gs_.GetAddressOf());
		if(shader->cb_ == nullptr)
			CreateConstantBufferFromShader(self->device_context_, shader, blob);
	}

	if (SUCCEEDED(D3DCompileFromFile(std::wstring(file_path.begin(), file_path.end()).c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "HS", "hs_5_0", SHADER_FLAGS, 0, &blob, &error)))
	{
		device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader->hs_.GetAddressOf());
		if (shader->cb_ == nullptr)
			CreateConstantBufferFromShader(self->device_context_, shader, blob);
	}

	if (SUCCEEDED(D3DCompileFromFile(std::wstring(file_path.begin(), file_path.end()).c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "DS", "ds_5_0", SHADER_FLAGS, 0, &blob, &error)))
	{
		device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader->ds_.GetAddressOf());
		if (shader->cb_ == nullptr)
			CreateConstantBufferFromShader(self->device_context_, shader, blob);
	}

	if (SUCCEEDED(D3DCompileFromFile(std::wstring(file_path.begin(), file_path.end()).c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", SHADER_FLAGS, 0, &blob, &error)))
	{
		device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader->ps_.GetAddressOf());
		if (shader->cb_ == nullptr)
			CreateConstantBufferFromShader(self->device_context_, shader, blob);
	}

	return this->impl_->shaders_.Load(shader);
}

const unsigned int Seed::Graphics::LoadTexture(const std::string & file_name)
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;

	this->impl_->device_context_->GetDevice(device.GetAddressOf());

	auto file_path = k_texture_dir + file_name;

	auto texture = std::make_unique<Impl::Texture>();

	auto image = std::make_unique<DirectX::ScratchImage>();
	
	DirectX::LoadFromWICFile(std::wstring(file_path.begin(), file_path.end()).c_str(), DirectX::WIC_FLAGS_NONE, nullptr, *image);

	texture->width_ = static_cast<unsigned int>(image->GetMetadata().width);
	texture->height_ = static_cast<unsigned int>(image->GetMetadata().height);

	DirectX::CreateShaderResourceView(device.Get(), image->GetImages(), image->GetImageCount(), image->GetMetadata(), texture->srv_.GetAddressOf());

	return this->impl_->textures_.Load(texture);
}

const unsigned int Seed::Graphics::LoadMesh(const std::string & file_name)
{
	auto file_path = k_mesh_dir + file_name;

	auto mesh = std::make_unique<Impl::Mesh>();

	return this->impl_->meshes_.Load(mesh);
}

void Seed::Graphics::UnloadRenderTarget(const unsigned int & render_target_id)
{
	this->impl_->render_targets_.Unload(render_target_id);
}

void Seed::Graphics::UnloadDepthsStencil(const unsigned int & depth_stencil_id)
{
	this->impl_->depth_stencils_.Unload(depth_stencil_id);
}

void Seed::Graphics::UnloadViewPort(const unsigned int & view_port_id)
{
	this->impl_->view_ports_.Unload(view_port_id);
}

void Seed::Graphics::UnloadShader(const unsigned int & shader_id)
{
	this->impl_->shaders_.Unload(shader_id);
}

void Seed::Graphics::UnloadTexture(const unsigned int & texture_id)
{
	this->impl_->textures_.Unload(texture_id);
}

void Seed::Graphics::UnloadMesh(const unsigned int & mesh_id)
{
	this->impl_->meshes_.Unload(mesh_id);
}

const DirectX::Vector2 Seed::Graphics::GetTextureSize(const unsigned int & texture_id) const
{
	return DirectX::Vector2(static_cast<float>(this->impl_->textures_.Get(texture_id)->width_), static_cast<float>(this->impl_->textures_.Get(texture_id)->height_));
}

void Seed::Graphics::DrawVertices(const Topology & topology, const unsigned int & vertex_cnt)
{
	auto & tmp = k_topology_s[static_cast<unsigned int>(topology)];

	this->impl_->device_context_->IASetPrimitiveTopology(tmp);
	this->impl_->device_context_->Draw(vertex_cnt, 0);
}

void Seed::Graphics::DrawLine(const DirectX::Vector2 & start, const DirectX::Vector2 & end)
{
	auto center = start + (end - start) * .5f;

	auto ang = atan2f(start.y - end.y, start.x - end.x);

	auto trans = DirectX::Matrix::CreateTranslation(center.x, center.y, 0);
	auto rot = DirectX::Matrix::CreateRotationZ(ang);
	auto scale = DirectX::Matrix::CreateScale(DirectX::Vector2::Distance(end, start) * 0.5f, 6.f, 1.f);

	this->SetWorld(scale * rot * trans);

	this->DrawVertices(Topology::TriangleStrip, 4);
}

void Seed::Graphics::DrawSquare(const float & x, const float & y, const float & width, const float & height, const float & ang)
{
	auto trans = DirectX::Matrix::CreateTranslation(x, y, 0);
	auto rot = DirectX::Matrix::CreateRotationZ(ang);
	auto scale = DirectX::Matrix::CreateScale(width, height, 1.f);

	this->SetWorld(scale * rot * trans);

	this->DrawVertices(Topology::TriangleStrip, 4);
}

void Seed::Graphics::DrawTexture(const unsigned int & texture_id, const DirectX::Vector2 & position, const DirectX::Vector2 & scale, const float & ang)
{
	auto size = this->GetTextureSize(texture_id);
	this->SetShaderResourceFromTexture(0, { texture_id });
	this->DrawSquare(position.x, position.y, size.x * scale.x, size.y * scale.y, ang);
}

void Seed::Graphics::DrawTextureFixed(const unsigned int & texture_id, const DirectX::Vector2 & position, const DirectX::Vector2 & size, const float & ang)
{
	this->SetShaderResourceFromTexture(0, { texture_id });
	this->DrawSquare(position.x, position.y, size.x, size.y, ang);
}
