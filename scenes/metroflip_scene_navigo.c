#include "../metroflip_i.h"
#include <datetime.h>
#include <dolphin/dolphin.h>
#include <locale/locale.h>
#include "navigo.h"

#include <nfc/protocols/iso14443_4b/iso14443_4b_poller.h>

#define TAG "Metroflip:Scene:Navigo"

int eventSizes[] = {8,  24, 8, 8,   8,  8, 24, 16, 16, 8,  16, 16, 8,  16,
                    16, 8,  5, 240, 16, 8, 16, 16, 16, 16, 16, 5,  16, 5};

int* get_bit_positions(const char* binary_string, int* count) {
    int length = strlen(binary_string);
    int* positions = malloc(length * sizeof(int));
    int pos_index = 0;

    for(int i = 0; i < length; i++) {
        if(binary_string[length - 1 - i] == '1') {
            positions[pos_index++] = i - 1;
        }
    }

    *count = pos_index;
    return positions;
}

int is_event_present(int* array, int size, int number) {
    for(int i = 0; i < size; i++) {
        if(array[i] == number) {
            return 1;
        }
    }
    return 0;
}

int check_events(int* array, int size, int number) {
    int total = 0;

    for(int i = 0; i < size; i++) {
        if(array[i] < number) {
            total += eventSizes[array[i]];
        }
    }

    return total + 53;
}

int select_new_app(
    int new_app,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    Iso14443_4bPoller* iso14443_4b_poller,
    Metroflip* app,
    MetroflipPollerEventType* stage) {
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
    default:
        return "Unknown";
    }
}

