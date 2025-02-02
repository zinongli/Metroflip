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

#include "metroflip_i.h"
#include <flipper_application.h>
#include "../../metroflip_plugins.h"
#include "../../api/metroflip/metroflip_api.h"
#include "../../api/suica/suica_assets.h"

#include <lib/nfc/protocols/felica/felica.h>
#include <lib/nfc/protocols/felica/felica_poller.h>
#include <lib/nfc/protocols/felica/felica_poller_i.h>
#include <lib/nfc/helpers/felica_crc.h>
#include <lib/bit_lib/bit_lib.h>

#include <applications/services/locale/locale.h>
#include <datetime.h>

// Probably not needed after upstream include this in their suica_i.h
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>
#define TAG "Suica:Scene:Suica"

#define SUICA_STATION_LIST_PATH         APP_ASSETS_PATH("suica/line_")
#define SUICA_IC_TYPE_CODE              0x31
#define SERVICE_CODE_HISTORY_IN_LE      (0x090FU)
#define SERVICE_CODE_TAPS_LOG_IN_LE     (0x108FU)
#define BLOCK_COUNT                     1
#define HISTORY_VIEW_PAGE_NUM           3
#define TERMINAL_NULL                   0x02
#define TERMINAL_BUS                    0x05
#define TERMINAL_TICKET_VENDING_MACHINE 0x12
#define TERMINAL_TURNSTILE              0x16
#define TERMINAL_MOBILE_PHONE           0x1B
#define TERMINAL_IN_CAR_SUPP_MACHINE    0x24
#define TERMINAL_POS_AND_TAXI           0xC7
#define TERMINAL_VENDING_MACHINE        0xC8
#define PROCESSING_CODE_NEW_ISSUE       0x02
#define ARROW_ANIMATION_FRAME_MS        350

const char* suica_service_names[] = {
    "Travel History",
    "Taps Log",
};

typedef enum {
    SuicaTrainRideEntry,
    SuicaTrainRideExit,
} SuicaTrainRideType;

typedef enum {
    SuicaRedrawScreen,
} MetroflipCustomEvent;

