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
#include <flipper_application.h>

#include <lib/nfc/protocols/felica/felica.h>
#include <lib/nfc/protocols/felica/felica_poller.h>
#include <lib/nfc/protocols/felica/felica_poller_i.h>
#include <lib/nfc/helpers/felica_crc.h>
#include <lib/bit_lib/bit_lib.h>

#include <applications/services/locale/locale.h>
#include <datetime.h>
#include "suica_assets.h"

#define SERVICE_CODE_HISTORY_IN_LE  (0x090FU)
#define SERVICE_CODE_TAPS_LOG_IN_LE (0x108FU)
#define BLOCK_COUNT                 1
#define HISTORY_VIEW_PAGE_NUM       3
#define TAG                         "Metroflip:Scene:Suica"
#define TERMINAL_NULL               0x02
#define TERMINAL_BUS                0x05
#define TERMINAL_POS_AND_TAXI       0xC7
#define TERMINAL_VENDING_MACHINE    0xC8
#define TERMINAL_TURNSTILE          0x16

#define ARROW_ANIMATION_FRAME_MS 500

const char* suica_service_names[] = {
    "Travel History",
    "Taps Log",
};

typedef enum {
    SuicaRedrawScreen,
} SuicaCustomEvent;

static void suica_model_initialize(SuicaHistoryViewModel* model, size_t initial_capacity) {
    model->travel_history =
        (uint8_t*)malloc(initial_capacity * FELICA_DATA_BLOCK_SIZE); // Each entry is 16 bytes
    model->size = 0;
    model->capacity = initial_capacity;
    model->entry = 1;
    model->page = 0;
}

static void suica_add_entry(SuicaHistoryViewModel* model, const uint8_t* entry) {
    if(model->size <= 0) {
        suica_model_initialize(model, 3);
    }
    // Check if resizing is needed
    if(model->size == model->capacity) {
        size_t new_capacity = model->capacity * 2; // Double the capacity
        uint8_t* new_data =
            (uint8_t*)realloc(model->travel_history, new_capacity * FELICA_DATA_BLOCK_SIZE);
        model->travel_history = new_data;
        model->capacity = new_capacity;
    }

    // Copy the 16-byte entry to the next slot
    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
        model->travel_history[(model->size * FELICA_DATA_BLOCK_SIZE) + i] = entry[i];
    }

    model->size++;
    FURI_LOG_I(TAG, "Added entry %d", model->size);
}

static SuicaTravelHistory suica_parse(uint8_t block[16]) {
    SuicaTravelHistory history;
    // FURI_LOG_I(TAG,"%02X", (uint8_t)block[0]);
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
    case TERMINAL_TURNSTILE:
        // Train rides
        history.history_type = SuicaHistoryTrain;
        uint8_t entry_line = block[6];
        uint8_t entry_station = block[7];
        for(size_t i = 0; i < RAILWAY_NUM; i++) {
            if(RailwaysList[i].line_code == entry_line) {
                history.entry_line = RailwaysList[i];
                break;
            }
        }
        for(size_t j = 0; j < history.entry_line.station_num; j++) {
            if(history.entry_line.line[j].station_code == entry_station) {
                history.entry_station = history.entry_line.line[j];
                break;
            }
        }
        if(block[0] > 0x15) {
            uint8_t exit_line = block[8];
            uint8_t exit_station = block[9];
            FURI_LOG_I(TAG, "Exit Line %02X, Exit Station %02X", exit_line, exit_station);
            history.rail_region = block[15];

            for(size_t i = 0; i < RAILWAY_NUM; i++) {
                if(RailwaysList[i].line_code == exit_line) {
                    history.exit_line = RailwaysList[i];
                    break;
                }
            }
            for(size_t j = 0; j < history.entry_line.station_num; j++) {
                FURI_LOG_I(
                    TAG,
                    "Matching Exit Station Code %02X",
                    history.exit_line.line[j].station_code);
                FURI_LOG_I(TAG, "Decoded Station Code %02X", exit_station);
                if(history.exit_line.line[j].station_code == exit_station) {
                    history.exit_station = history.exit_line.line[j];
                    break;
                }
            }
            FURI_LOG_I(
                TAG,
                "Exit Line %s, Exit Station Num %02X, station name %s",
                RailwaysList->long_name,
                history.exit_station.station_number,
                history.exit_station.name);
        }
        if(((uint8_t)block[4] + (uint8_t)block[5]) != 0) {
            history.year = ((uint8_t)block[4] & 0xFE) >> 1;
            history.month = (((uint8_t)block[4] & 0x01) << 3) | (((uint8_t)block[5] & 0xE0) >> 5);
            history.day = (uint8_t)block[5] & 0x1F;
            history.balance = ((uint16_t)block[10] << 8) | (uint16_t)block[11];
        }
        break;

    default:
        if(((uint8_t)block[4] + (uint8_t)block[5]) != 0) {
            history.year = ((uint8_t)block[4] & 0xFE) >> 1;
            history.month = (((uint8_t)block[4] & 0x01) << 3) | (((uint8_t)block[5] & 0xE0) >> 5);
            history.day = (uint8_t)block[5] & 0x1F;
            history.balance = ((uint16_t)block[10] << 8) | (uint16_t)block[11];
        }
        break;
    }
    return history;
}

