#ifndef _SLDRSPAWNPOINTSTRUCT
#define _SLDRSPAWNPOINTSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSpawnPointStruct {
    SLdrSpawnPointStruct();
    ~SLdrSpawnPointStruct();

    int power_beam;
    int dark_beam;
    int light_beam;
    int annihilator_beam;
    int power_beam_combo;
    int dark_beam_combo;
    int light_beam_combo;
    int annihilator_beam_combo;
    int charge_combo_upgrade;
    int combat_visor;
    int scan_visor;
    int dark_visor;
    int echo_visor;
    int varia_suit;
    int dark_suit;
    int light_suit;
    int morph_ball;
    int boost_ball;
    int spider_ball;
    int bomb;
    int light_bomb;
    int dark_bomb;
    int annihilator_bomb;
    int charge_upgrade;
    int grapple_beam;
    int double_jump;
    int gravity_boost;
    int seeker;
    int screw_attack;
    int translator_upgrade;
    int temple_key1;
    int temple_key2;
    int temple_key3;
    int temple_key4;
    int temple_key5;
    int temple_key6;
    int temple_key7;
    int temple_key8;
    int temple_key9;
    int sand_key1;
    int sand_key2;
    int sand_key3;
    int swamp_key1;
    int swamp_key2;
    int swamp_key3;
    int cliffside_key1;
    int cliffside_key2;
    int cliffside_key3;
    int energy;
    int energy_tank;
    int unknown_0x161898dc;
    int power_bomb;
    int missile;
    int dark_beam_ammo;
    int light_beam_ammo;
    int percentage_increase;
    int misc_counter1;
    int misc_counter2;
    int misc_counter3;
    int misc_counter4;
    int change_to_power_beam;
    int change_to_dark_beam;
    int change_to_light_beam;
    int change_to_annihilator_beam;
    int multi_charge_upgrade;
    int invisibility;
    int amplified_damage;
    int invincibility;
    int unknown_0x66ae338e;
    int unknown_0x210e495e;
    int unknown_0x1c6e60ee;
    int unknown_0xae4ebcfe;
    int frag_count;
    int died_count;
    int archenemy_count;
    int persistent_counter1;
    int persistent_counter2;
    int persistent_counter3;
    int persistent_counter4;
    int persistent_counter5;
    int persistent_counter6;
    int persistent_counter7;
    int persistent_counter8;
    int change_to_combat_visor;
    int change_to_scan_visor;
    int change_to_dark_visor;
    int change_to_echo_visor;
    int coin_amplifier;
    int coin_counter;
    int unlimited_missiles;
    int unlimited_beam_ammo;
    int dark_shield;
    int light_shield;
    int absorb_attack;
    int death_ball;
    int scan_virus;
    int visor_static;
    int beam_weapons_disabled;
    int missile_weapons_disabled;
    int unknown_0x32f5e918;
    int disable_ball;
    int disable_double_jump;
    int activate_morphball_boost;
    int hacked_effect;
    int activate_morphball_damage;
    int translator_upgrade1;
    int translator_upgrade2;
    int translator_upgrade3;
    int translator_upgrade4;
};

void LoadTypedefSLdrSpawnPointStruct(SLdrSpawnPointStruct&, CInputStream&);

#endif // _SLDRSPAWNPOINTSTRUCT