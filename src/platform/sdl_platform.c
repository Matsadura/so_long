#include "platform.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
# include <emscripten.h>
#endif

#define MAX_TEXTURES 64

struct s_platform
{
	SDL_Window		*window;
	SDL_Renderer	*renderer;
	SDL_Texture		*textures[MAX_TEXTURES];
	bool			img_ready;
	bool			vsync;
	int				logical_w;
	int				logical_h;
};

const char	*platform_error(void)
{
	return (SDL_GetError());
}

static bool	create_window(t_platform *p, const char *title)
{
	SDL_RendererInfo	info;

	p->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, p->logical_w, p->logical_h,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (p->window == NULL)
		return (false);
	p->renderer = SDL_CreateRenderer(p->window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
			| SDL_RENDERER_TARGETTEXTURE);
	if (p->renderer == NULL)
		p->renderer = SDL_CreateRenderer(p->window, -1,
				SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
	if (p->renderer == NULL)
		return (false);
	if (SDL_GetRendererInfo(p->renderer, &info) == 0)
		p->vsync = (info.flags & SDL_RENDERER_PRESENTVSYNC) != 0;
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);
	return (true);
}

t_platform	*platform_create(const char *title, int logical_w, int logical_h)
{
	t_platform	*p;

	p = calloc(1, sizeof(*p));
	if (p == NULL)
		return (NULL);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
	{
		free(p);
		return (NULL);
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	p->logical_w = logical_w;
	p->logical_h = logical_h;
	p->img_ready = (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0;
	if (!p->img_ready || !create_window(p, title))
	{
		platform_destroy(p);
		return (NULL);
	}
	platform_set_logical_size(p, logical_w, logical_h);
	return (p);
}

void	platform_destroy(t_platform *p)
{
	int	i;

	if (p == NULL)
		return ;
	i = 0;
	while (i < MAX_TEXTURES)
	{
		if (p->textures[i] != NULL)
			SDL_DestroyTexture(p->textures[i]);
		i++;
	}
	if (p->renderer != NULL)
		SDL_DestroyRenderer(p->renderer);
	if (p->window != NULL)
		SDL_DestroyWindow(p->window);
	if (p->img_ready)
		IMG_Quit();
	SDL_Quit();
	free(p);
}

/* Fits the window inside ~90% of the usable desktop, never upscaling. */
void	platform_set_logical_size(t_platform *p, int w, int h)
{
	SDL_Rect	bounds;
	double		scale;

	p->logical_w = w;
	p->logical_h = h;
	SDL_RenderSetLogicalSize(p->renderer, w, h);
#ifndef __EMSCRIPTEN__
	scale = 1.0;
	if (SDL_GetDisplayUsableBounds(0, &bounds) == 0
		&& bounds.w > 0 && bounds.h > 0)
	{
		if (w * scale > bounds.w * 0.9)
			scale = bounds.w * 0.9 / w;
		if (h * scale > bounds.h * 0.9)
			scale = bounds.h * 0.9 / h;
	}
	SDL_SetWindowSize(p->window, (int)(w * scale), (int)(h * scale));
	SDL_SetWindowPosition(p->window, SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED);
#else
	(void)bounds;
	(void)scale;
	SDL_SetWindowSize(p->window, w, h);
#endif
}

bool	platform_locate_assets(void)
{
#ifdef __EMSCRIPTEN__
	return (true);
#else
	char	*base;
	bool	ok;

	if (access("assets/sprites", R_OK) == 0)
		return (true);
	base = SDL_GetBasePath();
	if (base == NULL)
		return (false);
	ok = (chdir(base) == 0 && access("assets/sprites", R_OK) == 0);
	SDL_free(base);
	return (ok);
#endif
}

double	platform_time_seconds(void)
{
	return ((double)SDL_GetPerformanceCounter()
		/ (double)SDL_GetPerformanceFrequency());
}

void	platform_limit_fps(t_platform *p, double frame_start, int max_fps)
{
#ifdef __EMSCRIPTEN__
	(void)p;
	(void)frame_start;
	(void)max_fps;
#else
	double	remaining;

	if (p->vsync || max_fps <= 0)
		return ;
	remaining = 1.0 / max_fps - (platform_time_seconds() - frame_start);
	if (remaining > 0.001)
		SDL_Delay((Uint32)(remaining * 1000.0));
#endif
}

static t_tex	store_texture(t_platform *p, SDL_Texture *tex)
{
	int	i;

	if (tex == NULL)
		return (TEX_NONE);
	i = 0;
	while (i < MAX_TEXTURES && p->textures[i] != NULL)
		i++;
	if (i == MAX_TEXTURES)
	{
		SDL_DestroyTexture(tex);
		SDL_SetError("texture cache full");
		return (TEX_NONE);
	}
	p->textures[i] = tex;
	return (i);
}

t_tex	platform_load_texture(t_platform *p, const char *path)
{
	return (store_texture(p, IMG_LoadTexture(p->renderer, path)));
}

t_tex	platform_create_layer(t_platform *p, int w, int h)
{
	SDL_Texture	*tex;

	tex = SDL_CreateTexture(p->renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET, w, h);
	if (tex != NULL)
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	return (store_texture(p, tex));
}

void	platform_free_texture(t_platform *p, t_tex tex)
{
	if (tex < 0 || tex >= MAX_TEXTURES || p->textures[tex] == NULL)
		return ;
	SDL_DestroyTexture(p->textures[tex]);
	p->textures[tex] = NULL;
}

bool	platform_set_layer(t_platform *p, t_tex tex)
{
	SDL_Texture	*target;

	target = NULL;
	if (tex != TEX_NONE)
	{
		if (tex < 0 || tex >= MAX_TEXTURES || p->textures[tex] == NULL)
			return (false);
		target = p->textures[tex];
	}
	if (SDL_SetRenderTarget(p->renderer, target) != 0)
		return (false);
	if (target == NULL)
		SDL_RenderSetLogicalSize(p->renderer, p->logical_w, p->logical_h);
	return (true);
}

static void	set_color(t_platform *p, uint32_t rgba)
{
	SDL_SetRenderDrawColor(p->renderer, (rgba >> 24) & 0xFF,
		(rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF, rgba & 0xFF);
}

void	platform_clear(t_platform *p, uint32_t rgba)
{
	set_color(p, rgba);
	SDL_RenderClear(p->renderer);
}

void	platform_draw(t_platform *p, t_tex tex, int x, int y, int w, int h)
{
	SDL_Rect	dst;

	if (tex < 0 || tex >= MAX_TEXTURES || p->textures[tex] == NULL)
		return ;
	dst = (SDL_Rect){x, y, w, h};
	SDL_RenderCopy(p->renderer, p->textures[tex], NULL, &dst);
}

void	platform_fill_rect(t_platform *p, int x, int y, int w, int h,
		uint32_t rgba)
{
	SDL_Rect	r;

	r = (SDL_Rect){x, y, w, h};
	set_color(p, rgba);
	SDL_RenderFillRect(p->renderer, &r);
}

void	platform_present(t_platform *p)
{
	SDL_RenderPresent(p->renderer);
}