static void suica_model_initialize(SuicaHistoryViewModel* model, size_t initial_capacity) {
    model->travel_history =
        (uint8_t*)malloc(initial_capacity * FELICA_DATA_BLOCK_SIZE); // Each entry is 16 bytes
    model->size = 0;
    model->capacity = initial_capacity;
    model->entry = 1;
    model->page = 0;
    model->animator_tick = 0;
    model->history.entry_station.name = furi_string_alloc_set("Unknown");
    model->history.entry_station.jr_header = furi_string_alloc_set("0");
    model->history.exit_station.name = furi_string_alloc_set("Unknown");
    model->history.exit_station.jr_header = furi_string_alloc_set("0");
    model->history.entry_line = RailwaysList[SUICA_RAILWAY_NUM];
    model->history.exit_line = RailwaysList[SUICA_RAILWAY_NUM];
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

void suica_parse_train_code(
    uint8_t line_code,
    uint8_t station_code,
    SuicaTrainRideType ride_type,
    SuicaHistoryViewModel* model) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();

    FuriString* line_code_str = furi_string_alloc();
    FuriString* line_and_station_code_str = furi_string_alloc();

    furi_string_printf(line_code_str, "0x%02X", line_code);
    furi_string_printf(line_and_station_code_str, "0x%02X,0x%02X", line_code, station_code);

    FuriString* line_candidate = furi_string_alloc_set(SUICA_RAILWAY_UNKNOWN_NAME);
    FuriString* station_candidate = furi_string_alloc_set(SUICA_RAILWAY_UNKNOWN_NAME);
    FuriString* station_num_candidate = furi_string_alloc_set("0");
    FuriString* station_JR_header_candidate = furi_string_alloc_set("0");
    FuriString* line_copy = furi_string_alloc();
    size_t line_comma_ind = 0;
    size_t station_comma_ind = 0;
    size_t station_num_comma_ind = 0;
    size_t station_JR_header_comma_ind = 0;

    bool station_found = false;
    FuriString* file_name = furi_string_alloc();
    furi_string_printf(file_name, "%s0x%02X.txt", SUICA_STATION_LIST_PATH, line_code);
    if(file_stream_open(stream, furi_string_get_cstr(file_name), FSAM_READ, FSOM_OPEN_EXISTING)) {
        while(stream_read_line(stream, line) && !station_found) {
            // file is in csv format: station_group_id,station_id,station_sub_id,station_name
            // search for the station
            furi_string_replace_all(line, "\r", "");
            furi_string_replace_all(line, "\n", "");
            furi_string_set(line_copy, line); // 0xD5,0x02,Keikyu Main,Shinagawa,1,0

            if(furi_string_start_with(line, line_code_str)) {
                // set line name here
                furi_string_right(line_copy, 10); // Keikyu Main,Shinagawa,1,0
                furi_string_set(line_candidate, line_copy);
                line_comma_ind = furi_string_search_char(line_candidate, ',', 0);
                furi_string_left(line_candidate, line_comma_ind); // Keikyu Main
                // we cut the line and station code in the line line copy
                // and we leave only the line name for the line candidate
                if(furi_string_start_with(line, line_and_station_code_str)) {
                    furi_string_set(station_candidate, line_copy); // Keikyu Main,Shinagawa,1,0
                    furi_string_right(station_candidate, line_comma_ind + 1);
                    station_comma_ind =
                        furi_string_search_char(station_candidate, ',', 0); // Shinagawa,1,0
                    furi_string_left(station_candidate, station_comma_ind); //  Shinagawa
                    station_found = true;
                    break;
                }
            }
        }
    } else {
        FURI_LOG_E("Suica:Scene:Suica", "Failed to open stations.txt");
    }

    furi_string_set(station_num_candidate, line_copy); // Keikyu Main,Shinagawa,1,0
    furi_string_right(station_num_candidate, line_comma_ind + station_comma_ind + 2); // 1,0
    station_num_comma_ind = furi_string_search_char(station_num_candidate, ',', 0);
    furi_string_left(station_num_candidate, station_num_comma_ind); // 1

    furi_string_set(station_JR_header_candidate, line_copy); // Keikyu Main,Shinagawa,1,0
    furi_string_right(
        station_JR_header_candidate,
        line_comma_ind + station_comma_ind + station_num_comma_ind + 3); // 0
    station_JR_header_comma_ind = furi_string_search_char(station_JR_header_candidate, ',', 0);
    furi_string_left(station_JR_header_candidate, station_JR_header_comma_ind); // 0

    switch(ride_type) {
    case SuicaTrainRideEntry:
        for(size_t i = 0; i < SUICA_RAILWAY_NUM; i++) {
            if(furi_string_equal_str(line_candidate, RailwaysList[i].long_name)) {
                model->history.entry_line = RailwaysList[i];
                furi_string_set(model->history.entry_station.name, station_candidate);
                model->history.entry_station.station_number =
                    atoi(furi_string_get_cstr(station_num_candidate));
                furi_string_set(
                    model->history.entry_station.jr_header, station_JR_header_candidate);
                break;
            }
        }
        break;
    case SuicaTrainRideExit:
        for(size_t i = 0; i < SUICA_RAILWAY_NUM; i++) {
            if(furi_string_equal_str(line_candidate, RailwaysList[i].long_name)) {
                model->history.exit_line = RailwaysList[i];
                furi_string_set(model->history.exit_station.name, station_candidate);
                model->history.exit_station.station_number =
                    atoi(furi_string_get_cstr(station_num_candidate));
                furi_string_set(
                    model->history.exit_station.jr_header, station_JR_header_candidate);
                break;
            }
        }
        break;
    default:
        UNUSED(model);
        break;
    }

    furi_string_free(line);
    furi_string_free(line_copy);
    furi_string_free(line_code_str);
    furi_string_free(line_and_station_code_str);
    furi_string_free(line_candidate);
    furi_string_free(station_candidate);
    furi_string_free(station_num_candidate);
    furi_string_free(station_JR_header_candidate);
    file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

static void suica_parse(SuicaHistoryViewModel* my_model) {
    uint8_t current_block[FELICA_DATA_BLOCK_SIZE];
    // Parse the current block/entry
    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
        current_block[i] = my_model->travel_history[((my_model->entry - 1) * 16) + i];
    }

    if(((uint8_t)current_block[4] + (uint8_t)current_block[5]) != 0) {
        my_model->history.year = ((uint8_t)current_block[4] & 0xFE) >> 1;
        my_model->history.month = (((uint8_t)current_block[4] & 0x01) << 3) |
                                  (((uint8_t)current_block[5] & 0xE0) >> 5);
        my_model->history.day = (uint8_t)current_block[5] & 0x1F;
    } else {
        my_model->history.year = 0;
        my_model->history.month = 0;
        my_model->history.day = 0;
    }
    my_model->history.balance = ((uint16_t)current_block[11] << 8) | (uint16_t)current_block[10];
    // FURI_LOG_I(TAG,"%02X", (uint8_t)current_block[0]);
    my_model->history.area_code = current_block[15];
    if((uint8_t)current_block[0] >= TERMINAL_TICKET_VENDING_MACHINE &&
       (uint8_t)current_block[0] <= TERMINAL_IN_CAR_SUPP_MACHINE) {
        // Train rides
        // Will be overwritton is is ticket sale (TERMINAL_TICKET_VENDING_MACHINE)
        my_model->history.history_type = SuicaHistoryTrain;
        uint8_t entry_line = current_block[6];
        uint8_t entry_station = current_block[7];
        uint8_t exit_line = current_block[8];
        uint8_t exit_station = current_block[9];

        if((uint8_t)current_block[0] != TERMINAL_MOBILE_PHONE) {
            suica_parse_train_code(entry_line, entry_station, SuicaTrainRideEntry, my_model);
        }
        if((uint8_t)current_block[1] != PROCESSING_CODE_NEW_ISSUE) {
            suica_parse_train_code(exit_line, exit_station, SuicaTrainRideExit, my_model);
        }

        if(((uint8_t)current_block[4] + (uint8_t)current_block[5]) != 0) {
            my_model->history.year = ((uint8_t)current_block[4] & 0xFE) >> 1;
            my_model->history.month = (((uint8_t)current_block[4] & 0x01) << 3) |
                                      (((uint8_t)current_block[5] & 0xE0) >> 5);
            my_model->history.day = (uint8_t)current_block[5] & 0x1F;
        }
    }
    switch((uint8_t)current_block[0]) {
    case TERMINAL_BUS:
        // 6 & 7 bus line code
        // 8 & 9 bus stop code
        my_model->history.history_type = SuicaHistoryBus;
        break;
    case TERMINAL_POS_AND_TAXI:
    case TERMINAL_VENDING_MACHINE:
        // 6 & 7 are hour and minute
        my_model->history.history_type = ((uint8_t)current_block[0] == TERMINAL_POS_AND_TAXI) ?
                                             SuicaHistoryPosAndTaxi :
                                             SuicaHistoryVendingMachine;
        my_model->history.hour = ((uint8_t)current_block[6] & 0xF8) >> 3;
        my_model->history.minute = (((uint8_t)current_block[6] & 0x07) << 3) |
                                   (((uint8_t)current_block[7] & 0xE0) >> 5);
        my_model->history.shop_code = (uint8_t*)malloc(2);
        my_model->history.shop_code[0] = current_block[8];
        my_model->history.shop_code[1] = current_block[9];
        break;
    case TERMINAL_MOBILE_PHONE:
        if((uint8_t)current_block[1] == PROCESSING_CODE_NEW_ISSUE) {
            my_model->history.hour = ((uint8_t)current_block[6] & 0xF8) >> 3;
            my_model->history.minute = (((uint8_t)current_block[6] & 0x07) << 3) |
                                       (((uint8_t)current_block[7] & 0xE0) >> 5);
        }
        break;
    case TERMINAL_TICKET_VENDING_MACHINE:
        my_model->history.history_type = SuicaHistoryHappyBirthday;
        break;
    default:
        if((uint8_t)current_block[0] <= TERMINAL_NULL) {
            my_model->history.history_type = SuicaHistoryNull;
        }
        break;
    }
    if((uint8_t)current_block[1] == PROCESSING_CODE_NEW_ISSUE) {
        my_model->history.history_type = SuicaHistoryHappyBirthday;
    }
}

