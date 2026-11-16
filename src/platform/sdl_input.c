#include "platform.h"

#include <SDL.h>

void	input_push(t_input *in, t_action action)
{
	if (in->count < INPUT_MAX_ACTIONS)
		in->actions[in->count++] = action;
}

static void	map_key(t_input *in, SDL_Keycode key)
{
	if (key == SDLK_UP || key == SDLK_w)
		input_push(in, ACT_UP);
	else if (key == SDLK_DOWN || key == SDLK_s)
		input_push(in, ACT_DOWN);
	else if (key == SDLK_LEFT || key == SDLK_a)
		input_push(in, ACT_LEFT);
	else if (key == SDLK_RIGHT || key == SDLK_d)
		input_push(in, ACT_RIGHT);
	else if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)
		input_push(in, ACT_CONFIRM);
	else if (key == SDLK_r)
		input_push(in, ACT_RESTART);
	else if (key == SDLK_p)
		input_push(in, ACT_PAUSE);
#ifdef __EMSCRIPTEN__
	else if (key == SDLK_ESCAPE)
		input_push(in, ACT_PAUSE);
#else
	else if (key == SDLK_ESCAPE)
		input_push(in, ACT_QUIT);
#endif
}

/* Collects this frame's discrete key presses (OS key-repeat ignored). */
void	platform_poll_input(t_platform *p, t_input *in)
{
	SDL_Event	ev;

	(void)p;
	in->count = 0;
	in->quit_requested = false;
	in->targets_reset = false;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT)
			in->quit_requested = true;
		else if (ev.type == SDL_KEYDOWN && !ev.key.repeat)
			map_key(in, ev.key.keysym.sym);
		else if (ev.type == SDL_RENDER_TARGETS_RESET
			|| ev.type == SDL_RENDER_DEVICE_RESET)
			in->targets_reset = true;
	}
}
