#include "../suica_i.h"
#include <dolphin/dolphin.h>

#define TAG "Suica:Scene:About"

void suica_scene_about_on_enter(void* context) {
    Suica* app = context;
    Widget* widget = app->widget;

    dolphin_deed(DolphinDeedNfcReadSuccess);
    FuriString* str = furi_string_alloc();

    furi_string_printf(str, "\e#About:\n\n");
    furi_string_cat_printf(
        str,
        "Suica is a multi-protocol metro card reader app for the Flipper Zero, created by luu176, inspired by the Metrodroid project. It enables the parsing and analysis of metro cards from transit systems around the world, providing a proof-of-concept for exploring transit card data in a portable format.");

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(str));

    widget_add_button_element(widget, GuiButtonTypeRight, "Exit", suica_exit_widget_callback, app);

    furi_string_free(str);
    view_dispatcher_switch_to_view(app->view_dispatcher, SuicaViewWidget);
}

bool suica_scene_about_on_event(void* context, SceneManagerEvent event) {
    Suica* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(app->scene_manager);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SuicaSceneStart);
        consumed = true;
    }
    return consumed;
}

void suica_scene_about_on_exit(void* context) {
    Suica* app = context;
    widget_reset(app->widget);
    UNUSED(context);
}