static void suica_draw_train_page_1(
    Canvas* canvas,
    SuicaHistory history,
    SuicaHistoryViewModel* model,
    bool is_birthday) {
    // Entry logo
    switch(history.entry_line.type) {
    case SuicaKeikyu:
        canvas_draw_icon(canvas, 2, 11, &I_Suica_KeikyuLogo);
        break;
    case SuicaJR:
        canvas_draw_icon(canvas, 1, 12, &I_Suica_JRLogo);
        break;
    case SuicaTokyoMetro:
        canvas_draw_icon(canvas, 2, 12, &I_Suica_TokyoMetroLogo);
        break;
    case SuicaToei:
        canvas_draw_icon(canvas, 4, 11, &I_Suica_ToeiLogo);
        break;
    case SuicaTWR:
        canvas_draw_icon(canvas, 0, 12, &I_Suica_TWRLogo);
        break;
    case SuicaTokyoMonorail:
        canvas_draw_icon(canvas, 0, 11, &I_Suica_TokyoMonorailLogo);
        break;
    case SuicaRailwayTypeMax:
        canvas_draw_icon(canvas, 5, 11, &I_Suica_QuestionMarkSmall);
        break;
    default:
        break;
    }

    // Entry Text
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 26, 23, history.entry_line.long_name);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 34, furi_string_get_cstr(history.entry_station.name));

    if(!is_birthday) {
        // Exit logo
        switch(history.exit_line.type) {
        case SuicaKeikyu:
            canvas_draw_icon(canvas, 2, 39, &I_Suica_KeikyuLogo);
            break;
        case SuicaJR:
            canvas_draw_icon(canvas, 1, 40, &I_Suica_JRLogo);
            break;
        case SuicaTokyoMetro:
            canvas_draw_icon(canvas, 2, 40, &I_Suica_TokyoMetroLogo);
            break;
        case SuicaToei:
            canvas_draw_icon(canvas, 4, 39, &I_Suica_ToeiLogo);
            break;
        case SuicaTWR:
            canvas_draw_icon(canvas, 0, 40, &I_Suica_TWRLogo);
            break;
        case SuicaTokyoMonorail:
            canvas_draw_icon(canvas, 0, 39, &I_Suica_TokyoMonorailLogo);
            break;
        case SuicaRailwayTypeMax:
            canvas_draw_icon(canvas, 5, 39, &I_Suica_QuestionMarkSmall);
            break;
        default:
            break;
        }

        // Exit Text
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 26, 51, history.exit_line.long_name);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 62, furi_string_get_cstr(history.exit_station.name));
    } else {
        // Birthday
        canvas_draw_icon(canvas, 5, 42, &I_Suica_CrackingEgg);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 28, 56, "Suica issued");
    }

    // Separator
    canvas_draw_icon(canvas, 0, 37, &I_Suica_DashLine);

    // Arrow
    uint8_t arrow_bits[4] = {0b1000, 0b0100, 0b0010, 0b0001};

    // Arrow
    if(model->animator_tick > 3) {
        // 4 steps of animation
        model->animator_tick = 0;
    }
    uint8_t current_arrow_bits = arrow_bits[model->animator_tick];
    canvas_draw_icon(
        canvas,
        110,
        19,
        (current_arrow_bits & 0b1000) ? &I_Suica_FilledArrowDown : &I_Suica_EmptyArrowDown);
    canvas_draw_icon(
        canvas,
        110,
        29,
        (current_arrow_bits & 0b0100) ? &I_Suica_FilledArrowDown : &I_Suica_EmptyArrowDown);
    canvas_draw_icon(
        canvas,
        110,
        39,
        (current_arrow_bits & 0b0010) ? &I_Suica_FilledArrowDown : &I_Suica_EmptyArrowDown);
    canvas_draw_icon(
        canvas,
        110,
        49,
        (current_arrow_bits & 0b0001) ? &I_Suica_FilledArrowDown : &I_Suica_EmptyArrowDown);
}

