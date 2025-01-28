#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <stdlib.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/modules/validators.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#if __has_include(<suica_icons.h>)
#include <suica_icons.h>
#else
extern const Icon I_RFIDDolphinReceive_97x61;
#endif

#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/text_input.h>
#include <gui/modules/text_box.h>
#include <gui/modules/widget.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/popup.h>
#include "scenes/suica_scene.h"
#include <lib/flipper_format/flipper_format.h>
#include <toolbox/name_generator.h>
#include <lib/nfc/protocols/mf_ultralight/mf_ultralight.h>
#include <lib/nfc/helpers/nfc_data_generator.h>
#include <furi_hal_bt.h>
#include <notification/notification_messages.h>

#include <lib/nfc/nfc.h>
#include <nfc/nfc_poller.h>
#include <nfc/nfc_scanner.h>
#include <datetime.h>
#include <dolphin/dolphin.h>
#include <locale/locale.h>
#include <stdio.h>
#include <strings.h>
#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>

#include "scenes/suica_scene.h"

#include "api/suica/suica_structs.h"

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    NotificationApp* notifications;
    Submenu* submenu;
    TextInput* text_input;
    Popup* popup;
    uint8_t mac_buf[GAP_MAC_ADDR_SIZE];
    Widget* widget;

    Nfc* nfc;
    NfcPoller* poller;
    NfcScanner* scanner;
    NfcDevice* nfc_device;

    // Suica specific context
    SuicaContext* suica_context;
} Suica;

enum SuicaCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    SuicaCustomEventReserved = 100,

    SuicaCustomEventViewExit,
    SuicaCustomEventByteInputDone,
    SuicaCustomEventTextInputDone,
    SuicaCustomEventWorkerExit,

    SuicaCustomEventPollerDetect,
    SuicaCustomEventPollerSuccess,
    SuicaCustomEventPollerFail,
    SuicaCustomEventPollerSelectFailed,
    SuicaCustomEventPollerFileNotFound,

    SuicaCustomEventCardLost,
    SuicaCustomEventCardDetected,
    SuicaCustomEventWrongCard
};

typedef enum {
    SuicaPollerEventTypeStart,
    SuicaPollerEventTypeCardDetect,

    SuicaPollerEventTypeSuccess,
    SuicaPollerEventTypeFail,
} SuicaPollerEventType;

typedef enum {
    SuicaViewSubmenu,
    SuicaViewTextInput,
    SuicaViewByteInput,
    SuicaViewPopup,
    SuicaViewMenu,
    SuicaViewLoading,
    SuicaViewTextBox,
    SuicaViewWidget,
    SuicaViewUart,
    SuicaViewCanvas,
} SuicaView;

void suica_app_blink_start(Suica* suica);
void suica_app_blink_stop(Suica* suica);

#ifdef FW_ORIGIN_Official
#define submenu_add_lockable_item(                                             \
    submenu, label, index, callback, callback_context, locked, locked_message) \
    if(!(locked)) submenu_add_item(submenu, label, index, callback, callback_context)
#endif

void suica_exit_widget_callback(GuiButtonType result, InputType type, void* context);
