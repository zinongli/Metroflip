#include "../calypso_util.h"
#include "../cards/intercode.h"
#include "navigo_i.h"
#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

#ifndef NAVIGO_H
#define NAVIGO_H

const char* get_navigo_type(int type);

const char* get_navigo_station(int station_group_id, int station_id, int service_provider);

const char* get_navigo_sncf_train_line(int station_group_id);

const char* get_navigo_sncf_station(int station_group_id, int station_id);

const char* get_navigo_tram_line(int route_number);

void show_navigo_event_info(
    NavigoCardEvent* event,
    NavigoCardContract* contracts,
    FuriString* parsed_data);

void show_navigo_contract_info(NavigoCardContract* contract, FuriString* parsed_data);

void show_navigo_environment_info(NavigoCardEnv* environment, FuriString* parsed_data);

typedef enum {
    NAVIGO_EASY = 0,
    NAVIGO_DECOUVERTE = 1,
    NAVIGO_STANDARD = 2,
    NAVIGO_INTEGRAL = 6,
    IMAGINE_R = 14
} NAVIGO_CARD_STATUS;

typedef enum {
    NAVIGO_PROVIDER_SNCF = 2,
    NAVIGO_PROVIDER_RATP = 3,
    NAVIGO_PROVIDER_IDFM = 4,
    NAVIGO_PROVIDER_ORA = 8,
    NAVIGO_PROVIDER_VEOLIA_CSO = 115,
    NAVIGO_PROVIDER_VEOLIA_RBUS = 116,
    NAVIGO_PROVIDER_PHEBUS = 156,
    NAVIGO_PROVIDER_RATP_VEOLIA_SERVICE = 175
} NAVIGO_SERVICE_PROVIDER;

#endif // NAVIGO_H