static void
    suica_draw_train_page_2(Canvas* canvas, SuicaHistory history, SuicaHistoryViewModel* model) {
    FuriString* buffer = furi_string_alloc();

    // Entry
    switch(history.entry_line.type) {
    case SuicaKeikyu:
        canvas_draw_disc(canvas, 24, 38, 24);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_disc(canvas, 24, 38, 21);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontKeyboard);
        canvas_draw_icon(canvas, 16, 24, history.entry_line.logo_icon);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.entry_station.station_number);
        canvas_draw_str(canvas, 13, 51, furi_string_get_cstr(buffer));
        break;
    case SuicaTokyoMonorail:
        canvas_draw_rbox(canvas, 9, 23, 32, 32, 5);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 12, 26, 26, 26);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 25, 38, AlignCenter, AlignBottom, history.entry_line.short_name);
        canvas_draw_str(canvas, 17, 36, history.entry_line.short_name);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.entry_station.station_number);
        canvas_draw_str(canvas, 14, 51, furi_string_get_cstr(buffer));
        break;
    case SuicaJR:
        if(!furi_string_equal_str(history.entry_station.jr_header, "0")) {
            canvas_draw_rbox(canvas, 6, 14, 38, 48, 7);
            canvas_set_color(canvas, ColorWhite);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str_aligned(
                canvas,
                25,
                24,
                AlignCenter,
                AlignBottom,
                furi_string_get_cstr(history.entry_station.jr_header));
            canvas_draw_rbox(canvas, 9, 26, 32, 32, 5);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_frame(canvas, 12, 29, 26, 26);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(
                canvas, 25, 38, AlignCenter, AlignBottom, history.entry_line.short_name);
            canvas_set_font(canvas, FontBigNumbers);
            furi_string_printf(buffer, "%02d", history.entry_station.station_number);
            canvas_draw_str(canvas, 14, 53, furi_string_get_cstr(buffer));
        } else {
            canvas_draw_rframe(canvas, 9, 23, 32, 32, 5);
            canvas_draw_frame(canvas, 12, 26, 26, 26);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(
                canvas, 25, 35, AlignCenter, AlignBottom, history.entry_line.short_name);
            canvas_set_font(canvas, FontBigNumbers);
            furi_string_printf(buffer, "%02d", history.entry_station.station_number);
            canvas_draw_str(canvas, 14, 50, furi_string_get_cstr(buffer));
        }
        break;
    case SuicaTokyoMetro:
    case SuicaToei:
        canvas_draw_disc(canvas, 24, 38, 24);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_disc(canvas, 24, 38, 19);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_icon(
            canvas,
            17 + history.entry_line.logo_offset[0],
            22 + history.entry_line.logo_offset[1],
            history.entry_line.logo_icon);
        furi_string_printf(buffer, "%02d", history.entry_station.station_number);
        canvas_draw_str(canvas, 13, 53, furi_string_get_cstr(buffer));
        break;
    case SuicaTWR:
        canvas_draw_circle(canvas, 24, 38, 24);
        canvas_draw_circle(canvas, 24, 38, 20);
        canvas_draw_disc(canvas, 24, 38, 18);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_icon(canvas, 20, 23, history.entry_line.logo_icon);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.entry_station.station_number);
        canvas_draw_str(canvas, 13, 53, furi_string_get_cstr(buffer));
        canvas_set_color(canvas, ColorBlack);
        break;
    case SuicaRailwayTypeMax:
        canvas_draw_circle(canvas, 24, 38, 24);
        canvas_draw_circle(canvas, 24, 38, 19);
        canvas_draw_icon(canvas, 14, 22, &I_Suica_QuestionMarkBig);
        break;
    default:
        break;
    }

    // Exit
    switch(history.exit_line.type) {
    case SuicaKeikyu:
        canvas_draw_disc(canvas, 103, 38, 24);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_disc(canvas, 103, 38, 21);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_icon(canvas, 95, 24, history.exit_line.logo_icon);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.exit_station.station_number);
        canvas_draw_str(canvas, 92, 52, furi_string_get_cstr(buffer));
        break;
    case SuicaTokyoMonorail:
        canvas_draw_rbox(canvas, 86, 23, 32, 32, 5);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 89, 26, 26, 26);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 101, 35, AlignCenter, AlignBottom, history.exit_line.short_name);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.exit_station.station_number);
        canvas_draw_str(canvas, 91, 51, furi_string_get_cstr(buffer));
        break;
    case SuicaJR:
        if(!furi_string_equal_str(history.exit_station.jr_header, "0")) {
            canvas_draw_rbox(canvas, 83, 14, 38, 48, 7);
            canvas_set_color(canvas, ColorWhite);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str_aligned(
                canvas,
                101,
                24,
                AlignCenter,
                AlignBottom,
                furi_string_get_cstr(history.exit_station.jr_header));
            canvas_draw_rbox(canvas, 86, 26, 32, 32, 5);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_frame(canvas, 89, 29, 26, 26);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(
                canvas, 102, 38, AlignCenter, AlignBottom, history.exit_line.short_name);
            canvas_set_font(canvas, FontBigNumbers);
            furi_string_printf(buffer, "%02d", history.exit_station.station_number);
            canvas_draw_str(canvas, 91, 53, furi_string_get_cstr(buffer));
        } else {
            canvas_draw_rframe(canvas, 86, 23, 32, 32, 5);
            canvas_draw_frame(canvas, 89, 26, 26, 26);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(
                canvas, 102, 35, AlignCenter, AlignBottom, history.exit_line.short_name);
            canvas_set_font(canvas, FontBigNumbers);
            furi_string_printf(buffer, "%02d", history.exit_station.station_number);
            canvas_draw_str(canvas, 91, 50, furi_string_get_cstr(buffer));
        }
        break;
    case SuicaTokyoMetro:
    case SuicaToei:
        canvas_draw_disc(canvas, 103, 38, 24);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_disc(canvas, 103, 38, 19);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_icon(
            canvas,
            96 + history.exit_line.logo_offset[0],
            22 + history.exit_line.logo_offset[1],
            history.exit_line.logo_icon);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.exit_station.station_number);
        canvas_draw_str(canvas, 92, 53, furi_string_get_cstr(buffer));
        break;
    case SuicaTWR:
        canvas_draw_circle(canvas, 103, 38, 24);
        canvas_draw_circle(canvas, 103, 38, 20);
        canvas_draw_disc(canvas, 103, 38, 18);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_icon(canvas, 99, 23, history.exit_line.logo_icon);
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(buffer, "%02d", history.exit_station.station_number);
        canvas_draw_str(canvas, 92, 53, furi_string_get_cstr(buffer));
        canvas_set_color(canvas, ColorBlack);
        break;
    case SuicaRailwayTypeMax:
        canvas_draw_circle(canvas, 103, 38, 24);
        canvas_draw_circle(canvas, 103, 38, 19);
        canvas_draw_icon(canvas, 93, 22, &I_Suica_QuestionMarkBig);
    default:
        break;
    }

    uint8_t arrow_bits[3] = {0b100, 0b010, 0b001};

    // Arrow
    if(model->animator_tick > 2) {
        // 4 steps of animation
        model->animator_tick = 0;
    }
    uint8_t current_arrow_bits = arrow_bits[model->animator_tick];
    canvas_draw_icon(
        canvas,
        51,
        32,
        (current_arrow_bits & 0b100) ? &I_Suica_FilledArrowRight : &I_Suica_EmptyArrowRight);
    canvas_draw_icon(
        canvas,
        59,
        32,
        (current_arrow_bits & 0b010) ? &I_Suica_FilledArrowRight : &I_Suica_EmptyArrowRight);
    canvas_draw_icon(
        canvas,
        67,
        32,
        (current_arrow_bits & 0b001) ? &I_Suica_FilledArrowRight : &I_Suica_EmptyArrowRight);

    furi_string_free(buffer);
}

