#pragma once

#include <functional>
#include <cassert>
#include <map>

#include <SDL.h>
#undef main
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

namespace MyApp
{
	enum class Input: size_t
	{
		ESCAPE = SDL_SCANCODE_ESCAPE,
		SPACE = SDL_SCANCODE_SPACE,
		LEFT_MOUSE_BUTTON,
		RIGHT_MOUSE_BUTTON,
		SCROLL_WHEEL,
		R = SDL_SCANCODE_R
	};

	struct ColorBytes
	{
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		unsigned char a = 255;
	};

	constexpr const ColorBytes COLOR_WHITE = {255, 255, 255, 255};
	constexpr const ColorBytes COLOR_BLACK = {0, 0, 0, 255};
	constexpr const ColorBytes COLOR_RED = {255, 0, 0, 255};
	constexpr const ColorBytes COLOR_GREEN = {0, 255, 0, 255};
	constexpr const ColorBytes COLOR_BLUE = {0, 0, 255, 255};
	constexpr const ColorBytes COLOR_TRANSPARENT = {0, 0, 0, 0};

	class SdlManager
	{
	public:

		SdlManager() = delete;
		SdlManager(const unsigned int displaySize);
		~SdlManager();

		void RegisterInputCallback(const Input input, std::function<void(void)> callback);
		void RegisterMouseInputCallback(const Input input, std::function<void(const float, const float)> callback);
		void RegisterImguiCallback(std::function<void(void)> callback);
		void RegisterRenderCallback(std::function<void(void)> callback);

		bool Update();

		void RenderPoint(const float x, const float y, ColorBytes color);
		void RenderLine(const float x0, const float y0, const float x1, const float y1, ColorBytes color);
		void RenderFilledRect(const float xMin, const float xMax, const float yMin, const float yMax, ColorBytes color);

		const unsigned int displaySize;

	private:
		SDL_Event event_{};
		SDL_Window* pWindow_ = nullptr;
		SDL_Renderer* pRenderer_ = nullptr;
		std::map<Input, std::vector<std::function<void(void)>>> inputCallbacks_;
		std::map<Input, std::vector<std::function<void(const float, const float)>>> mouseInputCallbacks_;
		std::vector<std::function<void(void)>> imguiCallbacks_;
		std::vector<std::function<void(void)>> renderCallbacks_;

		ImGuiContext* pImguiContext_ = nullptr;
	};
}