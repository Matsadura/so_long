/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map5_funcs_bonus.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 17:02:59 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/08 17:15:10 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long_bonus.h"

static void	count_enemies(t_map *map, int *count)
{
	int (i), (j);
	i = 0;
	while (i < map->rows)
	{
		j = 0;
		while (j < map->cols)
		{
			if (map->map[i][j] == 'N')
				*count += 1;
			j++;
		}
		i++;
	}
}

void	get_enemies(t_map *map)
{
	int (i), (j), (count);
	i = 0;
	count = 0;
	count_enemies(map, &count);
	map->enemy_count = count;
	map->enemies = malloc(count * sizeof(t_enemy));
	count = 0;
	i = 0;
	while (i < map->rows)
	{
		j = 0;
		while (j < map->cols)
		{
			if (map->map[i][j] == 'N')
			{
				map->enemies[count].x = j;
				map->enemies[count].y = i;
				map->enemies[count].direction = 1;
				count++;
			}
			j++;
		}
		i++;
	}
}
