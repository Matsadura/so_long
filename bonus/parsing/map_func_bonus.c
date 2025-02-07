/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_func_bonus.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 20:29:33 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/07 14:36:05 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../so_long_bonus.h"

/**
 * check_map_extension - Checks the extension of the file
 * @file_name: The map's file name
 * Return: 1 if file name is valid otherwise 0
 */
int	check_map_extension(char *file_name)
{
	if (file_name == NULL)
		return (TRUE);
	if (ft_strlen(file_name) < 5)
		return (TRUE);
	while (ft_strlen(file_name) > 4)
		file_name++;
	if (ft_strncmp(file_name, ".ber", 5) == 0)
		return (TRUE);
	return (FALSE);
}

/**
 * open_map - Opens a file map
 * @file_name: The map's name
 * Return: The map's file descriptor
 */
int	open_map(char *file_name)
{
	int	fd;

	if (file_name == NULL)
		return (-1);
	if (check_map_extension(file_name) == FALSE)
	{
		ft_dprintf(STDERR, "%s: is not a valid .ber file\n", file_name);
		exit(EXIT_FAILURE);
	}
	fd = open(file_name, O_RDONLY);
	if (fd < 0)
	{
		perror(file_name);
		exit(EXIT_FAILURE);
	}
	return (fd);
}

/**
 *
 */
static void	free_and_exit(char *line, int map_fd)
{
	while (line != NULL)
	{
		free(line);
		line = get_next_line(map_fd);
	}
	ft_dprintf(2, "Error: Invalid map\n");
	exit(1);
}

/**
 * read_map - Reads a map file
 * @map_fd: The map's name
 * Return: An array of arrays representing the map
 */
char	**read_map(int map_fd)
{
	char	*line;
	char	*tmp;
	char	**map;

	if (map_fd < 0)
		return (NULL);
	tmp = NULL;
	line = get_next_line(map_fd);
	while (line != NULL)
	{
		if (line[0] == '\n')
		{
			free(tmp);
			free_and_exit(line, map_fd);
		}
		tmp = ft_strjoin_gnl(tmp, line);
		free(line);
		line = get_next_line(map_fd);
		if (tmp == NULL)
			return (NULL);
	}
	if (tmp == NULL)
		return (NULL);
	map = ft_split(tmp, '\n');
	return (free(tmp), map);
}
