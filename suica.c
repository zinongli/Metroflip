
#include "suica_i.h"

static bool suica_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Suica* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool suica_back_event_callback(void* context) {
    furi_assert(context);
    Suica* app = context;

    return scene_manager_handle_back_event(app->scene_manager);
}

Suica* suica_alloc() {
    Suica* app = malloc(sizeof(Suica));
    app->gui = furi_record_open(RECORD_GUI);
    //nfc device
    app->nfc = nfc_alloc();
    app->nfc_device = nfc_device_alloc();

    // notifs
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    // View Dispatcher and Scene Manager
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&suica_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(app->view_dispatcher, suica_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, suica_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Custom Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, SuicaViewWidget, widget_get_view(app->widget));

    // Gui Modules
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SuicaViewSubmenu, submenu_get_view(app->submenu));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SuicaViewTextInput, text_input_get_view(app->text_input));

    app->popup = popup_alloc();
popup_set_timeout(app->popup, 2000);
    view_dispatcher_add_view(app->view_dispatcher, SuicaViewPopup, popup_get_view(app->popup));
    app->nfc_device = nfc_device_alloc();

    return app;
}

void suica_free(Suica* app) {
    furi_assert(app);

    //nfc device
    nfc_free(app->nfc);
    nfc_device_free(app->nfc_device);

    //notifs
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Gui modules
    view_dispatcher_remove_view(app->view_dispatcher, SuicaViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, SuicaViewTextInput);
    text_input_free(app->text_input);
    view_dispatcher_remove_view(app->view_dispatcher, SuicaViewPopup);
    popup_free(app->popup);

    // Custom Widget
    view_dispatcher_remove_view(app->view_dispatcher, SuicaViewWidget);
    widget_free(app->widget);

    // View Dispatcher and Scene Manager
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Records
    furi_record_close(RECORD_GUI);
    free(app);
}

static const NotificationSequence suica_app_sequence_blink_start_blue = {
    &message_blink_start_10,
    &message_blink_set_color_blue,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence suica_app_sequence_blink_stop = {
    &message_blink_stop,
    NULL,
};

void suica_app_blink_start(Suica* suica) {
    notification_message(suica->notifications, &suica_app_sequence_blink_start_blue);
}

void suica_app_blink_stop(Suica* suica) {
    notification_message(suica->notifications, &suica_app_sequence_blink_stop);
}

void suica_exit_widget_callback(GuiButtonType result, InputType type, void* context) {
    Suica* app = context;
    UNUSED(result);

    if(type == InputTypeShort) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SuicaSceneStart);
    }
}

extern int32_t suica_ep(void* p) {
    UNUSED(p);
    Suica* app = suica_alloc();
    scene_manager_set_scene_state(app->scene_manager, SuicaSceneStart, SuicaSceneRead);
    scene_manager_next_scene(app->scene_manager, SuicaSceneStart);
    view_dispatcher_run(app->view_dispatcher);
    suica_free(app);
    return 0;
}