static void
    suica_draw_birthday_page_2(Canvas* canvas, SuicaHistory history, SuicaHistoryViewModel* model) {
    UNUSED(history);
    canvas_draw_icon(canvas, 27, 14, &I_Suica_PenguinHappyBirthday);
    canvas_draw_icon(canvas, 14, 14, &I_Suica_PenguinTodaysVIP);
    canvas_draw_rframe(canvas, 12, 12, 13, 52, 2); // VIP frame
    uint8_t star_bits[4] = {0b11000000, 0b11110000, 0b11111111, 0b00000000};

    // Arrow
    if(model->animator_tick > 3) {
        // 4 steps of animation
        model->animator_tick = 0;
    }
    uint8_t current_star_bits = star_bits[model->animator_tick];
    canvas_draw_icon(
        canvas, 87, 30, (current_star_bits & 0b10000000) ? &I_Suica_BigStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 90, 12, (current_star_bits & 0b01000000) ? &I_Suica_PlusStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 99, 34, (current_star_bits & 0b00100000) ? &I_Suica_SmallStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 103, 12, (current_star_bits & 0b00010000) ? &I_Suica_SmallStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 106, 21, (current_star_bits & 0b00001000) ? &I_Suica_BigStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 109, 43, (current_star_bits & 0b00000100) ? &I_Suica_PlusStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 117, 28, (current_star_bits & 0b00000010) ? &I_Suica_BigStar : &I_Suica_Nothing);
    canvas_draw_icon(
        canvas, 115, 16, (current_star_bits & 0b00000100) ? &I_Suica_PlusStar : &I_Suica_Nothing);
}

static void suica_draw_vending_machine_page_1(
    Canvas* canvas,
    SuicaHistory history,
    SuicaHistoryViewModel* model) {
    FuriString* buffer = furi_string_alloc();
    canvas_draw_icon(canvas, 0, 10, &I_Suica_VendingPage2Full);
    furi_string_printf(buffer, "%d", history.balance_change);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 100, 39, AlignRight, AlignBottom, furi_string_get_cstr(buffer));

    // Animate Bubbles and LCD Refresh
    if(model->animator_tick > 14) {
        // 14 steps of animation
        model->animator_tick = 0;
    }
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_line(canvas, 87, 50 + model->animator_tick, 128, 50 + model->animator_tick);
    switch(model->animator_tick % 7) {
    case 0:
        canvas_draw_circle(canvas, 12, 48, 1);
        canvas_draw_circle(canvas, 23, 39, 2);
        break;
    case 1:
        canvas_draw_circle(canvas, 11, 46, 1);
        canvas_draw_circle(canvas, 23, 39, 2);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_line(canvas, 24, 37, 22, 37);
        canvas_draw_line(canvas, 25, 40, 25, 38);
        canvas_set_color(canvas, ColorWhite);
        break;
    case 2:
        canvas_draw_circle(canvas, 12, 44, 1);
        canvas_draw_circle(canvas, 24, 50, 1);
        break;
    case 3:
        canvas_draw_icon(canvas, 12, 41, &I_Suica_SmallStar);
        canvas_draw_circle(canvas, 25, 48, 1);
        break;
    case 4:
        canvas_draw_icon(canvas, 14, 39, &I_Suica_SmallStar);
        canvas_draw_circle(canvas, 26, 46, 1);
        break;
    case 5:
        canvas_draw_icon(canvas, 24, 43, &I_Suica_SmallStar);
        canvas_draw_circle(canvas, 16, 38, 2);
        break;
    case 6:
        canvas_draw_icon(canvas, 23, 41, &I_Suica_SmallStar);
        canvas_draw_circle(canvas, 16, 38, 2);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_line(canvas, 15, 36, 17, 36);
        canvas_draw_line(canvas, 18, 39, 18, 37);
        canvas_set_color(canvas, ColorWhite);
        break;
    default:
        break;
    }
    furi_string_free(buffer);
    canvas_set_color(canvas, ColorBlack);
}

static void suica_draw_vending_machine_page_2(
    Canvas* canvas,
    SuicaHistory history,
    SuicaHistoryViewModel* model) {
    FuriString* buffer = furi_string_alloc();

    // Clock Component
    canvas_set_color(canvas, ColorWhite); // Erase part of old frame to allow for new frame
    canvas_draw_line(canvas, 91, 9, 94, 6);
    canvas_draw_line(canvas, 57, 9, 93, 9);
    canvas_set_color(canvas, ColorBlack);
    furi_string_printf(buffer, "%02d:%02d", history.hour, history.minute);
    canvas_draw_line(canvas, 63, 21, 60, 18);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 63, 19, furi_string_get_cstr(buffer));
    canvas_draw_line(canvas, 91, 21, 94, 18);
    canvas_draw_line(canvas, 64, 21, 91, 21);
    canvas_draw_line(canvas, 94, 6, 94, 17);
    canvas_draw_line(canvas, 60, 12, 60, 17);
    canvas_draw_line(canvas, 60, 12, 57, 9);

    // Vending Machine
    canvas_draw_icon(canvas, 5, 12, &I_Suica_VendingMachine);

    // Machine Code
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 75, 35, "Machine");
    canvas_draw_icon(canvas, 119, 25, &I_Suica_ShopPin);
    furi_string_printf(
        buffer, "%01d:%03d:%03d", history.area_code, history.shop_code[0], history.shop_code[1]);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 75, 45, furi_string_get_cstr(buffer));

    // Animate Vending Machine Flap
    if(model->animator_tick > 6) {
        // 6 steps of animation
        model->animator_tick = 0;
    }
    switch(model->animator_tick) {
    case 0:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap1);
        break;
    case 1:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap2);
        break;
    case 2:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap3);
        break;
    case 3:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap3);
        canvas_draw_icon(canvas, 59, 45, &I_Suica_VendingCan1);
        break;
    case 4:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap2);
        canvas_draw_icon(canvas, 74, 48, &I_Suica_VendingCan2);
        break;
    case 5:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap1);
        canvas_draw_icon(canvas, 89, 51, &I_Suica_VendingCan3);
        break;
    case 6:
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlapHollow);
        canvas_draw_icon(canvas, 44, 40, &I_Suica_VendingFlap1);
        canvas_draw_icon(canvas, 110, 54, &I_Suica_VendingCan4);
        break;
    default:
        break;
    }
    furi_string_free(buffer);
}

