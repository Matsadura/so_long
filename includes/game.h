#ifndef GAME_H
# define GAME_H

# include "map.h"
# include "platform.h"

/*
** Core game state. Pure logic: no rendering, no SDL. Time is passed in as
** dt (seconds) so speeds are identical at any refresh rate.
*/

# define ENEMY_STEP_SECONDS   0.45
# define ANIM_FRAME_SECONDS   0.5

typedef enum e_game_state
{
	GAME_PLAYING,
	GAME_LEVEL_CLEAR,
	GAME_COMPLETE,
	GAME_OVER,
	GAME_FATAL
}	t_game_state;

typedef enum e_dir
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
}	t_dir;

typedef struct s_enemy
{
	t_point	pos;
	int		dir;
}	t_enemy;

typedef struct s_game
{
	const char *const	*levels;
	int					level_count;
	int					level_index;
	/* Bumped on every successful level load so renderers can rebuild
	** cached, map-sized resources exactly once. */
	unsigned			level_generation;

	t_map				map;
	/* Working copy of map.tiles (collectibles get picked up). */
	char				*tiles;

	t_point				player;
	t_dir				facing;
	t_enemy				enemies[MAP_MAX_ENEMIES];
	int					enemy_count;

	int					collected;
	int					moves;
	t_game_state		state;
	t_map_error			error;
	int					error_level;

	double				enemy_timer;
	double				anim_time;
}	t_game;

/* Loads levels[0]. Returns MAP_OK or the load error (state = GAME_FATAL). */
t_map_error	game_init(t_game *g, const char *const *levels, int count);
void		game_free(t_game *g);

t_map_error	game_load_level(t_game *g, int index);
void		game_restart_level(t_game *g);

void		game_update(t_game *g, const t_input *in, double dt);

/* src/core/enemies.c */
void		enemies_reset(t_game *g);
void		enemies_update(t_game *g, double dt);
bool		enemy_at(const t_game *g, int x, int y);

/* src/map/levels.c */
extern const char *const	g_campaign_levels[];
extern const int			g_campaign_level_count;

#endif
