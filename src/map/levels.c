#include "game.h"

/* Default campaign, played in order when no map is given on the CLI. */
const char *const	g_campaign_levels[] = {
	"assets/maps/level1.ber",
	"assets/maps/level2.ber",
	"assets/maps/level3.ber",
	"assets/maps/level4.ber",
	"assets/maps/level5.ber",
	"assets/maps/level6.ber",
	"assets/maps/level7.ber",
	"assets/maps/level8.ber",
};

const int			g_campaign_level_count
	= sizeof(g_campaign_levels) / sizeof(g_campaign_levels[0]);
