#include "../metroflip_i.h"
#include <furi.h>
#include "../metroflip_plugins.h"
#define TAG "Metroflip:Scene:Parse"
#include <stdio.h>

void metroflip_scene_parse_on_enter(void* context) {
    Metroflip* app = context;
    metroflip_plugin_manager_alloc(app);

    // Check if card_type is empty or unknown
    if(app->card_type[0] == '\0') {
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventWrongCard);
    } else if(strcmp(app->card_type, "unknown") == 0) {
        view_dispatcher_send_custom_event(app->view_dispatcher, MetroflipCustomEventWrongCard);
    } else {
        char plugin_name[64]; // Adjust size as needed
        snprintf(plugin_name, sizeof(plugin_name), "%s_plugin.fal", app->card_type);

        // Construct the final path
        char path[128]; // Adjust for path length
        snprintf(path, sizeof(path), "/ext/apps_assets/metroflip/plugins/%s", plugin_name);
        FURI_LOG_I(TAG, "path %s", path);

        // Try loading the plugin
        if(plugin_manager_load_single(app->plugin_manager, path) != PluginManagerErrorNone) {
            FURI_LOG_E(TAG, "Failed to load parse plugin");
            return;
        }

        // Get and run the plugin's on_enter function
        const MetroflipPlugin* plugin = plugin_manager_get_ep(app->plugin_manager, 0);
        plugin->plugin_on_enter(app);
    }
}

bool metroflip_scene_parse_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MetroflipCustomEventWrongCard) {
            Popup* popup = app->popup;
            popup_set_header(popup, "card\n currently\n unsupported", 58, 31, AlignLeft, AlignTop);
            popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        return true;
    }

    // Get and run the plugin's on_event function
    const MetroflipPlugin* plugin = plugin_manager_get_ep(app->plugin_manager, 0);
    return plugin->plugin_on_event(app, event);
}

void metroflip_scene_parse_on_exit(void* context) {
    Metroflip* app = context;

    if(!((app->card_type[0] == '\0') || (strcmp(app->card_type, "unknown") == 0))) {
        // Get and run the plugin's on_exit function
        const MetroflipPlugin* plugin = plugin_manager_get_ep(app->plugin_manager, 0);
        plugin->plugin_on_exit(app);

        plugin_manager_free(app->plugin_manager);
        composite_api_resolver_free(app->resolver);
    }
    memset(app->card_type, 0, sizeof(app->card_type));
}
