#include "transit/navigo_i.h"
#include "transit/opus_i.h"
#include <furi.h>

#ifndef CALYPSO_I_H
#define CALYPSO_I_H

typedef enum {
    CALYPSO_CARD_NAVIGO,
    CALYPSO_CARD_OPUS,
    CALYPSO_CARD_UNKNOWN
} CALYPSO_CARD_TYPE;

typedef struct {
    NavigoCardData* navigo;
    OpusCardData* opus;

    CALYPSO_CARD_TYPE card_type;
    unsigned int card_number;

    int contracts_count;
} CalypsoCardData;

typedef struct {
    CalypsoCardData* card;
    int page_id;
    // mutex
    FuriMutex* mutex;
} CalypsoContext;

#endif // CALYPSO_I_H
