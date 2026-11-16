#include "map.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static t_map_error	map_extract_entities(t_map *map);

/* "dir/name.ber" -> true; ".ber", "x.ber.txt", "a.b" -> false. */
static bool	has_ber_extension(const char *path)
{
	const char	*base;
	size_t		len;

	base = strrchr(path, '/');
	base = (base != NULL) ? base + 1 : path;
	len = strlen(base);
	return (len > 4 && strcmp(base + len - 4, ".ber") == 0);
}

/* Reads the whole file into a heap buffer. The fd is always closed. */
static t_map_error	read_file(const char *path, char **out_buf, int *out_len)
{
	char	*buf;
	int		fd;
	int		len;
	ssize_t	r;

	*out_buf = NULL;
	*out_len = 0;
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (MAP_ERR_OPEN);
	buf = malloc(MAP_MAX_FILE_BYTES + 1);
	if (buf == NULL)
	{
		close(fd);
		return (MAP_ERR_ALLOC);
	}
	len = 0;
	r = 1;
	while (len <= MAP_MAX_FILE_BYTES && r > 0)
	{
		r = read(fd, buf + len, MAP_MAX_FILE_BYTES + 1 - len);
		if (r > 0)
			len += (int)r;
	}
	close(fd);
	if (r < 0 || len > MAP_MAX_FILE_BYTES)
	{
		free(buf);
		return (r < 0 ? MAP_ERR_READ : MAP_ERR_FILE_TOO_BIG);
	}
	*out_buf = buf;
	*out_len = len;
	return (MAP_OK);
}

t_map_error	map_load(const char *path, t_map *out)
{
	t_map_error	err;
	char		*buf;
	int			len;

	memset(out, 0, sizeof(*out));
	if (path == NULL || !has_ber_extension(path))
		return (MAP_ERR_EXTENSION);
	err = read_file(path, &buf, &len);
	if (err != MAP_OK)
		return (err);
	err = map_parse(buf, len, out);
	free(buf);
	return (err);
}

/*
** First pass: measure the grid. Accepts LF or CRLF line endings and any
** number of trailing newlines; rejects blank lines inside the map.
*/
static t_map_error	measure(const char *text, int len, int *cols, int *rows)
{
	int	i;
	int	start;
	int	end;

	*cols = -1;
	*rows = 0;
	i = 0;
	while (i <= len)
	{
		start = i;
		while (i < len && text[i] != '\n')
			i++;
		end = i;
		if (end > start && text[end - 1] == '\r')
			end--;
		if (end == start)
			return (MAP_ERR_EMPTY_LINE);
		if (end - start > MAP_MAX_COLS || *rows >= MAP_MAX_ROWS)
			return (MAP_ERR_TOO_LARGE);
		if (*cols < 0)
			*cols = end - start;
		else if (end - start != *cols)
			return (MAP_ERR_NOT_RECTANGULAR);
		(*rows)++;
		i++;
	}
	if (*rows < 3 || *cols < 3)
		return (MAP_ERR_TOO_SMALL);
	return (MAP_OK);
}

static void	copy_rows(const char *text, char *tiles, int cols, int rows)
{
	int	y;

	y = 0;
	while (y < rows)
	{
		memcpy(tiles + y * cols, text, cols);
		text += cols;
		if (y + 1 < rows)
		{
			if (*text == '\r')
				text++;
			text++;
		}
		y++;
	}
}

t_map_error	map_parse(const char *text, int len, t_map *out)
{
	t_map_error	err;
	int			cols;
	int			rows;

	memset(out, 0, sizeof(*out));
	while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r'))
		len--;
	if (len == 0)
		return (MAP_ERR_EMPTY);
	err = measure(text, len, &cols, &rows);
	if (err != MAP_OK)
		return (err);
	out->tiles = malloc((size_t)cols * rows);
	if (out->tiles == NULL)
		return (MAP_ERR_ALLOC);
	out->cols = cols;
	out->rows = rows;
	copy_rows(text, out->tiles, cols, rows);
	err = map_validate_layout(out->tiles, cols, rows);
	if (err == MAP_OK)
		err = map_extract_entities(out);
	if (err == MAP_OK)
		err = map_check_reachability(out);
	if (err != MAP_OK)
		map_free(out);
	return (err);
}

/* Records player/enemy/exit positions and turns P/N tiles into floor. */
static t_map_error	map_extract_entities(t_map *map)
{
	int		i;
	char	*t;

	i = 0;
	while (i < map->cols * map->rows)
	{
		t = &map->tiles[i];
		if (*t == TILE_PLAYER)
			map->player_start = (t_point){i % map->cols, i / map->cols};
		else if (*t == TILE_EXIT)
			map->exit = (t_point){i % map->cols, i / map->cols};
		else if (*t == TILE_COLLECTIBLE)
			map->collectibles++;
		else if (*t == TILE_ENEMY)
			map->enemy_start[map->enemy_count++]
				= (t_point){i % map->cols, i / map->cols};
		if (*t == TILE_PLAYER || *t == TILE_ENEMY)
			*t = TILE_FLOOR;
		i++;
	}
	return (MAP_OK);
}

void	map_free(t_map *map)
{
	free(map->tiles);
	memset(map, 0, sizeof(*map));
}

char	map_tile_at(const char *tiles, int cols, int rows, int x, int y)
{
	if (tiles == NULL || x < 0 || y < 0 || x >= cols || y >= rows)
		return (TILE_WALL);
	return (tiles[y * cols + x]);
}

const char	*map_error_str(t_map_error err)
{
	static const char	*msg[] = {
	[MAP_OK] = "OK",
	[MAP_ERR_EXTENSION] = "map file must have a .ber extension",
	[MAP_ERR_OPEN] = "cannot open map file",
	[MAP_ERR_READ] = "cannot read map file",
	[MAP_ERR_FILE_TOO_BIG] = "map file is too big",
	[MAP_ERR_EMPTY] = "map is empty",
	[MAP_ERR_EMPTY_LINE] = "map contains an empty line",
	[MAP_ERR_NOT_RECTANGULAR] = "map is not rectangular",
	[MAP_ERR_TOO_LARGE] = "map is too large (max 64x40)",
	[MAP_ERR_TOO_SMALL] = "map is too small (min 3x3)",
	[MAP_ERR_INVALID_CHAR] = "map contains invalid characters",
	[MAP_ERR_NOT_CLOSED] = "map is not surrounded by walls",
	[MAP_ERR_PLAYER_COUNT] = "map must contain exactly one player (P)",
	[MAP_ERR_EXIT_COUNT] = "map must contain exactly one exit (E)",
	[MAP_ERR_NO_COLLECTIBLE] = "map must contain at least one collectible (C)",
	[MAP_ERR_TOO_MANY_ENEMIES] = "map contains too many enemies (max 128)",
	[MAP_ERR_UNREACHABLE_COLLECTIBLE] = "not all collectibles are reachable",
	[MAP_ERR_UNREACHABLE_EXIT] = "exit is not reachable",
	[MAP_ERR_ALLOC] = "out of memory",
	};

	if ((int)err < 0 || (size_t)err >= sizeof(msg) / sizeof(msg[0])
		|| msg[err] == NULL)
		return ("unknown map error");
	return (msg[err]);
}
