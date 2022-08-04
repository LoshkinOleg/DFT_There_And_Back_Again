#include "SdlManager.h"

#include <easy/profiler.h>

MyApp::SdlManager::SdlManager(const unsigned int displaySize): displaySize(displaySize)
{
	auto err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	assert(err == 0 && "SDL failed to initialize.");
	err = SDL_CreateWindowAndRenderer((int)displaySize, (int)displaySize, SDL_WINDOW_SHOWN, &pWindow_, &pRenderer_);
	assert(err == 0 && pWindow_ && pRenderer_ && "SDL failed to create a window and a renderer.");
}

MyApp::SdlManager::~SdlManager()
{
	if (pRenderer_)SDL_DestroyRenderer(pRenderer_);
	if (pWindow_)SDL_DestroyWindow(pWindow_);
	SDL_Quit();
}

void MyApp::SdlManager::RegisterCallback(const Input input, std::function<void(void)> callback)
{
	callbacks_[input].push_back(callback);
}

bool MyApp::SdlManager::ProcessInputs()
{
	EASY_BLOCK("ProcessInputs()");
	while (SDL_PollEvent(&event_))
	{
		switch (event_.type)
		{
			case SDL_KEYDOWN:
			{
				const auto& callbacks = callbacks_[(Input)event_.key.keysym.scancode];
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
	return false;
}