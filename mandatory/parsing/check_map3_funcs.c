/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map3_funcs.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 17:42:52 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/07 09:23:29 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long.h"

/**
 * count_collectibles - Counts the 'C' in the map
 * @map: An array of arrays representing the map
 * Return: Number of collectibles
 */
int	count_collectibles(char **map)
{
	int	i;
	int	j;
	int	count;

	count = 0;
	i = 0;
	while (map[i] != NULL)
	{
		j = 0;
		while (map[i][j] != '\0')
		{
			if (map[i][j] == 'C')
				count++;
			j++;
		}
		i++;
	}
	return (count);
}

/**
 * find_coordinates - Finds the coordnates of START position
 * 		as well as setting the Height and Width
 * @map: An array of arrays representing the map
 * @info: A map structure
 */
void	find_coordinates(char **map, t_map *info, char c)
{
	int (i), (j);
	info->rows = 0;
	while (map[info->rows] != NULL)
		info->rows++;
	info->cols = ft_strlen(map[0]);
	i = -1;
	while (map[++i] != NULL)
	{
		j = -1;
		while (map[i][++j] != '\0')
		{
			if (map[i][j] == c && c == 'P')
			{
				info->start_x = j;
				info->start_y = i;
				break ;
			}
			else if (map[i][j] == c && c == 'E')
			{
				info->exit_x = j;
				info->exit_y = i;
				break ;
			}
		}
	}
}

/**
 * duplicate_map - Duplicates the map
 * @map: An array of arrays representing the map
 * Return: The new map
 */
static char	**duplicate_map(char **map)
{
	int		i;
	int		j;
	char	**map_dup;

	i = 0;
	while (map[i] != NULL)
		i++;
	map_dup = malloc(sizeof(char **) * i + sizeof(NULL));
	if (map_dup == NULL)
	{
		free_2darray(map);
		ft_dprintf(2, "Error: Map Flood filling failed\n");
		exit(1);
	}
	j = 0;
	while (j < i)
	{
		map_dup[j] = ft_strdup(map[j]);
		j++;
	}
	map_dup[j] = NULL;
	return (map_dup);
}

/**
 * dfs - Traverses the map filling all reachable position
 *	with a marker
 * @map: An array of arrays representing the map
 * @x: The x coordinates of the START position
 * @y: The y coordinates of the START position
 * @map_info: A map structure contains information
 */
static void	dfs(char **map, int x, int y, t_map *map_info)
{
	if (x <= 0 || x >= map_info->cols
		|| y <= 0 || y >= map_info->rows
		|| map[y][x] == '1' || map[y][x] == '#')
		return ;
	if (map[y][x] == 'C')
		map_info->c_count += 1;
	map[y][x] = '#';
	dfs(map, x, y - 1, map_info);
	dfs(map, x, y + 1, map_info);
	dfs(map, x - 1, y, map_info);
	dfs(map, x + 1, y, map_info);
}

/**
 * check_valid_path - Checks if all the collectibles and the exit
 *	are reachable
 * @map: An array of arrays representing the map
 */
void	check_valid_paths(char **map)
{
	t_map	map_info;
	char	**dup_map;

	dup_map = duplicate_map(map);
	find_coordinates(dup_map, &map_info, 'P');
	map_info.collects = count_collectibles(dup_map);
	map_info.c_count = 0;
	dfs(dup_map, map_info.start_x, map_info.start_y, &map_info);
	if (map_info.collects != map_info.c_count)
	{
		ft_dprintf(2, "Error: Not all collectables are reachable\n");
		free_2darray(map);
		free_2darray(dup_map);
		exit(1);
	}
	if (is_exit_reachable(dup_map) == FALSE)
	{
		ft_dprintf(2, "Error: Exit is not reachable\n");
		free_2darray(dup_map);
		free_2darray(map);
		exit(1);
	}
	free_2darray(dup_map);
}
