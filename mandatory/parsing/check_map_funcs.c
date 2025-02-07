/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map_funcs.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 18:09:10 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/04 19:37:08 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long.h"

/**
 * is_rectangle - Checks if the map is rectangular
 * @map: An array of arrays represanting the map
 * Return: True if rectangular, False otherwise
 */
int	is_rectangular(char **map)
{
	size_t	prev_len;
	int		i;

	if (map == NULL)
	{
		ft_dprintf(2, "Error: Map is invalid\n");
		exit(EXIT_FAILURE);
	}
	i = 0;
	if (map[i] == NULL)
	{
		ft_dprintf(2, "Error: Map is invalid\n");
		free(map);
		exit(EXIT_FAILURE);
	}
	prev_len = ft_strlen(map[i]);
	while (map[i] != NULL)
	{
		if (prev_len != ft_strlen(map[i]))
			return (FALSE);
		i++;
	}
	return (TRUE);
}

/**
 * check_first_and_last_row - Checks for and last row if all walled
 * @map: An array of arrays representing a map
 * @rowN: Pointer to before last row
 * Return: True if walled, Falsed otherwise
 */
static int	check_first_and_last_row(char **map, int *rowN)
{
	int	i;
	int	j;

	i = 0;
	j = 0;
	while (map[i][j] != '\0')
	{
		if (map[i][j] != '1')
			return (FALSE);
		j++;
	}
	while (map[i] != NULL)
		i++;
	i--;
	*rowN = i;
	j = 0;
	while (map[i][j] != '\0')
	{
		if (map[i][j] != '1')
			return (FALSE);
		j++;
	}
	return (TRUE);
}

/**
 * check_middle_rows - Checks if middle rows are starting
 *	and ending with a wall
 * @map: An array of arrays representing a map
 * @rowN: Pointer to before last row
 * Return: True if walled, Falsed otherwise
 */
static int	check_middle_rows(char **map, int rowN)
{
	int	i;

	if (rowN > 1)
		rowN--;
	i = 1;
	while (i <= rowN)
	{
		if (map[i][0] != '1' || map[i][ft_strlen(map[i]) - 1] != '1')
			return (FALSE);
		i++;
	}
	return (TRUE);
}

/**
 * is_closed_map - Checks if the map is surrounded by walls
 * @map: An array of arrays represanting the map
 * Return: True if surrounded, False otherwise
 */
void	is_closed_map(char **map)
{
	int	row;

	if (map == NULL || *map == NULL)
	{
		ft_dprintf(2, "Error\n");
		exit(1);
	}
	if (check_first_and_last_row(map, &row) == FALSE)
	{
		ft_dprintf(2, "Error: Map is not fully surrounded by walls\n");
		free_2darray(map);
		exit(1);
	}
	if (check_middle_rows(map, row) == FALSE)
	{
		ft_dprintf(2, "Error: Map edges are not fully surrounded by walls\n");
		free_2darray(map);
		exit(1);
	}
}
