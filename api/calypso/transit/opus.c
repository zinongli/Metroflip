#include "opus.h"
#include "opus_lists.h"
#include "../../../metroflip_i.h"

const char* get_opus_service_provider(int provider) {
    switch(provider) {
    case 0x01:
        return "STM";
    case 0x02:
        return "STM";
    case 0x03:
        return "RTL";
    case 0x04:
        return "RTM";
    case 0x05:
        return "RTC";
    case 0x06:
        return "STL";
    case 0x10:
        return "STLevis";
    default: {
        char* provider_str = malloc(10 * sizeof(char));
        if(!provider_str) {
            return "Unknown";
        }
        snprintf(provider_str, 10, "0x%02X", provider);
        return provider_str;
    }
    }
}

const char* get_opus_transport_type(int route_number) {
    if(route_number >= 0x01 && route_number <= 0x04) {
        return "Metro";
    } else {
        return "Bus";
    }
}

const char* get_opus_transport_line(int route_number) {
    if(OPUS_LINES_LIST[route_number]) {
        return OPUS_LINES_LIST[route_number];
    } else {
        char* line = malloc(4 * sizeof(char));
        if(!line) {
            return "Unknown";
        }
        snprintf(line, 4, "%d", route_number);
        return line;
    }
}

const char* get_opus_tariff(int tariff) {
    switch(tariff) {
    case 0xb1:
        return "Monthly";
    case 0xb2:
    case 0xc9:
        return "Weekly";
    case 0x1c7:
        return "Single Trips";
    case 0xa34:
        return "Monthly Student";
    case 0xa3e:
        return "Weekly";
    default: {
        char* tariff_str = malloc(9 * sizeof(char));
        if(!tariff_str) {
            return "Unknown";
        }
        snprintf(tariff_str, 9, "0x%02X", tariff);
        return tariff_str;
    }
    }
}

void show_opus_event_info(
    OpusCardEvent* event,
    OpusCardContract* contracts,
    FuriString* parsed_data) {
    UNUSED(contracts);
    furi_string_cat_printf(
        parsed_data,
        "%s %s\n",
        get_opus_transport_type(event->route_number),
        get_opus_transport_line(event->route_number));
    furi_string_cat_printf(
        parsed_data, "Transporter: %s\n", get_opus_service_provider(event->service_provider));
    if(event->used_contract_available) {
        furi_string_cat_printf(
            parsed_data,
            "Contract: %d - %s\n",
            event->used_contract,
            get_opus_tariff(contracts[event->used_contract - 1].tariff));
    }
    locale_format_datetime_cat(parsed_data, &event->date, true);
    furi_string_cat_printf(parsed_data, "\n");
}

void show_opus_contract_info(OpusCardContract* contract, FuriString* parsed_data) {
    furi_string_cat_printf(parsed_data, "Type: %s\n", get_opus_tariff(contract->tariff));
    furi_string_cat_printf(
        parsed_data, "Provider: %s\n", get_opus_service_provider(contract->provider));
    furi_string_cat_printf(parsed_data, "Valid\nfrom: ");
    locale_format_datetime_cat(parsed_data, &contract->start_date, false);
    furi_string_cat_printf(parsed_data, "\nto: ");
    locale_format_datetime_cat(parsed_data, &contract->end_date, false);
    furi_string_cat_printf(parsed_data, "\n");
    // furi_string_cat_printf(parsed_data, "Sold on: ");
    // locale_format_datetime_cat(parsed_data, &contract->sale_date, false);
    // furi_string_cat_printf(parsed_data, "\nStatus: %d\n", contract->status);
}

void show_opus_environment_info(OpusCardEnv* environment, FuriString* parsed_data) {
    furi_string_cat_printf(parsed_data, "App Version: %d\n", environment->app_version);
    furi_string_cat_printf(
        parsed_data, "Country: %s\n", get_country_string(environment->country_num));
    furi_string_cat_printf(
        parsed_data,
        "Network: %s\n",
        get_network_string(environment->country_num, environment->network_num));
    furi_string_cat_printf(parsed_data, "End of validity:\n");
    locale_format_datetime_cat(parsed_data, &environment->end_dt, false);
    furi_string_cat_printf(parsed_data, "\n");
}
