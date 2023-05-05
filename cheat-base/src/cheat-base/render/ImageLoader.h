#pragma once

#include <string>
#include <map>
#include <optional>

#include <Windows.h>

#include <imgui.h>

class ImageLoader
{
public:
	struct ImageData
	{
		ImTextureID textureID;
		ImVec2 size;
	};

	struct GIFData
	{
		std::vector<ImTextureID> textureIDs;
		std::vector<uint32_t> frame_delays; //show current frame for i*(1/100) of a sec
		std::vector<ImVec2> size;
		uint32_t current_frame = 0;
		std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point previous_time = std::chrono::system_clock::now();

		uint32_t get_next_frame_index() 
		{ 
			current_frame = (++current_frame) % textureIDs.size();
			return current_frame;
		}

		bool is_next_frame()
		{
			current_time = std::chrono::system_clock::now();
			std::chrono::duration<double, std::milli> elapsed_time = current_time - previous_time;
			if (elapsed_time.count() >= (frame_delays[current_frame]))
			{
				previous_time = current_time;
				return true;
			}
			return false;
		}

		ImTextureID get_next_frame()
		{
			return textureIDs[get_next_frame_index()];
		}

		ImTextureID get_current_frame()
		{
			return textureIDs[current_frame];
		}

		void render(ImVec2 size)
		{
			if (is_next_frame())
			{
				ImGui::Image((void*)get_next_frame(), size);
			}
			else
			{
				ImGui::Image((void*)get_current_frame(), size);
			}
		}
	};

	static std::optional<ImageData> GetImage(const std::string& imageName, const char* imageType = "PNG");
	static std::optional<GIFData*> GetGIF(const std::string& imageName);

private:
	inline static std::map<std::string, ImageData> s_Textures {};
	inline static std::map<std::string, GIFData> s_GIFS {};
};