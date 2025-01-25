#include "navigo.h"
#include "navigo_lists.h"
#include "../../../metroflip_i.h"

const char* get_navigo_transport_type(int type) {
    switch(type) {
    case URBAN_BUS:
        return "Urban Bus";
    case INTERURBAN_BUS:
        return "Interurban Bus";
    case METRO:
        return "Metro";
    case TRAM:
        return "Tram";
    case COMMUTER_TRAIN:
        return "Train";
    case PARKING:
        return "Parking";
    case EXPRESS_COMMUTER_TRAIN:
        return "TER";
    default:
        return "Unknown";
    }
}

const char* get_navigo_service_provider(int provider) {
    switch(provider) {
    case NAVIGO_PROVIDER_SNCF:
        return "SNCF";
    case NAVIGO_PROVIDER_RATP:
        return "RATP";
    case 4:
    case 10:
        return "IDF Mobilites";
    case NAVIGO_PROVIDER_ORA:
        return "ORA";
    case NAVIGO_PROVIDER_VEOLIA_CSO:
        return "CSO (VEOLIA)";
    case NAVIGO_PROVIDER_VEOLIA_RBUS:
        return "R'Bus (VEOLIA)";
    case NAVIGO_PROVIDER_PHEBUS:
        return "Phebus";
    case NAVIGO_PROVIDER_RATP_VEOLIA_NANTERRE:
        return "RATP (Veolia Transport Nanterre)";
    default: {
        char* provider_str = malloc(6 * sizeof(char));
        snprintf(provider_str, 6, "%d", provider);
        return provider_str;
    }
    }
}

const char* get_navigo_type(int type) {
    switch(type) {
    case NAVIGO_EASY:
        return "Navigo Easy";
    case NAVIGO_DECOUVERTE:
        return "Navigo Decouverte";
    case NAVIGO_STANDARD:
        return "Navigo Standard";
    case NAVIGO_INTEGRAL:
        return "Navigo Integral";
    case IMAGINE_R:
        return "Imagine R";
    default:
        return "Navigo";
    }
}

const char* get_navigo_tariff(int tariff) {
    switch(tariff) {
    case 0x0000:
        return "Navigo Mois";
    case 0x0001:
        return "Navigo Semaine";
    case 0x0002:
        return "Navigo Annuel";
    case 0x0003:
        return "Navigo Jour";
    case 0x0004:
        return "Imagine R Junior";
    case 0x0005:
        return "Imagine R Etudiant";
    case 0x000D:
        return "Navigo Jeunes Week-end";
    case 0x0015:
        return "Paris-Visite"; // Theoric
    case 0x1000:
        return "Navigo Liberte+";
    case 0x4000:
        return "Navigo Mois 75%%";
    case 0x4001:
        return "Navigo Semaine 75%%";
    case 0x4015:
        return "Paris-Visite (Enfant)"; // Theoric
    case 0x5000:
        return "Tickets T+";
    case 0x5004:
        return "Tickets OrlyBus"; // Theoric
    case 0x5005:
        return "Tickets RoissyBus"; // Theoric
    case 0x5006:
        return "Bus-Tram"; // Theoric
    case 0x5008:
        return "Metro-Train-RER"; // Theoric
    case 0x500b:
        return "Paris <> Aeroports"; // Theoric
    case 0x5010:
        return "Tickets T+ (Reduit)"; // Theoric
    case 0x5016:
        return "Bus-Tram (Reduit)"; // Theoric
    case 0x5018:
        return "Metro-Train-RER (Reduit)"; // Theoric
    case 0x501b:
        return "Paris <> Aeroports (Reduit)"; // Theoric
    case 0x8003:
        return "Navigo Solidarite Gratuit";
    default: {
        char* tariff_str = malloc(6 * sizeof(char));
        snprintf(tariff_str, 6, "%d", tariff);
        return tariff_str;
    }
    }
}

bool is_ticket_count_available(int tariff) {
    return tariff >= 0x5000 && tariff <= 0x501b;
}