static void
    suica_draw_store_page_1(Canvas* canvas, SuicaHistory history, SuicaHistoryViewModel* model) {
    FuriString* buffer = furi_string_alloc();
    furi_string_printf(buffer, "%d", history.balance_change);
    canvas_draw_icon(canvas, 0, 15, &I_Suica_StoreP1Counter);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 99, 39, AlignRight, AlignBottom, furi_string_get_cstr(buffer));
    canvas_draw_icon(canvas, 59, 27, &I_Suica_StoreReceiptDashLine);
    // Animate Taxi and LCD Refresh
    if(model->animator_tick > 11) {
        // 14 steps of animation
        model->animator_tick = 0;
    }

    switch(model->animator_tick % 6) {
    case 0:
    case 1:
    case 2:
        canvas_draw_icon(canvas, 41, 18, &I_Suica_StoreReceiptFrame1);
        break;
    case 3:
    case 4:
    case 5:
        canvas_draw_icon(canvas, 41, 18, &I_Suica_StoreReceiptFrame2);
        break;
    default:
        break;
    }

    switch(model->animator_tick % 6) {
    case 0:
    case 1:
        canvas_draw_icon(canvas, 0, 24, &I_Suica_StoreLightningVertical);
        break;
    case 2:
    case 3:
        canvas_draw_icon(canvas, 3, 31, &I_Suica_StoreLightningHorizontal);
        break;
    case 4:
    case 5:
        canvas_draw_icon(canvas, 0, 24, &I_Suica_StoreLightningVertical);
        canvas_draw_icon(canvas, 3, 31, &I_Suica_StoreLightningHorizontal);
        break;
    default:
        break;
    }
    furi_string_free(buffer);
}

static void
    suica_draw_store_page_2(Canvas* canvas, SuicaHistory history, SuicaHistoryViewModel* model) {
    FuriString* buffer = furi_string_alloc();
    // Clock Component
    canvas_set_color(canvas, ColorWhite); // Erase part of old frame to allow for new frame
    canvas_draw_line(canvas, 91, 9, 94, 6);
    canvas_draw_line(canvas, 57, 9, 93, 9);
    canvas_set_color(canvas, ColorBlack);
    furi_string_printf(buffer, "%02d:%02d", history.hour, history.minute);
    canvas_draw_line(canvas, 63, 21, 60, 18);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 63, 19, furi_string_get_cstr(buffer));
    canvas_draw_line(canvas, 91, 21, 94, 18);
    canvas_draw_line(canvas, 64, 21, 91, 21);
    canvas_draw_line(canvas, 94, 6, 94, 17);
    canvas_draw_line(canvas, 60, 12, 60, 17);
    canvas_draw_line(canvas, 60, 12, 57, 9);

    // Machine Code
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 75, 35, "Store");
    canvas_draw_icon(canvas, 104, 25, &I_Suica_ShopPin);
    furi_string_printf(
        buffer, "%01d:%03d:%03d", history.area_code, history.shop_code[0], history.shop_code[1]);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 75, 45, furi_string_get_cstr(buffer));

    // Store Frame
    canvas_draw_icon(canvas, 0, 13, &I_Suica_StoreFrame);
    // Sliding Door
    uint8_t door_position[7] = {20, 18, 14, 6, 2, 0, 0};
    if(model->animator_tick > 20) {
        // 14 steps of animation
        model->animator_tick = 0;
    }

    if(model->animator_tick < 7) {
        canvas_draw_icon(
            canvas, -1 - door_position[6 - model->animator_tick], 28, &I_Suica_StoreSlidingDoor);
    } else if(model->animator_tick < 14) {
        canvas_draw_icon(
            canvas, -1 - door_position[model->animator_tick - 7], 28, &I_Suica_StoreSlidingDoor);
    } else {
        canvas_draw_icon(canvas, -1, 28, &I_Suica_StoreSlidingDoor);
    }

    // Animate Neon and Fan
    switch(model->animator_tick % 4) {
    case 0:
    case 1:
        canvas_draw_icon(canvas, 37, 18, &I_Suica_StoreFan1);
        break;
    case 2:
    case 3:
        canvas_draw_icon(canvas, 37, 18, &I_Suica_StoreFan2);
        break;
    default:
        break;
    }
    furi_string_free(buffer);
}

