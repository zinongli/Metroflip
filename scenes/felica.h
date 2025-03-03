#ifndef FELICA_H
#define FELICA_H

#define SUICA_IC_TYPE_CODE        0x31
#define OCTOPUS_SERVICE_CODE      0x0117
#include "../metroflip_i.h"

typedef enum {
    CARD_TYPE_SUICA,
    CARD_TYPE_OCTOPUS,
    CARD_TYPE_FELICA_UNKNOWN
} FelicaCardType;

#endif // FELICA_H