const char* get_pay_method(int pay_method) {
    switch(pay_method) {
    case 0x30:
        return "Apple Pay/Google Pay";
    case 0x80:
        return "Debit PME";
    case 0x90:
        return "Cash";
    case 0xA0:
        return "Mobility Check";
    case 0xB3:
        return "Payment Card";
    case 0xA4:
        return "Check";
    case 0xA5:
        return "Vacation Check";
    case 0xB7:
        return "Telepayment";
    case 0xD0:
        return "Remote Payment";
    case 0xD7:
        return "Voucher, Prepayment, Exchange Voucher, Travel Voucher";
    case 0xD9:
        return "Discount Voucher";
    default:
        return "Unknown";
    }
}

const char* get_zones(int* zones) {
    if(zones[0] && zones[4]) {
        return "All Zones (1-5)";
    } else if(zones[0] && zones[3]) {
        return "Zones 1-4";
    } else if(zones[0] && zones[2]) {
        return "Zones 1-3";
    } else if(zones[0] && zones[1]) {
        return "Zones 1-2";
    } else if(zones[0]) {
        return "Zone 1";
    } else if(zones[1] && zones[4]) {
        return "Zones 2-5";
    } else if(zones[1] && zones[3]) {
        return "Zones 2-4";
    } else if(zones[1] && zones[2]) {
        return "Zones 2-3";
    } else if(zones[1]) {
        return "Zone 2";
    } else if(zones[2] && zones[4]) {
        return "Zones 3-5";
    } else if(zones[2] && zones[3]) {
        return "Zones 3-4";
    } else if(zones[2]) {
        return "Zone 3";
    } else if(zones[3] && zones[4]) {
        return "Zones 4-5";
    } else if(zones[3]) {
        return "Zone 4";
    } else if(zones[4]) {
        return "Zone 5";
    } else {
        return "Unknown";
    }
}

const char* get_intercode_version(int version) {
    // version is a 6 bits int
    // if the first 3 bits are 000, it's a 1.x version
    // if the first 3 bits are 001, it's a 2.x version
    // else, it's unknown
    int major = (version >> 3) & 0x07;
    if(major == 0) {
        return "Intercode I";
    } else if(major == 1) {
        return "Intercode II";
    }
    return "Unknown";
}

int get_intercode_subversion(int version) {
    // subversion is a 3 bits int
    return version & 0x07;
}

char* get_navigo_station(
    int station_group_id,
    int station_id,
    int station_sub_id,
    int service_provider) {
    switch(service_provider) {
    case NAVIGO_PROVIDER_SNCF: {
        if(station_group_id < 77 && station_id < 19) {
            const char* station_group_name =
                NAVIGO_SNCF_LOCATION_LIST[station_group_id][station_id];
            if(station_group_name) {
                // split station_name by '|' and return the station_sub_id - 1 element
                char* station_name = strdup(station_group_name);
                if(!station_name) {
                    return "Unknown";
                }
                char* token = strtok(station_name, "|");
                for(int i = 0; i < station_sub_id - 1; i++) {
                    token = strtok(NULL, "|");
                    if(!token) {
                        break;
                    }
                }
                if(token) {
                    return token;
                }
            }
        }
        // cast station_group_id-station_id-station_sub_id to a string
        char* station = malloc(12 * sizeof(char));
        if(!station) {
            return "Unknown";
        }
        snprintf(station, 10, "%d-%d-%d", station_group_id, station_id, station_sub_id);
        return station;
    }
    case NAVIGO_PROVIDER_RATP:
    case NAVIGO_PROVIDER_ORA: {
        if(station_group_id < 32 && station_id < 16) {
            const char* station_name = NAVIGO_RATP_LOCATION_LIST[station_group_id][station_id];
            if(station_name) {
                char* station;
                if((station = strdup(station_name)) != NULL) {
                    return station;
                }
            }
        }
        // cast station_group_id-station_id to a string
        char* station = malloc(12 * sizeof(char));
        if(!station) {
            return "Unknown";
        }
        snprintf(station, 10, "%d-%d", station_group_id, station_id);
        return station;
    }
    default: {
        // cast station_group_id-station_id to a string
        char* station = malloc(12 * sizeof(char));
        if(!station) {
            return "Unknown";
        }
        snprintf(station, 10, "%d-%d", station_group_id, station_id);
        return station;
    }
    }
}

