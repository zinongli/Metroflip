#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

#include "api/suica/suica_station_list.h"

// Rinkai
static const int RinkaiRLogoOffset[2] = {0, 0};

// Keikyu
static const int KeikyuKKLogoOffset[2] = {0, 0};

// Tokyo Metro
static const int GinzaGLogoOffset[2] = {0, 0};
static const int ChiyodaCLogoOffset[2] = {1, 0};
static const int YurakuchoYLogoOffset[2] = {1, 1};
static const int HibiyaHLogoOffset[2] = {1, 0};
static const int TozaiTLogoOffset[2] = {2, 1};
static const int MarunouchiMLogoOffset[2] = {0, 1};
static const int MarunouchiHonanchoMbLogoOffset[2] = {-5, 1};
static const int HanzomonZLogoOffset[2] = {2, 1};
static const int NambokuNLogoOffset[2] = {0, 1};

// Toei
static const int AsakusaALogoOffset[2] = {0, 0};
static const int MitaILogoOffset[2] = {4, 0};
static const int ShinjukuSLogoOffset[2] = {2, 0};
static const int OedoELogoOffset[2] = {3, 0};

// Unknown
static const int QuestionMarkBigLogoOffset[2] = {0, 0}; // Placeholder

// Railway

static const Railway RailwaysList[] = {
    // Japan Railway East JRE
    {0x01, JREKeihinTohoku, 0, "Keihin Tohoku", 14, SuicaJR, "JK", 0},
    {0x01, JRETokaidoMainLine, 0, "Tokaido Main", 21, SuicaJR, "JT", 0},
    {0x1D, JRENegishi, 0, "Negishi", 10, SuicaJR, "JK", 0},

    // Tokyo Waterfront Area Rapid Transit TWR
    {0x82, TWRRinkaiLine, RinkaiRLogoOffset, "Rinkai", 8, SuicaTWR, "R", &I_Suica_RinkaiR},

    // Tokyo Monorail
    {0xFA, TokyoMonorail, 0, "Tokyo Monorail", 11, SuicaTokyoMonorail, "MO", 0},

    // Keikyu
    {0xD5, KeikyuMain, KeikyuKKLogoOffset, "Keikyu Main", 50, SuicaKeikyu, "KK", &I_Suica_KeikyuKK},
    {0xD6, KeikyuAirport, KeikyuKKLogoOffset, "Keikyu Airport", 6, SuicaKeikyu, "KK", &I_Suica_KeikyuKK},

    // Tokyo Metro
    {0xE3, TokyoMetroGinza, GinzaGLogoOffset, "Ginza", 19, SuicaTokyoMetro, "G", &I_Suica_GinzaG},
    {0xE3, TokyoMetroChiyoda, ChiyodaCLogoOffset, "Chiyoda", 20, SuicaTokyoMetro, "C", &I_Suica_ChiyodaC},
    {0xE3, TokyoMetroYurakucho, YurakuchoYLogoOffset, "Yurakucho", 24, SuicaTokyoMetro, "Y", &I_Suica_YurakuchoY},
    {0xE4, TokyoMetroHibiya, HibiyaHLogoOffset, "Hibiya", 21, SuicaTokyoMetro, "H", &I_Suica_HibiyaH},
    {0xE4, TokyoMetroTozai, TozaiTLogoOffset, "Tozai", 23, SuicaTokyoMetro, "T", &I_Suica_TozaiT},
    {0xE5, TokyoMetroMarunouchi, MarunouchiMLogoOffset, "Marunouchi", 25, SuicaTokyoMetro, "M", &I_Suica_MarunouchiM},
    {0xE5, TokyoMetroMarunouchiHonancho, MarunouchiHonanchoMbLogoOffset, "M Honancho", 4, SuicaTokyoMetro, "Mb", &I_Suica_MarunouchiHonanchoMb},
    {0xE6, TokyoMetroHanzomon, HanzomonZLogoOffset, "Hanzomon", 14, SuicaTokyoMetro, "Z", &I_Suica_HanzomonZ},
    {0xE7, TokyoMetroNamboku, NambokuNLogoOffset, "Namboku", 19, SuicaTokyoMetro, "N", &I_Suica_NambokuN},
    
    // Toei
    {0xEF, ToeiAsakusa, AsakusaALogoOffset, "Asakusa", 20, SuicaToei, "A", &I_Suica_AsakusaA},
    {0xF0, ToeiMita,MitaILogoOffset, "Mita", 27, SuicaToei, "I", &I_Suica_MitaI},
    {0xF1, ToeiShinjuku, ShinjukuSLogoOffset, "Shinjuku", 21, SuicaToei, "S", &I_Suica_ShinjukuS},
    {0xF2, ToeiOedoPartA, OedoELogoOffset, "Oedo", 26, SuicaToei, "E", &I_Suica_OedoE},
    {0xF3, ToeiOedoPartB, OedoELogoOffset, "Oedo", 14, SuicaToei, "E", &I_Suica_OedoE},
    // Unknown
    {0x00, UnknownLine, QuestionMarkBigLogoOffset, "Unknown", 1, SuicaRailwayTypeMax, "??", &I_Suica_QuestionMarkBig},
};
