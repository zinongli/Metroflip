#include "../metroflip_i.h"
#include <datetime.h>
#include <dolphin/dolphin.h>
#include <notification/notification_messages.h>
#include <locale/locale.h>
#include "navigo.h"

#include <nfc/protocols/iso14443_4b/iso14443_4b_poller.h>

#define TAG "Metroflip:Scene:Navigo"

int select_new_app(
    int new_app_directory,
    int new_app,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    Iso14443_4bPoller* iso14443_4b_poller,
    Metroflip* app,
    MetroflipPollerEventType* stage) {
    select_app[5] = new_app_directory;
    select_app[6] = new_app;

    bit_buffer_reset(tx_buffer);
    bit_buffer_append_bytes(tx_buffer, select_app, sizeof(select_app));
    int error = iso14443_4b_poller_send_block(iso14443_4b_poller, tx_buffer, rx_buffer);
    if(error != Iso14443_4bErrorNone) {
        FURI_LOG_I(TAG, "Select File: iso14443_4b_poller_send_block error %d", error);
        *stage = MetroflipPollerEventTypeFail;
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventPollerFail);
        return error;
    }
    return 0;
}

int read_new_file(
    int new_file,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    Iso14443_4bPoller* iso14443_4b_poller,
    Metroflip* app,
    MetroflipPollerEventType* stage) {
    read_file[2] = new_file;
    bit_buffer_reset(tx_buffer);
    bit_buffer_append_bytes(tx_buffer, read_file, sizeof(read_file));
    Iso14443_4bError error =
        iso14443_4b_poller_send_block(iso14443_4b_poller, tx_buffer, rx_buffer);
    if(error != Iso14443_4bErrorNone) {
        FURI_LOG_I(TAG, "Read File: iso14443_4b_poller_send_block error %d", error);
        *stage = MetroflipPollerEventTypeFail;
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventPollerFail);
        return error;
    }
    return 0;
}

int check_response(
    BitBuffer* rx_buffer,
    Metroflip* app,
    MetroflipPollerEventType* stage,
    size_t* response_length) {
    *response_length = bit_buffer_get_size_bytes(rx_buffer);
    if(bit_buffer_get_byte(rx_buffer, *response_length - 2) != apdu_success[0] ||
       bit_buffer_get_byte(rx_buffer, *response_length - 1) != apdu_success[1]) {
        FURI_LOG_I(
            TAG,
            "Select profile app/file failed: %02x%02x",
            bit_buffer_get_byte(rx_buffer, *response_length - 2),
            bit_buffer_get_byte(rx_buffer, *response_length - 1));
        *stage = MetroflipPollerEventTypeFail;
        view_dispatcher_send_custom_event(
            app->view_dispatcher, MetroflipCustomEventPollerFileNotFound);
        return 1;
    }
    return 0;
}

const char* get_country(int country_num) {
    switch(country_num) {
    case 250:
        return "France";
    case 124:
        return "Canada";
    default: {
        char* country = malloc(4 * sizeof(char));
        snprintf(country, 4, "%d", country_num);
        return country;
    }
    }
}

const char* get_network(int country_num, int network_num) {
    switch(country_num) {
    case 250:
        switch(network_num) {
        case 901:
            return "IDFM";
        default:
            return "Unknown";
        }
    case 124:
        switch(network_num) {
        case 1:
            return "STM";
        default:
            return "Unknown";
        }
    default:
        return "Unknown";
    }
}

const char* get_transport_type(int type) {
    switch(type) {
    case BUS_URBAIN:
        return "Bus Urbain";
    case BUS_INTERURBAIN:
        return "Bus Interurbain";
    case METRO:
        return "Metro";
    case TRAM:
        return "Tram";
    case TRAIN:
        return "Train";
    case PARKING:
        return "Parking";
    default:
        return "Unknown";
    }
}

const char* get_service_provider(int provider) {
    switch(provider) {
    case 2:
        return "SNCF";
    case 3:
        return "RATP";
    case 4:
        return "IDF Mobilites";
    case 10:
        return "IDF Mobilites";
    case 115:
        return "CSO (VEOLIA)";
    case 116:
        return "R'Bus (VEOLIA)";
    case 156:
        return "Phebus";
    case 175:
        return "RATP (Veolia Transport Nanterre)";
    default:
        return "Unknown";
    }
}

