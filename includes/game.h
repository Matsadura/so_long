#ifndef GAME_H
# define GAME_H

# include "map.h"
# include "platform.h"

/*
** Core game state. Pure logic: no rendering, no SDL. Time is passed in as
** dt (seconds) so speeds are identical at any refresh rate.
*/

/* Pacing */
# define START_LIVES          3
# define HOP_SECONDS          0.11
# define DEATH_SECONDS        1.1
# define CLEAR_SECONDS        3.5
# define STAGE_BANNER_SECONDS 1.6
# define LANE_TIME_LIMIT      30.0
# define ENEMY_STEP_SECONDS   0.45
# define ANIM_FRAME_SECONDS   0.5
# define COMBO_WINDOW         3.0
# define COMBO_MAX            5

/* Scoring */
# define SCORE_STEP           10
# define SCORE_COIN           50
# define SCORE_GOAL           200
# define SCORE_TIME_PER_SEC   10
# define SCORE_STAGE          1000

/* Hit boxes in tiles */
# define CAR_HIT_MARGIN       0.2f
# define LOG_GRACE            0.15f

# define MAX_POPUPS           8

typedef enum e_game_state
{
	GAME_TITLE,
	GAME_PLAYING,
	GAME_PAUSED,
	GAME_DYING,
	GAME_STAGE_CLEAR,
	GAME_OVER,
	GAME_VICTORY,
	GAME_FATAL
}	t_game_state;

typedef enum e_death
{
	DEATH_NONE,
	DEATH_CAR,
	DEATH_WATER,
	DEATH_ENEMY,
	DEATH_TIME
}	t_death;

/*
** One-frame notifications for presentation (sound effects, etc.). The core
** only sets bits; main clears them at the start of every tick.
*/
# define EV_HOP          (1u << 0)
# define EV_COIN         (1u << 1)
# define EV_GOAL         (1u << 2)
# define EV_CAR          (1u << 3)
# define EV_SPLASH       (1u << 4)
# define EV_CAUGHT       (1u << 5)
# define EV_TIME_UP      (1u << 6)
# define EV_STAGE_CLEAR  (1u << 7)
# define EV_GAME_OVER    (1u << 8)
# define EV_VICTORY      (1u << 9)
# define EV_START        (1u << 10)
# define EV_PAUSE        (1u << 11)

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

/* Position is in tiles; x is fractional while riding a log. A hop
** interpolates from (from_x, from_y) to (to_x, to_y) over HOP_SECONDS. */
typedef struct s_player
{
	float	x;
	float	y;
	float	from_x;
	float	from_y;
	float	to_x;
	float	to_y;
	float	hop_t;
	bool	hopping;
	t_dir	facing;
	bool	has_queued;
	t_dir	queued;
	int		best_row;
}	t_player;

/* Floating "+50" style score feedback. */
typedef struct s_popup
{
	float	x;
	float	y;
	float	ttl;
	int		value;
}	t_popup;

typedef struct s_game
{
	const char *const	*levels;
	int					level_count;
	int					level_index;
	/* Bumped on every successful level load so renderers can rebuild
	** cached, map-sized resources exactly once. */
	unsigned			level_generation;

	t_map				map;
	/* Working copies of map.tiles / map.lanes for the current attempt. */
	char				*tiles;
	t_lane				*lanes;

	t_player			player;
	t_enemy				enemies[MAP_MAX_ENEMIES];
	int					enemy_count;
	double				enemy_timer;

	int					lives;
	int					score;
	int					best_score;
	int					moves;
	int					collected;
	int					goals_filled;
	int					combo;
	double				combo_timer;
	double				time_left;
	double				time_limit;

	/* Snapshot taken when a stage starts, restored by "retry stage". */
	int					stage_start_score;
	int					stage_start_lives;
	int					stage_start_moves;

	t_game_state		state;
	double				state_timer;
	double				stage_time;
	double				anim_time;
	t_death				death;
	int					clear_time_bonus;
	int					clear_stage_bonus;
	bool				new_best;
	t_popup				popups[MAX_POPUPS];

	unsigned			events;

	t_map_error			error;
	int					error_level;
}	t_game;

/* src/core/game.c */
t_map_error	game_init(t_game *g, const char *const *levels, int count);
void		game_free(t_game *g);
t_map_error	game_load_level(t_game *g, int index);
void		game_new_run(t_game *g);
void		game_retry_stage(t_game *g);
void		game_reset_stage(t_game *g);
void		game_update(t_game *g, const t_input *in, double dt);
bool		game_is_classic(const t_game *g);
char		game_tile(const t_game *g, int x, int y);

/* src/core/player.c */
void		player_respawn(t_game *g);
void		player_try_hop(t_game *g, t_dir dir);
void		player_update(t_game *g, double dt);
void		player_kill(t_game *g, t_death cause);
int			player_tile_x(const t_game *g);

/* src/core/lanes.c */
void		lanes_update(t_game *g, double dt);
int			lane_width(const t_game *g);
bool		lane_overlaps(const t_game *g, int row, float a, float b);

/* src/core/enemies.c */
void		enemies_reset(t_game *g);
void		enemies_update(t_game *g, double dt);
bool		enemy_at(const t_game *g, int x, int y);

/* src/core/scoring.c */
void		score_add(t_game *g, int points, float x, float y);
void		score_coin(t_game *g, float x, float y);
void		score_update(t_game *g, double dt);
void		score_commit_best(t_game *g);

/* src/map/levels.c */
extern const char *const	g_campaign_levels[];
extern const int			g_campaign_level_count;

#endif
