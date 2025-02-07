/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map2_funcs.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 20:30:13 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/04 19:36:59 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long.h"

/**
 * is_valid_key - Checks if the key is parts of the set
 * @c: The key to check
 * @set: The set of correct keys
 * Return: True if correct, False otherwise
 */
static int	is_valid_key(char c, char *set)
{
	int	i;

	i = 0;
	while (set[i] != '\0')
	{
		if (c == set[i])
			return (TRUE);
		i++;
	}
	return (FALSE);
}

/**
 * is_valid_line - Checks if the line is valid
 * @map_line: The line to check
 * Return: True if valid, False otherwise
 */
static int	is_valid_line(char *map_line)
{
	int	i;

	i = 0;
	while (map_line[i] != '\0')
	{
		if (is_valid_key(map_line[i], "ECP10") == FALSE)
			return (FALSE);
		i++;
	}
	return (TRUE);
}

/**
 * is_valid_map_lines - Checks if the lines of the map are valid
 * @map: An array of arrays representing the map
 * Return: True if valid, False otherwise
 */
void	is_valid_map_lines(char **map)
{
	int	i;

	i = 0;
	while (map[i] != NULL)
	{
		if (is_valid_line(map[i]) == FALSE)
		{
			ft_dprintf(2, "Error: Map contains invalid characters\n");
			free_2darray(map);
			exit(EXIT_FAILURE);
		}
		i++;
	}
}

/**
 *
 */
static void	count_elements(char **map, int *p, int *e, int *c)
{
	int	i;
	int	j;

	i = 0;
	while (map[i] != NULL)
	{
		j = 0;
		while (map[i][j] != '\0')
		{
			if (map[i][j] == 'P')
				*p += 1;
			else if (map[i][j] == 'E')
				*e += 1;
			else if (map[i][j] == 'C')
				*c += 1;
			j++;
		}
		i++;
	}
}

/**
 * does_contain_all - Check if 1P, 1E and at least 1C exist
 * @map: An array of arrays representing the map
 * Return: True of correct count of elements exist
 */
void	does_contain_all(char **map)
{
	int	p;
	int	e;
	int	c;

	p = 0;
	e = 0;
	c = 0;
	count_elements(map, &p, &e, &c);
	if (p != 1 || e != 1 || c < 1)
	{
		ft_dprintf(2, "Error: Map contains \
duplicate or unsufficiant elements\n");
		free_2darray(map);
		exit(1);
	}
}
