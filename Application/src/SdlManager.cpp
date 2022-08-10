#include "SdlManager.h"

#include <easy/profiler.h>

MyApp::SdlManager::SdlManager(const unsigned int displaySize): displaySize(displaySize)
{
	auto err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	assert(err == 0 && "SDL failed to initialize.");
	err = SDL_CreateWindowAndRenderer((int)displaySize, (int)displaySize, SDL_WINDOW_SHOWN, &pWindow_, &pRenderer_);
	assert(err == 0 && pWindow_ && pRenderer_ && "SDL failed to create a window and a renderer.");

	IMGUI_CHECKVERSION();
	pImguiContext_ = ImGui::CreateContext();
	ImGui::StyleColorsDark();

	err = ImGui_ImplSDL2_InitForSDLRenderer(pWindow_, pRenderer_);
	assert(err && "Failed to init imgui for sdl renderer.");
	err = ImGui_ImplSDLRenderer_Init(pRenderer_);
	assert(err && "Failed to init imgui for sdl renderer.");
}

MyApp::SdlManager::~SdlManager()
{
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext(pImguiContext_);

	if (pRenderer_)SDL_DestroyRenderer(pRenderer_);
	if (pWindow_)SDL_DestroyWindow(pWindow_);
	SDL_Quit();
}

void MyApp::SdlManager::RegisterInputCallback(const Input input, std::function<void(void)> callback)
{
	inputCallbacks_[input].push_back(callback);
}

void MyApp::SdlManager::RegisterMouseInputCallback(const Input input, std::function<void(const float, const float)> callback)
{
	mouseInputCallbacks_[input].push_back(callback);
}

void MyApp::SdlManager::RegisterImguiCallback(std::function<void(void)> callback)
{
	imguiCallbacks_.push_back(callback);
}

void MyApp::SdlManager::RegisterRenderCallback(std::function<void(void)> callback)
{
	renderCallbacks_.push_back(callback);
}

bool MyApp::SdlManager::Update()
{
	EASY_BLOCK("ProcessInputs()");
	while (SDL_PollEvent(&event_))
	{
		ImGui_ImplSDL2_ProcessEvent(&event_);

		switch (event_.type)
		{
			case SDL_KEYDOWN:
			{
				const auto& callbacks = inputCallbacks_[(Input)event_.key.keysym.scancode];
				for (auto& callback : callbacks)
				{
					callback();
				}
			}
			break;

			case SDL_MOUSEMOTION:
			{
				if (event_.motion.state & SDL_BUTTON_LMASK)
				{
					for (auto& callback : mouseInputCallbacks_[Input::LEFT_MOUSE_BUTTON])
					{
						callback(event_.motion.xrel, event_.motion.yrel);
					}
				}
				else if (event_.motion.state & SDL_BUTTON_RMASK)
				{
					for (auto& callback : mouseInputCallbacks_[Input::RIGHT_MOUSE_BUTTON])
					{
						callback(event_.motion.xrel, event_.motion.yrel);
					}
				}
			}break;

			case SDL_MOUSEWHEEL:
			{
				for (auto& callback : mouseInputCallbacks_[Input::SCROLL_WHEEL])
				{
					callback(event_.wheel.x, event_.wheel.y);
				}
			}break;

			case SDL_QUIT:
			{
				return true;
			}
			break;

			default:
			break;
		}
	}

	SDL_SetRenderDrawColor(pRenderer_, (Uint8)(128), (Uint8)(128), (Uint8)(128), (Uint8)(255));
	SDL_RenderClear(pRenderer_);

	for (size_t i = 0; i < renderCallbacks_.size(); i++)
	{
		renderCallbacks_[i]();
	}

	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	for (size_t i = 0; i < imguiCallbacks_.size(); i++)
	{
		imguiCallbacks_[i]();
	}
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(pRenderer_);

	return false;
}

void MyApp::SdlManager::RenderPoint(const float x, const float y, ColorBytes color)
{
	SDL_SetRenderDrawColor(pRenderer_, color.r, color.g, color.b, color.a);
	SDL_RenderDrawPointF(pRenderer_, x, y);
}

void MyApp::SdlManager::RenderLine(const float x0, const float y0, const float x1, const float y1, ColorBytes color)
{
	SDL_SetRenderDrawColor(pRenderer_, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLineF(pRenderer_, x0, y0, x1, y1);
}

void MyApp::SdlManager::RenderFilledRect(const float xMin, const float xMax, const float yMin, const float yMax, ColorBytes color)
{
	assert(xMax >= xMin && yMax >= yMin && "Invalid rectangle dimensions.");
	SDL_SetRenderDrawColor(pRenderer_, color.r, color.g, color.b, color.a);
	const SDL_Rect r =
	{
		(int)xMin,
		(int)yMin,
		(int)(xMax - xMin),
		(int)(yMax - yMin)
	};
	SDL_RenderFillRect(pRenderer_, &r);
}
