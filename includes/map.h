#ifndef MAP_H
# define MAP_H

# include <stdbool.h>

/*
** .ber map loading and validation.
**
** A map is a rectangle of tiles surrounded by walls:
**   '1' wall      '0' floor / safe median     'C' coin
**   'E' exit / goal slot                      'P' player start
**   'N' patrolling enemy (safe rows only)
**
** Lane rows (Frogger-style traffic) are rows whose interior is entirely:
**   road:  'R' asphalt, 'X' vehicle (runs of X = trucks), 'C' coin
**   river: 'W' water,   'L' log (runs of L = longer logs)
** A digit 2..9 in a lane row's left border sets its speed and makes it flow
** right; on the right border it flows left. Without a digit the lane uses
** speed 3 and alternates direction by row.
**
** Maps without lanes play by the classic so_long rules (collect every coin,
** then reach the single exit). Maps with lanes need 1..16 goal slots ('E')
** which must all be filled; coins become optional bonus pickups.
**
** map_load() never exits the process: every failure is reported through a
** t_map_error so callers (CLI, stage transitions, restarts) can recover.
*/

# define MAP_MAX_COLS        64
# define MAP_MAX_ROWS        40
# define MAP_MAX_ENEMIES     128
# define MAP_MAX_GOALS       16
# define MAP_MAX_FILE_BYTES  8192

# define LANE_SPEED_UNIT     0.55f
# define LANE_DEFAULT_DIGIT  3

# define TILE_WALL           '1'
# define TILE_FLOOR          '0'
# define TILE_COLLECTIBLE    'C'
# define TILE_EXIT           'E'
# define TILE_PLAYER         'P'
# define TILE_ENEMY          'N'
# define TILE_ROAD           'R'
# define TILE_CAR            'X'
# define TILE_WATER          'W'
# define TILE_LOG            'L'
/* Runtime only: a goal slot that has been filled. */
# define TILE_GOAL_FILLED    'F'

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
	MAP_ERR_LANE_CONTENT,
	MAP_ERR_LANE_SPEED,
	MAP_ERR_RIVER_NO_LOG,
	MAP_ERR_PLAYER_COUNT,
	MAP_ERR_EXIT_COUNT,
	MAP_ERR_GOAL_COUNT,
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

typedef enum e_lane_kind
{
	LANE_NONE,
	LANE_ROAD,
	LANE_RIVER
}	t_lane_kind;

/* A vehicle or log: x is its left edge in lane space [0, lane width),
** where lane space starts at map column 1 and wraps around. */
typedef struct s_lane_obj
{
	float	x;
	int		len;
}	t_lane_obj;

typedef struct s_lane
{
	t_lane_kind	kind;
	float		speed;
	int			count;
	t_lane_obj	objs[MAP_MAX_COLS];
}	t_lane;

typedef struct s_map
{
	int		cols;
	int		rows;
	/* rows * cols static tiles: '0', '1', 'C', 'E', 'R', 'W' remain.
	** Player/enemy starts become '0', vehicles 'R', logs 'W', digits '1'. */
	char	*tiles;
	/* One entry per row (kind LANE_NONE for ordinary rows). */
	t_lane	*lanes;
	bool	has_lanes;
	t_point	player_start;
	int		goals;
	int		collectibles;
	int		enemy_count;
	t_point	enemy_start[MAP_MAX_ENEMIES];
}	t_map;

/* Loads and fully validates a .ber file. On failure *out is left empty
** and nothing needs to be freed. */
t_map_error	map_load(const char *path, t_map *out);

/* Same as map_load but from an in-memory buffer (no extension check). */
t_map_error	map_parse(const char *text, int len, t_map *out);

void		map_free(t_map *map);
const char	*map_error_str(t_map_error err);

/* Bounds-safe accessor: anything outside the map reads as a wall. */
char		map_tile_at(const char *tiles, int cols, int rows, int x, int y);

/* Internal pipeline stages shared by src/map/ */
t_map_error	map_check_chars(const char *tiles, int cols, int rows);
t_map_error	map_build_lanes(t_map *map);
t_map_error	map_validate_layout(const t_map *map);
void		map_extract_entities(t_map *map);
t_map_error	map_check_reachability(const t_map *map);

#endif
