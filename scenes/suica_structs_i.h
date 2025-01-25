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
    uint8_t* shop_code;
} SuicaTravelHistory;


typedef struct {
    uint8_t entry; // Which entry we are currently viewing
    uint8_t page; // Which vertial page we are on
    uint8_t* travel_history; // Dynamic array for raw 16-byte entries
    size_t size; // Number of entries currently stored
    size_t capacity; // Allocated capacity
    uint8_t animator_tick; // Counter for the animations
    SuicaTravelHistory history; // Current history entry
} SuicaHistoryViewModel;
