/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_map2_funcs.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 20:30:13 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/02 20:30:15 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"


/**
 *
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
 *
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
 * 
 */
int	is_valid_map_lines(char **map)
{
	int	i;

	i = 0;
	while (map[i] != NULL)
	{
		if (is_valid_line(map[i]) == FALSE)
		{
			ft_dprintf(2, " Error: Map contains invalid characters\n");
			free_2darray(map);
			exit(EXIT_FAILURE);
		}
		i++;
	}
	return (TRUE);
}
