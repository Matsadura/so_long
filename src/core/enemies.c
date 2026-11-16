#include "game.h"

void	enemies_reset(t_game *g)
{
	int	i;

	g->enemy_count = g->map.enemy_count;
	i = 0;
	while (i < g->enemy_count)
	{
		g->enemies[i].pos = g->map.enemy_start[i];
		g->enemies[i].dir = 1;
		i++;
	}
	g->enemy_timer = 0.0;
}

bool	enemy_at(const t_game *g, int x, int y)
{
	int	i;

	i = 0;
	while (i < g->enemy_count)
	{
		if (g->enemies[i].pos.x == x && g->enemies[i].pos.y == y)
			return (true);
		i++;
	}
	return (false);
}

/* Horizontal patrol: turn around at walls, the exit, collectibles and
** other enemies. Touching the player ends the attempt. */
static void	step_enemy(t_game *g, t_enemy *e)
{
	int		nx;
	char	t;

	nx = e->pos.x + e->dir;
	t = map_tile_at(g->tiles, g->map.cols, g->map.rows, nx, e->pos.y);
	if (t == TILE_WALL || t == TILE_EXIT || t == TILE_COLLECTIBLE
		|| enemy_at(g, nx, e->pos.y))
	{
		e->dir = -e->dir;
		return ;
	}
	e->pos.x = nx;
	if (e->pos.x == g->player.x && e->pos.y == g->player.y)
		g->state = GAME_OVER;
}

/* Fixed-interval steps driven by accumulated dt (frame-rate independent). */
void	enemies_update(t_game *g, double dt)
{
	int	i;

	g->enemy_timer += dt;
	while (g->enemy_timer >= ENEMY_STEP_SECONDS && g->state == GAME_PLAYING)
	{
		g->enemy_timer -= ENEMY_STEP_SECONDS;
		i = 0;
		while (i < g->enemy_count && g->state == GAME_PLAYING)
			step_enemy(g, &g->enemies[i++]);
	}
}
