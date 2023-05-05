#include <pch.h>
#include "renderer.h"

#include <backends/imgui_impl_dx11.h>
#pragma comment(lib, "dxgi")
#include <backends/imgui_impl_dx12.h>

#include <backends/imgui_impl_win32.h>

#include <cheat-base/util.h>
#include <cheat-base/render/backend/dx11-hook.h>
#include <cheat-base/render/backend/dx12-hook.h>

#include <cheat-base/ResourceLoader.h>
#include <cheat-base/cheat/misc/Settings.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer
{
#pragma region Font

	static size_t ImWCharStringLen(const ImWchar* content)
	{
		size_t i = 0;
		while (content[i] != 0) i++;
		return i;
	}

	static ImWchar* ConcatGlyphRanges(const ImWchar* first, const ImWchar* second)
	{
		auto flen = ImWCharStringLen(first);
		auto slen = ImWCharStringLen(second);
		auto totalLength = flen + slen;
		auto content = new ImWchar[totalLength + 1];
		std::memcpy(content, first, flen * sizeof(ImWchar));
		std::memcpy(content + flen, second, slen * sizeof(ImWchar));
		return content;
	}

	Font::Font(const std::string& name, LPBYTE pData, DWORD dDataSize, FONT_RANGE ranges) :
		m_Name(name), m_Data(pData), m_DataSize(dDataSize), m_Ranges(nullptr)
	{
		AddRange(ranges);
	}

	Font::Font(const std::string& name, LPBYTE pData, DWORD dDataSize, const ImWchar* ranges)
		: m_Name(name), m_Data(pData), m_DataSize(dDataSize), m_Ranges(const_cast<ImWchar*>(ranges))
	{ }

	std::string Font::GetName() const
	{
		return m_Name;
	}

	ImFont* Font::AddImGuiFont(float fontSize, ImFontConfig* fontConfig)
	{
		ImGuiIO& io = ImGui::GetIO();
		return io.Fonts->AddFontFromMemoryTTF(GetData(), GetSize(), fontSize, fontConfig, GetGlyphRanges());
	}

	LPBYTE Font::GetData() const
	{
		return m_Data;
	}

	DWORD Font::GetSize() const
	{
		return m_DataSize;
	}

	const ImWchar* Font::GetGlyphRanges() const
	{
		return m_Ranges;
	}

	void Font::AddRange(FONT_RANGE ranges)
	{
		ImFontAtlas* fakeAtlas = nullptr;
#define RANGE_ADD(COMPARE_RTYPE, IMGUI_RANGE) if (ranges & COMPARE_RTYPE) { if (m_Ranges == nullptr) m_Ranges = const_cast<ImWchar*>(IMGUI_RANGE); else m_Ranges = ConcatGlyphRanges(m_Ranges, IMGUI_RANGE); }

		RANGE_ADD(FONT_RANGE_DEFAULT, fakeAtlas->GetGlyphRangesDefault());
		RANGE_ADD(FONT_RANGE_CHINESE_FULL, GetGlyphRangesChineseFull());
		RANGE_ADD(FONT_RANGE_CHINESE_SIMPLIFIED, fakeAtlas->GetGlyphRangesChineseSimplifiedCommon());
		RANGE_ADD(FONT_RANGE_CYRILLIC, fakeAtlas->GetGlyphRangesCyrillic());
		RANGE_ADD(FONT_RANGE_JAPANESE, fakeAtlas->GetGlyphRangesJapanese());
		RANGE_ADD(FONT_RANGE_KOREAN, fakeAtlas->GetGlyphRangesKorean());
		RANGE_ADD(FONT_RANGE_THAI, fakeAtlas->GetGlyphRangesThai());
		RANGE_ADD(FONT_RANGE_VIETNAMESE, fakeAtlas->GetGlyphRangesVietnamese());

#undef RANGE_ADD
	}

	void Font::AddRange(const ImWchar* ranges)
	{
		m_Ranges = ConcatGlyphRanges(m_Ranges, ranges);
	}

	std::shared_ptr<renderer::Font> Font::LoadFontFromResource(int resourceId, const char* resourceType, const std::string& fontName, FONT_RANGE ranges)
	{
		LPBYTE pData;
		DWORD dSize;

		if (ResourceLoader::LoadEx(resourceId, resourceType, pData, dSize))
			return std::make_shared<renderer::Font>(fontName, pData, dSize, ranges);

		return {};
	}

	std::shared_ptr<renderer::Font> Font::LoadFontFromResource(int resourceId, const char* resourceType, const std::string& fontName, const ImWchar* ranges)
	{
		LPBYTE pData;
		DWORD dSize;

		if (ResourceLoader::LoadEx(resourceId, resourceType, pData, dSize))
			return std::make_shared<renderer::Font>(fontName, pData, dSize, ranges);

		return {};
	}

	const ImWchar* Font::GetGlyphRangesChineseFull()
	{
		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x2000, 0x206F, // General Punctuation
			0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
			0x31F0, 0x31FF, // Katakana Phonetic Extensions
			0xFF00, 0xFFEF, // Half-width characters
			0xFFFD, 0xFFFD, // Invalid
			0x4e00, 0x9FAF, // CJK Ideograms
			0x3400, 0x4DFF, // CJK Unified Ideographs Extension A + Yijing Hexagram Symbols
			0,
		};
		return &ranges[0];
	}

