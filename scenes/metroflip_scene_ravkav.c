#include "../metroflip_i.h"
#include <datetime.h>
#include <dolphin/dolphin.h>
#include <locale/locale.h>

#include <nfc/protocols/iso14443_4b/iso14443_4b_poller.h>

#define Metroflip_POLLER_MAX_BUFFER_SIZE 1024

#define TAG "Metroflip:Scene:RavKav"

#define epoch_ravkav 852073200

uint8_t apdu_success[] = {0x90, 0x00};

// balance
uint8_t select_balance_file[] = {0x94, 0xA4, 0x00, 0x00, 0x02, 0x20, 0x2A, 0x00};
uint8_t read_balance[] = {0x94, 0xb2, 0x01, 0x04, 0x1D};

// balance
uint8_t select_events_aid[] = {0x94, 0xA4, 0x00, 0x00, 0x02, 0x20, 0x10, 0x00};
uint8_t read_event[] = {0x00, 0xb2, 0x01, 0x04, 0x1D};

void locale_format_datetime_cat(FuriString* out, const DateTime* dt) {
    // helper to print datetimes
    FuriString* s = furi_string_alloc();

    LocaleDateFormat date_format = locale_get_date_format();
    const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";
    locale_format_date(s, dt, date_format, separator);
    furi_string_cat(out, s);
    locale_format_time(s, dt, locale_get_time_format(), false);
    furi_string_cat_printf(out, "  ");
    furi_string_cat(out, s);

    furi_string_free(s);
}

void byte_to_binary(uint8_t byte, char* bits) {
    for(int i = 7; i >= 0; i--) {
        bits[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    bits[8] = '\0';
}

int binary_to_decimal(const char binary[]) {
    int decimal = 0;
    int length = strlen(binary);

    for(int i = 0; i < length; i++) {
        decimal = decimal * 2 + (binary[i] - '0');
    }

    return decimal;
}

void metroflip_charliecard_ravkav_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Metroflip* app = context;
    UNUSED(result);

    if(type == InputTypeShort) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
    }
}

