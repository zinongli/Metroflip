
#include "metroflip_i.h"

static bool metroflip_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Metroflip* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool metroflip_back_event_callback(void* context) {
    furi_assert(context);
    Metroflip* app = context;

    return scene_manager_handle_back_event(app->scene_manager);
}

Metroflip* metroflip_alloc() {
    Metroflip* app = malloc(sizeof(Metroflip));
    app->gui = furi_record_open(RECORD_GUI);
    //nfc device
    app->nfc = nfc_alloc();
    app->nfc_device = nfc_device_alloc();

    // key cache
    app->mfc_key_cache = mf_classic_key_cache_alloc();

    // notifs
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    // View Dispatcher and Scene Manager
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&metroflip_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, metroflip_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, metroflip_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Custom Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MetroflipViewWidget, widget_get_view(app->widget));

    // Gui Modules
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MetroflipViewSubmenu, submenu_get_view(app->submenu));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MetroflipViewTextInput, text_input_get_view(app->text_input));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MetroflipViewByteInput, byte_input_get_view(app->byte_input));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, MetroflipViewPopup, popup_get_view(app->popup));
    app->nfc_device = nfc_device_alloc();

    // TextBox
    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MetroflipViewTextBox, text_box_get_view(app->text_box));
    app->text_box_store = furi_string_alloc();

    return app;
}

void metroflip_free(Metroflip* app) {
    furi_assert(app);

    //nfc device
    nfc_free(app->nfc);
    nfc_device_free(app->nfc_device);

    // key cache
    mf_classic_key_cache_free(app->mfc_key_cache);

    //notifs
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Gui modules
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewTextInput);
    text_input_free(app->text_input);
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewByteInput);
    byte_input_free(app->byte_input);
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewPopup);
    popup_free(app->popup);

    // Custom Widget
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewWidget);
    widget_free(app->widget);

    // TextBox
    view_dispatcher_remove_view(app->view_dispatcher, MetroflipViewTextBox);
    text_box_free(app->text_box);
    furi_string_free(app->text_box_store);

    // View Dispatcher and Scene Manager
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Records
    furi_record_close(RECORD_GUI);
    free(app);
}

static const NotificationSequence metroflip_app_sequence_blink_start_blue = {
    &message_blink_start_10,
    &message_blink_set_color_blue,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence metroflip_app_sequence_blink_stop = {
    &message_blink_stop,
    NULL,
};

void metroflip_app_blink_start(Metroflip* metroflip) {
    notification_message(metroflip->notifications, &metroflip_app_sequence_blink_start_blue);
}

void metroflip_app_blink_stop(Metroflip* metroflip) {
    notification_message(metroflip->notifications, &metroflip_app_sequence_blink_stop);
}

void metroflip_exit_widget_callback(GuiButtonType result, InputType type, void* context) {
    Metroflip* app = context;
    UNUSED(result);

    if(type == InputTypeShort) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
    }
}

// Calypso

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

void locale_format_datetime_cat(FuriString* out, const DateTime* dt, bool time) {
    // helper to print datetimes
    FuriString* s = furi_string_alloc();

    LocaleDateFormat date_format = locale_get_date_format();
    const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";
    locale_format_date(s, dt, date_format, separator);
    furi_string_cat(out, s);
    if(time) {
        locale_format_time(s, dt, locale_get_time_format(), false);
        furi_string_cat_printf(out, "  ");
        furi_string_cat(out, s);
    }

    furi_string_free(s);
}

// read file
uint8_t read_file[5] = {0x94, 0xb2, 0x01, 0x04, 0x1D};
//                                 ^^^
//                                 |||
//                                 FID

// select app
uint8_t select_app[8] = {0x94, 0xA4, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00};
//                                                    ^^^^^^^^^
//                                                    |||||||||
//                                                    AID: 20XX

uint8_t apdu_success[2] = {0x90, 0x00};

int bit_slice_to_dec(const char* bit_representation, int start, int end) {
    char bit_slice[end - start + 2];
    strncpy(bit_slice, bit_representation + start, end - start + 1);
    bit_slice[end - start + 1] = '\0';
    return binary_to_decimal(bit_slice);
}

