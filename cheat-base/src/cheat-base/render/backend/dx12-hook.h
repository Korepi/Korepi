#pragma once
#include <Windows.h>

#include <d3d12.h>

#include <d3dcompiler.h>

#include <cheat-base/events/event.hpp>

namespace backend 
{
	void InitializeDX12Hooks();

	// Thanks to https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-directx11-users
	bool LoadTextureFromMemory(LPBYTE image_data, int image_width, int image_height, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle,
		ID3D12Resource** out_tex_resource, int* out_width, int* out_height);

	class DX12Events
	{
	public:
		inline static TEvent<> PreRenderEvent{};
		inline static TEvent<ID3D12GraphicsCommandList*> PostRenderEvent{};
		inline static TEvent<HWND, ID3D12Device*, UINT, ID3D12DescriptorHeap*> InitializeEvent{};
	};


}
