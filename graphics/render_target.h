#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace Seed
{
	struct RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
	};
}