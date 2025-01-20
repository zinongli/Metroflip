/*
 * Suica Scene
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../metroflip_i.h"
#include "suica.h"
#include <flipper_application.h>

#include <lib/nfc/protocols/felica/felica.h>
#include <lib/nfc/protocols/felica/felica_poller.h>
#include <lib/nfc/protocols/felica/felica_poller_i.h>
#include <lib/nfc/helpers/felica_crc.h>
#include <lib/bit_lib/bit_lib.h>

#include <applications/services/locale/locale.h>
#include <datetime.h>

#define SERVICE_CODE_HISTORY_IN_LE  (0x090FU)
#define SERVICE_CODE_TAPS_LOG_IN_LE (0x108FU)
#define BLOCK_COUNT                 1
#define HISTORY_VIEW_PAGE_NUM       3
#define TAG                         "Metroflip:Scene:Suica"
#define TERMINAL_NULL               0x02
#define TERMINAL_BUS                0x05
#define TERMINAL_POS_AND_TAXI       0xC7
#define TERMINAL_VENDING_MACHINE    0xC8

const char* suica_service_names[] = {
    "Travel History",
    "Taps Log",
};

static void suica_model_initialize(SuicaHistoryViewModel* model, size_t initial_capacity) {
    model->travel_history = (uint8_t*)malloc(initial_capacity * 16); // Each entry is 16 bytes
    model->size = 0;
    model->capacity = initial_capacity;
}

static void suica_add_entry(SuicaHistoryViewModel* model, const uint8_t* entry) {
    // Check if resizing is needed
    if(model->size == model->capacity) {
        size_t new_capacity = model->capacity * 2; // Double the capacity
        uint8_t* new_data = (uint8_t*)realloc(model->travel_history, new_capacity * 16);
        model->travel_history = new_data;
        model->capacity = new_capacity;
    }

    // Copy the 16-byte entry to the next slot
    uint8_t* target = model->travel_history + (model->size * 16);
    for(size_t i = 0; i < 16; i++) {
        target[i] = entry[i];
    }

    model->size++;
}

static SuicaTravelHistory suica_parse(uint8_t block[16]) {
    SuicaTravelHistory history;
    switch((uint8_t)block[0]) {
    case TERMINAL_NULL:
        history.history_type = SuicaHistoryNull;
        break;
    case TERMINAL_BUS:
        // 6 & 7 bus line code
        // 8 & 9 bus stop code
        history.history_type = SuicaHistoryBus;
        break;
    case TERMINAL_POS_AND_TAXI:
        history.history_type = SuicaHistoryPosAndTaxi;
        break;
    case TERMINAL_VENDING_MACHINE:
        // 6 & 7 are hour and minute
        history.history_type = SuicaHistoryVendingMachine;
        history.hour = ((uint8_t)block[6] & 0xF8) >> 3;
        history.minute = (((uint8_t)block[6] & 0x07) << 3) | (((uint8_t)block[7] & 0xE0) >> 5);
        break;
    default:
        // Train rides
        history.history_type = SuicaHistoryTrain;
        history.entry_line = block[6];
        history.entry_station = block[7];
        if(block[0] > 0x15) {
            history.exit_line = block[8];
            history.exit_station = block[9];
            history.rail_region = block[15];
        };
        break;
    }
    history.year = ((uint8_t)block[4] & 0xFE) >> 1;
    history.month = (((uint8_t)block[4] & 0x01) << 3) | (((uint8_t)block[5] & 0xE0) >> 5);
    history.day = (uint8_t)block[5] & 0x1F;
    history.balance = ((uint16_t)block[10] << 8) | (uint16_t)block[11];
    return history;
} 

static void suica_history_draw_callback(Canvas* canvas, void* model) {
    canvas_set_bitmap_mode(canvas, true);
    SuicaHistoryViewModel* my_model = (SuicaHistoryViewModel*)model;
    // catch the case where the page and entry are not initialized
    if (my_model->page < 1 || my_model->page > HISTORY_VIEW_PAGE_NUM) {
        my_model->page = 1;
    }
    if (my_model->entry >= my_model->size || my_model->entry < 1) {
        my_model->entry = 1;
    }
    uint8_t current_block[FELICA_DATA_BLOCK_SIZE];
    FuriString* buffer = furi_string_alloc();

    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
        current_block[i] = my_model->travel_history[((my_model->entry-1) * 16) + i];
    }
    SuicaTravelHistory history = suica_parse(current_block);

    // Main title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 8, "Suica");

    // Entry Num
    furi_string_printf(buffer, "%02d/%02d", my_model->entry, my_model->size+1);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 98, 8, furi_string_get_cstr(buffer));

    if(history.history_type == SuicaHistoryTrain) {

        switch((uint8_t)my_model->page) {
        case 0:
            // Date
            furi_string_printf(buffer, "%04d-%02d-%02d", history.year, history.month, history.day);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(buffer));

            canvas_draw_circle(canvas, 33, 40, 13);
            canvas_draw_circle(canvas, 98, 40, 13);
            canvas_draw_circle(canvas, 33, 40, 16);
            canvas_draw_circle(canvas, 98, 40, 16);
            
            furi_string_set(buffer, RailwayShort[history.entry_line]);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 27, 40, furi_string_get_cstr(buffer));
            // furi_string_set(buffer, RailwayLong[history.entry_line][history.entry_station][0]);
            // furi_string_cat(buffer, RailwayLong[history.entry_line][history.entry_station][1]);
            furi_string_printf(buffer, "%02d", history.entry_station + 10);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 28, 50, furi_string_get_cstr(buffer));

            furi_string_set(buffer, RailwayShort[history.exit_line]);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 92, 40, furi_string_get_cstr(buffer));
            // furi_string_set(buffer, RailwayLong[history.entry_line][history.entry_station][0]);
            // furi_string_cat(buffer, RailwayLong[history.entry_line][history.entry_station][1]);
            furi_string_printf(buffer, "%02d", history.entry_station + 10);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 93, 50, furi_string_get_cstr(buffer));
            break;
        case 1:
        default:
            break;
        }
    }
}

static bool suica_history_input_callback(InputEvent* event, void* context) {
    SuicaContext* suica = (SuicaContext*)context;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyLeft: {
            bool redraw = true;
            with_view_model(
                suica->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->entry > 1) {
                        model->entry--;
                    }
                },
                redraw);
            break;
        }
        case InputKeyRight: {
            bool redraw = true;
            with_view_model(
                suica->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->entry < model->size - 1) {
                        model->entry++;
                    }
                },
                redraw);
            break;
        }
        case InputKeyUp: {
            bool redraw = true;
            with_view_model(
                suica->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->page > 1) {
                        model->page--;
                    }
                },
                redraw);
            break;
        }
        case InputKeyDown: {
            bool redraw = true;
            with_view_model(
                suica->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->page < HISTORY_VIEW_PAGE_NUM) {
                        model->page++;
                    }
                },
                redraw);
            break;
        }
        default:
            // Handle other keys or do nothing
            break;
        }
    }

    return false;
}

static void suica_parse_detail_callback(GuiButtonType result, InputType type, void* context) {
    Metroflip* app = context;
    UNUSED(result);
    if(type == InputTypeShort) {
        view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewCanvas);
    }
}

static uint32_t suica_navigation_raw_callback(void* _context) {
    UNUSED(_context);
    return MetroflipViewWidget;
}

static NfcCommand metroflip_scene_suica_poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolFelica);
    NfcCommand command = NfcCommandContinue;
    MetroflipPollerEventType stage = MetroflipPollerEventTypeStart;

    Metroflip* app = context;
    FuriString* parsed_data = furi_string_alloc();
    SuicaContext* suica = app->suica_context;
    SuicaHistoryViewModel* model = view_get_model(suica->view_history);
    suica_model_initialize(model, 3);
    furi_string_reset(app->text_box_store);
    Widget* widget = app->widget;
    FURI_LOG_I(TAG, "We are in the poller callback");
    const uint16_t service_code[2] = {SERVICE_CODE_HISTORY_IN_LE, SERVICE_CODE_TAPS_LOG_IN_LE};

    const FelicaPollerEvent* felica_event = event.event_data;
    FelicaPollerReadCommandResponse* rx_resp;
    rx_resp->SF1 = 0;
    rx_resp->SF2 = 0;
    uint8_t blocks[1] = {0x00};
    FelicaPoller* felica_poller = event.instance;
    FURI_LOG_I(TAG, "Poller set");
    if(felica_event->type == FelicaPollerEventTypeRequestAuthContext) {
        if(stage == MetroflipPollerEventTypeStart) {
            nfc_device_set_data(
                app->nfc_device, NfcProtocolFelica, nfc_poller_get_data(app->poller));
            furi_string_printf(parsed_data, "\e#Suica\n");

            FelicaError error;
            // Authenticate with the card
            // Iterate through the two services
            for(int service_code_index = 0; service_code_index < 2; service_code_index++) {
                furi_string_cat_printf(
                    parsed_data, "%s: \n", suica_service_names[service_code_index]);
                rx_resp->SF1 = 0;
                rx_resp->SF2 = 0;
                blocks[0] = 0;
                while((rx_resp->SF1 + rx_resp->SF2) == 0) {
                    uint8_t block_data[16] = {0};
                    error = felica_poller_read_blocks(
                        felica_poller, 1, blocks, service_code[service_code_index], &rx_resp);
                    furi_string_cat_printf(parsed_data, "Block %02X\n", blocks[0]);
                    blocks[0]++;
                    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
                        furi_string_cat_printf(parsed_data, "%02X ", rx_resp->data[i]);
                        block_data[i] = rx_resp->data[i];
                    }
                    furi_string_cat_printf(parsed_data, "\n");
                    if(service_code_index == 0) {
                        suica_add_entry(model, block_data);
                    }
                    if(error != FelicaErrorNone) {
                        stage = MetroflipPollerEventTypeFail;
                        view_dispatcher_send_custom_event(
                            app->view_dispatcher, MetroflipCustomEventPollerFail);
                        break;
                    }
                    FURI_LOG_I(TAG, "We blocking");
                }
                FURI_LOG_I(TAG, "We in them service loops");
            }
            FURI_LOG_I(TAG, "We are out");
            metroflip_app_blink_stop(app);
            stage = (error == FelicaErrorNone) ? MetroflipPollerEventTypeSuccess :
                                                 MetroflipPollerEventTypeFail;

            widget_add_text_scroll_element(
                widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

            widget_add_button_element(
                widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);

            widget_add_button_element(
                widget, GuiButtonTypeCenter, "Parse", suica_parse_detail_callback, app);

            FURI_LOG_I(TAG, "We heading to widget");

            view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
            furi_string_free(parsed_data);
        }
    }

    command = NfcCommandStop;
    return command;
}

void metroflip_scene_suica_on_enter(void* context) {
    Metroflip* app = context;
    // Gui* gui = furi_record_open(RECORD_GUI);
    dolphin_deed(DolphinDeedNfcRead);

    app->suica_context = (SuicaContext*)malloc(sizeof(SuicaContext));
    app->suica_context->view_history = view_alloc();
    view_set_draw_callback(app->suica_context->view_history, suica_history_draw_callback);
    view_set_input_callback(app->suica_context->view_history, suica_history_input_callback);
    view_set_previous_callback(app->suica_context->view_history, suica_navigation_raw_callback);
    view_set_context(app->suica_context->view_history, app);
    view_allocate_model(
        app->suica_context->view_history, ViewModelTypeLockFree, sizeof(SuicaHistoryViewModel));
    view_dispatcher_add_view(
        app->view_dispatcher, MetroflipViewCanvas, app->suica_context->view_history);

    // Setup view
    Popup* popup = app->popup;
    popup_set_header(popup, "Apply\n card to\nthe back", 68, 30, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    // Start worker
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewPopup);
    nfc_scanner_alloc(app->nfc);
    app->poller = nfc_poller_alloc(app->nfc, NfcProtocolFelica);
    nfc_poller_start(app->poller, metroflip_scene_suica_poller_callback, app);

    metroflip_app_blink_start(app);
}

bool metroflip_scene_suica_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MetroflipCustomEventCardDetected) {
            Popup* popup = app->popup;
            popup_set_header(popup, "DON'T\nMOVE", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventCardLost) {
            Popup* popup = app->popup;
            popup_set_header(popup, "Card \n lost", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventWrongCard) {
            Popup* popup = app->popup;
            popup_set_header(popup, "WRONG \n CARD", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventPollerFail) {
            Popup* popup = app->popup;
            popup_set_header(popup, "Failed", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        consumed = true;
    }

    return consumed;
}

void metroflip_scene_suica_on_exit(void* context) {
    Metroflip* app = context;
    widget_reset(app->widget);
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewCanvas);
    free(app->suica_context);
    metroflip_app_blink_stop(app);
    if(app->poller) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }
}
