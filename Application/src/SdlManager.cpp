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

void MyApp::SdlManager::RegisterImguiCallback(std::function<void(void)> callback)
{
	imguiCallbacks_.push_back(callback);
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

			case SDL_QUIT:
			{
				return true;
			}
			break;

			default:
			break;
		}
	}

	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	for (size_t i = 0; i < imguiCallbacks_.size(); i++)
	{
		imguiCallbacks_[i]();
	}

	ImGui::Render();
	SDL_SetRenderDrawColor(pRenderer_, (Uint8)(128), (Uint8)(128), (Uint8)(128), (Uint8)(255));
	SDL_RenderClear(pRenderer_);
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(pRenderer_);

	return false;
}