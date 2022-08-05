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
		SPACE = SDL_SCANCODE_SPACE
	};

	class SdlManager
	{
	public:

		SdlManager() = delete;
		SdlManager(const unsigned int displaySize);
		~SdlManager();

		void RegisterInputCallback(const Input input, std::function<void(void)> callback);
		void RegisterImguiCallback(std::function<void(void)> callback);

		bool Update();

		const unsigned int displaySize;

	private:
		SDL_Event event_{};
		SDL_Window* pWindow_ = nullptr;
		SDL_Renderer* pRenderer_ = nullptr;
		std::map<Input, std::vector<std::function<void(void)>>> inputCallbacks_;
		std::vector<std::function<void(void)>> imguiCallbacks_;

		ImGuiContext* pImguiContext_ = nullptr;
	};
}