const char* get_navigo_sncf_train_line(int station_group_id) {
    if(station_group_id < 77) {
        return NAVIGO_SNCF_TRAIN_LINES_LIST[station_group_id];
    }
    return "Unknown";
}

const char* get_navigo_tram_line(int route_number) {
    switch(route_number) {
    case 1:
        return "T3a";
    case 9:
        return "T9";
    case 16:
        return "T6";
    case 18:
        return "T8";
    default: {
        char* line = malloc(5 * sizeof(char));
        if(!line) {
            return "Unknown";
        }
        snprintf(line, 5, "?%d?", route_number);
        return line;
    }
    }
}

void show_navigo_event_info(
    NavigoCardEvent* event,
    NavigoCardContract* contracts,
    FuriString* parsed_data) {
    if(event->used_contract == 0) {
        furi_string_cat_printf(parsed_data, "No event data\n");
        return;
    }
    char* station = get_navigo_station(
        event->station_group_id, event->station_id, event->station_sub_id, event->service_provider);
    char* sector = get_navigo_station(event->station_group_id, 0, 0, event->service_provider);

    if(event->transport_type == URBAN_BUS || event->transport_type == INTERURBAN_BUS ||
       event->transport_type == METRO || event->transport_type == TRAM) {
        if(event->route_number_available) {
            if(event->transport_type == METRO && event->route_number == 103) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s 3 bis\n%s\n",
                    get_navigo_transport_type(event->transport_type),
                    get_intercode_string_transition_type(event->transition));
            } else if(event->transport_type == TRAM) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s %s\n%s\n",
                    get_navigo_transport_type(event->transport_type),
                    get_navigo_tram_line(event->route_number),
                    get_intercode_string_transition_type(event->transition));
            } else {
                furi_string_cat_printf(
                    parsed_data,
                    "%s %d\n%s\n",
                    get_navigo_transport_type(event->transport_type),
                    event->route_number,
                    get_intercode_string_transition_type(event->transition));
            }
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s\n%s\n",
                get_navigo_transport_type(event->transport_type),
                get_intercode_string_transition_type(event->transition));
        }
        furi_string_cat_printf(
            parsed_data,
            "Transporter: %s\n",
            get_navigo_service_provider(event->service_provider));
        furi_string_cat_printf(parsed_data, "Station: %s\nSector: %s\n", station, sector);
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Gate: %d\n", event->location_gate);
        }
        if(event->device_available) {
            if(event->transport_type == URBAN_BUS || event->transport_type == INTERURBAN_BUS) {
                const char* side = event->side == 0 ? "right" : "left";
                furi_string_cat_printf(parsed_data, "Door: %d\nSide: %s\n", event->door, side);
            } else {
                furi_string_cat_printf(parsed_data, "Device: %d\n", event->device);
            }
        }
        if(event->mission_available) {
            furi_string_cat_printf(parsed_data, "Mission: %d\n", event->mission);
        }
        if(event->vehicle_id_available) {
            furi_string_cat_printf(parsed_data, "Vehicle: %d\n", event->vehicle_id);
        }
        if(event->used_contract_available) {
            furi_string_cat_printf(
                parsed_data,
                "Contract: %d - %s\n",
                event->used_contract,
                get_navigo_tariff(contracts[event->used_contract - 1].tariff));
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else if(event->transport_type == COMMUTER_TRAIN) {
        if(event->route_number_available) {
            furi_string_cat_printf(
                parsed_data,
                "RER %c\n%s\n",
                (65 + event->route_number - 17),
                get_intercode_string_transition_type(event->transition));
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s %s\n%s\n",
                get_navigo_transport_type(event->transport_type),
                get_navigo_sncf_train_line(event->station_group_id),
                get_intercode_string_transition_type(event->transition));
        }
        furi_string_cat_printf(
            parsed_data,
            "Transporter: %s\n",
            get_navigo_service_provider(event->service_provider));
        furi_string_cat_printf(parsed_data, "Station: %s\n", station);
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Gate: %d\n", event->location_gate);
        }
        if(event->device_available) {
            if(event->service_provider == NAVIGO_PROVIDER_SNCF) {
                furi_string_cat_printf(parsed_data, "Device: %d\n", event->device & 0xFF);
            } else {
                furi_string_cat_printf(parsed_data, "Device: %d\n", event->device);
            }
        }
        if(event->mission_available) {
            furi_string_cat_printf(parsed_data, "Mission: %d\n", event->mission);
        }
        if(event->vehicle_id_available) {
            furi_string_cat_printf(parsed_data, "Vehicle: %d\n", event->vehicle_id);
        }
        if(event->used_contract_available) {
            furi_string_cat_printf(
                parsed_data,
                "Contract: %d - %s\n",
                event->used_contract,
                get_navigo_tariff(contracts[event->used_contract - 1].tariff));
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else {
        furi_string_cat_printf(
            parsed_data,
            "%s - %s\n",
            get_navigo_transport_type(event->transport_type),
            get_intercode_string_transition_type(event->transition));
        furi_string_cat_printf(
            parsed_data,
            "Transporter: %s\n",
            get_navigo_service_provider(event->service_provider));
        furi_string_cat_printf(parsed_data, "Station: %s\n", station);
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Gate: %d\n", event->location_gate);
        }
        if(event->device_available) {
            furi_string_cat_printf(parsed_data, "Device: %d\n", event->device);
        }
        if(event->mission_available) {
            furi_string_cat_printf(parsed_data, "Mission: %d\n", event->mission);
        }
        if(event->vehicle_id_available) {
            furi_string_cat_printf(parsed_data, "Vehicle: %d\n", event->vehicle_id);
        }
        if(event->used_contract_available) {
            furi_string_cat_printf(
                parsed_data,
                "Contract: %d - %s\n",
                event->used_contract,
                get_navigo_tariff(contracts[event->used_contract - 1].tariff));
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    }

    free(station);
    free(sector);
}

