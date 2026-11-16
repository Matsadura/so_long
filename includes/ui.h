#ifndef UI_H
# define UI_H

# include "game.h"
# include "platform.h"

/*
** Presentation layer: turns a t_game into draw calls through platform.h.
*/

# define TILE_PX       64
# define HUD_PX        44
# define FOOTER_PX     28
# define MIN_VIEW_W    768

/* The original so_long palette: white on black, plus the enemy red and
** the yellow of the unused enemy variant. Grey is white at reduced alpha. */
# define COLOR_BG      0x000000FFu
# define COLOR_HUD     0x000000FFu
# define COLOR_WHITE   0xFFFFFFFFu
# define COLOR_TEXT    COLOR_WHITE
# define COLOR_RED     0xC50404FFu
# define COLOR_YELLOW  0xF2F000FFu
# define COLOR_MUTED   0xFFFFFF8Cu
# define COLOR_FAINT   0xFFFFFF33u
# define COLOR_ACCENT  COLOR_YELLOW
# define COLOR_DANGER  COLOR_RED
# define COLOR_OK      COLOR_YELLOW
# define COLOR_WATER   COLOR_WHITE
# define COLOR_DIM     0x000000C8u
# define COLOR_BAND    0x000000B4u

typedef enum e_sprite
{
	SPR_PLAYER_A,
	SPR_PLAYER_B,
	SPR_ENEMY_A,
	SPR_ENEMY_B,
	SPR_WALL,
	SPR_FLOOR,
	SPR_ROAD,
	SPR_WATER_A,
	SPR_WATER_B,
	SPR_COLLECTIBLE,
	SPR_EXIT_CLOSED,
	SPR_EXIT_OPEN,
	SPR_CAR,
	SPR_TRUCK_CAB,
	SPR_TRUCK_TRAILER,
	SPR_LOG_LEFT,
	SPR_LOG_MID,
	SPR_LOG_RIGHT,
	SPR_SPLAT,
	SPR_SPLASH,
	SPR_COUNT
}	t_sprite;

typedef struct s_renderer
{
	t_tex		sprites[SPR_COUNT];
	/* Static floor, roads and walls, pre-rendered once per level. */
	t_tex		layer;
	unsigned	layer_generation;
	int			view_w;
	int			view_h;
	int			map_x;
	int			map_y;
	/* Per-frame world offset (screen shake on death). */
	int			shake_x;
	int			shake_y;
}	t_renderer;

bool	renderer_init(t_renderer *r, t_platform *p);
void	renderer_invalidate(t_renderer *r);
void	renderer_draw(t_renderer *r, t_platform *p, const t_game *g);
void	renderer_view_size(const t_game *g, int *w, int *h);

/* src/ui/render.c helpers shared with the HUD */
void	draw_sprite(t_platform *p, const t_renderer *r, t_sprite s,
			const int rect[4]);

/* src/ui/font.c: 5x7 bitmap font drawn with rectangles. */
int		text_width(const char *s, int scale);
void	draw_text(t_platform *p, const char *s, int x, int y, int scale,
			uint32_t rgba);
void	draw_text_centered(t_platform *p, const char *s, int cx, int y,
			int scale, uint32_t rgba);
int		text_fit_scale(const char *s, int max_w, int preferred);

/* src/ui/hud.c */
void	hud_draw(t_platform *p, const t_renderer *r, const t_game *g);

/* src/ui/screens.c: title, banners and end screens. */
void	screens_draw(t_platform *p, const t_renderer *r, const t_game *g);

#endif
