#pragma once
#include <Windows.h>

#include <d3d11.h>
#include <d3d12.h>

#include <d3dcompiler.h>

#include <cheat-base/events/event.hpp>

namespace backend 
{
	void InitializeDX11Hooks();

	// Thanks to https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-directx11-users
	bool LoadTextureFromMemory(LPBYTE image_data, int image_width, int image_height, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

	class DX11Events
	{
	public:
		inline static TEvent<ID3D11DeviceContext*> RenderEvent{};
		inline static TEvent<HWND, ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*> InitializeEvent{};
		inline static TEvent<> FailedEvent{};
	};
}
