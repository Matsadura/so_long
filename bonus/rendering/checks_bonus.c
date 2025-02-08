/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checks_bonus.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 13:58:35 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/08 17:01:04 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long_bonus.h"

/**
 *
 */
void	victory(t_data *data, int new_x, int new_y)
{
	char	e;

	e = data->map.map[new_y][new_x];
	if (e == 'E' && data->map.collects == data->map.c_count)
	{
		ft_printf("Victory !\n");
		exit_window(ESC, data);
	}
}

/**
 *
 */
void	fix_door(t_data *data, int new_x, int new_y)
{
	char	e;

	e = data->map.map[new_y][new_x];
	if (e != data->map.map[data->map.exit_y][data->map.exit_x])
		data->map.map[data->map.exit_y][data->map.exit_x] = 'E';
}

/**
 *
 */
int	x_mark(t_data *data)
{
	int	i;

	mlx_clear_window(data->game.mlx, data->game.mlx_win);
	i = 0;
	while (i < 8)
	{
		mlx_destroy_image(data->game.mlx, data->imgs[i].img);
		i++;
	}
	mlx_destroy_window(data->game.mlx, data->game.mlx_win);
	mlx_destroy_display(data->game.mlx);
	free(data->game.mlx);
	free_2darray(data->map.map);
	exit(0);
	return (0);
}

/**
 *
 */
void	put_move_n(t_data *data, int moves, int new_x, int new_y)
{
	char	buff[127];
	int		x;
	int		y;

	x = new_x * 64 + 28;
	y = new_y * 64 + 40;
	ft_memset(buff, 0, 127);
	mlx_string_put(data->game.mlx, data->game.mlx_win, x, y, BLACK,
		ft_itoa_buff(buff, moves));
}

/**
 *
 */
void	put_p_img(t_data *d, t_img imgs[], int j, int i)
{
	static int	f;
	int			x;
	int			y;

	x = j * 64;
	y = i * 64;
	++f;
	if (f % 600 < 300)
		PUT_I(d->game.mlx, d->game.mlx_win, imgs[PLAYER].img, x, y);
	else
		PUT_I(d->game.mlx, d->game.mlx_win, imgs[PLAYER2].img, x, y);
}
