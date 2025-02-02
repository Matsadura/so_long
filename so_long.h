/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   so_long.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 19:40:38 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/02 17:32:33 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SO_LONG_H
# define SO_LONG_H
# include "libft/libft.h"

# define TRUE 1
# define FALSE 1

# define STDERR 2

/* Map handling functions */
int		check_map_extension(char *file_name);
int		open_map(char *file_name);
char	**read_map(int map_fd);

#endif /* SO_LONG_H */
