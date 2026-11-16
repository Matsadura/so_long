#include "game.h"

#define POPUP_SECONDS 1.0f
#define POPUP_RISE    0.9f

void	score_add(t_game *g, int points, float x, float y)
{
	t_popup	*slot;
	int		i;

	g->score += points;
	slot = &g->popups[0];
	i = 0;
	while (i < MAX_POPUPS)
	{
		if (g->popups[i].ttl < slot->ttl)
			slot = &g->popups[i];
		i++;
	}
	*slot = (t_popup){x, y, POPUP_SECONDS, points};
}

/* Coins picked up in quick succession build a multiplier (x2 .. x5). */
void	score_coin(t_game *g, float x, float y)
{
	if (g->combo_timer > 0.0 && g->combo < COMBO_MAX)
		g->combo++;
	else if (g->combo_timer <= 0.0)
		g->combo = 1;
	g->combo_timer = COMBO_WINDOW;
	score_add(g, SCORE_COIN * g->combo, x, y);
}

void	score_update(t_game *g, double dt)
{
	int	i;

	if (g->combo_timer > 0.0)
	{
		g->combo_timer -= dt;
		if (g->combo_timer <= 0.0)
		{
			g->combo_timer = 0.0;
			g->combo = 0;
		}
	}
	i = 0;
	while (i < MAX_POPUPS)
	{
		if (g->popups[i].ttl > 0.0f)
		{
			g->popups[i].ttl -= (float)dt;
			g->popups[i].y -= POPUP_RISE * (float)dt;
		}
		i++;
	}
}

void	score_commit_best(t_game *g)
{
	if (g->score > g->best_score)
	{
		g->best_score = g->score;
		g->new_best = true;
	}
}
