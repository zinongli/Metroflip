#include <gui/gui.h>
#include <gui/modules/widget_elements/widget_element.h>
#include "../api/calypso/transit/navigo.h"
#include "../api/calypso/transit/opus.h"
#include "../api/calypso/calypso_i.h"
#include <datetime.h>
#include <stdbool.h>

void metroflip_back_button_widget_callback(GuiButtonType result, InputType type, void* context);
void metroflip_next_button_widget_callback(GuiButtonType result, InputType type, void* context);
