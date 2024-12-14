#include "../metroflip_i.h"
#include <dolphin/dolphin.h>

#define TAG "Metroflip:Scene:ReadSuccess"

void metroflip_scene_read_success_on_enter(void* context) {
    Metroflip* app = context;
    Widget* widget = app->widget;

    dolphin_deed(DolphinDeedNfcReadSuccess);
    furi_string_reset(app->text_box_store);

    FuriString* str = furi_string_alloc();

    if(strcmp(app->card_type, "Metromoney") == 0) {
        FURI_LOG_I(TAG, "Metromoney card detected");
        furi_string_printf(
            str,
            "\e#Metromoney\nCard number: %lu\nBalance: %lu.%02u GEL",
            app->card_number,
            app->balance_lari,
            app->balance_tetri);
    } else {
        FURI_LOG_I(TAG, "Unknown card type");
        furi_string_printf(str, "\e#Unknown card\n");
    }

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(str));

    widget_add_button_element(
        widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);

    furi_string_free(str);
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
}

bool metroflip_scene_read_success_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(app->scene_manager);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        consumed = true;
    }
    return consumed;
}

void metroflip_scene_read_success_on_exit(void* context) {
    Metroflip* app = context;
    widget_reset(app->widget);
    UNUSED(context);
}
