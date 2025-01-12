#include <datetime.h>
#include <stdbool.h>

#ifndef OPUS_I_H
#define OPUS_I_H

typedef struct {
    int service_provider;
    int route_number;
    bool route_number_available;
    int used_contract;
    bool used_contract_available;
    DateTime date;
} OpusCardEvent;

typedef struct {
    int app_version;
    int country_num;
    int network_num;
    DateTime end_dt;
} OpusCardEnv;

typedef struct {
    int card_status;
    int commercial_id;
} OpusCardHolder;

typedef struct {
    int provider;
    int tariff;
    DateTime start_date;
    DateTime end_date;
    DateTime sale_date;
    int status;
    bool present;
} OpusCardContract;

typedef struct {
    OpusCardEnv environment;
    OpusCardHolder holder;
    OpusCardContract contracts[4];
    OpusCardEvent events[3];
} OpusCardData;

#endif // OPUS_I_H
