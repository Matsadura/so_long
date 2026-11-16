#include "ui.h"

#include <stdio.h>

/* Vertical layout helper: lines of centered text stacked from a top y. */
typedef struct s_stack
{
	t_platform			*p;
	const t_renderer	*r;
	int					y;
}	t_stack;

static void	line(t_stack *s, const char *text, int scale, uint32_t color)
{
	scale = text_fit_scale(text, s->r->view_w - 32, scale);
	draw_text_centered(s->p, text, s->r->view_w / 2, s->y, scale, color);
	s->y += 7 * scale + scale * 4;
}

static void	gap(t_stack *s, int px)
{
	s->y += px;
}

static t_stack	dim(t_platform *p, const t_renderer *r, int content_h)
{
	int	area;

	platform_fill_rect(p, 0, HUD_PX, r->view_w,
		r->view_h - HUD_PX - FOOTER_PX, COLOR_DIM);
	area = r->view_h - HUD_PX - FOOTER_PX;
	return ((t_stack){p, r, HUD_PX + (area - content_h) / 2});
}

static bool	blink(const t_game *g)
{
	return ((int)(g->anim_time * 2.0) % 2 == 0);
}

static void	title_screen(t_platform *p, const t_renderer *r, const t_game *g)
{
	t_stack	s;
	char	buf[48];

	s = dim(p, r, 330);
	line(&s, "SO LONG", 12, COLOR_ACCENT);
	line(&s, "A STREET-CROSSING ARCADE GAME", 2, COLOR_MUTED);
	gap(&s, 22);
	line(&s, blink(g) ? "PRESS ENTER TO START" : "", 4, COLOR_TEXT);
	gap(&s, 22);
	line(&s, "ARROWS / WASD  HOP     P  PAUSE", 2, COLOR_TEXT);
	line(&s, "R  RETRY STAGE     ESC  QUIT", 2, COLOR_TEXT);
	gap(&s, 10);
	line(&s, "DODGE TRAFFIC, RIDE THE LOGS, FILL EVERY GOAL", 2,
		COLOR_MUTED);
	if (g->best_score > 0)
	{
		snprintf(buf, sizeof(buf), "BEST %06d", g->best_score);
		gap(&s, 8);
		line(&s, buf, 2, COLOR_OK);
	}
}

/* Non-blocking intro band shown at the start of each stage. */
static void	stage_banner(t_platform *p, const t_renderer *r, const t_game *g)
{
	t_stack	s;
	char	buf[48];
	int		mid;

	mid = HUD_PX + (r->view_h - HUD_PX - FOOTER_PX) / 2;
	platform_fill_rect(p, 0, mid - 52, r->view_w, 104, COLOR_BAND);
	s = (t_stack){p, r, mid - 40};
	snprintf(buf, sizeof(buf), "STAGE %d", g->level_index + 1);
	line(&s, buf, 6, COLOR_ACCENT);
	if (game_is_classic(g))
		line(&s, "GRAB EVERY COIN, THEN REACH THE EXIT", 2, COLOR_TEXT);
	else
	{
		snprintf(buf, sizeof(buf), "FILL ALL %d GOAL SLOT%s", g->map.goals,
			g->map.goals == 1 ? "" : "S");
		line(&s, buf, 2, COLOR_TEXT);
	}
}

static void	death_banner(t_platform *p, const t_renderer *r, const t_game *g)
{
	static const char	*words[] = {"", "SPLAT!", "SPLASH!", "CAUGHT!",
		"TIME UP!"};
	t_stack				s;
	char				buf[32];
	int					mid;

	mid = HUD_PX + (r->view_h - HUD_PX - FOOTER_PX) / 2;
	platform_fill_rect(p, 0, mid - 52, r->view_w, 104, COLOR_BAND);
	s = (t_stack){p, r, mid - 40};
	line(&s, words[g->death], 7, g->death == DEATH_WATER
		? COLOR_WATER : COLOR_DANGER);
	if (g->lives > 1)
		snprintf(buf, sizeof(buf), "%d LIVES LEFT", g->lives - 1);
	else
		snprintf(buf, sizeof(buf), "%s", g->lives == 1 ? "NO LIVES LEFT" : "");
	line(&s, buf, 2, COLOR_TEXT);
}

static void	stage_clear(t_platform *p, const t_renderer *r, const t_game *g)
{
	t_stack	s;
	char	buf[48];

	s = dim(p, r, 200);
	line(&s, "STAGE CLEAR!", 7, COLOR_OK);
	gap(&s, 8);
	if (g->clear_time_bonus > 0)
	{
		snprintf(buf, sizeof(buf), "TIME BONUS   +%d", g->clear_time_bonus);
		line(&s, buf, 3, COLOR_TEXT);
	}
	snprintf(buf, sizeof(buf), "STAGE BONUS  +%d", g->clear_stage_bonus);
	line(&s, buf, 3, COLOR_TEXT);
	gap(&s, 12);
	line(&s, blink(g) ? "ENTER: NEXT STAGE" : "", 3, COLOR_ACCENT);
}

static void	end_screen(t_platform *p, const t_renderer *r, const t_game *g,
		bool won)
{
	t_stack	s;
	char	buf[48];

	s = dim(p, r, 230);
	line(&s, won ? "YOU MADE IT!" : "GAME OVER", 8,
		won ? COLOR_ACCENT : COLOR_DANGER);
	gap(&s, 8);
	snprintf(buf, sizeof(buf), "SCORE %06d", g->score);
	line(&s, buf, 4, COLOR_TEXT);
	if (g->new_best)
		line(&s, "NEW BEST!", 3, COLOR_OK);
	else
	{
		snprintf(buf, sizeof(buf), "BEST %06d", g->best_score);
		line(&s, buf, 3, COLOR_MUTED);
	}
	gap(&s, 14);
	line(&s, blink(g) ? "ENTER / R: PLAY AGAIN" : "", 3, COLOR_TEXT);
}

void	screens_draw(t_platform *p, const t_renderer *r, const t_game *g)
{
	t_stack	s;

	if (g->state == GAME_TITLE)
		title_screen(p, r, g);
	else if (g->state == GAME_PLAYING && g->stage_time < STAGE_BANNER_SECONDS)
		stage_banner(p, r, g);
	else if (g->state == GAME_DYING)
		death_banner(p, r, g);
	else if (g->state == GAME_PAUSED)
	{
		s = dim(p, r, 110);
		line(&s, "PAUSED", 8, COLOR_TEXT);
		line(&s, "P / ENTER: RESUME     R: RETRY STAGE", 2, COLOR_MUTED);
	}
	else if (g->state == GAME_STAGE_CLEAR)
		stage_clear(p, r, g);
	else if (g->state == GAME_OVER || g->state == GAME_VICTORY)
		end_screen(p, r, g, g->state == GAME_VICTORY);
}
