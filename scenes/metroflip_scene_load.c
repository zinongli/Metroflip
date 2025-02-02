#include "../metroflip_i.h"
#include <dolphin/dolphin.h>
#include <furi.h>
#include <bit_lib.h>
#include <lib/nfc/protocols/nfc_protocol.h>
#include "../api/metroflip/metroflip_api.h"
#define TAG "Metroflip:Scene:Load"

void metroflip_scene_load_on_enter(void* context) {
    Metroflip* app = (Metroflip*)context;
    DialogsFileBrowserOptions browser_options;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, STORAGE_APP_DATA_PATH_PREFIX);
    dialog_file_browser_set_basic_options(&browser_options, METROFLIP_FILE_EXTENSION, &I_icon);
    browser_options.base_path = STORAGE_APP_DATA_PATH_PREFIX;
    FuriString* file_path = furi_string_alloc_set(browser_options.base_path);
    // FuriString* buffer = furi_string_alloc();
    FURI_LOG_I(TAG, "Opening file path %s", furi_string_get_cstr(file_path));
    if(dialog_file_browser_show(app->dialogs, file_path, file_path, &browser_options)) {
        FlipperFormat* format = flipper_format_file_alloc(storage);
        do {
            if(!flipper_format_file_open_existing(format, furi_string_get_cstr(file_path))) break;
            // uint8_t byte_array_buffer[app->bytes_count];
            // for(int i = 0; i < LFRFID_T5577_BLOCK_COUNT; i++) {
            //     furi_string_printf(buffer, "Block %u", i);
            //     if(!flipper_format_read_hex(
            //            format, furi_string_get_cstr(buffer), byte_array_buffer, app->bytes_count))
            //         break;
            //     model->content[i] = byte_buffer_to_uint32(
            //         byte_array_buffer); // we only extract the raw data. configs are then updated from block 0
            // }
            // signal that the file was read successfully
        } while(0);
        flipper_format_free(format);
        furi_record_close(RECORD_STORAGE);
    }
    // view_dispatcher_switch_to_view(app->view_dispatcher, T5577WriterViewSubmenu);
}

bool metroflip_scene_load_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    UNUSED(event);
    bool consumed = false;
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
    consumed = true;

    return consumed;
}

void metroflip_scene_load_on_exit(void* context) {
    Metroflip* app = context;
    UNUSED(app);
    FURI_LOG_I(TAG, "Exiting auto scene");
}
