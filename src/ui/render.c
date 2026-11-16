#include "ui.h"

#include <math.h>
#include <stdio.h>

static const char	*g_sprite_paths[SPR_COUNT] = {
[SPR_PLAYER_A] = "assets/sprites/player_01.png",
[SPR_PLAYER_B] = "assets/sprites/player_02.png",
[SPR_ENEMY_A] = "assets/sprites/enemy_01.png",
[SPR_ENEMY_B] = "assets/sprites/enemy_02.png",
[SPR_WALL] = "assets/sprites/wall_01.png",
[SPR_FLOOR] = "assets/sprites/floor.png",
[SPR_ROAD] = "assets/sprites/road.png",
[SPR_WATER_A] = "assets/sprites/water_1.png",
[SPR_WATER_B] = "assets/sprites/water_2.png",
[SPR_COLLECTIBLE] = "assets/sprites/collectible.png",
[SPR_EXIT_CLOSED] = "assets/sprites/exit_closed.png",
[SPR_EXIT_OPEN] = "assets/sprites/exit_open.png",
[SPR_CAR] = "assets/sprites/car.png",
[SPR_TRUCK_CAB] = "assets/sprites/truck_cab.png",
[SPR_TRUCK_TRAILER] = "assets/sprites/truck_trailer.png",
[SPR_LOG_LEFT] = "assets/sprites/log_left.png",
[SPR_LOG_MID] = "assets/sprites/log_mid.png",
[SPR_LOG_RIGHT] = "assets/sprites/log_right.png",
[SPR_SPLAT] = "assets/sprites/splat.png",
[SPR_SPLASH] = "assets/sprites/splash.png",
};

/* Lane tints stay inside the original palette. */
static const uint32_t	g_car_colors[] = {
	COLOR_WHITE, COLOR_RED, COLOR_YELLOW,
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
	*h = HUD_PX + g->map.rows * TILE_PX + FOOTER_PX;
}

void	draw_sprite(t_platform *p, const t_renderer *r, t_sprite s,
		const int rect[4])
{
	platform_draw_ex(p, r->sprites[s], rect, COLOR_WHITE, false);
}

static t_sprite	static_sprite(char t)
{
	if (t == TILE_WALL)
		return (SPR_WALL);
	if (t == TILE_ROAD)
		return (SPR_ROAD);
	return (SPR_FLOOR);
}

/* Rebuilds the cached static layer and view size after a level change. */
static void	rebuild_layer(t_renderer *r, t_platform *p, const t_game *g)
{
	int	i;
	int	x;
	int	y;

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
	i = -1;
	while (++i < g->map.cols * g->map.rows)
	{
		x = i % g->map.cols * TILE_PX;
		y = i / g->map.cols * TILE_PX;
		if (g->map.tiles[i] == TILE_WALL)
			platform_draw(p, r->sprites[SPR_FLOOR], x, y, TILE_PX, TILE_PX);
		platform_draw(p, r->sprites[static_sprite(g->map.tiles[i])], x, y,
			TILE_PX, TILE_PX);
	}
	platform_set_layer(p, TEX_NONE);
}

/* World rect for a tile position (fractional tiles allowed). */
static void	cell(const t_renderer *r, float x, float y, int out[4])
{
	out[0] = r->map_x + r->shake_x + (int)lroundf(x * TILE_PX);
	out[1] = r->map_y + r->shake_y + (int)lroundf(y * TILE_PX);
	out[2] = TILE_PX;
	out[3] = TILE_PX;
}

static void	draw_ground(t_renderer *r, t_platform *p, const t_game *g)
{
	int		i;
	int		rect[4];
	char	t;

	if (r->layer != TEX_NONE)
		platform_draw(p, r->layer, r->map_x + r->shake_x,
			r->map_y + r->shake_y, g->map.cols * TILE_PX,
			g->map.rows * TILE_PX);
	i = -1;
	while (++i < g->map.cols * g->map.rows)
	{
		t = g->tiles[i];
		cell(r, i % g->map.cols, i / g->map.cols, rect);
		if (r->layer == TEX_NONE)
			draw_sprite(p, r, static_sprite(g->map.tiles[i]), rect);
		if (t == TILE_WATER)
			draw_sprite(p, r, SPR_WATER_A
				+ ((int)(g->anim_time / ANIM_FRAME_SECONDS) + i) % 2, rect);
	}
}

static void	draw_items(t_renderer *r, t_platform *p, const t_game *g)
{
	int		i;
	int		rect[4];
	char	t;
	bool	open;

	open = !game_is_classic(g) || g->collected == g->map.collectibles;
	i = -1;
	while (++i < g->map.cols * g->map.rows)
	{
		t = g->tiles[i];
		cell(r, i % g->map.cols, i / g->map.cols, rect);
		if (t == TILE_COLLECTIBLE)
		{
			rect[1] -= (int)(3 * sinf((float)g->anim_time * 4.0f + i));
			draw_sprite(p, r, SPR_COLLECTIBLE, rect);
		}
		else if (t == TILE_EXIT)
			draw_sprite(p, r, open ? SPR_EXIT_OPEN : SPR_EXIT_CLOSED, rect);
		else if (t == TILE_GOAL_FILLED)
		{
			draw_sprite(p, r, SPR_EXIT_OPEN, rect);
			platform_draw_ex(p, r->sprites[SPR_PLAYER_A], rect, COLOR_YELLOW,
				false);
		}
	}
}

