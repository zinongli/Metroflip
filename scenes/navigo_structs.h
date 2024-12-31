#include <datetime.h>
#include <stdbool.h>
#include <furi.h>

typedef struct {
    int transport_type;
    int transition;
    int service_provider;
    int station_group_id;
    int station_id;
    int location_gate;
    bool location_gate_available;
    int device;
    int door;
    int side;
    bool device_available;
    int route_number;
    bool route_number_available;
    int mission;
    bool mission_available;
    int vehicle_id;
    bool vehicle_id_available;
    int used_contract;
    bool used_contract_available;
    DateTime date;
} NavigoCardEvent;

typedef struct {
    int app_version;
    int country_num;
    int network_num;
    DateTime end_dt;
} NavigoCardEnv;

typedef struct {
    int card_status;
    int commercial_id;
} NavigoCardHolder;

typedef struct {
    int tariff;
    int serial_number;
    bool serial_number_available;
    int pay_method;
    bool pay_method_available;
    double price_amount;
    bool price_amount_available;
    DateTime start_date;
    DateTime end_date;
    bool end_date_available;
    int zones[5];
    bool zones_available;
    DateTime sale_date;
    int sale_agent;
    int sale_device;
    int status;
    int authenticator;
} NavigoCardContract;

typedef struct {
    NavigoCardEnv environment;
    NavigoCardHolder holder;
    NavigoCardContract contracts[2];
    NavigoCardEvent events[3];
    int ticket_count;
} NavigoCardData;

typedef struct {
    NavigoCardData* card;
    int page_id;
    // mutex
    FuriMutex* mutex;
} NavigoContext;