const char* get_transition_type(int transition) {
    switch(transition) {
    case 1:
        return "Entry";
    case 2:
        return "Exit";
    case 4:
        return "Controle volant (a bord)";
    case 5:
        return "Test validation";
    case 6:
        return "Interchange - Entry";
    case 7:
        return "Interchange - Exit";
    case 9:
        return "Validation cancelled";
    case 10:
        return "Entry";
    case 11:
        return "Exit";
    case 13:
        return "Distribution";
    case 15:
        return "Invalidation";
    default: {
        char* transition_str = malloc(6 * sizeof(char));
        snprintf(transition_str, 6, "%d", transition);
        return transition_str;
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

const char* get_tariff(int tariff) {
    switch(tariff) {
    case 0x0000:
        return "Navigo Mois"; // Theoric
    case 0x0001:
        return "Navigo Semaine"; // Theoric
    case 0x0002:
        return "Navigo Annuel";
    case 0x0003:
        return "Navigo Jour"; // Theoric
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
        return "Apple Pay";
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

const char* get_metro_station(int station_group_id, int station_id) {
    // Use NAVIGO_H constants
    if(station_group_id < 32 && station_id < 16) {
        return METRO_STATION_LIST[station_group_id][station_id];
    }
    // cast station_group_id-station_id to a string
    char* station = malloc(12 * sizeof(char));
    if(!station) {
        return "Unknown";
    }
    snprintf(station, 10, "%d-%d", station_group_id, station_id);
    return station;
}

const char* get_train_line(int station_group_id) {
    if(station_group_id < 77) {
        return TRAIN_LINES_LIST[station_group_id];
    }
    return "Unknown";
}

const char* get_train_station(int station_group_id, int station_id) {
    if(station_group_id < 77 && station_id < 19) {
        return TRAIN_STATION_LIST[station_group_id][station_id];
    }
    // cast station_group_id-station_id to a string
    char* station = malloc(12 * sizeof(char));
    if(!station) {
        return "Unknown";
    }
    snprintf(station, 10, "%d-%d", station_group_id, station_id);
    return station;
}

const char* get_tram_line(int route_number) {
    switch(route_number) {
    case 16:
        return "T6";
    default: {
        char* line = malloc(3 * sizeof(char));
        if(!line) {
            return "Unknown";
        }
        snprintf(line, 3, "T%d", route_number);
        return line;
    }
    }
}

void show_event_info(
    NavigoCardEvent* event,
    NavigoCardContract* contracts,
    FuriString* parsed_data) {
    if(event->used_contract == 0) {
        furi_string_cat_printf(parsed_data, "No event data\n");
        return;
    }
    if(event->transport_type == BUS_URBAIN || event->transport_type == BUS_INTERURBAIN ||
       event->transport_type == METRO || event->transport_type == TRAM) {
        if(event->route_number_available) {
            if(event->transport_type == METRO && event->route_number == 103) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s 3 bis\n%s\n",
                    get_transport_type(event->transport_type),
                    get_transition_type(event->transition));
            } else if(event->transport_type == TRAM) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s %s\n%s\n",
                    get_transport_type(event->transport_type),
                    get_tram_line(event->route_number),
                    get_transition_type(event->transition));
            } else {
                furi_string_cat_printf(
                    parsed_data,
                    "%s %d\n%s\n",
                    get_transport_type(event->transport_type),
                    event->route_number,
                    get_transition_type(event->transition));
            }
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s\n%s\n",
                get_transport_type(event->transport_type),
                get_transition_type(event->transition));
        }
        furi_string_cat_printf(
            parsed_data, "Transporter: %s\n", get_service_provider(event->service_provider));
        if(event->transport_type == METRO) {
            furi_string_cat_printf(
                parsed_data,
                "Station: %s\nSector: %s\n",
                get_metro_station(event->station_group_id, event->station_id),
                get_metro_station(event->station_group_id, 0));
        } else {
            furi_string_cat_printf(
                parsed_data, "Station ID: %d-%d\n", event->station_group_id, event->station_id);
        }
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Gate: %d\n", event->location_gate);
        }
        if(event->device_available) {
            if(event->transport_type == BUS_URBAIN || event->transport_type == BUS_INTERURBAIN) {
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
                get_tariff(contracts[event->used_contract - 1].tariff));
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else if(event->transport_type == TRAIN) {
        if(event->route_number_available) {
            furi_string_cat_printf(
                parsed_data,
                "RER %c\n%s\n",
                (65 + event->route_number - 17),
                get_transition_type(event->transition));
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s %s\n%s\n",
                get_transport_type(event->transport_type),
                get_train_line(event->station_group_id),
                get_transition_type(event->transition));
        }
        furi_string_cat_printf(
            parsed_data, "Transporter: %s\n", get_service_provider(event->service_provider));
        furi_string_cat_printf(
            parsed_data,
            "Station: %s\n",
            get_train_station(event->station_group_id, event->station_id));
        /* if(event->route_number_available) {
            furi_string_cat_printf(parsed_data, "Route: %d\n", event->route_number);
        } */
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Gate: %d\n", event->location_gate);
        }
        if(event->device_available) {
            if(event->service_provider == 2) {
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
                get_tariff(contracts[event->used_contract - 1].tariff));
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else {
        furi_string_cat_printf(
            parsed_data,
            "%s - %s\n",
            get_transport_type(event->transport_type),
            get_transition_type(event->transition));
        furi_string_cat_printf(
            parsed_data, "Transporter: %s\n", get_service_provider(event->service_provider));
        furi_string_cat_printf(
            parsed_data, "Station ID: %d-%d\n", event->station_group_id, event->station_id);
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
                get_tariff(contracts[event->used_contract - 1].tariff));
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    }
}

