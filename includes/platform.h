#ifndef PLATFORM_H
# define PLATFORM_H

# include <stdbool.h>
# include <stdint.h>

/*
** Hardware / OS abstraction. Everything that touches SDL lives behind this
** header (implementation in src/platform/). Game logic in src/core and
** src/map never includes SDL directly.
*/

typedef struct s_platform	t_platform;

/* Handle into the platform's texture cache. */
typedef int					t_tex;
# define TEX_NONE			(-1)

typedef enum e_action
{
	ACT_UP,
	ACT_DOWN,
	ACT_LEFT,
	ACT_RIGHT,
	ACT_CONFIRM,
	ACT_RESTART,
	ACT_PAUSE,
	ACT_QUIT
}	t_action;

# define INPUT_MAX_ACTIONS	16

typedef struct s_input
{
	t_action	actions[INPUT_MAX_ACTIONS];
	int			count;
	bool		quit_requested;
	/* GPU render targets were lost; cached layers must be rebuilt. */
	bool		targets_reset;
}	t_input;

t_platform	*platform_create(const char *title, int logical_w, int logical_h);
void		platform_destroy(t_platform *p);
const char	*platform_error(void);

/* Changes the virtual resolution and resizes the window to fit the screen. */
void		platform_set_logical_size(t_platform *p, int w, int h);

/* Natively: chdir next to the binary if ./assets is not reachable. */
bool		platform_locate_assets(void);

void		platform_poll_input(t_platform *p, t_input *in);
void		input_push(t_input *in, t_action action);

double		platform_time_seconds(void);
/* Native-only frame cap when vsync is unavailable; no-op in the browser. */
void		platform_limit_fps(t_platform *p, double frame_start, int max_fps);

t_tex		platform_load_texture(t_platform *p, const char *path);
t_tex		platform_create_layer(t_platform *p, int w, int h);
void		platform_free_texture(t_platform *p, t_tex tex);
/* Redirects drawing into a layer texture, or back to the screen (TEX_NONE). */
bool		platform_set_layer(t_platform *p, t_tex tex);

void		platform_clear(t_platform *p, uint32_t rgba);
void		platform_draw(t_platform *p, t_tex tex, int x, int y, int w, int h);
/* Color-modulated (tint 0xRRGGBBAA, white = unchanged), optionally
** mirrored horizontally. */
void		platform_draw_ex(t_platform *p, t_tex tex, const int rect[4],
				uint32_t tint, bool flip_h);
void		platform_set_clip(t_platform *p, int x, int y, int w, int h);
void		platform_clear_clip(t_platform *p);
void		platform_fill_rect(t_platform *p, int x, int y, int w, int h,
				uint32_t rgba);
void		platform_present(t_platform *p);

#endif
