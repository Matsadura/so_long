#ifndef MAP_H
# define MAP_H

# include <stdbool.h>

/*
** .ber map loading and validation.
**
** A map is a rectangle of tiles surrounded by walls:
**   '1' wall      '0' floor      'C' collectible
**   'E' exit      'P' player     'N' patrolling enemy
**
** map_load() never exits the process: every failure is reported through a
** t_map_error so callers (CLI, stage transitions, restarts) can recover.
*/

# define MAP_MAX_COLS        64
# define MAP_MAX_ROWS        40
# define MAP_MAX_ENEMIES     128
# define MAP_MAX_FILE_BYTES  8192

# define TILE_WALL           '1'
# define TILE_FLOOR          '0'
# define TILE_COLLECTIBLE    'C'
# define TILE_EXIT           'E'
# define TILE_PLAYER         'P'
# define TILE_ENEMY          'N'

typedef enum e_map_error
{
	MAP_OK = 0,
	MAP_ERR_EXTENSION,
	MAP_ERR_OPEN,
	MAP_ERR_READ,
	MAP_ERR_FILE_TOO_BIG,
	MAP_ERR_EMPTY,
	MAP_ERR_EMPTY_LINE,
	MAP_ERR_NOT_RECTANGULAR,
	MAP_ERR_TOO_LARGE,
	MAP_ERR_TOO_SMALL,
	MAP_ERR_INVALID_CHAR,
	MAP_ERR_NOT_CLOSED,
	MAP_ERR_PLAYER_COUNT,
	MAP_ERR_EXIT_COUNT,
	MAP_ERR_NO_COLLECTIBLE,
	MAP_ERR_TOO_MANY_ENEMIES,
	MAP_ERR_UNREACHABLE_COLLECTIBLE,
	MAP_ERR_UNREACHABLE_EXIT,
	MAP_ERR_ALLOC
}	t_map_error;

typedef struct s_point
{
	int	x;
	int	y;
}	t_point;

typedef struct s_map
{
	int		cols;
	int		rows;
	/* rows * cols static tiles: only '0', '1', 'C' and 'E' remain here.
	** Player and enemy start tiles are recorded below and stored as '0'. */
	char	*tiles;
	t_point	player_start;
	t_point	exit;
	int		collectibles;
	int		enemy_count;
	t_point	enemy_start[MAP_MAX_ENEMIES];
}	t_map;

/* Loads and fully validates a .ber file. On failure *out is left empty
** (tiles == NULL) and nothing needs to be freed. */
t_map_error	map_load(const char *path, t_map *out);

/* Same as map_load but from an in-memory buffer (no extension check). */
t_map_error	map_parse(const char *text, int len, t_map *out);

void		map_free(t_map *map);
const char	*map_error_str(t_map_error err);

/* Bounds-safe accessor: anything outside the map reads as a wall. */
char		map_tile_at(const char *tiles, int cols, int rows, int x, int y);

/* Internal helpers shared by src/map/ */
t_map_error	map_validate_layout(const char *tiles, int cols, int rows);
t_map_error	map_check_reachability(const t_map *map);

#endif
