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
	// Used to tell to what action a user callback is bound to. Using size_t to be able to cast it to this type.
	enum class Input: size_t
	{
		ESCAPE = SDL_SCANCODE_ESCAPE,
		SPACE = SDL_SCANCODE_SPACE,
		LEFT_MOUSE_BUTTON,
		RIGHT_MOUSE_BUTTON,
		SCROLL_WHEEL,
		R = SDL_SCANCODE_R
	};

	// 4 byte struct encoding a color.
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

	/**
	* Class responsible for processing user input and for displaying things on screen.
	*/
	class SdlManager
	{
	public:

		SdlManager() = delete;
		/**
		* Constructs an SdlManager.
		* 
		* @param displaySize Defines the side size, in pixels, of the display. Only square resolutions are currently used. TODO: add support for rectangular resolutions.
		*/
		SdlManager(const unsigned int displaySize);
		~SdlManager();

		/**
		* Binds a keyboard or mouse input to a user-defined action.
		* 
		* @param input The input to bind the action to.
		* @param callback The method to call when the input is received. Use lambdas.
		*/
		void RegisterInputCallback(const Input input, std::function<void(void)> callback);

		/**
		* Binds a mouse movement or scrollwheel input to a user-defined action.
		*
		* @param input The input to bind the action to.
		* @param callback The method to call when the input is received. Use lambdas. The callback should have the following signature: void callback(const float x, const float y) .
		*/
		void RegisterMouseInputCallback(const Input input, std::function<void(const float, const float)> callback);

		/**
		* Registers a function to call back upon rendering that contains Dear ImGui immediate mode rendering code.
		*
		* @param callback The method to call when the input is received. Use lambdas. Should (ideally) only contain Dear ImGui immediate mode rendering code.
		*/
		void RegisterImguiCallback(std::function<void(void)> callback);

		/**
		* Registers a function to call back upon rendering that contains either calls to RenderPoint(), RenderLine() and RenderFilledRect() or SDL's renderer code.
		*
		* @param callback The method to call when the input is received. Use lambdas. Should (ideally) only contain calls to RenderPoint(), RenderLine() and RenderFilledRect() or SDL's renderer code.
		*/
		void RegisterRenderCallback(std::function<void(void)> callback);

		/**
		* Clears renderCallbacks_.
		*/
		void ClearRenderCallbacks();

		/**
		* Processes window events and calls back functions stored in inputCallbacks_ and mouseInputCallbacks_, renderCallbacks_ and imguiCallbacks_ and updates the display.
		*/
		bool Update();

		/**
		* Use this to draw a pixel on the screen. Uses SDL screen-space coordinate system.
		* 
		* @param x Horizontal screen-space coordinate. Origin is at top-left of window. x grows rightwards.
		* @param y Vertical screen-space coordinate. Origin is at top-left of window. y grows downwards.
		* @param color The color of the pixel to be drawn with. Values of color range between 0 and 255.
		*/
		void RenderPoint(const float x, const float y, ColorBytes color);

		/**
		* Use this to draw a line on the screen. Uses SDL screen-space coordinate system.
		*
		* @param x0 Horizontal screen-space coordinate of the start of the line. Origin is at top-left of window. x grows rightwards.
		* @param y0 Vertical screen-space coordinate of the start of the line. Origin is at top-left of window. y grows downwards.
		* @param x1 Horizontal screen-space coordinate of the end of the line. Origin is at top-left of window. x grows rightwards.
		* @param y1 Vertical screen-space coordinate of the end of the line. Origin is at top-left of window. y grows downwards.
		* @param color The color of the pixel to be drawn with. Values of color range between 0 and 255.
		*/
		void RenderLine(const float x0, const float y0, const float x1, const float y1, ColorBytes color);

		/**
		* Use this to draw a filled rectangle on the screen. Uses SDL screen-space coordinate system.
		*
		* @param xMin Left horizontal bound of the rectangle. Origin is at top-left of window. x grows rightwards.
		* @param xMax Right horizontal bound of the rectangle. Origin is at top-left of window. x grows rightwards.
		* @param yMin Top vertical bound of the rectangle. Origin is at top-left of window. y grows downwards.
		* @param yMax Bottom vertical bound of the rectangle. Origin is at top-left of window. y grows downwards.
		* @param color The color of the pixel to be drawn with. Values of color range between 0 and 255.
		*/
		void RenderFilledRect(const float xMin, const float xMax, const float yMin, const float yMax, ColorBytes color);

		const unsigned int displaySize; // Side size, in pixels, of the display. Only square resolutions are supported for now. // TODO: implement support for rectangular windows.

	private:
		SDL_Event event_{}; // Member variable used to process SDL events.
		SDL_Window* pWindow_ = nullptr; // Pointer to SDL's window.
		SDL_Renderer* pRenderer_ = nullptr; // Pointer to SDL's renderer.
		std::map<Input, std::vector<std::function<void(void)>>> inputCallbacks_; // Map of functions to call when an input is received.
		std::map<Input, std::vector<std::function<void(const float, const float)>>> mouseInputCallbacks_; // Map of functions to call when an input is received.
		std::vector<std::function<void(void)>> imguiCallbacks_; // List of Dear Imgui rendering callbacks.
		std::vector<std::function<void(void)>> renderCallbacks_; // List of SDL rendering callbacks.

		ImGuiContext* pImguiContext_ = nullptr; // Dear ImGui's context.
	};
}