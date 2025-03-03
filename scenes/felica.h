#ifndef FELICA_H
#define FELICA_H

#include "../metroflip_i.h"
#include <lib/nfc/protocols/felica/felica.h>
#include <lib/nfc/protocols/felica/felica_poller.h>

#define SUICA_IC_TYPE_CODE        0x31
#define OCTOPUS_SERVICE_CODE      0x0880U
typedef enum {
    CARD_TYPE_SUICA,
    CARD_TYPE_OCTOPUS,
    CARD_TYPE_FELICA_UNKNOWN
} FelicaCardType;

bool suica_verify(const FelicaData* felica_data);

bool octopus_verify(FelicaPoller* felica_poller);

#endif // FELICA_H
