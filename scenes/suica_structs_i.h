#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

typedef enum {
    SuicaKeikyu,
    SuicaTokyoMetro,
    SuicaToei,
    SuicaJR,
    SuicaTWR,
    SuicaTokyoMonorail,
    SuicaRailwayTypeMax,
} SuicaRailwayCompany;

typedef enum {
    SuicaBalanceAdd,
    SuicaBalanceSub,
    SuicaBalanceEqual,
} SuicaBalanceChangeSign;

typedef struct {
    uint8_t station_code;
    uint8_t station_number;
    const char* name;
    const char* jr_header;
} Station;

typedef struct {
    uint8_t line_code;
    Station* line;
    const uint8_t* logo;
    const int* logo_position;
    const char* long_name;
    uint8_t station_num;
    SuicaRailwayCompany type;
    const char* short_name;
} Railway;

typedef struct {
    Railway entry_line;
    Station entry_station;
    Railway exit_line;
    Station exit_station;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint16_t balance;
    uint8_t history_type;
    uint8_t area_code;
    uint16_t previous_balance;
    uint16_t balance_change;
    SuicaBalanceChangeSign balance_sign;
} SuicaTravelHistory;
