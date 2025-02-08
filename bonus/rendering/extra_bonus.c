/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extra.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 13:13:48 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/08 16:56:25 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long_bonus.h"

/**
 *
 */
void	move_enemy(t_data *d, t_enemy *enemy)
{
	int	new_x;

	new_x = enemy->x + enemy->direction;
	if (new_x < 0 || new_x >= d->map.cols || d->map.map[enemy->y][new_x] == '1'
		|| d->map.map[enemy->y][new_x] == 'E'
		|| d->map.map[enemy->y][new_x] == 'C'
		|| d->map.map[enemy->y][new_x] == 'N')
	{
		enemy->direction *= -1;
		return ;
	}
	if (enemy->y == d->map.start_y && new_x == d->map.start_x)
	{
		ft_printf("Game Over!\n");
		exit_window(ESC, d);
	}
	d->map.map[enemy->y][enemy->x] = '0';
	d->map.map[enemy->y][new_x] = 'N';
	enemy->x = new_x;
}

void	update_enemy(t_data *d)
{
	static int	frame;
	int			i;

	frame = (frame + 1) % 300;
	if (frame != 0)
		return ;
	i = 0;
	while (i < d->map.enemy_count)
	{
		move_enemy(d, &d->map.enemies[i]);
		i++;
	}
}

/**
 *
 */
void	render_map_mini(t_data *d, int i, int j)
{
	int (n), (m);
	n = j * 64;
	m = i * 64;
	if (d->map.map[i][j] == '1')
		PUT_I(d->game.mlx, d->game.mlx_win, d->imgs[WALL].img, n, m);
	else if (j == d->map.start_x && i == d->map.start_y)
		put_p_img(d, d->imgs, j, i);
	else if (d->map.map[i][j] == 'E' && d->map.c_count == d->map.collects)
		PUT_I(d->game.mlx, d->game.mlx_win, d->imgs[EXIT2].img, n, m);
	else if (d->map.map[i][j] == 'E')
		PUT_I(d->game.mlx, d->game.mlx_win, d->imgs[EXIT1].img, n, m);
	else if (d->map.map[i][j] == 'C')
		PUT_I(d->game.mlx, d->game.mlx_win, d->imgs[C].img, n, m);
	else if (d->map.map[i][j] == '0')
		PUT_I(d->game.mlx, d->game.mlx_win, d->imgs[BG].img, n, m);
	else if (d->map.map[i][j] == 'N')
		put_n_img(d, d->imgs, n, m);
}
