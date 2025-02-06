/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   so_long.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 19:40:38 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/04 19:40:02 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SO_LONG_H
# define SO_LONG_H
# include "libft/libft.h"

# define TRUE 1
# define FALSE 0

# define STDERR 2

typedef struct s_map
{
	int	start_x;
	int	start_y;
	int	rows;
	int	cols;
	int	collects;
	int	c_count;
	char	**map;
}	t_map;

/* Map handling functions */
int		check_map_extension(char *file_name);
int		open_map(char *file_name);
int		is_rectangular(char **map);
void	is_closed_map(char **map);
void	is_valid_map_lines(char **map);
void	does_contain_all(char **map);
int		is_exit_reachable(char **map);
char	**read_map(int map_fd);
void	check_valid_paths(char **map);
void	is_map_fully_valid(char **map);
void	find_coordinates(char **map, t_map *info);
int		count_collectibles(char **map);
t_map	read_and_check_map(char *file_name);

/* 2D Array Functions */
void	print_2darray(char **arr);
void	free_2darray(char **arr);

#endif /* SO_LONG_H */
