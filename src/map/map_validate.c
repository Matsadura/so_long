#include "map.h"

#include <string.h>

static bool	is_digit_speed(char c)
{
	return (c >= '2' && c <= '9');
}

t_map_error	map_check_chars(const char *tiles, int cols, int rows)
{
	int	i;

	i = 0;
	while (i < cols * rows)
	{
		if (tiles[i] == '\0' || (strchr("01CEPNRXWL", tiles[i]) == NULL
				&& !is_digit_speed(tiles[i])))
			return (MAP_ERR_INVALID_CHAR);
		i++;
	}
	return (MAP_OK);
}

/* Runs after map_build_lanes(), so speed digits are already walls. */
static bool	is_closed(const char *tiles, int cols, int rows)
{
	int	x;
	int	y;

	x = 0;
	while (x < cols)
	{
		if (tiles[x] != TILE_WALL || tiles[(rows - 1) * cols + x] != TILE_WALL)
			return (false);
		x++;
	}
	y = 0;
	while (y < rows)
	{
		if (tiles[y * cols] != TILE_WALL
			|| tiles[y * cols + cols - 1] != TILE_WALL)
			return (false);
		y++;
	}
	return (true);
}

static int	count_tile(const t_map *map, char c)
{
	int	i;
	int	n;

	n = 0;
	i = 0;
	while (i < map->cols * map->rows)
		n += (map->tiles[i++] == c);
	return (n);
}

/* Closed perimeter and element counts (rules depend on the map mode). */
t_map_error	map_validate_layout(const t_map *map)
{
	int	exits;

	if (!is_closed(map->tiles, map->cols, map->rows))
		return (MAP_ERR_NOT_CLOSED);
	if (count_tile(map, TILE_PLAYER) != 1)
		return (MAP_ERR_PLAYER_COUNT);
	exits = count_tile(map, TILE_EXIT);
	if (!map->has_lanes && exits != 1)
		return (MAP_ERR_EXIT_COUNT);
	if (map->has_lanes && (exits < 1 || exits > MAP_MAX_GOALS))
		return (MAP_ERR_GOAL_COUNT);
	if (!map->has_lanes && count_tile(map, TILE_COLLECTIBLE) < 1)
		return (MAP_ERR_NO_COLLECTIBLE);
	if (count_tile(map, TILE_ENEMY) > MAP_MAX_ENEMIES)
		return (MAP_ERR_TOO_MANY_ENEMIES);
	return (MAP_OK);
}