void show_contract_info(NavigoCardContract* contract, FuriString* parsed_data) {
    furi_string_cat_printf(parsed_data, "Type: %s\n", get_tariff(contract->tariff));
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
        parsed_data, "Sales Agent: %s\n", get_service_provider(contract->sale_agent));
    furi_string_cat_printf(parsed_data, "Sales Terminal: %d\n", contract->sale_device);
    furi_string_cat_printf(parsed_data, "Status: %d\n", contract->status);
    furi_string_cat_printf(parsed_data, "Authenticity Code: %d\n", contract->authenticator);
}

void show_environment_info(NavigoCardEnv* environment, FuriString* parsed_data) {
    furi_string_cat_printf(
        parsed_data,
        "App Version: %s - v%d\n",
        get_intercode_version(environment->app_version),
        get_intercode_subversion(environment->app_version));
    furi_string_cat_printf(parsed_data, "Country: %s\n", get_country(environment->country_num));
    furi_string_cat_printf(
        parsed_data,
        "Network: %s\n",
        get_network(environment->country_num, environment->network_num));
    furi_string_cat_printf(parsed_data, "End of validity:\n");
    locale_format_datetime_cat(parsed_data, &environment->end_dt, false);
    furi_string_cat_printf(parsed_data, "\n");
}

void update_page_info(void* context, FuriString* parsed_data) {
    Metroflip* app = context;
    NavigoContext* ctx = app->navigo_context;
    if(ctx->page_id == 0) {
        furi_string_cat_printf(
            parsed_data,
            "\e#%s %u:\n",
            get_navigo_type(ctx->card->holder.card_status),
            ctx->card->card_number);
        furi_string_cat_printf(parsed_data, "\e#Contract 1:\n");
        show_contract_info(&ctx->card->contracts[0], parsed_data);
    } else if(ctx->page_id == 1) {
        furi_string_cat_printf(
            parsed_data,
            "\e#%s %u:\n",
            get_navigo_type(ctx->card->holder.card_status),
            ctx->card->card_number);
        furi_string_cat_printf(parsed_data, "\e#Contract 2:\n");
        show_contract_info(&ctx->card->contracts[1], parsed_data);
    } else if(ctx->page_id == 2) {
        furi_string_cat_printf(
            parsed_data,
            "\e#%s %u:\n",
            get_navigo_type(ctx->card->holder.card_status),
            ctx->card->card_number);
        furi_string_cat_printf(parsed_data, "\e#Contract 3:\n");
        show_contract_info(&ctx->card->contracts[2], parsed_data);
    } else if(ctx->page_id == 3) {
        furi_string_cat_printf(
            parsed_data,
            "\e#%s %u:\n",
            get_navigo_type(ctx->card->holder.card_status),
            ctx->card->card_number);
        furi_string_cat_printf(parsed_data, "\e#Contract 4:\n");
        show_contract_info(&ctx->card->contracts[3], parsed_data);
    } else if(ctx->page_id == 4) {
        furi_string_cat_printf(parsed_data, "\e#Environment:\n");
        show_environment_info(&ctx->card->environment, parsed_data);
    } else if(ctx->page_id == 5) {
        furi_string_cat_printf(parsed_data, "\e#Event 1:\n");
        show_event_info(&ctx->card->events[0], ctx->card->contracts, parsed_data);
    } else if(ctx->page_id == 6) {
        furi_string_cat_printf(parsed_data, "\e#Event 2:\n");
        show_event_info(&ctx->card->events[1], ctx->card->contracts, parsed_data);
    } else if(ctx->page_id == 7) {
        furi_string_cat_printf(parsed_data, "\e#Event 3:\n");
        show_event_info(&ctx->card->events[2], ctx->card->contracts, parsed_data);
    }
}

