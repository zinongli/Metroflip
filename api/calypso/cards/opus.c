#include <stdlib.h>
#include "opus.h"

CalypsoApp* get_opus_contract_structure() {
    CalypsoApp* OpusContractStructure = malloc(sizeof(CalypsoApp));

    if(!OpusContractStructure) {
        return NULL;
    }

    int app_elements_count = 2;

    OpusContractStructure->type = CALYPSO_APP_CONTRACT;
    OpusContractStructure->container = malloc(sizeof(CalypsoContainerElement));
    OpusContractStructure->container->elements =
        malloc(app_elements_count * sizeof(CalypsoElement));
    OpusContractStructure->container->size = app_elements_count;

    OpusContractStructure->container->elements[0] =
        make_calypso_final_element("ContractUnknownA", 3, "Unknown A", CALYPSO_FINAL_TYPE_NUMBER);
    OpusContractStructure->container->elements[1] = make_calypso_bitmap_element(
        "Contract",
        4,
        (CalypsoElement[]){
            make_calypso_final_element(
                "ContractProvider", 8, "Provider", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element("ContractTariff", 16, "Tariff", CALYPSO_FINAL_TYPE_TARIFF),
            make_calypso_bitmap_element(
                "ContractDates",
                2,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractStartDate", 14, "Start date", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "ContractEndDate", 14, "End date", CALYPSO_FINAL_TYPE_DATE),
                }),
            make_calypso_bitmap_element(
                "ContractSaleData",
                6,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractUnknownB", 17, "Unknown B", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "ContractSaleDate", 14, "Sale date", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "ContractSaleTime", 11, "Sale time", CALYPSO_FINAL_TYPE_TIME),
                    make_calypso_final_element(
                        "ContractUnknownC", 36, "Unknown C", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "ContractStatus", 8, "Status", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractUnknownD", 36, "Unknown D", CALYPSO_FINAL_TYPE_NUMBER),
                }),
        });

    return OpusContractStructure;
}

CalypsoApp* get_opus_event_structure() {
    CalypsoApp* OpusEventStructure = malloc(sizeof(CalypsoApp));

    if(!OpusEventStructure) {
        return NULL;
    }

    int app_elements_count = 4;

    OpusEventStructure->type = CALYPSO_APP_EVENT;
    OpusEventStructure->container = malloc(sizeof(CalypsoContainerElement));
    OpusEventStructure->container->elements = malloc(app_elements_count * sizeof(CalypsoElement));
    OpusEventStructure->container->size = app_elements_count;

    OpusEventStructure->container->elements[0] =
        make_calypso_final_element("EventDateStamp", 14, "Event date", CALYPSO_FINAL_TYPE_DATE);
    OpusEventStructure->container->elements[1] =
        make_calypso_final_element("EventTimeStamp", 11, "Event time", CALYPSO_FINAL_TYPE_TIME);
    OpusEventStructure->container->elements[2] =
        make_calypso_final_element("EventUnknownX", 19, "Unknown X", CALYPSO_FINAL_TYPE_NUMBER);
    OpusEventStructure->container->elements[3] = make_calypso_bitmap_element(
        "Event",
        9,
        (CalypsoElement[]){
            make_calypso_final_element("EventUnknownA", 8, "Unknown A", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventResult", 8, "Code Résultat", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventServiceProvider",
                8,
                "Identité de l’exploitant",
                CALYPSO_FINAL_TYPE_SERVICE_PROVIDER),
            make_calypso_final_element(
                "EventLocationId", 16, "Lieu de l’événement", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventRouteNumber", 16, "Référence de la ligne", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventUnknownD", 16, "Unknown D", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventUnknownE", 16, "Unknown E", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventContractPointer",
                5,
                "Référence du contrat concerné",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_bitmap_element(
                "EventData",
                7,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "EventDataDateFirstStamp",
                        14,
                        "Date de la première montée",
                        CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "EventDataTimeFirstStamp",
                        11,
                        "Heure de la première montée",
                        CALYPSO_FINAL_TYPE_TIME),
                    make_calypso_final_element(
                        "EventDataSimulation", 1, "Simulation", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventDataRouteDirection", 4, "Sens", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventUnknownG", 4, "Unknown G", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventUnknownH", 4, "Unknown H", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventUnknownI", 4, "Unknown I", CALYPSO_FINAL_TYPE_UNKNOWN),
                }),
        });

    return OpusEventStructure;
}

