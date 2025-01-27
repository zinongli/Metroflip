#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

#include "api/suica/suica_station_list.h"
#define SUICA_RAILWAY_UNKNOWN_NAME "Unknown"
// Railway

static const Railway RailwaysList[] = {
    // Japan Railway East JRE
    {0x01, JREKeihinTohoku, {0, 0}, "Keihin Tohoku", 14, SuicaJR, "JK", 0},
    {0x01, JRETokaidoMainLine, {0, 0}, "Tokaido Main", 21, SuicaJR, "JT", 0},
    {0x1D, JRENegishi, {0, 0}, "Negishi", 10, SuicaJR, "JK", 0},

    // Tokyo Waterfront Area Rapid Transit TWR
    {0x82, TWRRinkaiLine, {0, 0}, "Rinkai", 8, SuicaTWR, "R", &I_Suica_RinkaiR},

    // Tokyo Monorail
    {0xFA, TokyoMonorail, {0, 0}, "Tokyo Monorail", 11, SuicaTokyoMonorail, "MO", 0},

    // Keikyu
    {0xD5, KeikyuMain, {0, 0}, "Keikyu Main", 50, SuicaKeikyu, "KK", &I_Suica_KeikyuKK},
    {0xD6, KeikyuAirport, {0, 0}, "Keikyu Airport", 6, SuicaKeikyu, "KK", &I_Suica_KeikyuKK},

    // Tokyo Metro
    {0xE3, TokyoMetroGinza, {0, 0}, "Ginza", 19, SuicaTokyoMetro, "G", &I_Suica_GinzaG},
    {0xE3, TokyoMetroChiyoda, {1, 0}, "Chiyoda", 20, SuicaTokyoMetro, "C", &I_Suica_ChiyodaC},
    {0xE3, TokyoMetroYurakucho, {1, 1}, "Yurakucho", 24, SuicaTokyoMetro, "Y", &I_Suica_YurakuchoY},
    {0xE4, TokyoMetroHibiya, {1, 0}, "Hibiya", 21, SuicaTokyoMetro, "H", &I_Suica_HibiyaH},
    {0xE4, TokyoMetroTozai, {2, 1}, "Tozai", 23, SuicaTokyoMetro, "T", &I_Suica_TozaiT},
    {0xE5, TokyoMetroMarunouchi, {0, 1}, "Marunouchi", 25, SuicaTokyoMetro, "M", &I_Suica_MarunouchiM},
    {0xE5, TokyoMetroMarunouchiHonancho, {-5, 1}, "M Honancho", 4, SuicaTokyoMetro, "Mb", &I_Suica_MarunouchiHonanchoMb},
    {0xE6, TokyoMetroHanzomon, {2, 1}, "Hanzomon", 14, SuicaTokyoMetro, "Z", &I_Suica_HanzomonZ},
    {0xE7, TokyoMetroNamboku, {0, 1}, "Namboku", 19, SuicaTokyoMetro, "N", &I_Suica_NambokuN},
    
    // Toei
    {0xEF, ToeiAsakusa, {0, 0}, "Asakusa", 20, SuicaToei, "A", &I_Suica_AsakusaA},
    {0xF0, ToeiMita, {4, 0}, "Mita", 27, SuicaToei, "I", &I_Suica_MitaI},
    {0xF1, ToeiShinjuku, {2, 0}, "Shinjuku", 21, SuicaToei, "S", &I_Suica_ShinjukuS},
    {0xF2, ToeiOedoPartA, {3, 0}, "Oedo", 26, SuicaToei, "E", &I_Suica_OedoE},
    {0xF3, ToeiOedoPartB, {3, 0}, "Oedo", 14, SuicaToei, "E", &I_Suica_OedoE},
    // Unknown
    {0x00, UnknownLine, {0, 0}, SUICA_RAILWAY_UNKNOWN_NAME, 1, SuicaRailwayTypeMax, "??", &I_Suica_QuestionMarkBig}
};
