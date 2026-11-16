#include "game.h"

#include <stdlib.h>
#include <string.h>

t_map_error	game_init(t_game *g, const char *const *levels, int count)
{
	memset(g, 0, sizeof(*g));
	g->levels = levels;
	g->level_count = count;
	return (game_load_level(g, 0));
}

void	game_free(t_game *g)
{
	free(g->tiles);
	g->tiles = NULL;
	map_free(&g->map);
}

/* Resets the mutable per-attempt state from the pristine map. */
void	game_restart_level(t_game *g)
{
	if (g->tiles == NULL)
		return ;
	memcpy(g->tiles, g->map.tiles, (size_t)g->map.cols * g->map.rows);
	g->player = g->map.player_start;
	g->facing = DIR_RIGHT;
	g->collected = 0;
	g->moves = 0;
	g->anim_time = 0.0;
	enemies_reset(g);
	g->state = GAME_PLAYING;
}

/* On failure the current level stays loaded (nothing leaks) and the game
** enters GAME_FATAL with g->error set, so the caller can report it. */
t_map_error	game_load_level(t_game *g, int index)
{
	t_map		next;
	char		*tiles;
	t_map_error	err;

	g->error_level = index;
	if (index < 0 || index >= g->level_count)
		err = MAP_ERR_OPEN;
	else
		err = map_load(g->levels[index], &next);
	tiles = NULL;
	if (err == MAP_OK)
	{
		tiles = malloc((size_t)next.cols * next.rows);
		if (tiles == NULL)
		{
			map_free(&next);
			err = MAP_ERR_ALLOC;
		}
	}
	if (err != MAP_OK)
	{
		g->error = err;
		g->state = GAME_FATAL;
		return (err);
	}
	game_free(g);
	g->map = next;
	g->tiles = tiles;
	g->level_index = index;
	g->level_generation++;
	game_restart_level(g);
	return (MAP_OK);
}

static char	tile(const t_game *g, int x, int y)
{
	return (map_tile_at(g->tiles, g->map.cols, g->map.rows, x, y));
}

static void	try_move(t_game *g, int dx, int dy, t_dir facing)
{
	int		nx;
	int		ny;
	char	t;

	g->facing = facing;
	nx = g->player.x + dx;
	ny = g->player.y + dy;
	t = tile(g, nx, ny);
	if (t == TILE_WALL)
		return ;
	g->player = (t_point){nx, ny};
	g->moves++;
	if (enemy_at(g, nx, ny))
	{
		g->state = GAME_OVER;
		return ;
	}
	if (t == TILE_COLLECTIBLE)
	{
		g->tiles[ny * g->map.cols + nx] = TILE_FLOOR;
		g->collected++;
	}
	else if (t == TILE_EXIT && g->collected == g->map.collectibles)
	{
		if (g->level_index + 1 < g->level_count)
			g->state = GAME_LEVEL_CLEAR;
		else
			g->state = GAME_COMPLETE;
	}
}

static void	handle_action(t_game *g, t_action a)
{
	if (a == ACT_RESTART && g->state != GAME_FATAL)
		game_restart_level(g);
	else if (g->state == GAME_PLAYING)
	{
		if (a == ACT_UP)
			try_move(g, 0, -1, DIR_UP);
		else if (a == ACT_DOWN)
			try_move(g, 0, 1, DIR_DOWN);
		else if (a == ACT_LEFT)
			try_move(g, -1, 0, DIR_LEFT);
		else if (a == ACT_RIGHT)
			try_move(g, 1, 0, DIR_RIGHT);
	}
	else if (a == ACT_CONFIRM && g->state == GAME_LEVEL_CLEAR)
		game_load_level(g, g->level_index + 1);
	else if (a == ACT_CONFIRM && g->state == GAME_COMPLETE)
		game_load_level(g, 0);
	else if (a == ACT_CONFIRM && g->state == GAME_OVER)
		game_restart_level(g);
}

void	game_update(t_game *g, const t_input *in, double dt)
{
	int	i;

	i = 0;
	while (i < in->count && g->state != GAME_FATAL)
		handle_action(g, in->actions[i++]);
	if (dt < 0.0)
		dt = 0.0;
	g->anim_time += dt;
	if (g->state == GAME_PLAYING)
		enemies_update(g, dt);
}
