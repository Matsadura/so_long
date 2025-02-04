/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map4_funcs.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 18:23:08 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/04 19:09:06 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/**
 * is_exit_reachable - Checks if there is a path to the EXIT
 * @map: An array of arrays representing the map
 * Return: True if reachable, False otherwise
 */
int	is_exit_reachable(char **map)
{
	int	i;
	int	j;

	i = 0;
	while (map[i] != NULL)
	{
		j = 0;
		while (map[i][j] != '\0')
		{
			if (map[i][j] == 'E')
				return (FALSE);
			j++;
		}
		i++;
	}
	return (TRUE);
}

/**
 *
 */
void	is_map_fully_valid(char **map)
{
	if (is_rectangular(map) == FALSE)
	{
		ft_dprintf(2, "Error: Map is not rectangular");
		free_2darray(map);
	}
	is_closed_map(map);
	is_valid_map_lines(map);
	does_contain_all(map);
	check_valid_paths(map);
}