void show_navigo_special_event_info(NavigoCardSpecialEvent* event, FuriString* parsed_data) {
    char* station = get_navigo_station(
        event->station_group_id, event->station_id, event->station_sub_id, event->service_provider);
    char* sector = get_navigo_station(event->station_group_id, 0, 0, event->service_provider);

    if(event->transport_type == URBAN_BUS || event->transport_type == INTERURBAN_BUS ||
       event->transport_type == METRO || event->transport_type == TRAM) {
        if(event->route_number_available) {
            if(event->transport_type == METRO && event->route_number == 103) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s 3 bis\n%s\n",
                    get_navigo_transport_type(event->transport_type),
                    get_intercode_string_transition_type(event->transition));
            } else if(event->transport_type == TRAM) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s %s\n%s\n",
                    get_navigo_transport_type(event->transport_type),
                    get_navigo_tram_line(event->route_number),
                    get_intercode_string_transition_type(event->transition));
            } else {
                furi_string_cat_printf(
                    parsed_data,
                    "%s %d\n%s\n",
                    get_navigo_transport_type(event->transport_type),
                    event->route_number,
                    get_intercode_string_transition_type(event->transition));
            }
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s\n%s\n",
                get_navigo_transport_type(event->transport_type),
                get_intercode_string_transition_type(event->transition));
        }
        furi_string_cat_printf(
            parsed_data, "Result: %s\n", get_intercode_string_event_result(event->result));
        furi_string_cat_printf(
            parsed_data,
            "Transporter: %s\n",
            get_navigo_service_provider(event->service_provider));
        furi_string_cat_printf(parsed_data, "Station: %s\nSector: %s\n", station, sector);
        if(event->device_available) {
            furi_string_cat_printf(parsed_data, "Device: %d\n", event->device);
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else if(event->transport_type == COMMUTER_TRAIN) {
        if(event->route_number_available) {
            furi_string_cat_printf(
                parsed_data,
                "RER %c\n%s\n",
                (65 + event->route_number - 17),
                get_intercode_string_transition_type(event->transition));
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s %s\n%s\n",
                get_navigo_transport_type(event->transport_type),
                get_navigo_sncf_train_line(event->station_group_id),
                get_intercode_string_transition_type(event->transition));
        }
        furi_string_cat_printf(
            parsed_data, "Result: %s\n", get_intercode_string_event_result(event->result));
        furi_string_cat_printf(
            parsed_data,
            "Transporter: %s\n",
            get_navigo_service_provider(event->service_provider));
        furi_string_cat_printf(parsed_data, "Station: %s\n", station);
        if(event->device_available) {
            if(event->service_provider == NAVIGO_PROVIDER_SNCF) {
                furi_string_cat_printf(parsed_data, "Device: %d\n", event->device & 0xFF);
            } else {
                furi_string_cat_printf(parsed_data, "Device: %d\n", event->device);
            }
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else {
        furi_string_cat_printf(
            parsed_data,
            "%s - %s\n",
            get_navigo_transport_type(event->transport_type),
            get_intercode_string_transition_type(event->transition));
        furi_string_cat_printf(
            parsed_data, "Result: %s\n", get_intercode_string_event_result(event->result));
        furi_string_cat_printf(
            parsed_data,
            "Transporter: %s\n",
            get_navigo_service_provider(event->service_provider));
        furi_string_cat_printf(parsed_data, "Station: %s\n", station);
        if(event->device_available) {
            furi_string_cat_printf(parsed_data, "Device: %d\n", event->device);
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    }

    free(station);
    free(sector);
}

void show_navigo_contract_info(NavigoCardContract* contract, FuriString* parsed_data) {
    furi_string_cat_printf(parsed_data, "Type: %s\n", get_navigo_tariff(contract->tariff));
    if(is_ticket_count_available(contract->tariff)) {
        furi_string_cat_printf(parsed_data, "Remaining Tickets: %d\n", contract->counter.count);
    }
    if(contract->serial_number_available) {
        furi_string_cat_printf(parsed_data, "TCN Number: %d\n", contract->serial_number);
    }
    if(contract->pay_method_available) {
        furi_string_cat_printf(
            parsed_data, "Payment Method: %s\n", get_pay_method(contract->pay_method));
    }
    if(contract->price_amount_available) {
        furi_string_cat_printf(parsed_data, "Amount: %.2f EUR\n", contract->price_amount);
    }
    if(contract->end_date_available) {
        furi_string_cat_printf(parsed_data, "Valid\nfrom: ");
        locale_format_datetime_cat(parsed_data, &contract->start_date, false);
        furi_string_cat_printf(parsed_data, "\nto: ");
        locale_format_datetime_cat(parsed_data, &contract->end_date, false);
        furi_string_cat_printf(parsed_data, "\n");
    } else {
        furi_string_cat_printf(parsed_data, "Valid from\n");
        locale_format_datetime_cat(parsed_data, &contract->start_date, false);
        furi_string_cat_printf(parsed_data, "\n");
    }
    if(contract->zones_available) {
        furi_string_cat_printf(parsed_data, "%s\n", get_zones(contract->zones));
    }
    furi_string_cat_printf(parsed_data, "Sold on: ");
    locale_format_datetime_cat(parsed_data, &contract->sale_date, false);
    furi_string_cat_printf(parsed_data, "\n");
    furi_string_cat_printf(
        parsed_data, "Sales Agent: %s\n", get_navigo_service_provider(contract->sale_agent));
    furi_string_cat_printf(parsed_data, "Sales Terminal: %d\n", contract->sale_device);
    if(contract->status == 1) {
        furi_string_cat_printf(parsed_data, "Status: OK\n");
    } else {
        furi_string_cat_printf(parsed_data, "Status: Unknown (%d)\n", contract->status);
    }
    furi_string_cat_printf(parsed_data, "Authenticity Code: %d\n", contract->authenticator);
}

void show_navigo_environment_info(NavigoCardEnv* environment, FuriString* parsed_data) {
    furi_string_cat_printf(
        parsed_data,
        "App Version: %s - v%d\n",
        get_intercode_version(environment->app_version),
        get_intercode_subversion(environment->app_version));
    furi_string_cat_printf(
        parsed_data, "Country: %s\n", get_country_string(environment->country_num));
    furi_string_cat_printf(
        parsed_data,
        "Network: %s\n",
        get_network_string(guess_card_type(environment->country_num, environment->network_num)));
    furi_string_cat_printf(parsed_data, "End of validity:\n");
    locale_format_datetime_cat(parsed_data, &environment->end_dt, false);
    furi_string_cat_printf(parsed_data, "\n");
}
