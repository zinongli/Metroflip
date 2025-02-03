#include "../metroflip_i.h"
#include <dolphin/dolphin.h>
#include "../api/metroflip/metroflip_api.h"

#define TAG "Metroflip:Scene:SupportedCards"

void metroflip_scene_supported_on_enter(void* context) {
    Metroflip* app = context;
    Widget* widget = app->widget;

    dolphin_deed(DolphinDeedNfcReadSuccess);
    furi_string_reset(app->text_box_store);

    FuriString* str = furi_string_alloc();

    furi_string_printf(str, "\e#Supported Cards:\n\n");
    furi_string_cat_printf(
        str, " - Bip!:\nSantiago de Chile, Chile\nProtocol: Mifare Classic\n\n");
    furi_string_cat_printf(str, " - Charliecard:\nBoston, MA, USA\nProtocol: Mifare Classic\n\n");
    furi_string_cat_printf(
        str, " - Clipper:\nSan Francisco, CA, USA\nProtocol: Mifare DESFire\n\n");
    furi_string_cat_printf(str, " - ITSO:\nUnited Kingdom\nProtocol: Mifare DESFire\n\n");
    furi_string_cat_printf(str, " - Metromoney:\nTbilisi, Georgia\nProtocol: Mifare Classic\n\n");
    furi_string_cat_printf(
        str,
        " - myki:\n🇦🇺 Melbourne (and surrounds), VIC, Australia\nProtocol: Mifare DESFire\n\n");
    furi_string_cat_printf(str, " - Navigo:\nParis, France\nProtocol: Calypso\n\n");
    furi_string_cat_printf(
        str, " - Opal:\nSydney (and surrounds), NSW, Australia\nProtocol: Mifare DESFire\n\n");
    furi_string_cat_printf(str, " - Opus:\nMontreal, QC, Canada\nProtocol: Calypso\n\n");
    furi_string_cat_printf(str, " - Rav-Kav:\nIsrael\nProtocol: Calypso\n\n");
    furi_string_cat_printf(
        str, " - SmartRider:\nWestern Australia, Australia\nProtocol: Mifare Classic\n\n");
    furi_string_cat_printf(str, " - Troika:\nMoscow, Russia\nProtocol: Mifare Classic\n\n");

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(str));

    widget_add_button_element(
        widget, GuiButtonTypeRight, "Exit", metroflip_exit_widget_callback, app);

    furi_string_free(str);
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
}

bool metroflip_scene_supported_on_event(void* context, SceneManagerEvent event) {
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

void metroflip_scene_supported_on_exit(void* context) {
    Metroflip* app = context;
    widget_reset(app->widget);
    UNUSED(context);
}
