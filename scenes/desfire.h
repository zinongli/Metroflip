#ifndef DESFIRE_H
#define DESFIRE_H

#include "../metroflip_i.h"

typedef enum {
    CARD_TYPE_ITSO,
    CARD_TYPE_OPAL,
    CARD_TYPE_CLIPPER,
    CARD_TYPE_MYKI,
    CARD_TYPE_DESFIRE_UNKNOWN
} DesfireCardType;

bool itso_parse(const NfcDevice* device, FuriString* parsed_data);
bool opal_parse(const NfcDevice* device, FuriString* parsed_data);
bool clipper_parse(const NfcDevice* device, FuriString* parsed_data);
bool myki_parse(const NfcDevice* device, FuriString* parsed_data);

#endif // DESFIRE_H