CalypsoApp* get_opus_env_holder_structure() {
    CalypsoApp* OpusEnvHolderStructure = malloc(sizeof(CalypsoApp));
    if(!OpusEnvHolderStructure) {
        return NULL;
    }

    int app_elements_count = 3;

    OpusEnvHolderStructure->type = CALYPSO_APP_ENV_HOLDER;
    OpusEnvHolderStructure->container = malloc(sizeof(CalypsoContainerElement));
    OpusEnvHolderStructure->container->elements =
        malloc(app_elements_count * sizeof(CalypsoElement));
    OpusEnvHolderStructure->container->size = app_elements_count;

    OpusEnvHolderStructure->container->elements[0] = make_calypso_final_element(
        "EnvApplicationVersionNumber",
        6,
        "Numéro de version de l’application Billettique",
        CALYPSO_FINAL_TYPE_NUMBER);
    OpusEnvHolderStructure->container->elements[1] = make_calypso_bitmap_element(
        "Env",
        7,
        (CalypsoElement[]){
            make_calypso_final_element(
                "EnvNetworkId", 24, "Identification du réseau", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EnvApplicationIssuerId",
                8,
                "Identification de l’émetteur de l’application",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EnvApplicationValidityEndDate",
                14,
                "Date de fin de validité de l’application",
                CALYPSO_FINAL_TYPE_DATE),
            make_calypso_bitmap_element(
                "EnvData",
                4,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "EnvDataCardStatus", 1, "Statut de la carte", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EnvData2", 0, "Données complémentaires", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EnvData_CardUtilisation",
                        1,
                        "Utilisation de la carte",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EnvData4", 0, "Données complémentaires", CALYPSO_FINAL_TYPE_UNKNOWN),
                }),
            make_calypso_final_element("EnvUnknownA", 0, "Unknown A", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element("EnvUnknownB", 0, "Unknown B", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element("EnvUnknownC", 0, "Unknown C", CALYPSO_FINAL_TYPE_UNKNOWN),
        });

    OpusEnvHolderStructure->container->elements[2] = make_calypso_bitmap_element(
        "Holder",
        8,
        (CalypsoElement[]){
            make_calypso_bitmap_element(
                "HolderBirthBitmap",
                2,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "HolderBirthDate", 32, "Date de naissance", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "HolderBirthUnknownA", 0, "Unknown A", CALYPSO_FINAL_TYPE_UNKNOWN),
                }),
            make_calypso_repeater_element(
                "HolderProfilesList",
                4,
                make_calypso_bitmap_element(
                    "HolderProfile",
                    3,
                    (CalypsoElement[]){
                        make_calypso_final_element(
                            "HolderProfileNumber", 6, "Numéro de profil", CALYPSO_FINAL_TYPE_NUMBER),
                        make_calypso_final_element(
                            "HolderProfileDate", 14, "Date de profil", CALYPSO_FINAL_TYPE_DATE),
                        make_calypso_final_element(
                            "HolderProfileUnknownA", 0, "Unknown A", CALYPSO_FINAL_TYPE_UNKNOWN),
                    })),
            make_calypso_final_element(
                "HolderData_Language", 6, "Langue de l'utilisateur", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "HolderUnknownA", 0, "Unknown A", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "HolderUnknownB", 0, "Unknown B", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "HolderUnknownC", 0, "Unknown C", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "HolderUnknownD", 0, "Unknown D", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "HolderUnknownE", 0, "Unknown E", CALYPSO_FINAL_TYPE_UNKNOWN),
        });

    return OpusEnvHolderStructure;
}
