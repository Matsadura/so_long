#include "ui.h"

#include <stdio.h>

#define VOL_SFX     0.9f
#define VOL_HOP     0.45f
#define VOL_JINGLE  0.8f
#define VOL_MUSIC   0.45f

static const char	*g_sound_paths[SND_COUNT] = {
[SND_HOP] = "assets/sounds/sfx_hop.wav",
[SND_COIN_1] = "assets/sounds/sfx_coin_1.wav",
[SND_COIN_2] = "assets/sounds/sfx_coin_2.wav",
[SND_COIN_3] = "assets/sounds/sfx_coin_3.wav",
[SND_COIN_4] = "assets/sounds/sfx_coin_4.wav",
[SND_COIN_5] = "assets/sounds/sfx_coin_5.wav",
[SND_GOAL] = "assets/sounds/sfx_goal.wav",
[SND_SPLAT] = "assets/sounds/sfx_splat.wav",
[SND_SPLASH] = "assets/sounds/sfx_splash.wav",
[SND_CAUGHT] = "assets/sounds/sfx_caught.wav",
[SND_TIME_UP] = "assets/sounds/sfx_time_up.wav",
[SND_START] = "assets/sounds/sfx_start.wav",
[SND_PAUSE] = "assets/sounds/sfx_pause.wav",
[SND_JINGLE_CLEAR] = "assets/sounds/jingle_stage_clear.wav",
[SND_JINGLE_OVER] = "assets/sounds/jingle_game_over.wav",
[SND_JINGLE_VICTORY] = "assets/sounds/jingle_victory.wav",
[SND_MUSIC_GAME] = "assets/sounds/music_game.wav",
[SND_MUSIC_TITLE] = "assets/sounds/music_title.wav",
};

/* Missing or broken sound files are not fatal: that sound stays silent. */
void	sound_init(t_sound_bank *b, t_platform *p)
{
	bool	audio;
	int		i;

	audio = platform_audio_open(p);
	if (audio)
		platform_set_muted(p, platform_load_muted());
	i = 0;
	while (i < SND_COUNT)
	{
		b->sounds[i] = SOUND_NONE;
		if (audio)
		{
			b->sounds[i] = platform_load_sound(p, g_sound_paths[i]);
			if (b->sounds[i] == SOUND_NONE)
				fprintf(stderr, "Warning: cannot load %s: %s\n",
					g_sound_paths[i], platform_error());
		}
		i++;
	}
}

static void	play(const t_sound_bank *b, t_platform *p, t_sfx s, float vol)
{
	platform_play_sound(p, b->sounds[s], vol);
}

static void	play_events(const t_sound_bank *b, t_platform *p, const t_game *g)
{
	unsigned	ev;
	int			combo;

	ev = g->events;
	if (ev & EV_HOP)
		play(b, p, SND_HOP, VOL_HOP);
	combo = g->combo < 1 ? 1 : g->combo;
	if (ev & EV_COIN)
		play(b, p, SND_COIN_1 + (combo > 5 ? 5 : combo) - 1, VOL_SFX);
	if ((ev & EV_GOAL) && !(ev & (EV_STAGE_CLEAR | EV_VICTORY)))
		play(b, p, SND_GOAL, VOL_SFX);
	if (ev & EV_CAR)
		play(b, p, SND_SPLAT, VOL_SFX);
	if (ev & EV_SPLASH)
		play(b, p, SND_SPLASH, VOL_SFX);
	if (ev & EV_CAUGHT)
		play(b, p, SND_CAUGHT, VOL_SFX);
	if (ev & EV_TIME_UP)
		play(b, p, SND_TIME_UP, VOL_SFX);
	if (ev & EV_START)
		play(b, p, SND_START, VOL_SFX);
	if (ev & EV_PAUSE)
		play(b, p, SND_PAUSE, VOL_SFX);
	if (ev & EV_STAGE_CLEAR)
		play(b, p, SND_JINGLE_CLEAR, VOL_JINGLE);
	if (ev & EV_GAME_OVER)
		play(b, p, SND_JINGLE_OVER, VOL_JINGLE);
	if (ev & EV_VICTORY)
		play(b, p, SND_JINGLE_VICTORY, VOL_JINGLE);
}

/* Title theme on the title screen, game theme while playing, silence
** under the stage-clear / game-over / victory jingles. */
static void	update_music(const t_sound_bank *b, t_platform *p, const t_game *g)
{
	if (g->state == GAME_TITLE)
		platform_play_music(p, b->sounds[SND_MUSIC_TITLE], VOL_MUSIC);
	else if (g->state == GAME_PLAYING || g->state == GAME_DYING)
		platform_play_music(p, b->sounds[SND_MUSIC_GAME], VOL_MUSIC);
	else if (g->state == GAME_PAUSED)
		platform_pause_music(p, true);
	else
		platform_stop_music(p);
}

void	sound_update(const t_sound_bank *b, t_platform *p, const t_game *g)
{
	play_events(b, p, g);
	update_music(b, p, g);
}
