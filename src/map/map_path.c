#include "map.h"

#include <stdlib.h>

/*
** Iterative flood fill (explicit stack) from the player start. Recursion
** depth would otherwise reach rows*cols, which is unsafe on the small
** WebAssembly stack. Walls block; enemies patrol so their tiles count as
** walkable (they are already '0' after entity extraction).
*/
t_map_error	map_check_reachability(const t_map *map)
{
	static const int	dx[4] = {0, 0, -1, 1};
	static const int	dy[4] = {-1, 1, 0, 0};
	unsigned char		*seen;
	int					*stack;
	int					top;
	int					cur;
	int					d;
	int					nx;
	int					ny;
	int					found_c;
	bool				found_e;

	seen = calloc((size_t)map->cols * map->rows, 1);
	stack = malloc(sizeof(int) * (size_t)map->cols * map->rows);
	if (seen == NULL || stack == NULL)
	{
		free(seen);
		free(stack);
		return (MAP_ERR_ALLOC);
	}
	found_c = 0;
	found_e = false;
	top = 0;
	cur = map->player_start.y * map->cols + map->player_start.x;
	seen[cur] = 1;
	stack[top++] = cur;
	while (top > 0)
	{
		cur = stack[--top];
		found_c += (map->tiles[cur] == TILE_COLLECTIBLE);
		found_e |= (map->tiles[cur] == TILE_EXIT);
		d = 0;
		while (d < 4)
		{
			nx = cur % map->cols + dx[d];
			ny = cur / map->cols + dy[d];
			if (map_tile_at(map->tiles, map->cols, map->rows, nx, ny)
				!= TILE_WALL && !seen[ny * map->cols + nx])
			{
				seen[ny * map->cols + nx] = 1;
				stack[top++] = ny * map->cols + nx;
			}
			d++;
		}
	}
	free(seen);
	free(stack);
	if (found_c != map->collectibles)
		return (MAP_ERR_UNREACHABLE_COLLECTIBLE);
	if (!found_e)
		return (MAP_ERR_UNREACHABLE_EXIT);
	return (MAP_OK);
}
