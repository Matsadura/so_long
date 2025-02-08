/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_bonus.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 14:33:21 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/08 16:54:20 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long_bonus.h"
#include <mlx.h>

/**
 *
 */
int	img_init(t_game game, t_img imgs[])
{
	int			i;
	static char	*name[] = {"sprites/player_01.xpm", "sprites/wall_01.xpm",
		"sprites/effect_w.xpm", "sprites/door_01.xpm",
		"sprites/door_02.xpm", "sprites/enemy_01.xpm",
		"sprites/enemy_02.xpm", "sprites/player_02.xpm", "sprites/bg.xpm",
		NULL};

	i = 0;
	while (i < 9)
	{
		imgs[i].img = XPM_I(game.mlx, name[i], &imgs[i].w, &imgs[i].h);
		if (imgs[i].img == NULL)
		{
			return (FALSE);
		}
		i++;
	}
	return (TRUE);
}

/**
 *
 */
int	render_map(t_data *d)
{
	int (i), (j);
	update_enemy(d);
	i = -1;
	while (++i < d->map.rows)
	{
		j = -1;
		while (++j < d->map.cols)
			render_map_mini(d, i, j);
	}
	return (put_move_n(d, d->moves, d->map.start_x, d->map.start_y), 0);
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
		while (i < 9)
		{
			mlx_destroy_image(data->game.mlx, data->imgs[i].img);
			i++;
		}
		mlx_destroy_window(data->game.mlx, data->game.mlx_win);
		mlx_destroy_display(data->game.mlx);
		free(data->game.mlx);
		free(data->map.enemies);
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
	int	new_x;
	int	new_y;

	++data->moves;
	new_x = data->map.start_x + x;
	new_y = data->map.start_y + y;
	if (data->map.map[new_y][new_x] == '1')
		return ;
	victory(data, new_x, new_y);
	if (data->map.map[new_y][new_x] == 'C')
		data->map.c_count++;
	if (data->map.map[new_y][new_x] == 'N')
	{
		ft_printf("Game Over!\n");
		exit_window(ESC, data);
	}
	data->map.map[data->map.start_y][data->map.start_x] = '0';
	data->map.start_x = new_x;
	data->map.start_y = new_y;
	data->map.map[new_y][new_x] = 'P';
	fix_door(data, new_x, new_y);
	mlx_clear_window(data->game.mlx, data->game.mlx_win);
	render_map(data);
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
