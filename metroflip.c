
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


extern int32_t metroflip(void* p) {
    UNUSED(p);
    Metroflip* app = metroflip_alloc();
    scene_manager_set_scene_state(app->scene_manager, MetroflipSceneStart, MetroflipSceneSuica);
    scene_manager_next_scene(app->scene_manager, MetroflipSceneStart);
    view_dispatcher_run(app->view_dispatcher);
    metroflip_free(app);
    return 0;
}


