CC = cc
CFLAGS = -Wall -Werror -Wextra
CMLX = -Lmlx_linux -lmlx_Linux -L/usr/lib -Imlx_linux -lXext -lX11 -lm -lz
SRC = ./mandatory/parsing/check_map2_funcs.c ./mandatory/parsing/check_map3_funcs.c ./mandatory/parsing/check_map4_funcs.c ./mandatory/parsing/check_map_funcs.c ./mandatory/parsing/map_func.c ./mandatory/parsing/arr_funcs.c ./mandatory/rendering/render.c ./mandatory/main.c ./mandatory/rendering/checks.c
SRC_BONUS = ./bonus/parsing/arr_funcs_bonus.c ./bonus/parsing/check_map_funcs_bonus.c ./bonus/parsing/check_map3_funcs_bonus.c ./bonus/parsing/check_map4_funcs_bonus.c ./bonus/parsing/map_func_bonus.c ./bonus/parsing/check_map2_funcs_bonus.c ./bonus/rendering/checks_bonus.c ./bonus/rendering/render_bonus.c ./bonus/main_bonus.c
NAME = so_long
LIBFT = libft/libft.a
OBJECT = $(SRC:.c=.o)
OBJECT = $(SRC_BONUS:.c=.o)

all: $(NAME)

$(NAME): $(SRC) $(LIBFT)
	$(CC) $(CFLAGS) $(SRC) $(LIBFT) $(CMLX) -o $(NAME)

$(LIBFT):
	make -C libft

bonus: $(SRC_BONUS) $(LIBFT)
	$(CC) $(CFLAGS) $(SRC_BONUS) $(LIBFT) $(CMLX) -o $(NAME)


clean:
	rm -f $(OBJECT)
	rm -f $(OBJECT_BONUS)
	make -C libft clean

fclean: clean
	rm -f $(NAME)
	make -C libft fclean

re: fclean all

.PHONY: clean fclean all re bonus
