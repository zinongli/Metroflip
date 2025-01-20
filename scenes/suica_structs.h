#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

typedef struct {
    uint8_t entry; // Which entry we are currently viewing
    uint8_t page; // Which vertial page we are on
    uint8_t* travel_history; // Dynamic array for raw 16-byte entries
    size_t size; // Number of entries currently stored
    size_t capacity; // Allocated capacity
} SuicaHistoryViewModel;

typedef enum {
    SuicaHistoryNull,
    SuicaHistoryBus,
    SuicaHistoryPosAndTaxi,
    SuicaHistoryVendingMachine,
    SuicaHistoryTrain
} SuicaHistoryType;

typedef struct {
    uint8_t entry_line;
    uint8_t entry_station;
    uint8_t exit_line;
    uint8_t exit_station;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint16_t balance;
    uint8_t history_type;
    uint8_t rail_region;
} SuicaTravelHistory;

typedef struct {
    View* view_history;
} SuicaContext;
