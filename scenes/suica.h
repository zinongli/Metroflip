#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

// Railway 
static const char* RailwayShort[] = {
    [0xD5] = "KK",
    [0xD6] = "KK",
};

// Station
static const char* KeikyuAirport[] = {
    [0x00] = "Keikyu Airport",
    [0x01] = "11Keikyu Kamata",
    [0x02] = "12Kojiya",
    [0x03] = "13Otorii",
    [0x04] = "14Anamori-Inari",
    [0x05] = "15Tenkūbashi",
    [0x06] = "16Haneda Airport",
};

static const char* KeikyuMain[] = {
    [0x00] = "Keikyu Main",
    [0x01] = "11Keikyu Kamata",
    [0x02] = "12Kojiya",
    [0x03] = "13Otorii",
    [0x04] = "14Anamori-Inari",
    [0x05] = "15Tenkūbashi",
    [0x06] = "16Haneda Airport",
};

