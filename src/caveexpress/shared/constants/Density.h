#pragma once

namespace caveexpress {

#if 1  // cryham
	#define DENSITY_WATER			1000.0f
	#define DENSITY_NPC				900.0f
	#define DENSITY_NPC_BLOWING		10000.0f
	#define DENSITY_NPC_FISH		550.0f
	#define DENSITY_STONE			2400.0f
	#define DENSITY_BOMB			1200.0f
	#define DENSITY_PACKAGE			500.0f
	#define DENSITY_TREE			450.0f
	#define DENSITY_FRUIT			350.0f
	#define DENSITY_EGG				500.0f
	#define DENSITY_PLAYER			1000.0f  // 400
	#define DENSITY_AIR				1.2041f
	#define DENSITY_PACKAGETARGET	50.0f
	#define DENSITY_BRIDGE			1000.0f

#else  // original
	#define DENSITY_WATER			1000.0f
	#define DENSITY_NPC				900.0f
	#define DENSITY_NPC_BLOWING		10000.0f
	#define DENSITY_NPC_FISH		550.0f
	#define DENSITY_STONE			2400.0f
	#define DENSITY_BOMB			1200.0f
	#define DENSITY_PACKAGE			500.0f
	#define DENSITY_TREE			450.0f
	#define DENSITY_FRUIT			350.0f
	#define DENSITY_EGG				500.0f
	#define DENSITY_PLAYER			400.0f
	#define DENSITY_AIR				1.2041f
	#define DENSITY_PACKAGETARGET	450.0f
	#define DENSITY_BRIDGE			1000.0f
#endif

}
