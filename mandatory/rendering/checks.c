/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checks.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 13:58:35 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/07 13:58:37 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long.h"

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
	return (0);
}
