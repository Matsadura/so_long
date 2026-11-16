#include "map.h"

#include <string.h>

static bool	is_valid_tile(char c)
{
	return (c != '\0' && strchr("01CEPN", c) != NULL);
}

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

/* Character set, closed perimeter and element counts. */
t_map_error	map_validate_layout(const char *tiles, int cols, int rows)
{
	int	i;
	int	players;
	int	exits;
	int	collectibles;
	int	enemies;

	players = 0;
	exits = 0;
	collectibles = 0;
	enemies = 0;
	i = 0;
	while (i < cols * rows)
	{
		if (!is_valid_tile(tiles[i]))
			return (MAP_ERR_INVALID_CHAR);
		players += (tiles[i] == TILE_PLAYER);
		exits += (tiles[i] == TILE_EXIT);
		collectibles += (tiles[i] == TILE_COLLECTIBLE);
		enemies += (tiles[i] == TILE_ENEMY);
		i++;
	}
	if (!is_closed(tiles, cols, rows))
		return (MAP_ERR_NOT_CLOSED);
	if (players != 1)
		return (MAP_ERR_PLAYER_COUNT);
	if (exits != 1)
		return (MAP_ERR_EXIT_COUNT);
	if (collectibles < 1)
		return (MAP_ERR_NO_COLLECTIBLE);
	if (enemies > MAP_MAX_ENEMIES)
		return (MAP_ERR_TOO_MANY_ENEMIES);
	return (MAP_OK);
}
