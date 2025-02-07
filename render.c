#include "libft/ft_printf.h"
#include "so_long.h"
#include <mlx.h>


/**
 *
 */
int	img_init(t_game game, t_img imgs[])
{
	int	i;
	static char *name[] = {
		"sprites/player_01.xpm",
		"sprites/wall_01.xpm",
		"sprites/effect_w.xpm",
		"sprites/door_01.xpm",
		"sprites/door_02.xpm",
		"sprites/enemy_01.xpm",
		"sprites/enemy_02.xpm",
		"sprites/bg.xpm",
		NULL
	};
	i = 0;
	while (i < 5)
	{
		imgs[i].img = XPM_I(game.mlx, name[i], &imgs[i].w, &imgs[i].h);
		if (imgs[i].img == NULL)
		{
			/*Free all*/
			return (FALSE);
		}
		i++;
	}
	return (TRUE);
}

/**
 *
 */
void	render_map(t_game game, t_img imgs[], t_map map)
{
	int	i;
	int	j;

	i = 0;
	while (i < map.rows)
	{
		j = 0;
		while (j < map.cols)
		{
			if (map.map[i][j] == '1')
				mlx_put_image_to_window(game.mlx, game.mlx_win, imgs[WALL].img, j * 64, i * 64);
			else if (j == map.start_x && i == map.start_y)
				mlx_put_image_to_window(game.mlx, game.mlx_win, imgs[PLAYER].img, j * 64, i * 64);
			else if (map.map[i][j] == 'E' && map.c_count == map.collects)
				mlx_put_image_to_window(game.mlx, game.mlx_win, imgs[EXIT2].img, j * 64, i * 64);
			else if (map.map[i][j] == 'E')
				mlx_put_image_to_window(game.mlx, game.mlx_win, imgs[EXIT1].img, j * 64, i * 64);
			else if (map.map[i][j] == 'C')
				mlx_put_image_to_window(game.mlx, game.mlx_win, imgs[COLLECTIBLE].img, j * 64, i * 64);
			j++;
		}
		i++;
	}
}

/**
 *
 */
int	exit_window(int keycode, t_data *data)
{
	int	i;

	if (keycode == ESC)
	{
		mlx_clear_window(data->game.mlx, data->game.mlx_win);
		i = 0;
		while (i < 5)
		{
			mlx_destroy_image(data->game.mlx, data->imgs[i].img);
			i++;
		}
		mlx_destroy_window(data->game.mlx, data->game.mlx_win);
		mlx_destroy_display(data->game.mlx);
		free(data->game.mlx);
		free_2darray(data->map.map);
		exit(0);
	}	
	return (0);
}

/**
 *
 */
void	move_player(t_data *data, int x, int y)
{
	static int	moves;
	int	new_x;
	int	new_y;

	ft_printf("Moves: %d\n", ++moves);
	new_x = data->map.start_x + x;
	new_y = data->map.start_y + y;
	if (data->map.map[new_y][new_x] == '1')
		return ;
	if (data->map.map[new_y][new_x] == 'E' && data->map.collects == data->map.c_count)
		exit_window(ESC, data);
	if (data->map.map[new_y][new_x] == 'C')
		data->map.c_count++;
	ft_printf("c_count: %d\n", data->map.c_count);
	data->map.map[data->map.start_y][data->map.start_x] = '0';

	data->map.start_x = new_x;
	data->map.start_y = new_y;
	data->map.map[new_y][new_x] = 'P';
	if (data->map.map[new_y][new_x] != data->map.map[data->map.exit_y][data->map.exit_x])
		data->map.map[data->map.exit_y][data->map.exit_x] = 'E';
	mlx_clear_window(data->game.mlx, data->game.mlx_win);
	render_map(data->game, data->imgs, data->map);
}

/**
 *
 */
int	on_keypress(int key, t_data *data)
{
	if (key == ESC)
		exit_window(ESC, data);
	else if (key == 'w' || key == UP)
		move_player(data, 0, -1);
	else if (key == 's' || key == DOWN)
		move_player(data, 0, 1);
	else if (key == 'd' || key == RIGHT)
		move_player(data, 1, 0);
	else if (key == 'a' || key == LEFT)
		move_player(data, -1, 0);
	return (0);
}

int	main(int ac, char **av)
{
	t_img	imgs[5];
	t_data	data;

	if (ac == 2)
	{
		data.map = read_and_check_map(av[1]);

		/*Render WINDOW*/
		data.game.mlx = mlx_init();
		if (data.game.mlx == NULL)
			return (EXIT_FAILURE);
		data.game.mlx_win = mlx_new_window(data.game.mlx, 64 * data.map.cols, 64 * data.map.rows, "so_long");
		if (data.game.mlx_win == NULL)
			return (EXIT_FAILURE);
		data.imgs = imgs;
		if (img_init(data.game, data.imgs) == FALSE)
			return (EXIT_FAILURE);
		/*Print Images*/
		render_map(data.game, data.imgs, data.map);

		//mlx_key_hook(data.game.mlx_win, exit_window, &data);
		mlx_key_hook(data.game.mlx_win, on_keypress, &data);
		/*Loop*/
		mlx_loop(data.game.mlx);
	}
	return (0);
}
