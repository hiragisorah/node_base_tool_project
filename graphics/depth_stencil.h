#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace Seed
{
	struct DepthStencil
	{
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv_;
	};
}