#pragma endregion

#pragma region Font Composit

	FontComposit::FontComposit(const std::string& name, const std::initializer_list<Font>& fonts) : m_Name(name), m_Fonts(fonts)
	{ }

	FontComposit::FontComposit(const std::string& name, const std::vector<Font>& fonts) : m_Name(name), m_Fonts(fonts)
	{ }

	std::string FontComposit::GetName() const
	{
		return m_Name;
	}

	ImFont* FontComposit::AddImGuiFont(float fontSize, ImFontConfig* fontConfig)
	{
		ImFontConfig configCopy = *fontConfig;
		configCopy.MergeMode = true;

		ImGuiIO& io = ImGui::GetIO();
		ImFont* font = nullptr;

		if (!fontConfig->MergeMode)
			font = io.Fonts->AddFontDefault();

		for (auto& font : m_Fonts)
		{
			font.AddImGuiFont(fontSize, &configCopy);
		}

		return font;
	}

#pragma endregion

	struct TokenData
	{
		std::shared_ptr<IFont> font;
		float fontSize;
		ImFont* fontImGui;
	};

	static std::unordered_set<void*> _inputLockers;

	static ImFont* _currentFontImgui;
	static std::shared_ptr<IFont> _currentFont = {};
	static float _currentFontSize = 12.0f;

	static std::unordered_map<std::string, std::shared_ptr<IFont>> _fonts;
	static std::mutex _fontsMutex;

	static FontToken _maxToken = 0x1;

	static std::unordered_map<FontToken, TokenData> _fontTokens;
	static std::mutex _fontTokensMutex;

	static std::atomic<bool> _fontReloadRequested;

	static WNDPROC OriginalWndProcHandler;
	static ID3D11RenderTargetView* mainRenderTargetView;

	static void OnRenderDX11(ID3D11DeviceContext* pContext);
	static void OnInitializeDX11(HWND window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, IDXGISwapChain* pChain);

	static void OnPreRenderDX12();
	static void OnPostRenderDX12(ID3D12GraphicsCommandList* commandList);
	static void OnInitializeDX12(HWND window, ID3D12Device* pDevice, UINT buffersCounts, ID3D12DescriptorHeap* pDescriptorHeapImGuiRender);

	void Init(DXVersion version)
	{
		LOG_DEBUG("Initialize IMGui...");

		switch (version)
		{
		case renderer::DXVersion::D3D11:
			backend::DX11Events::RenderEvent += FUNCTION_HANDLER(OnRenderDX11);
			backend::DX11Events::InitializeEvent += FUNCTION_HANDLER(OnInitializeDX11);
			backend::InitializeDX11Hooks();
			break;
		case renderer::DXVersion::D3D12:
			backend::DX12Events::InitializeEvent += FUNCTION_HANDLER(OnInitializeDX12);
			backend::DX12Events::PreRenderEvent += FUNCTION_HANDLER(OnPreRenderDX12);
			backend::DX12Events::PostRenderEvent += FUNCTION_HANDLER(OnPostRenderDX12);
			backend::InitializeDX12Hooks();
			break;
		case renderer::DXVersion::D3D9:
		case renderer::DXVersion::D3D10:
		default:
			LOG_ERROR("Used unsupported version of DX.");
		}
	}

	void AddFont(std::shared_ptr<IFont> font)
	{
		std::lock_guard<std::mutex> _lock(_fontsMutex);
		
		if (_fonts.contains(font->GetName()))
			return;

		_fonts[font->GetName()] = font;
	}

	void SetDefaultFont(const std::string& fontName)
	{
		std::lock_guard<std::mutex> _lock(_fontsMutex);

		if (!_fonts.contains(fontName))
			return;

		_currentFont = _fonts.at(fontName);
		_fontReloadRequested = true;
	}

	ImFont* GetDefaultImGuiFont()
	{
		if (_currentFontImgui != nullptr)
			return _currentFontImgui;

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		return io.FontDefault;
	}

	std::string GetDefaultFontName()
	{
		if (!_currentFont)
			return {};

		return _currentFont->GetName();
	}

	void SetDefaultFontSize(float fontSize)
	{
		_currentFontSize = fontSize;
		_fontReloadRequested = true;
	}

	float GetDefaultFontSize()
	{
		return _currentFontSize;
	}

	renderer::FontToken CreateFontToken(const std::string& fontName, float fontSize)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		std::lock_guard<std::mutex> _lock2(_fontsMutex);

		if (!_fonts.contains(fontName))
			return {};

		_fontTokens[++_maxToken] = { _fonts.at(fontName), 12.0f, nullptr };
		_fontReloadRequested = true;
		return _maxToken;
	}

	bool DestroyFontToken(FontToken token)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		if (!_fontTokens.contains(token))
			return false;

		_fontTokens.erase(token);
		_fontReloadRequested = true;
		return true;
	}

	void SetTokenFontSize(FontToken token, float size)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		if (!_fontTokens.contains(token))
			return;

		_fontTokens.at(token).fontSize = size;
		_fontReloadRequested = true;
	}

	float GetTokenFontSize(FontToken token)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		if (!_fontTokens.contains(token))
			return 0.0f;

		return _fontTokens.at(token).fontSize;
	}

	void SetTokenFont(FontToken token, const std::string& fontName)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		if (!_fontTokens.contains(token))
			return;

		std::lock_guard<std::mutex> _lock2(_fontsMutex);
		if (!_fonts.contains(fontName))
			return;

		_fontTokens.at(token).font = _fonts.at(fontName);
		_fontReloadRequested = true;
	}

	std::string GetTokenFontName(FontToken token)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		if (!_fontTokens.contains(token))
			return {};

		return _fontTokens.at(token).font->GetName();
	}

	ImFont* GetTokenFont(FontToken token)
	{
		std::lock_guard<std::mutex> _lock(_fontTokensMutex);
		if (!_fontTokens.contains(token))
			return nullptr;

		auto& font = _fontTokens.at(token);
		if (font.fontImGui == nullptr)
		{
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			return io.FontDefault;
		}

		return font.fontImGui;
	}

	void CheckFonts()
	{
		if (!_fontReloadRequested)
			return;

		_fontReloadRequested = false;

		{
			std::lock_guard<std::mutex> _lock(_fontTokensMutex);
			for (auto& [token, font] : _fontTokens)
			{
				font.fontImGui = nullptr;
			}
		}

		_currentFontImgui = nullptr;

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->Clear();

		ImFontConfig fontConfig = {};
		fontConfig.FontDataOwnedByAtlas = false;

		if (_currentFont)
			_currentFontImgui = _currentFont->AddImGuiFont(_currentFontSize, &fontConfig);

		{
			std::lock_guard<std::mutex> _lock(_fontTokensMutex);
			for (auto& [token, font] : _fontTokens)
			{
				font.fontImGui = font.font->AddImGuiFont(font.fontSize, &fontConfig);
			}
		}
	}

	void SetInputLock(void* id, bool value)
	{
		if (value)
			AddInputLocker(id);
		else
			RemoveInputLocker(id);
	}

	void AddInputLocker(void* id)
	{
		if (_inputLockers.count(id) == 0)
			_inputLockers.insert(id);
	}

	void RemoveInputLocker(void* id)
	{
		if (_inputLockers.count(id) > 0)
			_inputLockers.erase(id);
	}

	bool IsInputLocked()
	{
		return _inputLockers.size() > 0;
	}

	static void LoadImGuiStyles()
	{
		auto& themes = cheat::feature::Settings::GetInstance();
		themes.Init();
	}

	static LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPreRenderDX12()
	{
		CheckFonts();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		if (!io.Fonts->IsBuilt())
		{
			io.Fonts->Build();
			ImGui_ImplDX11_InvalidateDeviceObjects();
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();

		if (_currentFontImgui != nullptr)
			io.FontDefault = _currentFontImgui;

		ImGui::NewFrame();

		events::RenderEvent();

		ImGui::Render();
	}

	void OnPostRenderDX12(ID3D12GraphicsCommandList* commandList)
	{
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	}

	void OnInitializeDX12(HWND window, ID3D12Device* pDevice, UINT buffersCounts, ID3D12DescriptorHeap* pDescriptorHeapImGuiRender)
	{
		LOG_DEBUG("ImGUI: DirectX12 backend initialized successfully.");

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		LoadImGuiStyles();

		//Set OriginalWndProcHandler to the Address of the Original WndProc function
		OriginalWndProcHandler = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(hWndProc)));

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX12_Init(pDevice, buffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM,
			pDescriptorHeapImGuiRender,
			pDescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
			pDescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());

		ImGui_ImplDX12_CreateDeviceObjects();
		ImGui::GetIO().ImeWindowHandle = window;

		static const std::string imguiPath = (util::GetCurrentPath() / "imgui.ini").string();
		ImGui::GetIO().IniFilename = imguiPath.c_str();
		io.SetPlatformImeDataFn = nullptr; // F**king bug take 4 hours of my life
	}

	static void OnInitializeDX11(HWND window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, IDXGISwapChain* pChain)
	{

		LOG_DEBUG("ImGUI: DirectX11 backend initialized successfully.");

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		static const std::string imguiPath = (util::GetCurrentPath() / "imgui.ini").string();
		io.IniFilename = imguiPath.c_str();

		LoadImGuiStyles();

		//Set OriginalWndProcHandler to the Address of the Original WndProc function
		OriginalWndProcHandler = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(hWndProc)));

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(pDevice, pContext);

		ID3D11Texture2D* pBackBuffer;
		pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
		pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
		pBackBuffer->Release();

		io.SetPlatformImeDataFn = nullptr; // F**king bug take 4 hours of my life
	}

	static void OnRenderDX11(ID3D11DeviceContext* pContext)
	{
		CheckFonts();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		if (!io.Fonts->IsBuilt())
		{
			io.Fonts->Build();
			ImGui_ImplDX11_InvalidateDeviceObjects();
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();


		if (_currentFontImgui != nullptr)
			io.FontDefault = _currentFontImgui;

		ImGui::NewFrame();

		events::RenderEvent();

		ImGui::EndFrame();
		ImGui::Render();

		pContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	static LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ImGuiIO& io = ImGui::GetIO();
		POINT mPos;
		GetCursorPos(&mPos);
		ScreenToClient(hWnd, &mPos);
		ImGui::GetIO().MousePos.x = static_cast<float>(mPos.x);
		ImGui::GetIO().MousePos.y = static_cast<float>(mPos.y);

		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

		if (!events::WndProcEvent(hWnd, uMsg, wParam, lParam))
			return true;

		short key;
		bool keyUpEvent = true;
		switch (uMsg)
		{
		case WM_LBUTTONUP:
			key = VK_LBUTTON;
			break;
		case WM_RBUTTONUP:
			key = VK_RBUTTON;
			break;
		case WM_MBUTTONUP:
			key = VK_MBUTTON;
			break;
		case WM_XBUTTONUP:
			key = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
			break;
		case WM_KEYUP:
			key = static_cast<short>(wParam);
			break;
		default:
			keyUpEvent = false;
			break;
		}

		bool canceled = false;
		if (keyUpEvent)
			canceled = !events::KeyUpEvent(key);

		if (IsInputLocked() || canceled)
			return true;

		return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
	}
}