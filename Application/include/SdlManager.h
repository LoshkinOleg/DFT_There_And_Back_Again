#pragma once

#include <functional>
#include <cassert>
#include <map>

#include <SDL.h>
#undef main

namespace MyApp
{
	enum class Input: size_t
	{
		ESCAPE = SDL_SCANCODE_ESCAPE
	};

	class SdlManager
	{
	public:

		SdlManager() = delete;
		SdlManager(const size_t displaySize);
		~SdlManager();

		void RegisterCallback(const Input input, std::function<void(void)> callback);

		bool ProcessInputs();

		const size_t displaySize;

	private:
		SDL_Event event_{};
		SDL_Window* pWindow_ = nullptr;
		SDL_Renderer* pRenderer_ = nullptr;
		std::map<Input, std::vector<std::function<void(void)>>> callbacks_;
	};
}