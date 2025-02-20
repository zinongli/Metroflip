
#include <flipper_application.h>
#include "../../metroflip_i.h"

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>

#include <dolphin/dolphin.h>
#include <bit_lib.h>
#include <furi_hal.h>
#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/nfc_listener.h>
#include "../../api/metroflip/metroflip_api.h"
#include "../../metroflip_plugins.h"

#define TAG "Metroflip:Scene:gocard"

unsigned short byteArrayToIntReversed(unsigned int dec1, unsigned int dec2) {
    unsigned char byte1 = (unsigned char)dec1;
    unsigned char byte2 = (unsigned char)dec2;
    return ((unsigned short)byte2 << 8) | byte1;
}

static bool gocard_parse(FuriString* parsed_data, const MfClassicData* data) {
    bool parsed = false;

    do {
        // Verify key
        //const uint8_t ticket_sector_number = 1;
        //const uint8_t ticket_block_number = 1;

        //const MfClassicSectorTrailer* sec_tr =
        //    mf_classic_get_sector_trailer_by_sector(data, ticket_sector_number);

        //const uint64_t key =
        //    bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        ///if(key != gocard_1k_keys[ticket_sector_number].a) break;
        //FURI_LOG_D(TAG, "passed key check");
        // Parse data
        //const uint8_t start_block_num =
        //    mf_classic_get_first_block_num_of_sector(ticket_sector_number);

        //const uint8_t* block_start_ptr =
        //    &data->block[start_block_num + ticket_block_number].data[0];

        //uint32_t balance = bit_lib_bytes_to_num_le(block_start_ptr, 4) - 100;

        //uint32_t balance_lari = balance / 100;
        //uint8_t balance_tetri = balance % 100;

        int balance_slot = 4;

        if(data->block[balance_slot].data[13] <= data->block[balance_slot + 1].data[13])
            balance_slot++;

        unsigned short balancecents = byteArrayToIntReversed(
            data->block[balance_slot].data[2], data->block[balance_slot].data[3]);

        // Check if the sign flag is set in 'balance'
        if((balancecents & 0x8000) == 0x8000) {
            balancecents = balancecents & 0x7fff; // Clear the sign flag.
            balancecents *= -1; // Negate the balance.
        }
        // Otherwise, check the sign flag in data->block[4].data[1]
        else if((data->block[balance_slot].data[1] & 0x80) == 0x80) {
            // seq_go uses a sign flag in an adjacent byte.
            balancecents *= -1;
        }

        double balance = balancecents / 100.0;
        furi_string_printf(parsed_data, "\e#Go card\nValue: A$%.2f\n", balance);

        parsed = true;
    } while(false);

    return parsed;
}

static void gocard_on_enter(Metroflip* app) {
    dolphin_deed(DolphinDeedNfcRead);

    app->sec_num = 0;

    if(app->data_loaded) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        FlipperFormat* ff = flipper_format_file_alloc(storage);
        if(flipper_format_file_open_existing(ff, app->file_path)) {
            MfClassicData* mfc_data = mf_classic_alloc();
            mf_classic_load(mfc_data, ff, 2);
            FuriString* parsed_data = furi_string_alloc();
            Widget* widget = app->widget;

            furi_string_reset(app->text_box_store);
            if(!gocard_parse(parsed_data, mfc_data)) {
                furi_string_reset(app->text_box_store);
                FURI_LOG_I(TAG, "Unknown card type");
                furi_string_printf(parsed_data, "\e#Unknown card\n");
            }
            widget_add_text_scroll_element(
                widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

            widget_add_button_element(
                widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);
            widget_add_button_element(
                widget, GuiButtonTypeCenter, "Delete", metroflip_delete_widget_callback, app);
            mf_classic_free(mfc_data);
            furi_string_free(parsed_data);
            view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
        }
        flipper_format_free(ff);
    } else {
        // Setup view
        Popup* popup = app->popup;
        popup_set_header(popup, "unsupported", 68, 30, AlignLeft, AlignTop);
        popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);
    }
}

static bool gocard_on_event(Metroflip* app, SceneManagerEvent event) {
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
        scene_manager_set_scene_state(app->scene_manager, MetroflipSceneStart, MetroflipSceneAuto);
        consumed = true;
    }

    return consumed;
}

static void gocard_on_exit(Metroflip* app) {
    widget_reset(app->widget);

    if(app->poller && !app->data_loaded) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }

    // Clear view
    popup_reset(app->popup);

    metroflip_app_blink_stop(app);
}

/* Actual implementation of app<>plugin interface */
static const MetroflipPlugin gocard_plugin = {
    .card_name = "gocard",
    .plugin_on_enter = gocard_on_enter,
    .plugin_on_event = gocard_on_event,
    .plugin_on_exit = gocard_on_exit,

};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor gocard_plugin_descriptor = {
    .appid = METROFLIP_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = METROFLIP_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &gocard_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* gocard_plugin_ep(void) {
    return &gocard_plugin_descriptor;
}