extern int32_t metroflip(void* p) {
    UNUSED(p);
    Metroflip* app = metroflip_alloc();
    scene_manager_set_scene_state(app->scene_manager, MetroflipSceneStart, MetroflipSceneRavKav);
    scene_manager_next_scene(app->scene_manager, MetroflipSceneStart);
    view_dispatcher_run(app->view_dispatcher);
    metroflip_free(app);
    return 0;
}

void dec_to_bits(char dec_representation, char* bit_representation) {
    int decimal = dec_representation - '0';
    for(int i = 7; i >= 0; --i) {
        bit_representation[i] = (decimal & (1 << i)) ? '1' : '0';
    }
}

KeyfileManager manage_keyfiles(char uid_str[]) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* source = storage_file_alloc(storage);
    char source_path[64];

    FURI_LOG_I("TAG", "%s", uid_str);
    size_t source_required_size =
        strlen("/ext/nfc/.cache/") + strlen(uid_str) + strlen(".keys") + 1;
    snprintf(source_path, source_required_size, "/ext/nfc/.cache/%s.keys", uid_str);
    bool cache_file = storage_file_open(source, source_path, FSAM_READ, FSOM_OPEN_EXISTING);
    /*-----------------Open assets cache file (if exists)------------*/

    File* dest = storage_file_alloc(storage);
    char dest_path[64];
    size_t dest_required_size =
        strlen("/ext/nfc/assets/.") + strlen(uid_str) + strlen(".keys") + 1;
    snprintf(dest_path, dest_required_size, "/ext/nfc/assets/.%s.keys", uid_str);
    bool dest_cache_file = storage_file_open(dest, dest_path, FSAM_READ, FSOM_OPEN_EXISTING);

    /*-----------------Check cache file------------*/
    if(!cache_file) {
        /*-----------------Check assets cache file------------*/
        FURI_LOG_I("TAG", "cache dont exist, checking assets");

        if(!dest_cache_file) {
            FURI_LOG_I("TAG", "assets dont exist, prompting user to fix..");
            storage_file_close(source);
            storage_file_close(dest);
            return MISSING_KEYFILE;
        } else {
            size_t dest_file_length = storage_file_size(dest);

            FURI_LOG_I(
                "TAG", "assets exist, but cache doesnt, proceeding to copy assets to cache");
            // Close, then open both files
            storage_file_close(source);
            storage_file_close(dest);
            storage_file_open(
                source, source_path, FSAM_WRITE, FSOM_OPEN_ALWAYS); // create new file
            storage_file_open(
                dest, dest_path, FSAM_READ, FSOM_OPEN_EXISTING); // open existing assets keyfile
            FURI_LOG_I("TAG", "creating cache file at %s from %s", source_path, dest_path);
            /*-----Clone keyfile from assets to cache (creates temporary buffer)----*/
            uint8_t* cloned_buffer = malloc(dest_file_length);
            storage_file_read(dest, cloned_buffer, dest_file_length);
            storage_file_write(source, cloned_buffer, dest_file_length);
            free(cloned_buffer);
            storage_file_close(source);
            storage_file_close(dest);
            return SUCCESSFUL;
        }
    } else {
        size_t source_file_length = storage_file_size(source);
        if(source_file_length > 1216) {
            FURI_LOG_I("TAG", "cache exist, creating assets cache if not already exists");
            storage_file_close(dest);
            storage_file_close(source);
            storage_file_open(dest, dest_path, FSAM_WRITE, FSOM_OPEN_ALWAYS);
            storage_file_open(source, source_path, FSAM_READ, FSOM_OPEN_EXISTING);
            FURI_LOG_I("TAG", "creating assets cache");
            /*-----Clone keyfile from assets to cache (creates temporary buffer)----*/
            uint8_t* cloned_buffer = malloc(source_file_length);
            storage_file_read(source, cloned_buffer, source_file_length);
            storage_file_write(dest, cloned_buffer, source_file_length);
            free(cloned_buffer);
            storage_file_close(source);
            storage_file_close(dest);
            return SUCCESSFUL;

        } else {
            FURI_LOG_I("TAG", "incomplete cache file, aborting.");
            storage_file_close(source);
            storage_file_close(dest);
            return INCOMPLETE_KEYFILE;
        }
    }
    FURI_LOG_I("TAG", "proceeding to read");
    storage_file_close(source);
    storage_file_close(dest);
}