static t_sprite	piece_sprite(const t_lane *lane, int i, int len)
{
	bool	front;

	if (lane->kind == LANE_RIVER)
	{
		if (len == 1)
			return (SPR_LOG_MID);
		if (i == 0)
			return (SPR_LOG_LEFT);
		return (i == len - 1 ? SPR_LOG_RIGHT : SPR_LOG_MID);
	}
	if (len == 1)
		return (SPR_CAR);
	front = (lane->speed >= 0.0f) ? (i == len - 1) : (i == 0);
	return (front ? SPR_TRUCK_CAB : SPR_TRUCK_TRAILER);
}

static void	draw_lane(t_renderer *r, t_platform *p, const t_game *g, int y)
{
	const t_lane	*lane;
	uint32_t		tint;
	int				rect[4];
	int				i;
	int				k;
	int				wrap;

	lane = &g->lanes[y];
	tint = COLOR_WHITE;
	if (lane->kind == LANE_ROAD)
		tint = g_car_colors[y % (sizeof(g_car_colors) / sizeof(*g_car_colors))];
	i = -1;
	while (++i < lane->count)
	{
		wrap = -1;
		while (++wrap < 2)
		{
			k = -1;
			while (++k < lane->objs[i].len)
			{
				cell(r, 1.0f + lane->objs[i].x - wrap * lane_width(g) + k, y,
					rect);
				platform_draw_ex(p, r->sprites[piece_sprite(lane, k,
							lane->objs[i].len)], rect, tint, lane->speed < 0.0f
					&& lane->kind == LANE_ROAD);
			}
		}
	}
}

static void	draw_lanes(t_renderer *r, t_platform *p, const t_game *g)
{
	int	y;

	platform_set_clip(p, r->map_x + r->shake_x + TILE_PX,
		r->map_y + r->shake_y, lane_width(g) * TILE_PX,
		g->map.rows * TILE_PX);
	y = -1;
	while (++y < g->map.rows)
		if (g->lanes[y].kind != LANE_NONE)
			draw_lane(r, p, g, y);
	platform_clear_clip(p);
}

static void	draw_player(t_renderer *r, t_platform *p, const t_game *g)
{
	const t_player	*pl;
	int				rect[4];
	float			arc;
	int				grow;

	pl = &g->player;
	cell(r, pl->x, pl->y, rect);
	if (g->state == GAME_DYING && g->death == DEATH_CAR)
	{
		platform_draw_ex(p, r->sprites[SPR_SPLAT], rect, COLOR_DANGER, false);
		return ;
	}
	if (g->state == GAME_DYING && g->death == DEATH_WATER)
	{
		grow = (int)((DEATH_SECONDS - g->state_timer) * 24.0);
		rect[0] -= grow / 2;
		rect[1] -= grow / 2;
		rect[2] += grow;
		rect[3] += grow;
		platform_draw_ex(p, r->sprites[SPR_SPLASH], rect, COLOR_WATER, false);
		return ;
	}
	if (g->state == GAME_DYING && (int)(g->state_timer * 12.0) % 2)
		return ;
	arc = pl->hopping ? sinf(pl->hop_t * 3.14159265f) : 0.0f;
	grow = (int)(arc * 12.0f);
	rect[0] -= grow / 2;
	rect[1] -= grow / 2 + (int)(arc * 10.0f);
	rect[2] += grow;
	rect[3] += grow;
	platform_draw_ex(p, r->sprites[pl->hopping ? SPR_PLAYER_B : SPR_PLAYER_A],
		rect, g->state == GAME_DYING ? COLOR_DANGER : COLOR_WHITE,
		pl->facing == DIR_LEFT);
}

static void	draw_actors(t_renderer *r, t_platform *p, const t_game *g)
{
	int		i;
	int		rect[4];
	int		frame;
	char	buf[16];

	frame = (int)(g->anim_time / ANIM_FRAME_SECONDS) % 2;
	i = -1;
	while (++i < g->enemy_count)
	{
		cell(r, g->enemies[i].pos.x, g->enemies[i].pos.y, rect);
		draw_sprite(p, r, SPR_ENEMY_A + frame, rect);
	}
	if (g->state != GAME_TITLE)
		draw_player(r, p, g);
	i = -1;
	while (++i < MAX_POPUPS)
	{
		if (g->popups[i].ttl <= 0.0f)
			continue ;
		snprintf(buf, sizeof(buf), "+%d", g->popups[i].value);
		cell(r, g->popups[i].x, g->popups[i].y, rect);
		draw_text_centered(p, buf, rect[0] + TILE_PX / 2, rect[1], 2,
			(COLOR_ACCENT & 0xFFFFFF00u)
			| (uint32_t)(255 * fminf(1.0f, g->popups[i].ttl * 2.0f)));
	}
}

static void	update_shake(t_renderer *r, const t_game *g)
{
	double	t;

	r->shake_x = 0;
	r->shake_y = 0;
	if (g->state != GAME_DYING)
		return ;
	t = DEATH_SECONDS - g->state_timer;
	if (t > 0.35)
		return ;
	r->shake_x = (int)(sin(t * 90.0) * 8.0 * (0.35 - t) / 0.35);
	r->shake_y = (int)(cos(t * 70.0) * 5.0 * (0.35 - t) / 0.35);
}

void	renderer_draw(t_renderer *r, t_platform *p, const t_game *g)
{
	if (g->tiles == NULL)
		return ;
	if (r->layer_generation != g->level_generation)
		rebuild_layer(r, p, g);
	update_shake(r, g);
	platform_clear(p, COLOR_BG);
	draw_ground(r, p, g);
	draw_items(r, p, g);
	draw_lanes(r, p, g);
	draw_actors(r, p, g);
	hud_draw(p, r, g);
	screens_draw(p, r, g);
	platform_present(p);
}
