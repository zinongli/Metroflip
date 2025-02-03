#include "../metroflip_i.h"
#include <dolphin/dolphin.h>
#include <furi.h>
#include <bit_lib.h>
#include <lib/nfc/protocols/nfc_protocol.h>
#include <lib/nfc/protocols/felica/felica.h>
#include "../api/metroflip/metroflip_api.h"
#include "../api/suica/suica_structs_i.h"
#define TAG "Metroflip:Scene:Load"

static void suica_add_entry(SuicaHistoryViewModel* model, const uint8_t* entry) {
    if(model->size <= 0) {
        model->travel_history =
            (uint8_t*)malloc(3 * FELICA_DATA_BLOCK_SIZE); // Each entry is 16 bytes
        model->size = 0;
        model->capacity = 3;
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
}

void metroflip_scene_load_on_enter(void* context) {
    Metroflip* app = (Metroflip*)context;
    app->data_loaded = false;
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
            uint8_t* byte_array_buffer = (uint8_t*)malloc(FELICA_DATA_BLOCK_SIZE);
            FuriString* card_type = furi_string_alloc_set("suica");
            FuriString* entry_preamble = furi_string_alloc();
            if(!flipper_format_read_string(format, "Card Type", card_type)) break;
            if(furi_string_equal_str(card_type, "suica")) {
                // Initialize the Suica context
                app->suica_context = malloc(sizeof(SuicaContext));
                app->suica_context->view_history = view_alloc();
                view_set_context(app->suica_context->view_history, app);
                view_allocate_model(
                    app->suica_context->view_history,
                    ViewModelTypeLockFree,
                    sizeof(SuicaHistoryViewModel));
                SuicaHistoryViewModel* model = view_get_model(app->suica_context->view_history);
                FURI_LOG_I(TAG, "Model allocated");
                // Read the travel history entries
                for(uint8_t i = 0; i < SUICA_MAX_HISTORY_ENTRIES; i++) {
                    furi_string_printf(entry_preamble, "Travel %02X", i);
                    if(!flipper_format_read_hex(
                           format,
                           furi_string_get_cstr(entry_preamble),
                           byte_array_buffer,
                           FELICA_DATA_BLOCK_SIZE))
                        break;
                    FURI_LOG_I(TAG, "Adding entry %d", i);
                    uint8_t block_data[16] = {0};
                    for(size_t j = 0; j < FELICA_DATA_BLOCK_SIZE; j++) {
                        block_data[j] = byte_array_buffer[j];
                    }
                    suica_add_entry(model, block_data);
                    FURI_LOG_I(TAG, "Entry successfully added %d", i);
                }
                app->data_loaded = true;
                furi_string_free(entry_preamble);
                furi_string_free(card_type);
                free(byte_array_buffer);
            }
            // signal that the file was read successfully
        } while(0);
        flipper_format_free(format);
    }
    furi_string_free(file_path);
    furi_record_close(RECORD_STORAGE);

    if(app->data_loaded) {
        if(strcmp(app->card_type, "suica")) {
            strncpy(app->card_type, "suica", sizeof(app->card_type) - 1);
            scene_manager_next_scene(app->scene_manager, MetroflipSceneParse);
        }
    } else {
        scene_manager_next_scene(app->scene_manager, MetroflipSceneStart);
    }
}

bool metroflip_scene_load_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    UNUSED(event);
    bool consumed = false;
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
