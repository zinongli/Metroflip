#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

typedef enum {
    SuicaHistoryNull,
    SuicaHistoryBus,
    SuicaHistoryPosAndTaxi,
    SuicaHistoryVendingMachine,
    SuicaHistoryHappyBirthday,
    SuicaHistoryTrain
} SuicaHistoryType;

typedef struct {
    View* view_history;
    FuriTimer* timer;
} SuicaContext;