static void suica_history_draw_callback(Canvas* canvas, void* model) {
    canvas_set_bitmap_mode(canvas, true);
    SuicaHistoryViewModel* my_model = (SuicaHistoryViewModel*)model;
    // catch the case where the page and entry are not initialized

    if(my_model->entry > my_model->size || my_model->entry < 1) {
        my_model->entry = 1;
    }

    uint8_t current_block[FELICA_DATA_BLOCK_SIZE];
    FuriString* buffer = furi_string_alloc();

    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
        current_block[i] = my_model->travel_history[((my_model->entry - 1) * 16) + i];
    }
    SuicaTravelHistory history = suica_parse(current_block);
    // Main title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 8, "Suica");

    // Separator
    canvas_draw_line(canvas, 29, 0, 29, 8);

    // Date
    furi_string_printf(buffer, "20%02d-%02d-%02d", history.year, history.month, history.day);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 33, 8, furi_string_get_cstr(buffer));

    // Separator
    canvas_draw_line(canvas, 95, 0, 95, 8);

    // Entry Num
    furi_string_printf(buffer, "%02d/%02d", my_model->entry, my_model->size);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 99, 8, furi_string_get_cstr(buffer));
    FURI_LOG_I(TAG, "History Type %d", history.history_type);

    // Floor
    canvas_draw_line(canvas, 0, 9, 128, 9);

    switch((uint8_t)my_model->page) {
    case 0:
        

        if(history.history_type == SuicaHistoryTrain) {
            // Station Logos Circle
            canvas_draw_circle(canvas, 33, 36, 13);
            canvas_draw_circle(canvas, 98, 36, 13);
            canvas_draw_circle(canvas, 33, 36, 16);
            canvas_draw_circle(canvas, 98, 36, 16);

            // Entry Station Logo
            furi_string_set(buffer, history.entry_line.short_name);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 27, 36, furi_string_get_cstr(buffer));
            furi_string_printf(buffer, "%d", history.entry_station.station_number);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 28, 46, furi_string_get_cstr(buffer));

            // Exit Station Logo
            furi_string_set(buffer, history.exit_line.short_name);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 92, 36, furi_string_get_cstr(buffer));
            FURI_LOG_I(TAG, "Exit station %d", history.exit_station.station_number);
            furi_string_printf(buffer, "%d", history.exit_station.station_number);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 93, 46, furi_string_get_cstr(buffer));

            // Arrow
            if(my_model->arrow_flow > 4) {
                // 4 steps of animation
                my_model->arrow_flow = 0;
            }
            canvas_draw_xbm(canvas, 51 + my_model->arrow_flow * 3, 29, 18, 15, ArrowRight);
            my_model->arrow_flow++;
        }
        break;
    case 1:
        if(history.history_type == SuicaHistoryTrain) {
            // Logo
            canvas_draw_xbm(canvas, 2, 13, 21, 13, TokyoMetroLogo);
            canvas_draw_xbm(canvas, 1, 42, 23, 12, JRLogo);
            

            // Entry
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 26, 25, history.entry_line.long_name);

            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 2, 35, history.entry_station.name);

            // Separator
            canvas_draw_xbm(canvas, 0, 39, 91, 1, DashLine);

            // Exit
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 26, 54, history.exit_line.long_name);

            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 2, 64, history.exit_station.name);

            // Arrow
            if(my_model->arrow_flow > 9) {
                // 11 steps of animation
                my_model->arrow_flow = 0;
            }
            canvas_draw_xbm(canvas, 110, 43 - my_model->arrow_flow * 3, 15, 18, ArrowUp);
            my_model->arrow_flow++;
        }
        break;
    case 2:
        // Balance
        furi_string_printf(buffer, "%d", history.balance);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(
            canvas,
            64,
            32,
            AlignCenter,
            AlignCenter,
            furi_string_get_cstr(buffer)); // Centered

        canvas_draw_xbm(canvas, 19, 24, 12, 16, YenSign);
        canvas_draw_xbm(canvas, 95, 24, 16, 16, YenKanji);
        break;
    default:
        break;
    }
}

