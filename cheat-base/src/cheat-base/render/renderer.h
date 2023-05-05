#pragma once
#include <Windows.h>
#include <cheat-base/events/event.hpp>
#include <imgui.h>

namespace renderer
{
	enum class DXVersion
	{
		D3D9,
		D3D10,
		D3D11,
		D3D12
	};

	class IFont
	{
	public:
		virtual ~IFont() = default;
		virtual std::string GetName() const = 0;
		virtual ImFont* AddImGuiFont(float fontSize, ImFontConfig* fontConfig) = 0;
	};

	class Font : public IFont
	{
	public:
		enum FONT_RANGE : uint32_t
		{
			FONT_RANGE_NONE = 0,
			FONT_RANGE_DEFAULT = 1,
			FONT_RANGE_CHINESE_FULL = 1 << 1,
			FONT_RANGE_CHINESE_SIMPLIFIED = 1 << 2,
			FONT_RANGE_CYRILLIC = 1 << 3,
			FONT_RANGE_JAPANESE = 1 << 4,
			FONT_RANGE_KOREAN = 1 << 5,
			FONT_RANGE_THAI = 1 << 6,
			FONT_RANGE_VIETNAMESE = 1 << 7
		};

	public:
		Font(const std::string& name, LPBYTE pData, DWORD dDataSize, FONT_RANGE ranges);
		Font(const std::string& name, LPBYTE pData, DWORD dDataSize, const ImWchar* ranges);

		std::string GetName() const override;
		ImFont* AddImGuiFont(float fontSize, ImFontConfig* fontConfig) override;

		inline LPBYTE GetData() const;
		inline DWORD GetSize() const;
		inline const ImWchar* GetGlyphRanges() const;

		// Fix Chinese missing characters
		const ImWchar* GetGlyphRangesChineseFull();

		void AddRange(FONT_RANGE ranges);
		void AddRange(const ImWchar* ranges);

		static std::shared_ptr<Font> LoadFontFromResource(int resourceId, const char* resourceType, const std::string& fontName, FONT_RANGE ranges);
		static std::shared_ptr<Font> LoadFontFromResource(int resourceId, const char* resourceType, const std::string& fontName, const ImWchar* ranges);

	private:
		std::string m_Name;
		LPBYTE m_Data;
		DWORD m_DataSize;
		ImWchar* m_Ranges;
	};

	class FontComposit : public IFont
	{
	public:
		FontComposit(const std::string& name, const std::initializer_list<Font>& fonts);
		FontComposit(const std::string& name, const std::vector<Font>& fonts);

		std::string GetName() const override;
		ImFont* AddImGuiFont(float fontSize, ImFontConfig* fontConfig) override;

	private:
		std::string m_Name;
		std::vector<Font> m_Fonts;
	};

	void Init(DXVersion version = DXVersion::D3D11);

	// Font system
	void AddFont(std::shared_ptr<IFont> font);

	void SetDefaultFont(const std::string& fontName);
	ImFont* GetDefaultImGuiFont();
	std::string GetDefaultFontName();

	void SetDefaultFontSize(float fontSize);
	float GetDefaultFontSize();

	// Font token system
	using FontToken = uint32_t;
	FontToken CreateFontToken(const std::string& fontName, float fontSize);
	bool DestroyFontToken(FontToken token);

	void SetTokenFontSize(FontToken token, float size);
	float GetTokenFontSize(FontToken token);

	void SetTokenFont(FontToken token, const std::string& fontName);
	std::string GetTokenFontName(FontToken token);

	ImFont* GetTokenFont(FontToken token);

	// Input lock
	void SetInputLock(void* id, bool value);
	void AddInputLocker(void* id);
	void RemoveInputLocker(void* id);
	bool IsInputLocked();
}
