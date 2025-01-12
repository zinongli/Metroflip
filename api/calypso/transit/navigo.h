#include "../calypso_util.h"
#include "../cards/intercode.h"
#include "navigo_i.h"
#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

#ifndef NAVIGO_H
#define NAVIGO_H

const char* get_navigo_type(int type);

const char* get_navigo_metro_station(int station_group_id, int station_id);

const char* get_navigo_train_line(int station_group_id);

const char* get_navigo_train_station(int station_group_id, int station_id);

const char* get_navigo_tram_line(int route_number);

void show_navigo_event_info(
    NavigoCardEvent* event,
    NavigoCardContract* contracts,
    FuriString* parsed_data);

void show_navigo_contract_info(NavigoCardContract* contract, FuriString* parsed_data);

void show_navigo_environment_info(NavigoCardEnv* environment, FuriString* parsed_data);

typedef enum {
    BUS_URBAIN = 1,
    BUS_INTERURBAIN = 2,
    METRO = 3,
    TRAM = 4,
    TRAIN = 5,
    PARKING = 8
} NAVIGO_TRANSPORT_TYPE;

typedef enum {
    NAVIGO_EASY = 0,
    NAVIGO_DECOUVERTE = 1,
    NAVIGO_STANDARD = 2,
    NAVIGO_INTEGRAL = 6,
    IMAGINE_R = 14
} NAVIGO_CARD_STATUS;

#endif // NAVIGO_H
