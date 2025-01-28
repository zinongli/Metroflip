#include "../suica_i.h"

void suica_scene_start_submenu_callback(void* context, uint32_t index) {
    Suica* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void suica_scene_start_on_enter(void* context) {
    Suica* app = context;
    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, "Suica Reader");

    submenu_add_item(
        submenu, "Read", SuicaSceneRead, suica_scene_start_submenu_callback, app);

    submenu_add_item(
        submenu, "About", SuicaSceneAbout, suica_scene_start_submenu_callback, app);

    submenu_add_item(
        submenu, "Credits", SuicaSceneCredits, suica_scene_start_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SuicaSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, SuicaViewSubmenu);
}

bool suica_scene_start_on_event(void* context, SceneManagerEvent event) {
    Suica* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SuicaSceneStart, event.event);
        consumed = true;
        scene_manager_next_scene(app->scene_manager, event.event);
    }

    return consumed;
}

void suica_scene_start_on_exit(void* context) {
    Suica* app = context;
    submenu_reset(app->submenu);
}