static void
    suica_draw_balance_page(Canvas* canvas, SuicaHistory history, SuicaHistoryViewModel* model) {
    FuriString* buffer = furi_string_alloc();

    // Balance
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_icon(canvas, 0, 48, &I_Suica_YenSign);
    canvas_draw_icon(canvas, 111, 48, &I_Suica_YenKanji);

    furi_string_printf(buffer, "%d", history.balance);
    canvas_draw_str_aligned(
        canvas, 109, 64, AlignRight, AlignBottom, furi_string_get_cstr(buffer));

    furi_string_printf(buffer, "%d", history.previous_balance);
    canvas_draw_str_aligned(
        canvas, 109, 26, AlignRight, AlignBottom, furi_string_get_cstr(buffer));

    furi_string_printf(buffer, "%d", history.balance_change);
    canvas_draw_str_aligned(
        canvas, 109, 43, AlignRight, AlignBottom, furi_string_get_cstr(buffer));

    // Separator
    canvas_draw_line(canvas, 26, 45, 128, 45);
    canvas_draw_line(canvas, 26, 46, 128, 46);

    if(history.balance_sign == SuicaBalanceAdd) {
        // Animate plus sign
        if(model->animator_tick > 2) {
            // 9 steps of animation
            model->animator_tick = 0;
        }
        switch(model->animator_tick) {
        case 0:
            canvas_draw_icon(canvas, 28, 28, &I_Suica_PlusSign1);
            break;
        case 1:
            canvas_draw_icon(canvas, 27, 27, &I_Suica_PlusSign2);
            break;
        case 2:
            canvas_draw_icon(canvas, 26, 26, &I_Suica_PlusSign3);
            break;
        default:
            break;
        }
    } else if(history.balance_sign == SuicaBalanceSub) {
        // Animate plus sign
        if(model->animator_tick > 12) {
            // 9 steps of animation
            model->animator_tick = 0;
        }
        switch(model->animator_tick) {
        case 0:
        case 1:
        case 2:
        case 3:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign0);
            break;
        case 4:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign1);
            break;
        case 5:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign2);
            break;
        case 6:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign3);
            break;
        case 7:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign4);
            break;
        case 8:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign5);
            break;
        case 9:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign6);
            break;
        case 10:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign7);
            break;
        case 11:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign8);
            break;
        case 12:
            canvas_draw_icon(canvas, 28, 32, &I_Suica_MinusSign9);
            break;
        default:
            break;
        }
    } else {
        canvas_draw_str(canvas, 30, 28, "=");
    }
}

static void suica_history_draw_callback(Canvas* canvas, void* model) {
    canvas_set_bitmap_mode(canvas, true);
    SuicaHistoryViewModel* my_model = (SuicaHistoryViewModel*)model;
    FuriString* buffer = furi_string_alloc();
    // catch the case where the page and entry are not initialized

    if(my_model->entry > my_model->size || my_model->entry < 1) {
        my_model->entry = 1;
    }

    // Get previous balance if we are not at the earliest entry
    if(my_model->entry < my_model->size) {
        my_model->history.previous_balance = my_model->travel_history[(my_model->entry * 16) + 10];
        my_model->history.previous_balance |= my_model->travel_history[(my_model->entry * 16) + 11]
                                              << 8;
    } else {
        my_model->history.previous_balance = 0;
    }
    // Calculate balance change
    if(my_model->history.previous_balance < my_model->history.balance) {
        my_model->history.balance_change =
            my_model->history.balance - my_model->history.previous_balance;
        my_model->history.balance_sign = SuicaBalanceAdd;
    } else if(my_model->history.previous_balance > my_model->history.balance) {
        my_model->history.balance_change =
            my_model->history.previous_balance - my_model->history.balance;
        my_model->history.balance_sign = SuicaBalanceSub;
    } else {
        my_model->history.balance_change = 0;
        my_model->history.balance_sign = SuicaBalanceEqual;
    }

    // Main title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 8, "Suica");

    // Date
    furi_string_printf(
        buffer,
        "20%02d-%02d-%02d",
        my_model->history.year,
        my_model->history.month,
        my_model->history.day);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 34, 8, furi_string_get_cstr(buffer));

    // Entry Num
    furi_string_printf(buffer, "%02d/%02d", my_model->entry, my_model->size);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 99, 8, furi_string_get_cstr(buffer));

    // Frame
    canvas_draw_line(canvas, 0, 9, 26, 9);
    canvas_draw_line(canvas, 27, 9, 29, 7);
    canvas_draw_line(canvas, 29, 0, 29, 6);

    canvas_draw_line(canvas, 31, 0, 31, 7);
    canvas_draw_line(canvas, 33, 9, 31, 7);
    canvas_draw_line(canvas, 90, 9, 34, 9);
    canvas_draw_line(canvas, 91, 9, 94, 6);
    canvas_draw_line(canvas, 94, 0, 94, 6);

    canvas_draw_line(canvas, 96, 0, 96, 6);
    canvas_draw_line(canvas, 99, 9, 96, 6);
    canvas_draw_line(canvas, 100, 9, 128, 9);

    switch((uint8_t)my_model->page) {
    case 0:
        switch(my_model->history.history_type) {
        case SuicaHistoryTrain:
            suica_draw_train_page_1(canvas, my_model->history, my_model, false);
            break;
        case SuicaHistoryHappyBirthday:
            suica_draw_train_page_1(canvas, my_model->history, my_model, true);
            break;
        case SuicaHistoryVendingMachine:
            suica_draw_vending_machine_page_1(canvas, my_model->history, my_model);
            break;
        case SuicaHistoryPosAndTaxi:
            suica_draw_store_page_1(canvas, my_model->history, my_model);
            break;
        default:
            break;
        }
        break;
    case 1:
        switch(my_model->history.history_type) {
        case SuicaHistoryTrain:
            suica_draw_train_page_2(canvas, my_model->history, my_model);
            break;
        case SuicaHistoryHappyBirthday:
            suica_draw_birthday_page_2(canvas, my_model->history, my_model);
            break;
        case SuicaHistoryVendingMachine:
            suica_draw_vending_machine_page_2(canvas, my_model->history, my_model);
            break;
        case SuicaHistoryPosAndTaxi:
            suica_draw_store_page_2(canvas, my_model->history, my_model);
            break;
        default:
            break;
        }
        break;
    case 2:
        suica_draw_balance_page(canvas, my_model->history, my_model);
        break;
    default:
        break;
    }
    furi_string_free(buffer);
}

static void suica_parse_detail_callback(GuiButtonType result, InputType type, void* context) {
    Metroflip* app = context;
    UNUSED(result);
    if(type == InputTypeShort) {
        SuicaHistoryViewModel* my_model = view_get_model(app->suica_context->view_history);
        suica_parse(my_model);
        FURI_LOG_I(TAG, "Draw Callback: We have %d entries", my_model->size);
        view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewCanvas);
    }
}

