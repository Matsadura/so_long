/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checks2_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 17:00:01 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/08 17:02:35 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long_bonus.h"

void	put_n_img(t_data *d, t_img imgs[], int j, int i)
{
	static int	a;
	int			x;
	int			y;

	x = j;
	y = i;
	++a;
	if (a % 600 < 300)
		PUT_I(d->game.mlx, d->game.mlx_win, imgs[EN1].img, x, y);
	else
		PUT_I(d->game.mlx, d->game.mlx_win, imgs[EN2].img, x, y);
}
