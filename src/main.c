#include "game.h"
#include "platform.h"
#include "ui.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
# include <emscripten.h>
#endif

#define MAX_DT       0.1
#define NATIVE_FPS   240

typedef struct s_app
{
	t_platform	*platform;
	t_game		game;
	t_renderer	renderer;
	char		**custom_levels;
	int			custom_count;
	double		last_time;
	bool		running;
	int			exit_code;
	/* Debug / test automation (see usage) */
	long		frame_limit;
	long		frames;
	const char	*script;
	double		fixed_dt;
}	t_app;

static void	usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [options] [map.ber ...]\n"
		"  With no maps, plays the built-in campaign (assets/maps).\n"
		"  --check          validate the given maps and exit\n"
		"  --frames N       quit after N frames (testing)\n"
		"  --dt SECONDS     fixed simulation step per frame (testing)\n"
		"  --keys STR       scripted input, one char per frame:\n"
		"                   wasd move, e enter, r restart, q quit, . wait\n",
		prog);
}

static int	check_maps(char **paths, int count)
{
	t_map		map;
	t_map_error	err;
	int			failed;
	int			i;

	failed = 0;
	i = 0;
	while (i < count)
	{
		err = map_load(paths[i], &map);
		if (err == MAP_OK)
			printf("%s: OK (%dx%d, %d coins, %d enemies)\n", paths[i],
				map.cols, map.rows, map.collectibles, map.enemy_count);
		else
			printf("%s: Error: %s\n", paths[i], map_error_str(err));
		failed += (err != MAP_OK);
		map_free(&map);
		i++;
	}
	return (failed != 0);
}

static void	app_shutdown(t_app *app)
{
	int	i;

	game_free(&app->game);
	platform_destroy(app->platform);
	app->platform = NULL;
	i = 0;
	while (i < app->custom_count)
		free(app->custom_levels[i++]);
	free(app->custom_levels);
	app->custom_levels = NULL;
	app->custom_count = 0;
}

static void	scripted_input(t_app *app, t_input *in)
{
	static const char		keys[] = "wsader";
	static const t_action	acts[] = {ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT,
		ACT_CONFIRM, ACT_RESTART};
	const char				*k;

	if (app->script == NULL || *app->script == '\0')
		return ;
	k = strchr(keys, *app->script);
	if (*app->script == 'q')
		in->quit_requested = true;
	else if (k != NULL)
		input_push(in, acts[k - keys]);
	app->script++;
}

static bool	wants_quit(const t_input *in)
{
	int	i;

	if (in->quit_requested)
		return (true);
	i = 0;
	while (i < in->count)
		if (in->actions[i++] == ACT_QUIT)
			return (true);
	return (false);
}

/* One frame: input -> update(dt) -> render. Never blocks. */
static void	tick(void *arg)
{
	t_app	*app;
	t_input	in;
	double	now;
	double	dt;

	app = arg;
	now = platform_time_seconds();
	dt = now - app->last_time;
	app->last_time = now;
	if (dt > MAX_DT)
		dt = MAX_DT;
	if (app->fixed_dt > 0.0)
		dt = app->fixed_dt;
	platform_poll_input(app->platform, &in);
	scripted_input(app, &in);
	if (in.targets_reset)
		renderer_invalidate(&app->renderer);
	if (wants_quit(&in))
		app->running = false;
	else
	{
		game_update(&app->game, &in, dt);
		if (app->game.state == GAME_FATAL)
		{
			fprintf(stderr, "Error: level %d: %s\n",
				app->game.error_level + 1, map_error_str(app->game.error));
			app->running = false;
			app->exit_code = EXIT_FAILURE;
		}
		else
			renderer_draw(&app->renderer, app->platform, &app->game);
	}
	if (app->frame_limit > 0 && ++app->frames >= app->frame_limit)
		app->running = false;
#ifdef __EMSCRIPTEN__
	if (!app->running)
	{
		emscripten_cancel_main_loop();
		app_shutdown(app);
	}
#endif
}

/* Resolves user map paths before platform_locate_assets() may chdir. */
static bool	store_custom_levels(t_app *app, char **paths, int count)
{
	char	resolved[PATH_MAX];

	app->custom_levels = calloc(count, sizeof(char *));
	if (app->custom_levels == NULL)
		return (false);
	while (app->custom_count < count)
	{
		if (realpath(paths[app->custom_count], resolved) != NULL)
			app->custom_levels[app->custom_count] = strdup(resolved);
		else
			app->custom_levels[app->custom_count]
				= strdup(paths[app->custom_count]);
		if (app->custom_levels[app->custom_count] == NULL)
			return (false);
		app->custom_count++;
	}
	return (true);
}

static int	parse_args(t_app *app, int ac, char **av, bool *check)
{
	int	i;

	i = 1;
	while (i < ac && strncmp(av[i], "--", 2) == 0)
	{
		if (strcmp(av[i], "--check") == 0)
			*check = true;
		else if (strcmp(av[i], "--frames") == 0 && i + 1 < ac)
			app->frame_limit = strtol(av[++i], NULL, 10);
		else if (strcmp(av[i], "--dt") == 0 && i + 1 < ac)
			app->fixed_dt = strtod(av[++i], NULL);
		else if (strcmp(av[i], "--keys") == 0 && i + 1 < ac)
			app->script = av[++i];
		else
			return (-1);
		i++;
	}
	return (i);
}

static int	start(t_app *app)
{
	t_map_error	err;
	int			w;
	int			h;

	if (!platform_locate_assets())
		return (fprintf(stderr, "Error: assets/ directory not found\n"), 1);
	if (app->custom_count > 0)
		err = game_init(&app->game, (const char *const *)app->custom_levels,
				app->custom_count);
	else
		err = game_init(&app->game, g_campaign_levels, g_campaign_level_count);
	if (err != MAP_OK)
		return (fprintf(stderr, "Error: %s: %s\n",
				app->game.levels[0], map_error_str(err)), 1);
	renderer_view_size(&app->game, &w, &h);
	app->platform = platform_create("so_long", w, h);
	if (app->platform == NULL)
		return (fprintf(stderr, "Error: SDL: %s\n", platform_error()), 1);
	if (!renderer_init(&app->renderer, app->platform))
		return (1);
	return (0);
}

int	main(int ac, char **av)
{
	static t_app	app;
	bool			check;
	int				first;

	check = false;
	first = parse_args(&app, ac, av, &check);
	if (first < 0 || (check && first >= ac))
		return (usage(av[0]), EXIT_FAILURE);
	if (check)
		return (check_maps(av + first, ac - first));
	if (first < ac && !store_custom_levels(&app, av + first, ac - first))
		return (app_shutdown(&app), perror("so_long"), EXIT_FAILURE);
	if (start(&app) != 0)
		return (app_shutdown(&app), EXIT_FAILURE);
	app.running = true;
	app.last_time = platform_time_seconds();
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(tick, &app, 0, 1);
#else
	while (app.running)
	{
		double frame_start = platform_time_seconds();
		tick(&app);
		platform_limit_fps(app.platform, frame_start, NATIVE_FPS);
	}
	if (app.script != NULL || app.frame_limit > 0)
		printf("[status] level=%d state=%s moves=%d coins=%d/%d\n",
			app.game.level_index + 1, (const char *[]){"PLAYING",
			"LEVEL_CLEAR", "COMPLETE", "GAME_OVER", "FATAL"}[app.game.state],
			app.game.moves,
			app.game.collected, app.game.map.collectibles);
	app_shutdown(&app);
#endif
	return (app.exit_code);
}
