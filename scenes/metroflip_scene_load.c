#include "../metroflip_i.h"
#include <dolphin/dolphin.h>
#include <furi.h>
#include <bit_lib.h>
#include <lib/nfc/protocols/nfc_protocol.h>
#include "../api/metroflip/metroflip_api.h"
#define TAG "Metroflip:Scene:Load"

void metroflip_scene_load_on_enter(void* context) {
    Metroflip* app = (Metroflip*)context;
    // We initialized this to be false every time we enter
    app->data_loaded = false;
    // The same string we will use to direct parse scene which plugin to call
    // Extracted from the file
    FuriString* card_type = furi_string_alloc();

    // All the app_data browser stuff. Don't worry about this
    DialogsFileBrowserOptions browser_options;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, STORAGE_APP_DATA_PATH_PREFIX);
    dialog_file_browser_set_basic_options(&browser_options, METROFLIP_FILE_EXTENSION, &I_icon);
    browser_options.base_path = STORAGE_APP_DATA_PATH_PREFIX;
    FuriString* file_path = furi_string_alloc_set(browser_options.base_path);

    if(dialog_file_browser_show(app->dialogs, file_path, file_path, &browser_options)) {
        FlipperFormat* format = flipper_format_file_alloc(storage);
        do {
            if(!flipper_format_file_open_existing(format, furi_string_get_cstr(file_path))) break;
            if(!flipper_format_read_string(format, "Card Type", card_type)) break;
            if(furi_string_equal_str(card_type, "suica")) {
            }
            app->data_loaded = true;
        } while(0);
        flipper_format_free(format);
    }

    if(app->data_loaded) {
        // Direct to the parsing screen just like the auto scene does
        app->card_type = furi_string_get_cstr(card_type);
        scene_manager_next_scene(app->scene_manager, MetroflipSceneParse);
    } else {
        scene_manager_next_scene(app->scene_manager, MetroflipSceneStart);
    }
    furi_string_free(file_path);
    furi_string_free(card_type);
    furi_record_close(RECORD_STORAGE);
}

bool metroflip_scene_load_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    UNUSED(event);
    bool consumed = false;
    // If they don't select any file in the brwoser and press back button,
    // the data is not loaded
    if(!app->data_loaded) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
    }
    consumed = true;

    return consumed;
}

void metroflip_scene_load_on_exit(void* context) {
    Metroflip* app = context;
    UNUSED(app);
}
