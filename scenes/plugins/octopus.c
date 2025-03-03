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

#include "../../metroflip_plugins.h"
#include "../../api/metroflip/metroflip_api.h"


#include <lib/nfc/protocols/felica/felica.h>
#include <lib/nfc/protocols/felica/felica_poller.h>
// #include <lib/nfc/protocols/felica/felica_poller_i.h>
#include <lib/bit_lib/bit_lib.h>

#include <applications/services/locale/locale.h>
#include <datetime.h>

#define TAG "Metroflip:Scene:Octopus"

const char* octopus_service_names[] = {
    "Balance",
};

static NfcCommand octopus_poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolFelica);
    NfcCommand command = NfcCommandContinue;
    MetroflipPollerEventType stage = MetroflipPollerEventTypeStart;

    Metroflip* app = context;
    FuriString* parsed_data = furi_string_alloc();

    Widget* widget = app->widget;

    const uint16_t service_code[1] = {OCTOPUS_SERVICE_CODE};

    const FelicaPollerEvent* felica_event = event.event_data;
    FelicaPollerReadCommandResponse* rx_resp;
    rx_resp->SF1 = 0;
    rx_resp->SF2 = 0;
    uint8_t blocks[1] = {0x00};
    FelicaPoller* felica_poller = event.instance;
    FURI_LOG_I(TAG, "Poller set");
    if(felica_event->type == FelicaPollerEventTypeRequestAuthContext) {
        command = NfcCommandContinue;
        FURI_LOG_D(TAG, "Non-Auth finished");
        furi_delay_ms(10); 
        // somehow these delays are crucial for timing
        // without them the poller will freeze
        if(stage == MetroflipPollerEventTypeStart) {
            nfc_device_set_data(
                app->nfc_device, NfcProtocolFelica, nfc_poller_get_data(app->poller));
            furi_string_printf(parsed_data, "\e#Octopus\n");

            FelicaError error = FelicaErrorNone;
            int service_code_index = 0;
            // Iterate through the services
            FURI_LOG_D(TAG, "Reading balance");
            furi_delay_ms(10);
            while(service_code_index < 1 && error == FelicaErrorNone) {
                furi_string_cat_printf(
                    parsed_data, "%s: \n", octopus_service_names[service_code_index]);
                rx_resp->SF1 = 0;
                rx_resp->SF2 = 0;
                blocks[0] = 0; // firmware api requires this to be a list
                while((rx_resp->SF1 + rx_resp->SF2) == 0) {
                    error = felica_poller_read_blocks(
                        felica_poller, 1, blocks, service_code[service_code_index], &rx_resp);
                    if(error != FelicaErrorNone || (rx_resp->SF1 + rx_resp->SF2) != 0) {
                        break;
                    }
                    furi_string_cat_printf(parsed_data, "Block %02X\n", blocks[0]);
                    blocks[0]++;
                    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
                        furi_string_cat_printf(parsed_data, "%02X ", rx_resp->data[i]);
                    }
                    furi_string_cat_printf(parsed_data, "\n");
                }
                service_code_index++;
            }
            metroflip_app_blink_stop(app);

            if(blocks[0] == 0) { // Have to let the poller run once before knowing we failed
                furi_string_printf(
                    parsed_data,
                    "\e#Octopus\nSorry, no data found.\nPlease let the developers know and we will add support.");
            }
            widget_add_text_scroll_element(
                widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

            widget_add_button_element(
                widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);
            widget_add_button_element(
                widget, GuiButtonTypeLeft, "Del", metroflip_delete_widget_callback, app);

            view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
        }
    }
    furi_string_free(parsed_data);
    command = NfcCommandStop;
    return command;
}

static void octopus_on_enter(Metroflip* app) {
    // Gui* gui = furi_record_open(RECORD_GUI);
    dolphin_deed(DolphinDeedNfcRead);

    if(app->data_loaded == false) {
        // popup_set_header(app->popup, "Apply\n card to\nthe back", 68, 30, AlignLeft, AlignTop);
        // popup_set_icon(app->popup, 0, 3, &I_RFIDDolphinReceive_97x61);
        // view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewPopup);

        nfc_scanner_alloc(app->nfc);
        app->poller = nfc_poller_alloc(app->nfc, NfcProtocolFelica);
        nfc_poller_start(app->poller, octopus_poller_callback, app);
        FURI_LOG_I(TAG, "Poller started");

        metroflip_app_blink_start(app);
    } else {
        Widget* widget = app->widget;
        FuriString* parsed_data = furi_string_alloc();
        furi_string_printf(parsed_data, "\e#Octopus\n");
        furi_string_cat_printf(parsed_data, "Data loading not supported yet.\n");
        
        widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

        widget_add_button_element(
            widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);
        widget_add_button_element(
            widget, GuiButtonTypeLeft, "Del", metroflip_delete_widget_callback, app);

        view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
        furi_string_free(parsed_data);
    }
}

static bool octopus_on_event(Metroflip* app, SceneManagerEvent event) {
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

static void octopus_on_exit(Metroflip* app) {
    widget_reset(app->widget);
    if(app->poller && !app->data_loaded) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }
}

/* Actual implementation of app<>plugin interface */
static const MetroflipPlugin octopus_plugin = {
    .card_name = "Octopus",
    .plugin_on_enter = octopus_on_enter,
    .plugin_on_event = octopus_on_event,
    .plugin_on_exit = octopus_on_exit,

};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor octopus_plugin_descriptor = {
    .appid = METROFLIP_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = METROFLIP_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &octopus_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* octopus_plugin_ep(void) {
    return &octopus_plugin_descriptor;
}
