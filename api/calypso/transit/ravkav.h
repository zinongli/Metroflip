#include "../calypso_util.h"
#include "../cards/ravkav.h"
#include "ravkav_i.h"

#ifndef RAVKAV_H
#define RAVKAV_H

void show_ravkav_event_info(RavKavCardEvent* event, FuriString* parsed_data);

void show_ravkav_contract_info(RavKavCardContract* contract, FuriString* parsed_data);

void show_ravkav_environment_info(RavKavCardEnv* environment, FuriString* parsed_data);

#endif // RAVKAV_H
