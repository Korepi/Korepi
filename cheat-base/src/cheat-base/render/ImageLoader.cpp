#include <pch.h>
#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cheat-base/util.h>
#include <cheat-base/render/backend/dx11-hook.h>
#include <cheat-base/ResourceLoader.h>

std::optional<ImageLoader::ImageData> ImageLoader::GetImage(const std::string& imageName, const char* type)
{
	if (s_Textures.count(imageName) > 0)
		return s_Textures[imageName];

	LPBYTE pDestination;
	DWORD size;

	bool loadResult = ResourceLoader::LoadEx(imageName.c_str(), type, pDestination, size);
	if (!loadResult)
	{
		// LOG_WARNING("Failed to load image: %s from resources", imageName);
		return {};
	}

	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory(pDestination, size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
	{
		// LOG_WARNING("Failed to convert image '%s' to RGBA by 'stb_image.h'", imageName.c_str());
		return {};
	}

	ImageLoader::ImageData imageData = {};

	ID3D11ShaderResourceView* new_texture = NULL;
	int width, height;
	bool textureResult = backend::LoadTextureFromMemory(image_data, image_width, image_height, 
		reinterpret_cast<ID3D11ShaderResourceView**>(&imageData.textureID), &width, &height);
	stbi_image_free(image_data);

	if (!textureResult)
	{
		// LOG_WARNING("Failed to load texture by DX11 for image: %s", imageName.c_str());
		return {};
	}

	imageData.size.x = static_cast<float>(width);
	imageData.size.y = static_cast<float>(height);
	s_Textures[imageName] = imageData;
	return imageData;
}

// Return reference to GIFData object so struct data will be updated
std::optional<ImageLoader::GIFData*> ImageLoader::GetGIF(const std::string& imageName)
{
	if (s_GIFS.count(imageName) > 0)
		return &s_GIFS[imageName];

	LPBYTE pDestination;
	DWORD size;

	bool loadResult = ResourceLoader::LoadEx(imageName.c_str(), "GIF", pDestination, size);
	if (!loadResult)
	{
		// LOG_WARNING("Failed to load gif: %s from resources", imageName);
		return {};
	}

	// Load from disk into raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	int frame_count = 0;
	int* delays = NULL;
	unsigned char* image_data = stbi_load_gif_from_memory(pDestination, size, &delays, &image_width, &image_height, &frame_count, NULL, 4);
	if (image_data == NULL || !frame_count)
	{
		// LOG_WARNING("Failed to convert GIF '%s' to RGBA buffer by 'stb_image.h'", imageName.c_str());
		return {};
	}

	ImageLoader::GIFData gifData = {};

	gifData.textureIDs.resize(frame_count);
	gifData.frame_delays.resize(frame_count);
	gifData.size.resize(frame_count);
	uint32_t nextFrameDataOffset = 0;  // Current byte offset to next frame in image.data
	for (int i = 0; i < frame_count; ++i)
	{
		nextFrameDataOffset = image_width * image_height * 4 * i;

		int width, height;
		bool textureResult = backend::LoadTextureFromMemory(((unsigned char*)image_data)+nextFrameDataOffset, image_width, image_height,
			reinterpret_cast<ID3D11ShaderResourceView**>(&gifData.textureIDs[i]), &width, &height);

		if (!textureResult)
		{
			// LOG_WARNING("Failed to load texture by DX11 for GIF frame %i: %s", i, imageName.c_str());
			return {};
		}

		gifData.size[i].x = static_cast<float>(width);
		gifData.size[i].y = static_cast<float>(height);
		gifData.frame_delays[i] = static_cast<uint32_t>(delays[i]);
	}

	stbi_image_free(delays);
	stbi_image_free(image_data);

	s_GIFS[imageName] = gifData;
	return &s_GIFS[imageName];
}