static uint32_t suica_navigation_raw_callback(void* _context) {
    UNUSED(_context);
    return MetroflipViewWidget;
}

static NfcCommand suica_poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolFelica);
    NfcCommand command = NfcCommandContinue;
    MetroflipPollerEventType stage = MetroflipPollerEventTypeStart;

    Metroflip* app = context;
    FuriString* parsed_data = furi_string_alloc();
    SuicaHistoryViewModel* model = view_get_model(app->suica_context->view_history);

    Widget* widget = app->widget;
    const uint16_t service_code[2] = {SERVICE_CODE_HISTORY_IN_LE, SERVICE_CODE_TAPS_LOG_IN_LE};

    const FelicaPollerEvent* felica_event = event.event_data;
    FelicaPollerReadCommandResponse* rx_resp;
    rx_resp->SF1 = 0;
    rx_resp->SF2 = 0;
    uint8_t blocks[1] = {0x00};
    FelicaPoller* felica_poller = event.instance;
    FURI_LOG_I(TAG, "Poller set");
    if(felica_event->type == FelicaPollerEventTypeRequestAuthContext &&
       felica_poller->data->pmm.data[0] == SUICA_IC_TYPE_CODE) {
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventCardDetected);
        command = NfcCommandContinue;

        if(stage == MetroflipPollerEventTypeStart) {
            nfc_device_set_data(
                app->nfc_device, NfcProtocolFelica, nfc_poller_get_data(app->poller));
            furi_string_printf(parsed_data, "\e#Suica\n");

            FelicaError error = FelicaErrorNone;
            int service_code_index = 0;
            // Authenticate with the card
            // Iterate through the two services
            while(service_code_index < 2 && error == FelicaErrorNone) {
                furi_string_cat_printf(
                    parsed_data, "%s: \n", suica_service_names[service_code_index]);
                rx_resp->SF1 = 0;
                rx_resp->SF2 = 0;
                blocks[0] = 0; // firmware api requires this to be a list
                uint8_t max_blocks = 25; // Arbitrary limit to prevent infinite loops
                while((rx_resp->SF1 + rx_resp->SF2) == 0 && blocks[0] < max_blocks &&
                      error == FelicaErrorNone) {
                    uint8_t block_data[16] = {0};
                    error = felica_poller_read_blocks(
                        felica_poller, 1, blocks, service_code[service_code_index], &rx_resp);
                    FURI_LOG_I(TAG, "Erorr = %d", error);
                    if(error != FelicaErrorNone) {
                        view_dispatcher_send_custom_event(
                            app->view_dispatcher, MetroflipCustomEventCardLost);
                        command = NfcCommandStop;
                        break;
                    }
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
                }
                service_code_index++;
            }
            metroflip_app_blink_stop(app);

            if(model->size == 1) { // Have to let the poller run once before knowing we failed
                furi_string_printf(
                    parsed_data,
                    "\e#Suica\nSorry, no data found.\nPlease let the developers know and we will add support.");
            }
            widget_add_text_scroll_element(
                widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

            widget_add_button_element(
                widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);

            if(model->size > 1) {
                widget_add_button_element(
                    widget, GuiButtonTypeCenter, "Parse", suica_parse_detail_callback, app);
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
        }
    }
    furi_string_free(parsed_data);
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
                    suica_parse(model);
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
                    suica_parse(model);
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
                SuicaHistoryViewModel * model,
                { model->animator_tick++; },
                redraw);
            return true;
        }
    default:
        return false;
    }
}

static void suica_on_enter(Metroflip* app) {
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

    popup_set_header(app->popup, "Apply\n card to\nthe back", 68, 30, AlignLeft, AlignTop);
    popup_set_icon(app->popup, 0, 3, &I_RFIDDolphinReceive_97x61);
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewPopup);

    nfc_scanner_alloc(app->nfc);
    app->poller = nfc_poller_alloc(app->nfc, NfcProtocolFelica);
    nfc_poller_start(app->poller, suica_poller_callback, app);
    FURI_LOG_I(TAG, "Poller started");

    metroflip_app_blink_start(app);

}

static bool suica_on_event(Metroflip* app, SceneManagerEvent event) {
    bool consumed = false;
    Popup* popup = app->popup;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MetroflipCustomEventCardDetected) {
            popup_set_header(popup, "DON'T\nMOVE", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventCardLost) {
            popup_set_header(popup, "Card \n lost", 68, 30, AlignLeft, AlignTop);
            // popup_set_timeout(popup, 2000);
            // popup_enable_timeout(popup);
            // view_dispatcher_switch_to_view(app->view_dispatcher, SuicaViewPopup);
            // popup_disable_timeout(popup);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, MetroflipSceneStart);
            consumed = true;
        } else if(event.event == MetroflipCustomEventWrongCard) {
            popup_set_header(popup, "WRONG \n CARD", 68, 30, AlignLeft, AlignTop);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, MetroflipSceneStart);
            consumed = true;
        } else if(event.event == MetroflipCustomEventPollerFail) {
            popup_set_header(popup, "Failed", 68, 30, AlignLeft, AlignTop);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, MetroflipSceneStart);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        UNUSED(popup);
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        consumed = true;
    }

    return consumed;
}

static void suica_on_exit(Metroflip* app) {
    widget_reset(app->widget);
    view_free(app->suica_context->view_history);
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewCanvas);
    free(app->suica_context);
    metroflip_app_blink_stop(app);
    if(app->poller) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }
}

/* Actual implementation of app<>plugin interface */
static const MetroflipPlugin suica_plugin = {
    .card_name = "Suica",
    .plugin_on_enter = suica_on_enter,
    .plugin_on_event = suica_on_event,
    .plugin_on_exit = suica_on_exit,

};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor suica_plugin_descriptor = {
    .appid = METROFLIP_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = METROFLIP_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &suica_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* suica_plugin_ep(void) {
    return &suica_plugin_descriptor;
}
