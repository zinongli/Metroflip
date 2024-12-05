
#include <flipper_application.h>
#include "../metroflip_i.h"

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>

#include <dolphin/dolphin.h>
#include <bit_lib.h>
#include <furi_hal.h>
#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/nfc_listener.h>

#define TAG "Metroflip:Scene:Metromoney"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const MfClassicKeyPair metromoney_1k_keys[] = {
    {.a = 0x2803BCB0C7E1, .b = 0x4FA9EB49F75E},
    {.a = 0x9C616585E26D, .b = 0xD1C71E590D16},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0x112233445566, .b = 0x361A62F35BC9},
    {.a = 0x112233445566, .b = 0x361A62F35BC9},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
};

static bool metromoney_parse(const NfcDevice* device, const MfClassicData* data, Metroflip* app) {
    furi_assert(device);

    bool parsed = false;

    do {
        // Verify key
        const uint8_t ticket_sector_number = 1;
        const uint8_t ticket_block_number = 1;

        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, ticket_sector_number);

        const uint64_t key =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != metromoney_1k_keys[ticket_sector_number].a) break;
        FURI_LOG_I(TAG, "passed key check");
        // Parse data
        const uint8_t start_block_num =
            mf_classic_get_first_block_num_of_sector(ticket_sector_number);

        const uint8_t* block_start_ptr =
            &data->block[start_block_num + ticket_block_number].data[0];

        uint32_t balance = bit_lib_bytes_to_num_le(block_start_ptr, 4) - 100;

        uint32_t balance_lari = balance / 100;
        uint8_t balance_tetri = balance % 100;

        size_t uid_len = 0;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        uint32_t card_number = bit_lib_bytes_to_num_le(uid, 4);
        strncpy(app->card_type, "Metromoney", sizeof(app->card_type));
        app->balance_lari = balance_lari;
        app->balance_tetri = balance_tetri;
        app->card_number = card_number;
        parsed = true;
    } while(false);

    return parsed;
}

static NfcCommand
    metroflip_scene_metromoney_poller_callback(NfcGenericEvent event, void* context) {
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
        bit_lib_num_to_bytes_be(metromoney_1k_keys[app->sec_num].a, COUNT_OF(key.data), key.data);

        MfClassicKeyType key_type = MfClassicKeyTypeA;
        mfc_event->data->read_sector_request_data.sector_num = app->sec_num;
        mfc_event->data->read_sector_request_data.key = key;
        mfc_event->data->read_sector_request_data.key_type = key_type;
        mfc_event->data->read_sector_request_data.key_provided = true;
        if(app->sec_num == 16) {
            mfc_event->data->read_sector_request_data.key_provided = false;
            app->sec_num = 0;
        }
        app->sec_num++;
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        nfc_device_set_data(
            app->nfc_device, NfcProtocolMfClassic, nfc_poller_get_data(app->poller));
        const MfClassicData* mfc_data = nfc_device_get_data(app->nfc_device, NfcProtocolMfClassic);
        metromoney_parse(app->nfc_device, mfc_data, app);
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventPollerSuccess);
        command = NfcCommandStop;
        metroflip_app_blink_stop(app);
    } else if(mfc_event->type == MfClassicPollerEventTypeFail) {
        FURI_LOG_I(TAG, "fail");
        command = NfcCommandStop;
    }

    return command;
}

void metroflip_scene_metromoney_on_enter(void* context) {
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
    nfc_poller_start(app->poller, metroflip_scene_metromoney_poller_callback, app);

    metroflip_app_blink_start(app);
}

bool metroflip_scene_metromoney_on_event(void* context, SceneManagerEvent event) {
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

void metroflip_scene_metromoney_on_exit(void* context) {
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