const char* get_network(int network_num) {
    switch(network_num) {
    case 901:
        return "Ile-de-France Mobilites";
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
        return "Validation - entree";
    case 2:
        return "Validation - sortie";
    case 4:
        return "Controle volant (a bord)";
    case 5:
        return "Validation de test";
    case 6:
        return "Validation en correspondance - entree";
    case 7:
        return "Validation en correspondance - sortie";
    case 9:
        return "Annulation de validation";
    case 10:
        return "Validation - entree";
    case 13:
        return "Distribution";
    case 15:
        return "Invalidation";
    default: {
        char* transition_str = malloc(13);
        snprintf(transition_str, 13, "Unknown (%d)", transition);
        return transition_str;
    }
    }
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

void show_event_info(NavigoCardEvent* event, FuriString* parsed_data) {
    if(event->transport_type == BUS_URBAIN || event->transport_type == BUS_INTERURBAIN ||
       event->transport_type == METRO || event->transport_type == TRAM) {
        if(event->route_number_available) {
            if(event->transport_type == METRO && event->route_number == 103) {
                furi_string_cat_printf(
                    parsed_data,
                    "%s 3 bis\n%s\n",
                    get_transport_type(event->transport_type),
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
            parsed_data, "Transporteur : %s\n", get_service_provider(event->service_provider));
        if(event->transport_type == METRO) {
            furi_string_cat_printf(
                parsed_data,
                "Station : %s\nSecteur : %s\n",
                get_metro_station(event->station_group_id, event->station_id),
                get_metro_station(event->station_group_id, 0));
        } else {
            furi_string_cat_printf(
                parsed_data, "ID Station : %d-%d\n", event->station_group_id, event->station_id);
        }
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Passage : %d\n", event->location_gate);
        }
        if(event->device_available) {
            if(event->transport_type == BUS_URBAIN || event->transport_type == BUS_INTERURBAIN) {
                const char* side = event->side == 0 ? "droit" : "gauche";
                furi_string_cat_printf(parsed_data, "Porte : %d\nCote %s\n", event->door, side);
            } else {
                furi_string_cat_printf(parsed_data, "Equipement : %d\n", event->device);
            }
        }
        if(event->mission_available) {
            furi_string_cat_printf(parsed_data, "Mission : %d\n", event->mission);
        }
        if(event->vehicle_id_available) {
            furi_string_cat_printf(parsed_data, "Vehicule : %d\n", event->vehicle_id);
        }
        if(event->used_contract_available) {
            furi_string_cat_printf(parsed_data, "Contrat : %d\n", event->used_contract);
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    } else if(event->transport_type == TRAIN) {
        if(event->route_number_available) {
            furi_string_cat_printf(
                parsed_data,
                "RER %c\nStation : %s\n",
                (65 + event->route_number - 17),
                get_train_station(event->station_group_id, event->station_id));
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s %s\nStation : %s\n",
                get_transport_type(event->transport_type),
                get_train_line(event->station_group_id),
                get_train_station(event->station_group_id, event->station_id));
        }
        if(event->route_number_available) {
            furi_string_cat_printf(parsed_data, "Route : %d\n", event->route_number);
        }
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Passage : %d\n", event->location_gate);
        }
        if(event->device_available) {
            furi_string_cat_printf(parsed_data, "Equipement : %d\n", event->device);
        }
        if(event->mission_available) {
            furi_string_cat_printf(parsed_data, "Mission : %d\n", event->mission);
        }
        if(event->vehicle_id_available) {
            furi_string_cat_printf(parsed_data, "Vehicule : %d\n", event->vehicle_id);
        }
        if(event->used_contract_available) {
            furi_string_cat_printf(parsed_data, "Contrat : %d\n", event->used_contract);
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
            parsed_data, "Transporteur : %s\n", get_service_provider(event->service_provider));
        furi_string_cat_printf(
            parsed_data, "ID Station : %d-%d\n", event->station_group_id, event->station_id);
        if(event->location_gate_available) {
            furi_string_cat_printf(parsed_data, "Passage : %d\n", event->location_gate);
        }
        if(event->device_available) {
            furi_string_cat_printf(parsed_data, "Equipement : %d\n", event->device);
        }
        if(event->mission_available) {
            furi_string_cat_printf(parsed_data, "Mission : %d\n", event->mission);
        }
        if(event->vehicle_id_available) {
            furi_string_cat_printf(parsed_data, "Vehicule : %d\n", event->vehicle_id);
        }
        if(event->used_contract_available) {
            furi_string_cat_printf(parsed_data, "Contrat : %d\n", event->used_contract);
        }
        locale_format_datetime_cat(parsed_data, &event->date, true);
        furi_string_cat_printf(parsed_data, "\n");
    }
}

void show_contract_info(NavigoCardContract* contract, FuriString* parsed_data) {
    furi_string_cat_printf(parsed_data, "Balance : %.2f EUR\n", (double)contract->balance);
    furi_string_cat_printf(parsed_data, "Debut de validite:\n");
    locale_format_datetime_cat(parsed_data, &contract->start_dt, false);
    furi_string_cat_printf(parsed_data, "\n");
}

void show_environment_info(NavigoCardEnv* environment, FuriString* parsed_data) {
    furi_string_cat_printf(
        parsed_data, "Version de l'application : %d\n", environment->app_version);
    if(environment->country_num == 250) {
        furi_string_cat_printf(parsed_data, "Pays : France\n");
    } else {
        furi_string_cat_printf(parsed_data, "Pays : %d\n", environment->country_num);
    }
    if(environment->network_num == 901) {
        furi_string_cat_printf(parsed_data, "Reseau : Ile-de-France Mobilites\n");
    } else {
        furi_string_cat_printf(parsed_data, "Reseau : %d\n", environment->network_num);
    }
    furi_string_cat_printf(parsed_data, "Fin de validite:\n");
    locale_format_datetime_cat(parsed_data, &environment->end_dt, false);
    furi_string_cat_printf(parsed_data, "\n");
}

void update_page_info(NavigoContext* ctx, FuriString* parsed_data) {
    if(ctx->page_id == 0) {
        furi_string_cat_printf(parsed_data, "\e#Navigo :\n");
        furi_string_cat_printf(parsed_data, "\e#Contrat 1:\n");
        show_contract_info(&ctx->card->contracts[0], parsed_data);
    } else if(ctx->page_id == 1) {
        furi_string_cat_printf(parsed_data, "\e#Environnement :\n");
        show_environment_info(&ctx->card->environment, parsed_data);
    } else if(ctx->page_id == 2) {
        furi_string_cat_printf(parsed_data, "\e#Event 1 :\n");
        show_event_info(&ctx->card->events[0], parsed_data);
    } else if(ctx->page_id == 3) {
        furi_string_cat_printf(parsed_data, "\e#Event 2 :\n");
        show_event_info(&ctx->card->events[1], parsed_data);
    } else if(ctx->page_id == 4) {
        furi_string_cat_printf(parsed_data, "\e#Event 3 :\n");
        show_event_info(&ctx->card->events[2], parsed_data);
    }
}

void update_widget_elements(Widget* widget, NavigoContext* ctx, void* context) {
    if(ctx->page_id < 4) {
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
    NavigoContext* ctx = context;
    Metroflip* app = ctx->app;
    UNUSED(result);

    Widget* widget = app->widget;

    if(type == InputTypePress) {
        widget_reset(widget);

        FuriString* parsed_data = furi_string_alloc();

        FURI_LOG_I(TAG, "Page ID: %d -> %d", ctx->page_id, ctx->page_id - 1);

        if(ctx->page_id > 0) {
            ctx->page_id -= 1;
        }

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_page_info(ctx, parsed_data);
        furi_mutex_release(ctx->mutex);

        widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_widget_elements(widget, ctx, context);
        furi_mutex_release(ctx->mutex);

        furi_string_free(parsed_data);
    }
}

void metroflip_next_button_widget_callback(GuiButtonType result, InputType type, void* context) {
    NavigoContext* ctx = context;
    Metroflip* app = ctx->app;
    UNUSED(result);

    Widget* widget = app->widget;

    if(type == InputTypePress) {
        widget_reset(widget);

        FuriString* parsed_data = furi_string_alloc();

        FURI_LOG_I(TAG, "Page ID: %d -> %d", ctx->page_id, ctx->page_id + 1);

        if(ctx->page_id < 4) {
            ctx->page_id += 1;
        } else {
            ctx->page_id = 0;
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, MetroflipSceneStart);
        }

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_page_info(ctx, parsed_data);
        furi_mutex_release(ctx->mutex);

        widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

        // Ensure no nested mutexes
        furi_mutex_acquire(ctx->mutex, FuriWaitForever);
        update_widget_elements(widget, ctx, context);
        furi_mutex_release(ctx->mutex);

        furi_string_free(parsed_data);
    }
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
            nfc_device_set_data(
                app->nfc_device, NfcProtocolIso14443_4b, nfc_poller_get_data(app->poller));

            Iso14443_4bError error;
            size_t response_length = 0;

            do {
                NavigoCardData* card = malloc(sizeof(NavigoCardData));

                // Initialize the card
                card->contracts = malloc(sizeof(NavigoCardContract));
                card->events = malloc(3 * sizeof(NavigoCardEvent));

                // Select app for contract 1
                error =
                    select_new_app(0x20, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
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

                char bit_representation[response_length * 8 + 1];
                bit_representation[0] = '\0';
                for(size_t i = 0; i < response_length; i++) {
                    char bits[9];
                    uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                    byte_to_binary(byte, bits);
                    strlcat(bit_representation, bits, sizeof(bit_representation));
                }
                bit_representation[response_length * 8] = '\0';
                int start = 55, end = 70;
                float decimal_value = bit_slice_to_dec(bit_representation, start, end);
                card->contracts[0].balance = decimal_value / 100;
                start = 80, end = 93;
                decimal_value = bit_slice_to_dec(bit_representation, start, end);
                uint64_t start_date_timestamp = (decimal_value * 24 * 3600) + (float)epoch + 3600;
                datetime_timestamp_to_datetime(start_date_timestamp, &card->contracts[0].start_dt);

                // Select app for environment
                error = select_new_app(0x1, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
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
                start = 0;
                end = 5;
                card->environment.app_version =
                    bit_slice_to_dec(environment_bit_representation, start, end);
                start = 13;
                end = 36;
                decimal_value = bit_slice_to_dec(environment_bit_representation, start, end);
                FURI_LOG_I(TAG, "Network ID: %d", (int)decimal_value);
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
                decimal_value = bit_slice_to_dec(environment_bit_representation, start, end);
                uint64_t end_validity_timestamp =
                    (decimal_value * 24 * 3600) + (float)epoch + 3600;
                datetime_timestamp_to_datetime(end_validity_timestamp, &card->environment.end_dt);

                // Select app for events
                error =
                    select_new_app(0x10, tx_buffer, rx_buffer, iso14443_4b_poller, app, &stage);
                if(error != 0) {
                    break;
                }

                // Check the response after selecting app
                if(check_response(rx_buffer, app, &stage, &response_length) != 0) {
                    break;
                }

                // furi_string_cat_printf(parsed_data, "\e#Events :\n");
                // Now send the read command
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
                    FURI_LOG_I(
                        TAG, "Event %d bit_representation: %s", i, event_bit_representation);

                    // furi_string_cat_printf(parsed_data, "Event 0%d :\n", i);
                    int count = 0;
                    int start = 25, end = 53;
                    char bit_slice[end - start + 2];
                    strncpy(bit_slice, event_bit_representation + start, end - start + 1);
                    bit_slice[end - start + 1] = '\0';
                    int* positions = get_bit_positions(bit_slice, &count);
                    /*FURI_LOG_I(TAG, "Positions: ");
                    for(int i = 0; i < count; i++) {
                        FURI_LOG_I(TAG, "%d ", positions[i]);
                    }*/

                    // 2. EventCode
                    // 8 bits
                    int event_number = 2;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        int start = positionOffset, end = positionOffset + 7;
                        int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].transport_type = decimal_value >> 4;
                        card->events[i - 1].transition = decimal_value & 15;
                        FURI_LOG_I(
                            TAG,
                            "%s - %s",
                            TRANSPORT_LIST[card->events[i - 1].transport_type],
                            TRANSITION_LIST[card->events[i - 1].transition]);
                    }

                    // 4. EventServiceProvider
                    // 8 bits
                    event_number = 4;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 7;
                        card->events[i - 1].service_provider =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        FURI_LOG_I(
                            TAG,
                            "Transporteur : %s",
                            SERVICE_PROVIDERS[card->events[i - 1].service_provider]);
                    }

                    // 8. EventLocationId
                    // 16 bits
                    event_number = 8;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 15;
                        int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].station_group_id = decimal_value >> 9;
                        card->events[i - 1].station_id = (decimal_value >> 4) & 31;
                        if(card->events[i - 1].transport_type == METRO) {
                            FURI_LOG_I(
                                TAG,
                                "Secteur %s - Station %s",
                                METRO_STATION_LIST[card->events[i - 1].station_group_id][0],
                                METRO_STATION_LIST[card->events[i - 1].station_group_id]
                                                  [card->events[i - 1].station_id]);
                        } else if(card->events[i - 1].transport_type == TRAIN) {
                            FURI_LOG_I(
                                TAG,
                                "Ligne %s - Station %s",
                                TRAIN_LINES_LIST[card->events[i - 1].station_group_id],
                                TRAIN_STATION_LIST[card->events[i - 1].station_group_id]
                                                  [card->events[i - 1].station_id]);
                        } else {
                            FURI_LOG_I(
                                TAG,
                                "Groupe ID %d - Station ID %d",
                                card->events[i - 1].station_group_id,
                                card->events[i - 1].station_id);
                        }
                    }

                    // 9. EventLocationGate
                    // 8 bits
                    event_number = 9;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 7;
                        card->events[i - 1].location_gate =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].location_gate_available = true;
                        FURI_LOG_I(TAG, "Passage : %d", card->events[i - 1].location_gate);
                    }

                    // 10. EventDevice
                    // 16 bits
                    event_number = 10;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 15;
                        int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].device = decimal_value >> 8;
                        card->events[i - 1].door = card->events[i - 1].device / 2 + 1;
                        card->events[i - 1].side = card->events[i - 1].device % 2;
                        card->events[i - 1].device_available = true;
                        const char* side = card->events[i - 1].side == 0 ? "droit" : "gauche";
                        FURI_LOG_I(TAG, "Equipement : %d", card->events[i - 1].device);
                        FURI_LOG_I(TAG, "Porte : %d - Côté %s", card->events[i - 1].door, side);
                    }

                    // 11. EventRouteNumber
                    // 16 bits
                    event_number = 11;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 15;
                        card->events[i - 1].route_number =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].route_number_available = true;
                        FURI_LOG_I(TAG, "Route : %d", card->events[i - 1].route_number);
                    }

                    // 13. EventJourneyRun
                    // 16 bits
                    event_number = 13;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 15;
                        card->events[i - 1].mission =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].mission_available = true;
                        FURI_LOG_I(TAG, "Mission : %d", card->events[i - 1].mission);
                    }

                    // 14. EventVehicleId
                    // 16 bits
                    event_number = 14;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 15;
                        card->events[i - 1].vehicle_id =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].vehicle_id_available = true;
                        FURI_LOG_I(TAG, "Vehicule : %d", card->events[i - 1].vehicle_id);
                    }

                    // 25. EventContractPointer
                    // 5 bits
                    event_number = 25;
                    if(is_event_present(positions, count, event_number)) {
                        int positionOffset = check_events(positions, count, event_number);
                        start = positionOffset, end = positionOffset + 4;
                        card->events[i - 1].used_contract =
                            bit_slice_to_dec(event_bit_representation, start, end);
                        card->events[i - 1].used_contract_available = true;
                        FURI_LOG_I(TAG, "Contrat : %d", card->events[i - 1].used_contract);
                    }

                    free(positions);

                    // EventDate
                    // 14 bits
                    start = 0, end = 13;
                    int decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                    uint64_t date_timestamp = (decimal_value * 24 * 3600) + epoch + 3600;
                    datetime_timestamp_to_datetime(date_timestamp, &card->events[i - 1].date);

                    // EventTime
                    // 11 bits
                    start = 14, end = 24;
                    decimal_value = bit_slice_to_dec(event_bit_representation, start, end);
                    card->events[i - 1].date.hour = (decimal_value * 60) / 3600;
                    card->events[i - 1].date.minute = ((decimal_value * 60) % 3600) / 60;
                    card->events[i - 1].date.second = ((decimal_value * 60) % 3600) % 60;
                    FURI_LOG_I(
                        TAG,
                        "Date : %02d/%02d/%04d %02dh%02d",
                        card->events[i - 1].date.day,
                        card->events[i - 1].date.month,
                        card->events[i - 1].date.year,
                        card->events[i - 1].date.hour,
                        card->events[i - 1].date.minute);
                }
                UNUSED(TRANSITION_LIST);
                UNUSED(TRANSPORT_LIST);
                UNUSED(SERVICE_PROVIDERS);

                widget_add_text_scroll_element(
                    widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

                NavigoContext* context = malloc(sizeof(NavigoContext));
                context->app = app;
                context->card = card;
                context->page_id = 0;
                context->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

                // Ensure no nested mutexes
                furi_mutex_acquire(context->mutex, FuriWaitForever);
                update_page_info(context, parsed_data);
                furi_mutex_release(context->mutex);

                widget_add_text_scroll_element(
                    widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

                // Ensure no nested mutexes
                furi_mutex_acquire(context->mutex, FuriWaitForever);
                update_widget_elements(widget, context, context);
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
}
