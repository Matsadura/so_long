#ifndef UI_H
# define UI_H

# include "game.h"
# include "platform.h"

/*
** Presentation layer: turns a t_game into draw calls through platform.h.
*/

# define TILE_PX       64
# define HUD_PX        40
# define MIN_VIEW_W    768

# define COLOR_BG      0x0B0E14FFu
# define COLOR_HUD     0x151A23FFu
# define COLOR_TEXT    0xE8ECF1FFu
# define COLOR_ACCENT  0xFFD23FFFu
# define COLOR_DANGER  0xFF5A5AFFu
# define COLOR_OK      0x5AE38CFFu
# define COLOR_DIM     0x000000B4u

typedef enum e_sprite
{
	SPR_PLAYER_A,
	SPR_PLAYER_B,
	SPR_ENEMY_A,
	SPR_ENEMY_B,
	SPR_WALL,
	SPR_FLOOR,
	SPR_COLLECTIBLE,
	SPR_EXIT_CLOSED,
	SPR_EXIT_OPEN,
	SPR_COUNT
}	t_sprite;

typedef struct s_renderer
{
	t_tex		sprites[SPR_COUNT];
	/* Static floor + walls, pre-rendered once per level. */
	t_tex		layer;
	unsigned	layer_generation;
	int			view_w;
	int			view_h;
	int			map_x;
	int			map_y;
}	t_renderer;

bool	renderer_init(t_renderer *r, t_platform *p);
void	renderer_invalidate(t_renderer *r);
void	renderer_draw(t_renderer *r, t_platform *p, const t_game *g);
void	renderer_view_size(const t_game *g, int *w, int *h);

/* src/ui/font.c — 5x7 bitmap font drawn with rectangles. */
int		text_width(const char *s, int scale);
void	draw_text(t_platform *p, const char *s, int x, int y, int scale,
			uint32_t rgba);
void	draw_text_centered(t_platform *p, const char *s, int cx, int y,
			int scale, uint32_t rgba);

/* src/ui/hud.c */
void	hud_draw(t_platform *p, const t_renderer *r, const t_game *g);
void	overlay_draw(t_platform *p, const t_renderer *r, const t_game *g);

#endif
