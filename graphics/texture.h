#pragma once

#include <d3d11.h>
#include <string>
#include <wrl/client.h>

namespace Seed
{
	struct Texture
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vertex_Texture_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> unique_cb_;
	};
}