#include "ui.h"

#include <stdio.h>

#define HUD_SCALE 3

void	hud_draw(t_platform *p, const t_renderer *r, const t_game *g)
{
	char	buf[96];
	int		y;

	platform_fill_rect(p, 0, 0, r->view_w, HUD_PX, COLOR_HUD);
	y = (HUD_PX - 7 * HUD_SCALE) / 2;
	snprintf(buf, sizeof(buf), "LEVEL %d/%d", g->level_index + 1,
		g->level_count);
	draw_text(p, buf, 16, y, HUD_SCALE, COLOR_TEXT);
	snprintf(buf, sizeof(buf), "MOVES %d", g->moves);
	draw_text_centered(p, buf, r->view_w / 2, y, HUD_SCALE, COLOR_TEXT);
	snprintf(buf, sizeof(buf), "COINS %d/%d", g->collected,
		g->map.collectibles);
	draw_text(p, buf, r->view_w - 16 - text_width(buf, HUD_SCALE), y,
		HUD_SCALE, (g->collected == g->map.collectibles)
		? COLOR_OK : COLOR_ACCENT);
}

/* Picks a title scale that fits the view width. */
static int	fit_scale(const char *s, int max_w, int preferred)
{
	while (preferred > 1 && text_width(s, preferred) > max_w)
		preferred--;
	return (preferred);
}

static void	draw_banner(t_platform *p, const t_renderer *r, const char *title,
		const char *hint, uint32_t color)
{
	int	cy;
	int	ts;
	int	hs;

	platform_fill_rect(p, 0, HUD_PX, r->view_w, r->view_h - HUD_PX,
		COLOR_DIM);
	ts = fit_scale(title, r->view_w - 32, 7);
	hs = fit_scale(hint, r->view_w - 32, 3);
	cy = HUD_PX + (r->view_h - HUD_PX) / 2;
	draw_text_centered(p, title, r->view_w / 2, cy - 7 * ts, ts, color);
	draw_text_centered(p, hint, r->view_w / 2, cy + 7 * hs, hs, COLOR_TEXT);
}

void	overlay_draw(t_platform *p, const t_renderer *r, const t_game *g)
{
	if (g->state == GAME_LEVEL_CLEAR)
		draw_banner(p, r, "LEVEL CLEAR!", "ENTER: NEXT LEVEL", COLOR_OK);
	else if (g->state == GAME_COMPLETE)
		draw_banner(p, r, "YOU WIN!", "ENTER: PLAY AGAIN", COLOR_ACCENT);
	else if (g->state == GAME_OVER)
		draw_banner(p, r, "GAME OVER", "R / ENTER: RETRY", COLOR_DANGER);
}
