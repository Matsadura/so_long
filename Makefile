CC = cc
CFLAGS = -Wall -Werror -Wextra
CMLX = -Lmlx_linux -lmlx_Linux -L/usr/lib -Imlx_linux -lXext -lX11 -lm -lz
SRC = ./mandatory/parsing/check_map2_funcs.c ./mandatory/parsing/check_map3_funcs.c ./mandatory/parsing/check_map4_funcs.c ./mandatory/parsing/check_map_funcs.c ./mandatory/parsing/map_func.c ./mandatory/parsing/arr_funcs.c ./mandatory/rendering/render.c ./mandatory/main.c
NAME = so_long
LIBFT = libft/libft.a

OBJECT = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(SRC) $(LIBFT)
	$(CC) $(CFLAGS) $(SRC) $(LIBFT) $(CMLX) -o $(NAME)

$(LIBFT):
	make -C libft

clean:
	rm -f $(OBJECT)
	make -C libft clean

fclean: clean
	rm -f $(NAME)
	make -C libft fclean

re: fclean all

.PHONY: clean fclean all re