static void suica_parse_detail_callback(GuiButtonType result, InputType type, void* context) {
    Metroflip* app = context;
    UNUSED(result);
    if(type == InputTypeShort) {
        SuicaHistoryViewModel* my_model = view_get_model(app->suica_context->view_history);
        FURI_LOG_I(TAG, "Draw Callback: We have %d entries", my_model->size);
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

    Metroflip* app = (Metroflip*)context;
    FuriString* parsed_data = furi_string_alloc();
    SuicaHistoryViewModel* model = view_get_model(app->suica_context->view_history);
    furi_string_reset(app->text_box_store);
    Widget* widget = app->widget;
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
                        FURI_LOG_I(TAG, "Service code %d, adding entry", service_code_index);
                        suica_add_entry(model, block_data);
                    }
                    if(error != FelicaErrorNone) {
                        stage = MetroflipPollerEventTypeFail;
                        view_dispatcher_send_custom_event(
                            app->view_dispatcher, MetroflipCustomEventPollerFail);
                        break;
                    }
                }
            }
            metroflip_app_blink_stop(app);
            stage = (error == FelicaErrorNone) ? MetroflipPollerEventTypeSuccess :
                                                 MetroflipPollerEventTypeFail;

            widget_add_text_scroll_element(
                widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

            widget_add_button_element(
                widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);

            widget_add_button_element(
                widget, GuiButtonTypeCenter, "Parse", suica_parse_detail_callback, app);

            view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
            furi_string_free(parsed_data);
        }
    }

    command = NfcCommandStop;
    return command;
}

static bool suica_history_input_callback(InputEvent* event, void* context) {
    Metroflip* app = (Metroflip*)context;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyLeft: {
            bool redraw = true;
            with_view_model(
                app->suica_context->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->entry > 1) {
                        model->entry--;
                    }
                    FURI_LOG_I(TAG, "Viewing entry %d", model->entry);
                },
                redraw);
            break;
        }
        case InputKeyRight: {
            bool redraw = true;
            with_view_model(
                app->suica_context->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->entry < model->size) {
                        model->entry++;
                    }
                    FURI_LOG_I(TAG, "Viewing entry %d", model->entry);
                },
                redraw);
            break;
        }
        case InputKeyUp: {
            bool redraw = true;
            with_view_model(
                app->suica_context->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->page > 0) {
                        model->page--;
                    }
                },
                redraw);
            break;
        }
        case InputKeyDown: {
            bool redraw = true;
            with_view_model(
                app->suica_context->view_history,
                SuicaHistoryViewModel * model,
                {
                    if(model->page < HISTORY_VIEW_PAGE_NUM - 1) {
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

static void suica_view_history_timer_callback(void* context) {
    Metroflip* app = (Metroflip*)context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

static void suica_view_history_enter_callback(void* context) {
    uint32_t period = furi_ms_to_ticks(ARROW_ANIMATION_FRAME_MS);
    Metroflip* app = (Metroflip*)context;
    furi_assert(app->suica_context->timer == NULL);
    app->suica_context->timer =
        furi_timer_alloc(suica_view_history_timer_callback, FuriTimerTypePeriodic, context);
    furi_timer_start(app->suica_context->timer, period);
}

/**
 * @brief      Callback when the user exits the game screen.
 * @details    This function is called when the user exits the game screen.  We stop the timer.
 * @param      context  The context - SkeletonApp object.
*/
static void suica_view_history_exit_callback(void* context) {
    Metroflip* app = (Metroflip*)context;
    furi_timer_stop(app->suica_context->timer);
    furi_timer_free(app->suica_context->timer);
    app->suica_context->timer = NULL;
}

static bool suica_view_history_custom_event_callback(uint32_t event, void* context) {
    Metroflip* app = (Metroflip*)context;
    switch(event) {
    case 0:
        // Redraw screen by passing true to last parameter of with_view_model.
        {
            bool redraw = true;
            with_view_model(
                app->suica_context->view_history,
                SuicaHistoryViewModel * _model,
                { UNUSED(_model); },
                redraw);
            return true;
        }
    default:
        return false;
    }
}

void metroflip_scene_suica_on_enter(void* context) {
    Metroflip* app = context;
    // Gui* gui = furi_record_open(RECORD_GUI);
    dolphin_deed(DolphinDeedNfcRead);

    app->suica_context = (SuicaContext*)malloc(sizeof(SuicaContext));
    app->suica_context->view_history = view_alloc();
    view_set_context(app->suica_context->view_history, app);
    view_set_input_callback(app->suica_context->view_history, suica_history_input_callback);
    view_set_previous_callback(app->suica_context->view_history, suica_navigation_raw_callback);
    view_set_enter_callback(app->suica_context->view_history, suica_view_history_enter_callback);
    view_set_exit_callback(app->suica_context->view_history, suica_view_history_exit_callback);
    view_set_custom_callback(
        app->suica_context->view_history, suica_view_history_custom_event_callback);
    view_set_draw_callback(app->suica_context->view_history, suica_history_draw_callback);
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
