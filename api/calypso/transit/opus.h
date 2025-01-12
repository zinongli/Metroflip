#include "../calypso_util.h"
#include "../cards/opus.h"
#include "opus_i.h"

#ifndef OPUS_H
#define OPUS_H

void show_opus_event_info(
    OpusCardEvent* event,
    OpusCardContract* contracts,
    FuriString* parsed_data);

void show_opus_contract_info(OpusCardContract* contract, FuriString* parsed_data);

void show_opus_environment_info(OpusCardEnv* environment, FuriString* parsed_data);

#endif // OPUS_H