void update_widget_elements(void* context) {
    Metroflip* app = context;
    NavigoContext* ctx = app->navigo_context;
    Widget* widget = app->widget;
    if(ctx->page_id < 5) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "Next", metroflip_next_button_widget_callback, context);
    } else {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "Exit", metroflip_next_button_widget_callback, context);
    }
    if(ctx->page_id > 0) {
        widget_add_button_element(
            widget, GuiButtonTypeLeft, "Back", metroflip_back_button_widget_callback, context);
    }
}

void metroflip_back_button_widget_callback(GuiButtonType result, InputType type, void* context) {
    Metroflip* app = context;
    NavigoContext* ctx = app->navigo_context;
    UNUSED(result);

    Widget* widget = app->widget;

    if(type == InputTypePress) {
        widget_reset(widget);

        FURI_LOG_I(TAG, "Page ID: %d -> %d", ctx->page_id, ctx->page_id - 1);

        if(ctx->page_id > 0) {
            if(ctx->page_id == 4 && ctx->card->contracts[3].present == 0) {
                ctx->page_id -= 1;
            }
            if(ctx->page_id == 3 && ctx->card->contracts[2].present == 0) {
                ctx->page_id -= 1;
            }
            if(ctx->page_id == 2 && ctx->card->contracts[1].present == 0) {
                ctx->page_id -= 1;
            }
            ctx->page_id -= 1;
        }

        FuriString* parsed_data = furi_string_alloc();

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_page_info(app, parsed_data);
        furi_mutex_release(ctx->mutex);

        widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));
        // widget_add_icon_element(widget, 0, 0, &I_RFIDDolphinReceive_97x61);

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_widget_elements(app);
        furi_mutex_release(ctx->mutex);

        furi_string_free(parsed_data);
    }
}

void metroflip_next_button_widget_callback(GuiButtonType result, InputType type, void* context) {
    Metroflip* app = context;
    NavigoContext* ctx = app->navigo_context;
    UNUSED(result);

    Widget* widget = app->widget;

    if(type == InputTypePress) {
        widget_reset(widget);

        FURI_LOG_I(TAG, "Page ID: %d -> %d", ctx->page_id, ctx->page_id + 1);

        if(ctx->page_id < 7) {
            if(ctx->page_id == 0 && ctx->card->contracts[1].present == 0) {
                ctx->page_id += 1;
            }
            if(ctx->page_id == 1 && ctx->card->contracts[2].present == 0) {
                ctx->page_id += 1;
            }
            if(ctx->page_id == 2 && ctx->card->contracts[3].present == 0) {
                ctx->page_id += 1;
            }
            ctx->page_id += 1;
        } else {
            ctx->page_id = 0;
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, MetroflipSceneStart);
            return;
        }

        FuriString* parsed_data = furi_string_alloc();

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_page_info(app, parsed_data);
        furi_mutex_release(ctx->mutex);

        widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_widget_elements(app);
        furi_mutex_release(ctx->mutex);

        furi_string_free(parsed_data);
    }
}

void delay(int milliseconds) {
    furi_thread_flags_wait(0, FuriFlagWaitAny, milliseconds);
}

