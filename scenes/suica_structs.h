#include <datetime.h>
#include <stdbool.h>
#include <furi.h>
typedef struct {
    uint8_t entry; // Which entry we are currently viewing
    uint8_t page; // Which vertial page we are on
    uint8_t* travel_history; // Dynamic array for raw 16-byte entries
    size_t size; // Number of entries currently stored
    size_t capacity; // Allocated capacity
    uint8_t animator_tick; // Counter for the animations
} SuicaHistoryViewModel;

typedef enum {
    SuicaHistoryNull,
    SuicaHistoryBus,
    SuicaHistoryPosAndTaxi,
    SuicaHistoryVendingMachine,
    SuicaHistoryTrain
} SuicaHistoryType;

typedef struct {
    View* view_history;
    FuriTimer* timer;
} SuicaContext;
