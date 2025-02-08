/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map4_funcs.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 18:23:08 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/07 09:24:55 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long.h"

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
 * is_map_fully_valid - Checks if all map conditions are true
 * @map: An array of arrays representing the map
 */
void	is_map_fully_valid(char **map)
{
	if (is_rectangular(map) == FALSE)
	{
		ft_dprintf(STDERR, "Error: Map is not rectangular\n");
		free_2darray(map);
		exit(1);
	}
	is_closed_map(map);
	is_valid_map_lines(map);
	does_contain_all(map);
	check_valid_paths(map);
}

/**
 *
 */
t_map	read_and_check_map(char *file_name)
{
	int		fd;
	t_map	map_info;

	fd = open_map(file_name);
	map_info.map = read_map(fd);
	is_map_fully_valid(map_info.map);
	find_coordinates(map_info.map, &map_info, 'P');
	find_coordinates(map_info.map, &map_info, 'E');
	if (map_info.rows > 21 || map_info.cols > 40)
	{
		free_2darray(map_info.map);
		ft_dprintf(STDERR, "Error: Map is too large\n");
		exit(1);
	}
	map_info.collects = count_collectibles(map_info.map);
	map_info.c_count = 0;
	return (map_info);
}