static NfcCommand metroflip_scene_navigo_poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4b);
    NfcCommand next_command = NfcCommandContinue;
    MetroflipPollerEventType stage = MetroflipPollerEventTypeStart;

    Metroflip* app = context;
    FuriString* parsed_data = furi_string_alloc();
    Widget* widget = app->widget;
    furi_string_reset(app->text_box_store);

    const Iso14443_4bPollerEvent* iso14443_4b_event = event.event_data;

    Iso14443_4bPoller* iso14443_4b_poller = event.instance;

    BitBuffer* tx_buffer = bit_buffer_alloc(Metroflip_POLLER_MAX_BUFFER_SIZE);
    BitBuffer* rx_buffer = bit_buffer_alloc(Metroflip_POLLER_MAX_BUFFER_SIZE);

    if(iso14443_4b_event->type == Iso14443_4bPollerEventTypeReady) {
        if(stage == MetroflipPollerEventTypeStart) {
            // Start Flipper vibration
            NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
            notification_message(notification, &sequence_set_vibro_on);
            delay(50);
            notification_message(notification, &sequence_reset_vibro);
            nfc_device_set_data(
                app->nfc_device, NfcProtocolIso14443_4b, nfc_poller_get_data(app->poller));

            Iso14443_4bError error;
            size_t response_length = 0;

            do {
                // Initialize the card data
                NavigoCardData* card = malloc(sizeof(NavigoCardData));

                // Select app ICC
                error = select_new_app(
                    0x00, 0x02, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after selecting app
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                // Now send the read command for ICC
                error = read_new_file(0x01, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after reading the file
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                char icc_bit_representation[response_length * 8 + 1];
                icc_bit_representation[0] = '\0';
                for(size_t i = 0; i < response_length; i++) {
                    char bits[9];
                    uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                    byte_to_binary(byte, bits);
                    strlcat(icc_bit_representation, bits, sizeof(icc_bit_representation));
                }
                icc_bit_representation[response_length * 8] = '\0';

                FURI_LOG_I(TAG, "ICC bit representation: %s", icc_bit_representation);

                int start = 128, end = 159;
                card->card_number = bit_slice_to_dec(icc_bit_representation, start, end);

                // Select app for contracts
                error = select_new_app(
                    0x20, 0x20, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after selecting app
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                // Prepare calypso structure
                CalypsoApp* NavigoContractStructure = get_navigo_contract_structure();
                if(!NavigoContractStructure) {
                    FURI_LOG_E(TAG, "Failed to load Navigo Contract structure");
                    break;
                }

                // Now send the read command for contracts
                for(size_t i = 1; i < 5; i++) {
                    error =
                        read_new_file(i, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                    if(error != 0) {
                        break;
                    }

                    // Check the response after reading the file
                    if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                        break;
                    }

                    char bit_representation[response_length * 8 + 1];
                    bit_representation[0] = '\0';
                    for(size_t i = 0; i < response_length; i++) {
                        char bits[9];
                        uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                        byte_to_binary(byte, bits);
                        strlcat(bit_representation, bits, sizeof(bit_representation));
                    }
                    bit_representation[response_length * 8] = '\0';

                    /* int count = 0;
                    int start = 0, end = NavigoContractStructure->elements[0].bitmap->size;
                    char bit_slice[end - start + 1];
                    strncpy(bit_slice, bit_representation + start, end - start);
                    bit_slice[end - start] = '\0';
                    int* positions = get_bit_positions(bit_slice, &count);

                    FURI_LOG_I(TAG, "Contract %d bit positions: %d", i, count);

                    // print positions
                    for(int i = 0; i < count; i++) {
                        char* key =
                            (NavigoContractStructure->elements[0]
                                         .bitmap->elements[positions[i]]
                                         .type == CALYPSO_ELEMENT_TYPE_FINAL ?
                                 NavigoContractStructure->elements[0]
                                     .bitmap->elements[positions[i]]
                                     .final->key :
                                 NavigoContractStructure->elements[0]
                                     .bitmap->elements[positions[i]]
                                     .bitmap->key);
                        int offset = get_calypso_node_offset(
                            bit_representation, key, NavigoContractStructure);
                        FURI_LOG_I(
                            TAG, "Position: %d, Key: %s, Offset: %d", positions[i], key, offset);
                    } */

                    if(bit_slice_to_dec(
                           bit_representation,
                           0,
                           NavigoContractStructure->elements[0].bitmap->size - 1) == 0) {
                        break;
                    }

                    card->contracts[i - 1].present = 1;

                    // 2. ContractTariff
                    const char* contract_key = "ContractTariff";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].tariff =
                            bit_slice_to_dec(bit_representation, start, end);
                    }

                    // 3. ContractSerialNumber
                    contract_key = "ContractSerialNumber";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].serial_number =
                            bit_slice_to_dec(bit_representation, start, end);
                        card->contracts[i - 1].serial_number_available = true;
                    }

                    // 8. ContractPayMethod
                    contract_key = "ContractPayMethod";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].pay_method =
                            bit_slice_to_dec(bit_representation, start, end);
                        card->contracts[i - 1].pay_method_available = true;
                    }

                    // 10. ContractPriceAmount
                    contract_key = "ContractPriceAmount";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].price_amount =
                            bit_slice_to_dec(bit_representation, start, end) / 100.0;
                        card->contracts[i - 1].price_amount_available = true;
                    }

                    // 13.0. ContractValidityStartDate
                    contract_key = "ContractValidityStartDate";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        float decimal_value =
                            bit_slice_to_dec(bit_representation, start, end) * 24 * 3600;
                        uint64_t start_validity_timestamp = (decimal_value + (float)epoch) + 3600;
                        datetime_timestamp_to_datetime(
                            start_validity_timestamp, &card->contracts[i - 1].start_date);
                    }

                    // 13.2. ContractValidityEndDate
                    contract_key = "ContractValidityEndDate";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        float decimal_value =
                            bit_slice_to_dec(bit_representation, start, end) * 24 * 3600;
                        uint64_t end_validity_timestamp = (decimal_value + (float)epoch) + 3600;
                        datetime_timestamp_to_datetime(
                            end_validity_timestamp, &card->contracts[i - 1].end_date);
                        card->contracts[i - 1].end_date_available = true;
                    }

                    // 13.6. ContractValidityZones
                    contract_key = "ContractValidityZones";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int start = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        // binary form is 00011111 for zones 5, 4, 3, 2, 1
                        for(int j = 0; j < 5; j++) {
                            card->contracts[i - 1].zones[j] =
                                bit_slice_to_dec(bit_representation, start + 3 + j, start + 3 + j);
                        }
                        card->contracts[i - 1].zones_available = true;
                    }

                    // 13.7. ContractValidityJourneys  -- pas sûr de le mettre lui

                    // 15.0. ContractValiditySaleDate
                    contract_key = "ContractValiditySaleDate";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        float decimal_value =
                            bit_slice_to_dec(bit_representation, start, end) * 24 * 3600;
                        uint64_t sale_timestamp = (decimal_value + (float)epoch) + 3600;
                        datetime_timestamp_to_datetime(
                            sale_timestamp, &card->contracts[i - 1].sale_date);
                    }

                    // 15.2. ContractValiditySaleAgent - FIX NEEDED
                    contract_key = "ContractValiditySaleAgent";
                    /* if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) { */
                    int positionOffset = get_calypso_node_offset(
                        bit_representation, contract_key, NavigoContractStructure);
                    int start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                    card->contracts[i - 1].sale_agent =
                        bit_slice_to_dec(bit_representation, start, end);
                    // }

                    // 15.3. ContractValiditySaleDevice
                    contract_key = "ContractValiditySaleDevice";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].sale_device =
                            bit_slice_to_dec(bit_representation, start, end);
                    }

                    // 16. ContractStatus  -- 0x1 ou 0xff
                    contract_key = "ContractStatus";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].status =
                            bit_slice_to_dec(bit_representation, start, end);
                    }

                    // 18. ContractAuthenticator
                    contract_key = "ContractAuthenticator";
                    if(is_calypso_node_present(
                           bit_representation, contract_key, NavigoContractStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            bit_representation, contract_key, NavigoContractStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(contract_key, NavigoContractStructure) - 1;
                        card->contracts[i - 1].authenticator =
                            bit_slice_to_dec(bit_representation, start, end);
                    }
                }

                // Free the calypso structure
                free_calypso_structure(NavigoContractStructure);

                // Select app for environment
                error = select_new_app(
                    0x20, 0x1, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after selecting app
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                // read file 1
                error = read_new_file(1, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after reading the file
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                char environment_bit_representation[response_length * 8 + 1];
                environment_bit_representation[0] = '\0';
                for(size_t i = 0; i < response_length; i++) {
                    char bits[9];
                    uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                    byte_to_binary(byte, bits);
                    strlcat(
                        environment_bit_representation,
                        bits,
                        sizeof(environment_bit_representation));
                }
                // FURI_LOG_I(
                //     TAG, "Environment bit_representation: %s", environment_bit_representation);
                start = 0;
                end = 5;
                card->environment.app_version =
                    bit_slice_to_dec(environment_bit_representation, start, end);
                start = 13;
                end = 16;
                card->environment.country_num =
                    bit_slice_to_dec(environment_bit_representation, start, end) * 100 +
                    bit_slice_to_dec(environment_bit_representation, start + 4, end + 4) * 10 +
                    bit_slice_to_dec(environment_bit_representation, start + 8, end + 8);
                start = 25;
                end = 28;
                card->environment.network_num =
                    bit_slice_to_dec(environment_bit_representation, start, end) * 100 +
                    bit_slice_to_dec(environment_bit_representation, start + 4, end + 4) * 10 +
                    bit_slice_to_dec(environment_bit_representation, start + 8, end + 8);
                start = 45;
                end = 58;
                float decimal_value = bit_slice_to_dec(environment_bit_representation, start, end);
                uint64_t end_validity_timestamp =
                    (decimal_value * 24 * 3600) + (float)epoch + 3600;
                datetime_timestamp_to_datetime(end_validity_timestamp, &card->environment.end_dt);

                start = 95;
                end = 98;
                card->holder.card_status =
                    bit_slice_to_dec(environment_bit_representation, start, end);

                start = 99;
                end = 104;
                card->holder.commercial_id =
                    bit_slice_to_dec(environment_bit_representation, start, end);

                // Select app for counters (remaining tickets on Navigo Easy)
                error = select_new_app(
                    0x20, 0x69, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after selecting app
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                // read file 1
                error = read_new_file(1, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after reading the file
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                char counter_bit_representation[response_length * 8 + 1];
                counter_bit_representation[0] = '\0';
                for(size_t i = 0; i < response_length; i++) {
                    char bits[9];
                    uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                    byte_to_binary(byte, bits);
                    strlcat(counter_bit_representation, bits, sizeof(counter_bit_representation));
                }
                // FURI_LOG_I(TAG, "Counter bit_representation: %s", counter_bit_representation);

                // Ticket counts (contracts 1-4)
                for(int i = 0; i < 4; i++) {
                    start = 0;
                    end = 5;
                    card->contracts[i].counter.count =
                        bit_slice_to_dec(counter_bit_representation, 24 * i + start, 24 * i + end);

                    start = 6;
                    end = 23;
                    card->contracts[i].counter.relative_first_stamp_15mn =
                        bit_slice_to_dec(counter_bit_representation, 24 * i + start, 24 * i + end);
                }

                // Select app for events
                error = select_new_app(
                    0x20, 0x10, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after selecting app
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                // Load the calypso structure for events
                CalypsoApp* NavigoEventStructure = get_navigo_event_structure();
                if(!NavigoEventStructure) {
                    FURI_LOG_E(TAG, "Failed to load Navigo Event structure");
                    break;
                }

                // furi_string_cat_printf(parsed_data, "\e#Events :\n");
                // Now send the read command for events
                for(size_t i = 1; i < 4; i++) {
                    error =
                        read_new_file(i, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                    if(error != 0) {
                        break;
                    }

                    // Check the response after reading the file
                    if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                        break;
                    }

                    char event_bit_representation[response_length * 8 + 1];
                    event_bit_representation[0] = '\0';
                    for(size_t i = 0; i < response_length; i++) {
                        char bits[9];
                        uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                        byte_to_binary(byte, bits);
                        strlcat(event_bit_representation, bits, sizeof(event_bit_representation));
                    }

                    // furi_string_cat_printf(parsed_data, "Event 0%d :\n", i);
                    /* int count = 0;
                    int start = 25, end = 52;
                    char bit_slice[end - start + 2];
                    strncpy(bit_slice, event_bit_representation + start, end - start + 1);
                    bit_slice[end - start + 1] = '\0';
                    int* positions = get_bit_positions(bit_slice, &count);
                    FURI_LOG_I(TAG, "Positions: ");
                    for(int i = 0; i < count; i++) {
                        FURI_LOG_I(TAG, "%d ", positions[i]);
                    } */

                    // 2. EventCode
                    const char* event_key = "EventCode";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        int start = positionOffset,
                            end = positionOffset +
                                  get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].transport_type = decimal_value >> 4;
                        card->events[i - 1].transition = decimal_value & 15;
                    }

                    // 4. EventServiceProvider
                    event_key = "EventServiceProvider";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        card->events[i - 1].service_provider =
                            bit_slice_to_dec(event_bit_representation, start, end);
                    }

                    // 8. EventLocationId
                    event_key = "EventLocationId";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].station_group_id = decimal_value >> 9;
                        card->events[i - 1].station_id = (decimal_value >> 4) & 31;
                    }

                    // 9. EventLocationGate
                    event_key = "EventLocationGate";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        card->events[i - 1].location_gate =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].location_gate_available = true;
                    }

                    // 10. EventDevice
                    event_key = "EventDevice";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].device = decimal_value;
                        int bus_device = decimal_value >> 8;
                        card->events[i - 1].door = bus_device / 2 + 1;
                        card->events[i - 1].side = bus_device % 2;
                        card->events[i - 1].device_available = true;
                    }

                    // 11. EventRouteNumber
                    event_key = "EventRouteNumber";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        card->events[i - 1].route_number =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].route_number_available = true;
                    }

                    // 13. EventJourneyRun
                    event_key = "EventJourneyRun";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        card->events[i - 1].mission =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].mission_available = true;
                    }

                    // 14. EventVehicleId
                    event_key = "EventVehicleId";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        card->events[i - 1].vehicle_id =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].vehicle_id_available = true;
                    }

                    // 25. EventContractPointer
                    event_key = "EventContractPointer";
                    if(is_calypso_node_present(
                           event_bit_representation, event_key, NavigoEventStructure)) {
                        int positionOffset = get_calypso_node_offset(
                            event_bit_representation, event_key, NavigoEventStructure);
                        start = positionOffset,
                        end = positionOffset +
                              get_calypso_node_size(event_key, NavigoEventStructure) - 1;
                        card->events[i - 1].used_contract =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].used_contract_available = true;
                    }

                    // EventDateStamp
                    event_key = "EventDateStamp";
                    int positionOffset = get_calypso_node_offset(
                        event_bit_representation, event_key, NavigoEventStructure);
                    start = positionOffset,
                    end = positionOffset + get_calypso_node_size(event_key, NavigoEventStructure) -
                          1;
                    int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                    uint64_t date_timestamp = (decimal_value * 24 * 3600) + epoch + 3600;
                    datetime_timestamp_to_datetime(date_timestamp, &card->events[i - 1].date);

                    // EventTimeStamp
                    event_key = "EventTimeStamp";
                    positionOffset = get_calypso_node_offset(
                        event_bit_representation, event_key, NavigoEventStructure);
                    start = positionOffset,
                    end = positionOffset + get_calypso_node_size(event_key, NavigoEventStructure) -
                          1;
                    decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                    card->events[i - 1].date.hour = (decimal_value * 60) / 3600;
                    card->events[i - 1].date.minute = ((decimal_value * 60) % 3600) / 60;
                    card->events[i - 1].date.second = ((decimal_value * 60) % 3600) % 60;
                }

                // Free the calypso structure
                free_calypso_structure(NavigoEventStructure);

                UNUSED(TRANSITION_LIST);
                UNUSED(TRANSPORT_LIST);
                UNUSED(SERVICE_PROVIDERS);

                widget_add_text_scroll_element(
                    widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

                NavigoContext* context = malloc(sizeof(NavigoContext));
                context->card = card;
                context->page_id = 0;
                context->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
                app->navigo_context = context;

                // Ensure no nested mutexes
                furi_mutex_acquire(context->mutex, FuriWaitForever);
                update_page_info(app, parsed_data);
                furi_mutex_release(context->mutex);

                widget_add_text_scroll_element(
                    widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

                // Ensure no nested mutexes
                furi_mutex_acquire(context->mutex, FuriWaitForever);
                update_widget_elements(app);
                furi_mutex_release(context->mutex);

                furi_string_free(parsed_data);
                view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
                metroflip_app_blink_stop(app);
                stage = MetroflipPollerEventTypeSuccess;
                next_command = NfcCommandStop;
            } while(false);

            if(stage != MetroflipPollerEventTypeSuccess) {
                next_command = NfcCommandStop;
            }
        }
    }
    bit_buffer_free(tx_buffer);
    bit_buffer_free(rx_buffer);

    return next_command;
}

