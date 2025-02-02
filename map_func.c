/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_func.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 20:29:33 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/02 17:32:07 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft/get_next_line_bonus.h"
#include "so_long.h"

/**
 * check_map_extension - Checks the extension of the file
 * @file_name: The map's file name
 * Return: 1 if file name is valid otherwise 0
 */
int	check_map_extension(char *file_name)
{
	if (file_name == NULL)
		return (0);
	if (ft_strlen(file_name) < 5)
		return (0);
	while (ft_strlen(file_name) > 4)
		file_name++;
	if (ft_strncmp(file_name, ".ber", 5) == 0)
		return (1);
	return (0);
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
	if (check_map_extension(file_name) == 0)
	{
		ft_dprintf(STDERR, "%s: is not a valid .ber file\n", file_name);
		exit(1);
	}
	fd = open(file_name, O_RDONLY);
	if (fd < 0)
	{
		perror(file_name);
		exit(1);
	}
	return (fd);
}

/**
 * read_map - Reads a map file
 * @file_name: The map's name
 * Return: An array of arrays representing the map
 */
char	**read_map(int map_fd)
{
	char	*line;
	char	*tmp;
	char	**map;

	if (map_fd < 0)
		return (NULL);
	line = get_next_line(map_fd);
	tmp = ft_strjoin_gnl(line, "");
	while (line != NULL)
	{
		line = get_next_line(map_fd);
		tmp = ft_strjoin_gnl(tmp, line);
		free(line);
	}
	map = ft_split(tmp, '\n');
	free(line);
	free(tmp);
	return (map);
}
