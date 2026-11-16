#include "map.h"

static bool	is_speed_digit(char c)
{
	return (c >= '2' && c <= '9');
}

/* Classifies a row from its interior: road, river, ordinary, or invalid. */
static t_map_error	classify_row(const char *row, int cols, t_lane_kind *kind)
{
	int		x;
	bool	road;
	bool	river;
	bool	other;
	bool	coin;

	road = false;
	coin = false;
	river = false;
	other = false;
	x = 1;
	while (x < cols - 1)
	{
		road |= (row[x] == TILE_ROAD || row[x] == TILE_CAR);
		river |= (row[x] == TILE_WATER || row[x] == TILE_LOG);
		coin |= (row[x] == TILE_COLLECTIBLE);
		other |= (row[x] != TILE_ROAD && row[x] != TILE_CAR
				&& row[x] != TILE_WATER && row[x] != TILE_LOG
				&& row[x] != TILE_COLLECTIBLE);
		x++;
	}
	*kind = LANE_NONE;
	if (road && river)
		return (MAP_ERR_LANE_CONTENT);
	if (road || river)
	{
		*kind = road ? LANE_ROAD : LANE_RIVER;
		if (other || (river && coin))
			return (MAP_ERR_LANE_CONTENT);
	}
	return (MAP_OK);
}

static t_map_error	lane_speed(char *row, int cols, int y, t_lane *lane)
{
	bool	left;
	bool	right;

	left = is_speed_digit(row[0]);
	right = is_speed_digit(row[cols - 1]);
	if (left && right)
		return (MAP_ERR_LANE_SPEED);
	if (left)
		lane->speed = (row[0] - '0') * LANE_SPEED_UNIT;
	else if (right)
		lane->speed = -(row[cols - 1] - '0') * LANE_SPEED_UNIT;
	else
		lane->speed = LANE_DEFAULT_DIGIT * LANE_SPEED_UNIT
			* ((y % 2 == 0) ? 1.0f : -1.0f);
	if (left)
		row[0] = TILE_WALL;
	if (right)
		row[cols - 1] = TILE_WALL;
	return (MAP_OK);
}

/* Turns runs of X / L into moving objects and replaces them with the
** lane's ground tile. Coins on roads stay as static tiles. */
static void	extract_objects(char *row, int cols, t_lane *lane)
{
	char	obj;
	char	ground;
	int		x;
	int		start;

	obj = (lane->kind == LANE_ROAD) ? TILE_CAR : TILE_LOG;
	ground = (lane->kind == LANE_ROAD) ? TILE_ROAD : TILE_WATER;
	x = 1;
	while (x < cols - 1)
	{
		if (row[x] != obj)
		{
			x++;
			continue ;
		}
		start = x;
		while (x < cols - 1 && row[x] == obj)
			row[x++] = ground;
		lane->objs[lane->count++] = (t_lane_obj){(float)(start - 1),
			x - start};
	}
}

t_map_error	map_build_lanes(t_map *map)
{
	t_map_error	err;
	t_lane		*lane;
	char		*row;
	int			y;

	y = 0;
	while (y < map->rows)
	{
		row = map->tiles + y * map->cols;
		lane = &map->lanes[y];
		err = classify_row(row, map->cols, &lane->kind);
		if (err != MAP_OK)
			return (err);
		if (lane->kind == LANE_NONE)
		{
			if (is_speed_digit(row[0]) || is_speed_digit(row[map->cols - 1]))
				return (MAP_ERR_LANE_SPEED);
			y++;
			continue ;
		}
		err = lane_speed(row, map->cols, y, lane);
		if (err != MAP_OK)
			return (err);
		extract_objects(row, map->cols, lane);
		if (lane->kind == LANE_RIVER && lane->count == 0)
			return (MAP_ERR_RIVER_NO_LOG);
		if (lane->kind == LANE_ROAD && lane->count == 1
			&& lane->objs[0].len == map->cols - 2)
			return (MAP_ERR_LANE_CONTENT);
		map->has_lanes = true;
		y++;
	}
	return (MAP_OK);
}
