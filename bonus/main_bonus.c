/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 09:59:16 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/07 14:37:12 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long_bonus.h"
#include <mlx.h>

int	main(int ac, char **av)
{
	t_img	imgs[5];
	t_data	data;

	if (ac == 2)
	{
		data.map = read_and_check_map(av[1]);
		data.game.mlx = mlx_init();
		if (data.game.mlx == NULL)
			return (EXIT_FAILURE);
		data.game.mlx_win = mlx_new_window(data.game.mlx, \
				64 * data.map.cols, 64 * data.map.rows, "so_long");
		if (data.game.mlx_win == NULL)
			return (EXIT_FAILURE);
		data.imgs = imgs;
		if (img_init(data.game, data.imgs) == FALSE)
			return (EXIT_FAILURE);
		render_map(data.game, data.imgs, data.map);
		mlx_key_hook(data.game.mlx_win, on_keypress, &data);
		mlx_hook(data.game.mlx_win, 17, 1L << 17, x_mark, &data);
		mlx_loop(data.game.mlx);
	}
	return (0);
}
