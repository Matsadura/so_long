/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   so_long.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 19:40:38 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/02 20:30:02 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SO_LONG_H
# define SO_LONG_H
# include "libft/libft.h"

# define TRUE 1
# define FALSE 0

# define STDERR 2

/* Map handling functions */
int		check_map_extension(char *file_name);
int		open_map(char *file_name);
int		is_rectangular(char **map);
int		is_closed_map(char **map);
int		is_valid_map_lines(char **map);
char	**read_map(int map_fd);

/* 2D Array Functions */
void	print_2darray(char **arr);
void	free_2darray(char **arr);

#endif /* SO_LONG_H */
