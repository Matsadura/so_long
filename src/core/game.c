#include "game.h"

#include <stdlib.h>
#include <string.h>

t_map_error	game_init(t_game *g, const char *const *levels, int count)
{
	t_map_error	err;

	memset(g, 0, sizeof(*g));
	g->levels = levels;
	g->level_count = count;
	g->lives = START_LIVES;
	err = game_load_level(g, 0);
	if (err == MAP_OK)
		g->state = GAME_TITLE;
	return (err);
}

void	game_free(t_game *g)
{
	free(g->tiles);
	free(g->lanes);
	g->tiles = NULL;
	g->lanes = NULL;
	map_free(&g->map);
}

bool	game_is_classic(const t_game *g)
{
	return (!g->map.has_lanes);
}

char	game_tile(const t_game *g, int x, int y)
{
	return (map_tile_at(g->tiles, g->map.cols, g->map.rows, x, y));
}

/* Classic maps get time proportional to their size; lane maps a fixed
** per-attempt limit, as in the arcade original. */
static double	stage_time_limit(const t_map *map)
{
	if (map->has_lanes)
		return (LANE_TIME_LIMIT);
	return (20.0 + (map->rows + map->cols) * 1.5);
}

/* Resets the mutable per-attempt state from the pristine map. */
void	game_reset_stage(t_game *g)
{
	if (g->tiles == NULL)
		return ;
	memcpy(g->tiles, g->map.tiles, (size_t)g->map.cols * g->map.rows);
	memcpy(g->lanes, g->map.lanes, sizeof(t_lane) * g->map.rows);
	g->collected = 0;
	g->goals_filled = 0;
	g->combo = 0;
	g->combo_timer = 0.0;
	g->stage_time = 0.0;
	g->time_limit = stage_time_limit(&g->map);
	memset(g->popups, 0, sizeof(g->popups));
	enemies_reset(g);
	player_respawn(g);
	g->stage_start_score = g->score;
	g->stage_start_lives = g->lives;
	g->stage_start_moves = g->moves;
}

/* On failure the current level stays loaded (nothing leaks) and the game
** enters GAME_FATAL with g->error set, so the caller can report it. */
t_map_error	game_load_level(t_game *g, int index)
{
	t_map		next;
	char		*tiles;
	t_lane		*lanes;
	t_map_error	err;

	g->error_level = index;
	if (index < 0 || index >= g->level_count)
		err = MAP_ERR_OPEN;
	else
		err = map_load(g->levels[index], &next);
	tiles = NULL;
	lanes = NULL;
	if (err == MAP_OK)
	{
		tiles = malloc((size_t)next.cols * next.rows);
		lanes = malloc(sizeof(t_lane) * next.rows);
		if (tiles == NULL || lanes == NULL)
		{
			free(tiles);
			free(lanes);
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
	g->lanes = lanes;
	g->level_index = index;
	g->level_generation++;
	game_reset_stage(g);
	return (MAP_OK);
}

void	game_new_run(t_game *g)
{
	g->score = 0;
	g->lives = START_LIVES;
	g->moves = 0;
	g->new_best = false;
	if (game_load_level(g, 0) == MAP_OK)
	{
		g->state = GAME_PLAYING;
		g->events |= EV_START;
	}
}

void	game_retry_stage(t_game *g)
{
	g->score = g->stage_start_score;
	g->lives = g->stage_start_lives;
	g->moves = g->stage_start_moves;
	game_reset_stage(g);
	g->state = GAME_PLAYING;
	g->events |= EV_START;
}

static void	advance_stage(t_game *g)
{
	if (game_load_level(g, g->level_index + 1) == MAP_OK)
		g->state = GAME_PLAYING;
}

static void	handle_move(t_game *g, t_action a)
{
	if (a == ACT_UP)
		player_try_hop(g, DIR_UP);
	else if (a == ACT_DOWN)
		player_try_hop(g, DIR_DOWN);
	else if (a == ACT_LEFT)
		player_try_hop(g, DIR_LEFT);
	else if (a == ACT_RIGHT)
		player_try_hop(g, DIR_RIGHT);
}

static void	handle_action(t_game *g, t_action a)
{
	t_game_state	s;

	s = g->state;
	if (s == GAME_TITLE && a == ACT_CONFIRM)
		game_new_run(g);
	else if ((s == GAME_OVER || s == GAME_VICTORY)
		&& (a == ACT_CONFIRM || a == ACT_RESTART))
		game_new_run(g);
	else if ((s == GAME_PLAYING || s == GAME_PAUSED || s == GAME_DYING)
		&& a == ACT_RESTART)
		game_retry_stage(g);
	else if (s == GAME_PLAYING && a == ACT_PAUSE)
	{
		g->state = GAME_PAUSED;
		g->events |= EV_PAUSE;
	}
	else if (s == GAME_PAUSED && (a == ACT_PAUSE || a == ACT_CONFIRM))
	{
		g->state = GAME_PLAYING;
		g->events |= EV_PAUSE;
	}
	else if (s == GAME_STAGE_CLEAR && a == ACT_CONFIRM)
		advance_stage(g);
	else if (s == GAME_PLAYING)
		handle_move(g, a);
}

/* End of the death animation: lose a life, then respawn or game over. */
static void	finish_dying(t_game *g)
{
	g->lives--;
	if (g->lives <= 0)
	{
		g->lives = 0;
		score_commit_best(g);
		g->state = GAME_OVER;
		g->events |= EV_GAME_OVER;
		return ;
	}
	player_respawn(g);
	g->state = GAME_PLAYING;
}

static void	update_playing(t_game *g, double dt)
{
	g->stage_time += dt;
	lanes_update(g, dt);
	player_update(g, dt);
	if (g->state == GAME_PLAYING)
		enemies_update(g, dt);
	if (g->state != GAME_PLAYING)
		return ;
	g->time_left -= dt;
	if (g->time_left <= 0.0)
	{
		g->time_left = 0.0;
		player_kill(g, DEATH_TIME);
	}
}

void	game_update(t_game *g, const t_input *in, double dt)
{
	int	i;

	i = 0;
	while (i < in->count && g->state != GAME_FATAL)
		handle_action(g, in->actions[i++]);
	if (dt < 0.0)
		dt = 0.0;
	if (g->state == GAME_PAUSED || g->state == GAME_FATAL)
		return ;
	g->anim_time += dt;
	score_update(g, dt);
	if (g->state == GAME_PLAYING)
		update_playing(g, dt);
	else
		lanes_update(g, dt);
	if (g->state == GAME_DYING || g->state == GAME_STAGE_CLEAR)
	{
		g->state_timer -= dt;
		if (g->state_timer <= 0.0 && g->state == GAME_DYING)
			finish_dying(g);
		else if (g->state_timer <= 0.0)
			advance_stage(g);
	}
}
