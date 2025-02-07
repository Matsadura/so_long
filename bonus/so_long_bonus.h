/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   so_long_bonus.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 19:40:38 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/07 15:25:25 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SO_LONG_BONUS_H
# define SO_LONG_BONUS_H
# include "../libft/libft.h"
# include <mlx.h>

/* Bools */
# define TRUE 1
# define FALSE 0

/* FD */
# define STDERR 2

/* Sprites */
# define PLAYER 0
# define WALL 1
# define C 2
# define EXIT1 3
# define EXIT2 4
# define EN1 5
# define EN2 6
# define PLAYER2 7

# define XPM_I mlx_xpm_file_to_image
# define PUT_I mlx_put_image_to_window

/* Keycode Macros*/
# define ESC 65307
# define UP 65362
# define DOWN 65364
# define RIGHT 65363
# define LEFT 65361

# define BLACK 0x000000

typedef struct s_map
{
	int		start_x;
	int		start_y;
	int		exit_x;
	int		exit_y;
	int		rows;
	int		cols;
	int		collects;
	int		c_count;
	char	**map;
}	t_map;

typedef struct s_img
{
	void	*img;
	char	addr;
	int		bits_per_pixed;
	int		line_length;
	int		endian;
	int		h;
	int		w;
}	t_img;

typedef struct s_game
{
	void	*mlx;
	void	*mlx_win;
}	t_game;

typedef struct s_data
{
	t_game	game;
	t_img	*imgs;
	t_map	map;
}	t_data;

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
void	find_coordinates(char **map, t_map *info, char c);
int		count_collectibles(char **map);
t_map	read_and_check_map(char *file_name);
int		img_init(t_game game, t_img imgs[]);
void	render_map(t_game game, t_img imgs[], t_map map);

/* Game Logic */
int		on_keypress(int key, t_data *data);
int		exit_window(int keycode, t_data *data);
void	fix_door(t_data *data, int new_x, int new_y);
void	victory(t_data *data, int new_x, int new_y);
int		x_mark(t_data *data);
void	put_move_n(t_data *data, int moves, int new_x, int new_y);

/* 2D Array Functions */
void	print_2darray(char **arr);
void	free_2darray(char **arr);

#endif /* SO_LONG_BONUS_H */