static NfcCommand metroflip_scene_ravkav_poller_callback(NfcGenericEvent event, void* context) {
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
                // Select file of balance
                bit_buffer_append_bytes(
                    tx_buffer, select_balance_file, sizeof(select_balance_file));
                error = iso14443_4b_poller_send_block(iso14443_4b_poller, tx_buffer, rx_buffer);
                if(error != Iso14443_4bErrorNone) {
                    FURI_LOG_I(TAG, "Select File: iso14443_4b_poller_send_block error %d", error);
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFail);
                    break;
                }

                // Check the response after selecting file
                response_length = bit_buffer_get_size_bytes(rx_buffer);
                if(bit_buffer_get_byte(rx_buffer, response_length - 2) != apdu_success[0] ||
                   bit_buffer_get_byte(rx_buffer, response_length - 1) != apdu_success[1]) {
                    FURI_LOG_I(
                        TAG,
                        "Select file failed: %02x%02x",
                        bit_buffer_get_byte(rx_buffer, response_length - 2),
                        bit_buffer_get_byte(rx_buffer, response_length - 1));
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFileNotFound);
                    break;
                }

                // Now send the read command
                bit_buffer_reset(tx_buffer);
                bit_buffer_append_bytes(tx_buffer, read_balance, sizeof(read_balance));
                error = iso14443_4b_poller_send_block(iso14443_4b_poller, tx_buffer, rx_buffer);
                if(error != Iso14443_4bErrorNone) {
                    FURI_LOG_I(TAG, "Read File: iso14443_4b_poller_send_block error %d", error);
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFail);
                    break;
                }

                // Check the response after reading the file
                response_length = bit_buffer_get_size_bytes(rx_buffer);
                if(bit_buffer_get_byte(rx_buffer, response_length - 2) != apdu_success[0] ||
                   bit_buffer_get_byte(rx_buffer, response_length - 1) != apdu_success[1]) {
                    FURI_LOG_I(
                        TAG,
                        "Read file failed: %02x%02x",
                        bit_buffer_get_byte(rx_buffer, response_length - 2),
                        bit_buffer_get_byte(rx_buffer, response_length - 1));
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFileNotFound);
                    break;
                }

                // Process the response data
                if(response_length < 3) {
                    FURI_LOG_I(TAG, "Response too short: %d bytes", response_length);
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFail);
                    break;
                }

                uint32_t value = 0;
                for(uint8_t i = 0; i < 3; i++) {
                    value = (value << 8) | bit_buffer_get_byte(rx_buffer, i);
                }

                float result = value / 100.0f;
                FURI_LOG_I(TAG, "Value: %.2f ILS", (double)result);
                furi_string_printf(parsed_data, "\e#Rav-Kav:\n");
                furi_string_cat_printf(parsed_data, "Card Type: Anonymous\n");
                furi_string_cat_printf(parsed_data, "Balance: %.2f ILS\n", (double)result);

                // Select app for events
                bit_buffer_reset(tx_buffer);
                bit_buffer_append_bytes(tx_buffer, select_events_aid, sizeof(select_events_aid));
                error = iso14443_4b_poller_send_block(iso14443_4b_poller, tx_buffer, rx_buffer);
                if(error != Iso14443_4bErrorNone) {
                    FURI_LOG_I(TAG, "Select File: iso14443_4b_poller_send_block error %d", error);
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFail);
                    break;
                }

                // Check the response after selecting app
                response_length = bit_buffer_get_size_bytes(rx_buffer);
                if(bit_buffer_get_byte(rx_buffer, response_length - 2) != apdu_success[0] ||
                   bit_buffer_get_byte(rx_buffer, response_length - 1) != apdu_success[1]) {
                    FURI_LOG_I(
                        TAG,
                        "Select events app failed: %02x%02x",
                        bit_buffer_get_byte(rx_buffer, response_length - 2),
                        bit_buffer_get_byte(rx_buffer, response_length - 1));
                    stage = MetroflipPollerEventTypeFail;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher, MetroflipCustomEventPollerFileNotFound);
                    break;
                }

                // Now send the read command
                for(size_t i = 1; i < 7; i++) {
                    read_event[2] = i;
                    bit_buffer_reset(tx_buffer);
                    bit_buffer_append_bytes(tx_buffer, read_event, sizeof(read_event));
                    error =
                        iso14443_4b_poller_send_block(iso14443_4b_poller, tx_buffer, rx_buffer);
                    if(error != Iso14443_4bErrorNone) {
                        FURI_LOG_I(
                            TAG, "Read File: iso14443_4b_poller_send_block error %d", error);
                        stage = MetroflipPollerEventTypeFail;
                        view_dispatcher_send_custom_event(
                            app->view_dispatcher, MetroflipCustomEventPollerFail);
                        break;
                    }

                    // Check the response after reading the file
                    response_length = bit_buffer_get_size_bytes(rx_buffer);
                    if(bit_buffer_get_byte(rx_buffer, response_length - 2) != apdu_success[0] ||
                       bit_buffer_get_byte(rx_buffer, response_length - 1) != apdu_success[1]) {
                        FURI_LOG_I(
                            TAG,
                            "Read file failed: %02x%02x",
                            bit_buffer_get_byte(rx_buffer, response_length - 2),
                            bit_buffer_get_byte(rx_buffer, response_length - 1));
                        stage = MetroflipPollerEventTypeFail;
                        view_dispatcher_send_custom_event(
                            app->view_dispatcher, MetroflipCustomEventPollerFileNotFound);
                        break;
                    }
                    char bit_representation
                        [response_length * 8 + 1]; // Total bits in the response (each byte = 8 bits)
                    bit_representation[0] = '\0'; // Initialize the string to empty
                    for(size_t i = 0; i < response_length; i++) {
                        char bits[9]; // Temporary string for each byte (8 bits + null terminator)
                        uint8_t byte = bit_buffer_get_byte(rx_buffer, i);
                        byte_to_binary(byte, bits);
                        strcat(bit_representation, bits); // Append binary string to the result
                    }
                    int start = 23, end = 52;
                    char bit_slice[end - start + 2];
                    strncpy(bit_slice, bit_representation + start, end - start + 1);
                    bit_slice[end - start + 1] = '\0';
                    int decimal_value = binary_to_decimal(bit_slice);
                    uint64_t result_timestamp = decimal_value + epoch_ravkav + (3600 * 3);
                    DateTime dt = {0};
                    datetime_timestamp_to_datetime(result_timestamp, &dt);
                    furi_string_cat_printf(parsed_data, "\nEvent 0%d:\n", i);
                    locale_format_datetime_cat(parsed_data, &dt);
                    furi_string_cat_printf(parsed_data, "\n\n");
                }

                widget_add_text_scroll_element(
                    widget, 0, 0, 128, 64, furi_string_get_cstr(parsed_data));

                widget_add_button_element(
                    widget,
                    GuiButtonTypeRight,
                    "Exit",
                    metroflip_charliecard_ravkav_widget_callback,
                    app);

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

void metroflip_scene_ravkav_on_enter(void* context) {
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
    nfc_poller_start(app->poller, metroflip_scene_ravkav_poller_callback, app);

    metroflip_app_blink_start(app);
}

bool metroflip_scene_ravkav_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MetroflipPollerEventTypeCardDetect) {
            Popup* popup = app->popup;
            popup_set_header(popup, "Scanning..", 68, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MetroflipCustomEventPollerFileNotFound) {
            Popup* popup = app->popup;
            popup_set_header(popup, "No\nRecord\nFile", 68, 30, AlignLeft, AlignTop);
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

void metroflip_scene_ravkav_on_exit(void* context) {
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
