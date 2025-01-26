#include <flipper_application.h>
#include "../metroflip_i.h"

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>
#include "../api/mosgortrans/mosgortrans_util.h"

#include <dolphin/dolphin.h>
#include <bit_lib.h>
#include <furi_hal.h>
#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/nfc_listener.h>

#define TAG "Metroflip:Scene:Troika"

static bool troika_get_card_config(TroikaCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->data_sector = 11;
        config->keys = troika_1k_keys;
    } else if(type == MfClassicType4k) {
        config->data_sector = 8; // Further testing needed
        config->keys = troika_4k_keys;
    } else {
        success = false;
    }

    return success;
}

static bool troika_parse(FuriString* parsed_data, const MfClassicData* data) {
    bool parsed = false;

    do {
        // Verify card type
        TroikaCardConfig cfg = {};
        if(!troika_get_card_config(&cfg, data->type)) break;

        // Verify key
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.data_sector);

        const uint64_t key =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != cfg.keys[cfg.data_sector].a) break;

        FuriString* metro_result = furi_string_alloc();
        FuriString* ground_result = furi_string_alloc();
        FuriString* tat_result = furi_string_alloc();

        bool is_metro_data_present =
            mosgortrans_parse_transport_block(&data->block[32], metro_result);
        bool is_ground_data_present =
            mosgortrans_parse_transport_block(&data->block[28], ground_result);
        bool is_tat_data_present = mosgortrans_parse_transport_block(&data->block[16], tat_result);

        furi_string_cat_printf(parsed_data, "\e#Troyka card\n");
        if(is_metro_data_present && !furi_string_empty(metro_result)) {
            render_section_header(parsed_data, "Metro", 22, 21);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(metro_result));
        }

        if(is_ground_data_present && !furi_string_empty(ground_result)) {
            render_section_header(parsed_data, "Ediny", 22, 22);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(ground_result));
        }

        if(is_tat_data_present && !furi_string_empty(tat_result)) {
            render_section_header(parsed_data, "TAT", 24, 23);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(tat_result));
        }

        furi_string_free(tat_result);
        furi_string_free(ground_result);
        furi_string_free(metro_result);

        parsed = is_metro_data_present || is_ground_data_present || is_tat_data_present;
    } while(false);

    return parsed;
}

bool checked = false;

static NfcCommand metroflip_scene_troika_poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCommand command = NfcCommandContinue;
    const MfClassicPollerEvent* mfc_event = event.event_data;
    Metroflip* app = context;

    if(mfc_event->type == MfClassicPollerEventTypeCardDetected) {
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventCardDetected);
        command = NfcCommandContinue;
    } else if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventCardLost);
        app->sec_num = 0;
        command = NfcCommandStop;
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        mfc_event->data->poller_mode.mode = MfClassicPollerModeRead;

    } else if(mfc_event->type == MfClassicPollerEventTypeRequestReadSector) {
        MfClassicKey key = {0};
        MfClassicKeyType key_type = MfClassicKeyTypeA;
        bit_lib_num_to_bytes_be(troika_1k_keys[app->sec_num].a, COUNT_OF(key.data), key.data);
        if(!checked) {
            mfc_event->data->read_sector_request_data.sector_num = app->sec_num;
            mfc_event->data->read_sector_request_data.key = key;
            mfc_event->data->read_sector_request_data.key_type = key_type;
            mfc_event->data->read_sector_request_data.key_provided = true;
            app->sec_num++;
            checked = true;
        }
        nfc_device_set_data(
            app->nfc_device, NfcProtocolMfClassic, nfc_poller_get_data(app->poller));
        const MfClassicData* mfc_data = nfc_device_get_data(app->nfc_device, NfcProtocolMfClassic);
        if(mfc_data->type == MfClassicType1k) {
            bit_lib_num_to_bytes_be(troika_1k_keys[app->sec_num].a, COUNT_OF(key.data), key.data);

            mfc_event->data->read_sector_request_data.sector_num = app->sec_num;
            mfc_event->data->read_sector_request_data.key = key;
            mfc_event->data->read_sector_request_data.key_type = key_type;
            mfc_event->data->read_sector_request_data.key_provided = true;
            if(app->sec_num == 16) {
                mfc_event->data->read_sector_request_data.key_provided = false;
                app->sec_num = 0;
            }
            app->sec_num++;
        } else if(mfc_data->type == MfClassicType4k) {
            bit_lib_num_to_bytes_be(troika_4k_keys[app->sec_num].a, COUNT_OF(key.data), key.data);

            mfc_event->data->read_sector_request_data.sector_num = app->sec_num;
            mfc_event->data->read_sector_request_data.key = key;
            mfc_event->data->read_sector_request_data.key_type = key_type;
            mfc_event->data->read_sector_request_data.key_provided = true;
            if(app->sec_num == 40) {
                mfc_event->data->read_sector_request_data.key_provided = false;
                app->sec_num = 0;
            }
            app->sec_num++;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        const MfClassicData* mfc_data = nfc_device_get_data(app->nfc_device, NfcProtocolMfClassic);
        FuriString* parsed_data = furi_string_alloc();
        Widget* widget = app->widget;
        if(!troika_parse(parsed_data, mfc_data)) {
            furi_string_reset(app->text_box_store);
            FURI_LOG_I(TAG, "Unknown card type");
            furi_string_printf(parsed_data, "\e#Unknown card\n");
        }
        widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

        widget_add_button_element(
            widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);

        furi_string_free(parsed_data);
        view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
        metroflip_app_blink_stop(app);
        command = NfcCommandStop;
    } else if(mfc_event->type == MfClassicPollerEventTypeFail) {
        FURI_LOG_I(TAG, "fail");
        command = NfcCommandStop;
    }

    return command;
}

void metroflip_scene_troika_on_enter(void* context) {
    Metroflip* app = context;
    dolphin_deed(DolphinDeedNfcRead);

    app->sec_num = 0;

    // Setup view
    Popup* popup = app->popup;
    popup_set_header(popup, "Apply\n card to\nthe back", 68, 30, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    // Start worker
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewPopup);
    nfc_scanner_alloc(app->nfc);
    app->poller = nfc_poller_alloc(app->nfc, NfcProtocolMfClassic);
    nfc_poller_start(app->poller, metroflip_scene_troika_poller_callback, app);

    metroflip_app_blink_start(app);
}

bool metroflip_scene_troika_on_event(void* context, SceneManagerEvent event) {
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
        } else if(event.event == MetroflipCustomEventPollerSuccess) {
            scene_manager_next_scene(app->scene_manager, MetroflipSceneReadSuccess);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        consumed = true;
    }

    return consumed;
}

void metroflip_scene_troika_on_exit(void* context) {
    Metroflip* app = context;
    widget_reset(app->widget);

    if(app->poller) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }

    // Clear view
    popup_reset(app->popup);

    metroflip_app_blink_stop(app);
}
