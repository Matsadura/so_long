#include "ui.h"

#include <stdio.h>

#define HUD_SCALE    3
#define FOOTER_SCALE 2
#define HEART_PX     30

static void	draw_lives(t_platform *p, const t_renderer *r, const t_game *g)
{
	int	i;
	int	x;

	x = r->view_w - 12 - START_LIVES * HEART_PX;
	i = 0;
	while (i < START_LIVES)
	{
		platform_draw_ex(p, r->sprites[SPR_HEART],
			(const int [4]){x + i * HEART_PX, (HUD_PX - HEART_PX) / 2 + 1,
			HEART_PX, HEART_PX},
			i < g->lives ? COLOR_WHITE : 0x3A404CFFu, false);
		i++;
	}
}

static void	draw_top_bar(t_platform *p, const t_renderer *r, const t_game *g)
{
	char	buf[64];
	int		y;

	platform_fill_rect(p, 0, 0, r->view_w, HUD_PX, COLOR_HUD);
	y = (HUD_PX - 7 * HUD_SCALE) / 2;
	snprintf(buf, sizeof(buf), "STAGE %d/%d", g->level_index + 1,
		g->level_count);
	draw_text(p, buf, 16, y, HUD_SCALE, COLOR_TEXT);
	snprintf(buf, sizeof(buf), "SCORE %06d", g->score);
	draw_text_centered(p, buf, r->view_w * 41 / 100, y, HUD_SCALE,
		COLOR_ACCENT);
	snprintf(buf, sizeof(buf), "MOVES %d", g->moves);
	draw_text_centered(p, buf, r->view_w * 69 / 100, y, HUD_SCALE,
		COLOR_TEXT);
	draw_lives(p, r, g);
}

static void	draw_timer(t_platform *p, const t_renderer *r, const t_game *g,
		int top)
{
	int			bar_x;
	int			bar_w;
	int			fill;
	double		ratio;
	uint32_t	color;

	draw_text(p, "TIME", 16, top + (FOOTER_PX - 14) / 2, FOOTER_SCALE,
		COLOR_MUTED);
	bar_x = 16 + text_width("TIME", FOOTER_SCALE) + 12;
	bar_w = r->view_w * 42 / 100 - bar_x;
	ratio = (g->time_limit > 0.0) ? g->time_left / g->time_limit : 0.0;
	fill = (int)(bar_w * ratio);
	color = COLOR_OK;
	if (ratio < 0.5)
		color = COLOR_ACCENT;
	if (ratio < 0.2)
		color = COLOR_DANGER;
	platform_fill_rect(p, bar_x, top + 8, bar_w, FOOTER_PX - 16, 0x262C38FFu);
	platform_fill_rect(p, bar_x, top + 8, fill, FOOTER_PX - 16, color);
}

static void	draw_footer(t_platform *p, const t_renderer *r, const t_game *g)
{
	char	buf[96];
	int		top;
	int		ty;
	int		n;

	top = r->view_h - FOOTER_PX;
	platform_fill_rect(p, 0, top, r->view_w, FOOTER_PX, COLOR_HUD);
	draw_timer(p, r, g, top);
	ty = top + (FOOTER_PX - 7 * FOOTER_SCALE) / 2;
	if (game_is_classic(g))
		n = snprintf(buf, sizeof(buf), "COINS %d/%d", g->collected,
				g->map.collectibles);
	else
		n = snprintf(buf, sizeof(buf), "GOALS %d/%d   COINS %d",
				g->goals_filled, g->map.goals, g->collected);
	if (g->combo > 1 && n > 0 && n < (int)sizeof(buf))
		snprintf(buf + n, sizeof(buf) - n, "   COMBO X%d", g->combo);
	draw_text(p, buf, r->view_w - 16 - text_width(buf, FOOTER_SCALE), ty,
		FOOTER_SCALE, g->combo > 1 ? COLOR_ACCENT : COLOR_TEXT);
}

void	hud_draw(t_platform *p, const t_renderer *r, const t_game *g)
{
	draw_top_bar(p, r, g);
	draw_footer(p, r, g);
}
