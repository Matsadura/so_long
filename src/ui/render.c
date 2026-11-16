#include "ui.h"

#include <stdio.h>

static const char	*g_sprite_paths[SPR_COUNT] = {
[SPR_PLAYER_A] = "assets/sprites/player_01.png",
[SPR_PLAYER_B] = "assets/sprites/player_02.png",
[SPR_ENEMY_A] = "assets/sprites/enemy_01.png",
[SPR_ENEMY_B] = "assets/sprites/enemy_02.png",
[SPR_WALL] = "assets/sprites/wall_01.png",
[SPR_FLOOR] = "assets/sprites/floor.png",
[SPR_COLLECTIBLE] = "assets/sprites/collectible.png",
[SPR_EXIT_CLOSED] = "assets/sprites/exit_closed.png",
[SPR_EXIT_OPEN] = "assets/sprites/exit_open.png",
};

/* Textures are owned by the platform cache and freed in platform_destroy. */
bool	renderer_init(t_renderer *r, t_platform *p)
{
	int	i;

	r->layer = TEX_NONE;
	r->layer_generation = 0;
	i = 0;
	while (i < SPR_COUNT)
	{
		r->sprites[i] = platform_load_texture(p, g_sprite_paths[i]);
		if (r->sprites[i] == TEX_NONE)
		{
			fprintf(stderr, "Error: cannot load %s: %s\n",
				g_sprite_paths[i], platform_error());
			return (false);
		}
		i++;
	}
	return (true);
}

void	renderer_invalidate(t_renderer *r)
{
	r->layer_generation = 0;
}

void	renderer_view_size(const t_game *g, int *w, int *h)
{
	*w = g->map.cols * TILE_PX;
	if (*w < MIN_VIEW_W)
		*w = MIN_VIEW_W;
	*h = g->map.rows * TILE_PX + HUD_PX;
}

/* Rebuilds the cached static layer and view size after a level change. */
static void	rebuild_layer(t_renderer *r, t_platform *p, const t_game *g)
{
	int	x;
	int	y;
	int	spr;

	platform_free_texture(p, r->layer);
	renderer_view_size(g, &r->view_w, &r->view_h);
	r->map_x = (r->view_w - g->map.cols * TILE_PX) / 2;
	r->map_y = HUD_PX;
	platform_set_logical_size(p, r->view_w, r->view_h);
	r->layer = platform_create_layer(p, g->map.cols * TILE_PX,
			g->map.rows * TILE_PX);
	r->layer_generation = g->level_generation;
	if (r->layer == TEX_NONE || !platform_set_layer(p, r->layer))
		return ;
	platform_clear(p, 0x00000000u);
	y = -1;
	while (++y < g->map.rows)
	{
		x = -1;
		while (++x < g->map.cols)
		{
			spr = SPR_FLOOR;
			if (g->map.tiles[y * g->map.cols + x] == TILE_WALL)
				spr = SPR_WALL;
			platform_draw(p, r->sprites[spr], x * TILE_PX, y * TILE_PX,
				TILE_PX, TILE_PX);
		}
	}
	platform_set_layer(p, TEX_NONE);
}

/* Fallback when render targets are unavailable: draw the static tiles. */
static void	draw_static_tiles(t_renderer *r, t_platform *p, const t_game *g)
{
	int	i;
	int	spr;

	i = 0;
	while (i < g->map.cols * g->map.rows)
	{
		spr = SPR_FLOOR;
		if (g->map.tiles[i] == TILE_WALL)
			spr = SPR_WALL;
		platform_draw(p, r->sprites[spr], r->map_x + i % g->map.cols * TILE_PX,
			r->map_y + i / g->map.cols * TILE_PX, TILE_PX, TILE_PX);
		i++;
	}
}

static void	draw_cell(t_renderer *r, t_platform *p, int spr, t_point at)
{
	platform_draw(p, r->sprites[spr], r->map_x + at.x * TILE_PX,
		r->map_y + at.y * TILE_PX, TILE_PX, TILE_PX);
}

static void	draw_entities(t_renderer *r, t_platform *p, const t_game *g)
{
	int		i;
	int		frame;
	char	t;

	i = -1;
	while (++i < g->map.cols * g->map.rows)
	{
		t = g->tiles[i];
		if (t == TILE_COLLECTIBLE)
			draw_cell(r, p, SPR_COLLECTIBLE,
				(t_point){i % g->map.cols, i / g->map.cols});
		else if (t == TILE_EXIT)
			draw_cell(r, p, (g->collected == g->map.collectibles)
				? SPR_EXIT_OPEN : SPR_EXIT_CLOSED,
				(t_point){i % g->map.cols, i / g->map.cols});
	}
	frame = (int)(g->anim_time / ANIM_FRAME_SECONDS) % 2;
	i = -1;
	while (++i < g->enemy_count)
		draw_cell(r, p, SPR_ENEMY_A + frame, g->enemies[i].pos);
	draw_cell(r, p, SPR_PLAYER_A + frame, g->player);
}

void	renderer_draw(t_renderer *r, t_platform *p, const t_game *g)
{
	if (g->tiles == NULL)
		return ;
	if (r->layer_generation != g->level_generation)
		rebuild_layer(r, p, g);
	platform_clear(p, COLOR_BG);
	if (r->layer != TEX_NONE)
		platform_draw(p, r->layer, r->map_x, r->map_y,
			g->map.cols * TILE_PX, g->map.rows * TILE_PX);
	else
		draw_static_tiles(r, p, g);
	draw_entities(r, p, g);
	hud_draw(p, r, g);
	overlay_draw(p, r, g);
	platform_present(p);
}