void metroflip_scene_navigo_on_enter(void* context) {
    Metroflip* app = context;
    dolphin_deed(DolphinDeedNfcRead);

    // Setup view
    Popup* popup = app->popup;
    popup_set_header(popup, "Apply\n card to\nthe back", 68, 30, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    // Start worker
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewPopup);
    nfc_scanner_alloc(app->nfc);
    app->poller = nfc_poller_alloc(app->nfc, NfcProtocolIso14443_4b);
    nfc_poller_start(app->poller, metroflip_scene_navigo_poller_callback, app);

    metroflip_app_blink_start(app);
}

bool metroflip_scene_navigo_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MetroflipPollerEventTypeCardDetect) {
            Popup* popup = app->popup;
            popup_set_header(popup, "Scanning..", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventPollerFileNotFound) {
            Popup* popup = app->popup;
            popup_set_header(popup, "Read Error,\n wrong card", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventPollerFail) {
            Popup* popup = app->popup;
            popup_set_header(popup, "Error, try\n again", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        consumed = true;
    }

    return consumed;
}

void metroflip_scene_navigo_on_exit(void* context) {
    Metroflip* app = context;

    if(app->poller) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }
    metroflip_app_blink_stop(app);
    widget_reset(app->widget);

    // Clear view
    popup_reset(app->popup);

    if(app->navigo_context) {
        NavigoContext* ctx = app->navigo_context;
        free(ctx->card);
        furi_mutex_free(ctx->mutex);
        free(ctx);
        app->navigo_context = NULL;
    }
}
