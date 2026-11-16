#include "game.h"

#include <math.h>

int	lane_width(const t_game *g)
{
	return (g->map.cols - 2);
}

/* Moves every vehicle/log by speed * dt and wraps it around the lane. */
void	lanes_update(t_game *g, double dt)
{
	t_lane	*lane;
	float	w;
	int		y;
	int		i;

	w = (float)lane_width(g);
	y = 0;
	while (y < g->map.rows)
	{
		lane = &g->lanes[y++];
		if (lane->kind == LANE_NONE)
			continue ;
		i = 0;
		while (i < lane->count)
		{
			lane->objs[i].x = fmodf(lane->objs[i].x + lane->speed * (float)dt,
					w);
			if (lane->objs[i].x < 0.0f)
				lane->objs[i].x += w;
			i++;
		}
	}
}

/* True if [a, b) in lane space overlaps any object of the row's lane,
** taking wrap-around into account. */
bool	lane_overlaps(const t_game *g, int row, float a, float b)
{
	const t_lane	*lane;
	float			w;
	float			lo;
	int				i;
	int				shift;

	if (row < 0 || row >= g->map.rows)
		return (false);
	lane = &g->lanes[row];
	w = (float)lane_width(g);
	i = 0;
	while (i < lane->count)
	{
		shift = -1;
		while (shift <= 1)
		{
			lo = lane->objs[i].x + shift * w;
			if (a < lo + lane->objs[i].len && b > lo)
				return (true);
			shift++;
		}
		i++;
	}
	return (false);
}
