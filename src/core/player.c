#include "game.h"

#include <math.h>

static t_lane_kind	row_kind(const t_game *g, int y)
{
	if (y < 0 || y >= g->map.rows)
		return (LANE_NONE);
	return (g->lanes[y].kind);
}

int	player_tile_x(const t_game *g)
{
	return ((int)floorf(g->player.x + 0.5f));
}

void	player_respawn(t_game *g)
{
	t_player	*p;

	p = &g->player;
	p->x = (float)g->map.player_start.x;
	p->y = (float)g->map.player_start.y;
	p->from_x = p->x;
	p->from_y = p->y;
	p->to_x = p->x;
	p->to_y = p->y;
	p->hop_t = 0.0f;
	p->hopping = false;
	p->has_queued = false;
	p->facing = DIR_UP;
	p->best_row = g->map.player_start.y;
	g->time_left = g->time_limit;
	g->death = DEATH_NONE;
}

void	player_kill(t_game *g, t_death cause)
{
	static const unsigned	ev[] = {0, EV_CAR, EV_SPLASH, EV_CAUGHT,
		EV_TIME_UP};

	if (g->state != GAME_PLAYING)
		return ;
	g->death = cause;
	g->events |= ev[cause];
	g->state = GAME_DYING;
	g->state_timer = DEATH_SECONDS;
	g->combo = 0;
	g->combo_timer = 0.0;
	g->player.hopping = false;
	g->player.has_queued = false;
}

static void	start_hop(t_game *g, t_dir dir)
{
	t_player	*p;
	int			dx;
	int			ty;
	float		tx;
	char		t;

	p = &g->player;
	p->facing = dir;
	dx = (dir == DIR_RIGHT) - (dir == DIR_LEFT);
	ty = (int)p->y + (dir == DIR_DOWN) - (dir == DIR_UP);
	tx = p->x + dx;
	if (dx == 0 && row_kind(g, ty) != LANE_RIVER)
		tx = floorf(tx + 0.5f);
	t = game_tile(g, (int)floorf(tx + 0.5f), ty);
	if (t == TILE_WALL || t == TILE_GOAL_FILLED)
		return ;
	p->from_x = p->x;
	p->from_y = p->y;
	p->to_x = tx;
	p->to_y = (float)ty;
	p->hop_t = 0.0f;
	p->hopping = true;
	g->moves++;
	g->events |= EV_HOP;
}

/* Hops are buffered: a press during a hop starts right after it lands. */
void	player_try_hop(t_game *g, t_dir dir)
{
	if (g->player.hopping)
	{
		g->player.has_queued = true;
		g->player.queued = dir;
		return ;
	}
	start_hop(g, dir);
}

static void	stage_clear(t_game *g)
{
	g->clear_time_bonus = 0;
	if (game_is_classic(g))
		g->clear_time_bonus = (int)g->time_left * SCORE_TIME_PER_SEC;
	g->clear_stage_bonus = SCORE_STAGE;
	g->score += g->clear_time_bonus + g->clear_stage_bonus;
	if (g->level_index + 1 >= g->level_count)
	{
		score_commit_best(g);
		g->state = GAME_VICTORY;
		g->events |= EV_VICTORY;
		return ;
	}
	g->state = GAME_STAGE_CLEAR;
	g->events |= EV_STAGE_CLEAR;
	g->state_timer = CLEAR_SECONDS;
}

static void	reach_goal(t_game *g, int x, int y)
{
	if (game_is_classic(g))
	{
		if (g->collected == g->map.collectibles)
			stage_clear(g);
		return ;
	}
	g->tiles[y * g->map.cols + x] = TILE_GOAL_FILLED;
	g->goals_filled++;
	g->events |= EV_GOAL;
	score_add(g, SCORE_GOAL + (int)g->time_left * SCORE_TIME_PER_SEC,
		(float)x, (float)y);
	if (g->goals_filled == g->map.goals)
		stage_clear(g);
	else
		player_respawn(g);
}

static void	land(t_game *g)
{
	t_player	*p;
	int			x;
	int			y;
	char		t;

	p = &g->player;
	p->x = p->to_x;
	p->y = p->to_y;
	p->hopping = false;
	y = (int)p->y;
	if (row_kind(g, y) != LANE_RIVER)
		p->x = floorf(p->x + 0.5f);
	if (y < p->best_row)
	{
		p->best_row = y;
		score_add(g, SCORE_STEP, p->x, p->y);
	}
	x = player_tile_x(g);
	t = game_tile(g, x, y);
	if (t == TILE_COLLECTIBLE)
	{
		g->tiles[y * g->map.cols + x]
			= (row_kind(g, y) == LANE_ROAD) ? TILE_ROAD : TILE_FLOOR;
		g->collected++;
		score_coin(g, (float)x, (float)y);
		g->events |= EV_COIN;
	}
	else if (t == TILE_EXIT)
		reach_goal(g, x, y);
}

static void	check_hazards(t_game *g)
{
	t_player	*p;
	int			y;
	float		u;

	p = &g->player;
	y = (int)p->y;
	u = p->x - 1.0f;
	if (row_kind(g, y) == LANE_ROAD
		&& lane_overlaps(g, y, u + CAR_HIT_MARGIN, u + 1.0f - CAR_HIT_MARGIN))
		player_kill(g, DEATH_CAR);
	else if (row_kind(g, y) == LANE_RIVER
		&& (p->x < 0.5f || p->x > g->map.cols - 1.5f
			|| !lane_overlaps(g, y, u + 0.5f - LOG_GRACE,
				u + 0.5f + LOG_GRACE)))
		player_kill(g, DEATH_WATER);
	else if (enemy_at(g, player_tile_x(g), y))
		player_kill(g, DEATH_ENEMY);
}

/* Drift on logs, advance hops, then check hazards when standing. */
void	player_update(t_game *g, double dt)
{
	t_player	*p;
	float		drift;
	int			row;

	p = &g->player;
	row = (int)(p->hopping ? p->from_y : p->y);
	drift = 0.0f;
	if (row_kind(g, row) == LANE_RIVER)
		drift = g->lanes[row].speed * (float)dt;
	p->from_x += drift;
	p->to_x += drift;
	if (!p->hopping)
		p->x += drift;
	else
	{
		p->hop_t += (float)(dt / HOP_SECONDS);
		if (p->hop_t >= 1.0f)
			land(g);
		else
		{
			p->x = p->from_x + (p->to_x - p->from_x) * p->hop_t;
			p->y = p->from_y + (p->to_y - p->from_y) * p->hop_t;
		}
	}
	if (g->state == GAME_PLAYING && !p->hopping)
		check_hazards(g);
	if (g->state == GAME_PLAYING && !p->hopping && p->has_queued)
	{
		p->has_queued = false;
		start_hop(g, p->queued);
	}
}
