#pragma once

#include <d3d11.h>
#include <string>
#include <wrl/client.h>

namespace Seed
{
	struct Shader
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometry_shader_;
		Microsoft::WRL::ComPtr<ID3D11HullShader> hull_shader_;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> domain_shader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> unique_cb_;
	};
}