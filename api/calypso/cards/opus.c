#include <stdlib.h>
#include "opus.h"

CalypsoApp* get_opus_contract_structure() {
    /*
    En1545FixedInteger(CONTRACT_UNKNOWN_A, 3),
    En1545Bitmap(
            En1545FixedInteger(CONTRACT_PROVIDER, 8),
            En1545FixedInteger(CONTRACT_TARIFF, 16),
            En1545Bitmap(
                    En1545FixedInteger.date(CONTRACT_START),
                    En1545FixedInteger.date(CONTRACT_END)
            ),
            En1545Container(
                    En1545FixedInteger(CONTRACT_UNKNOWN_B, 17),
                    En1545FixedInteger.date(CONTRACT_SALE),
                    En1545FixedInteger.timeLocal(CONTRACT_SALE),
                    En1545FixedHex(CONTRACT_UNKNOWN_C, 36),
                    En1545FixedInteger(CONTRACT_STATUS, 8),
                    En1545FixedHex(CONTRACT_UNKNOWN_D, 36)
            )
    )
    */
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
    /*
    En1545Container(
                En1545FixedInteger.date(EVENT),
                En1545FixedInteger.timeLocal(EVENT),
                En1545FixedInteger("UnknownX", 19), // Possibly part of following bitmap
                En1545Bitmap(
                        En1545FixedInteger(EVENT_UNKNOWN_A, 8),
                        En1545FixedInteger(EVENT_UNKNOWN_B, 8),
                        En1545FixedInteger(EVENT_SERVICE_PROVIDER, 8),
                        En1545FixedInteger(EVENT_UNKNOWN_C, 16),
                        En1545FixedInteger(EVENT_ROUTE_NUMBER, 16),
                        // How 32 bits are split among next 2 fields is unclear
                        En1545FixedInteger(EVENT_UNKNOWN_D, 16),
                        En1545FixedInteger(EVENT_UNKNOWN_E, 16),
                        En1545FixedInteger(EVENT_CONTRACT_POINTER, 5),
                        En1545Bitmap(
                                En1545FixedInteger.date(EVENT_FIRST_STAMP),
                                En1545FixedInteger.timeLocal(EVENT_FIRST_STAMP),
                                En1545FixedInteger("EventDataSimulation", 1),
                                En1545FixedInteger(EVENT_UNKNOWN_F, 4),
                                En1545FixedInteger(EVENT_UNKNOWN_G, 4),
                                En1545FixedInteger(EVENT_UNKNOWN_H, 4),
                                En1545FixedInteger(EVENT_UNKNOWN_I, 4)
                        )
                )
        )
    */
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
        make_calypso_final_element("EventDate", 14, "Event date", CALYPSO_FINAL_TYPE_DATE);
    OpusEventStructure->container->elements[1] =
        make_calypso_final_element("EventTime", 11, "Event time", CALYPSO_FINAL_TYPE_TIME);
    OpusEventStructure->container->elements[2] =
        make_calypso_final_element("EventUnknownX", 19, "Unknown X", CALYPSO_FINAL_TYPE_NUMBER);
    OpusEventStructure->container->elements[3] = make_calypso_bitmap_element(
        "Event",
        9,
        (CalypsoElement[]){
            make_calypso_final_element("EventUnknownA", 8, "Unknown A", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element("EventUnknownB", 8, "Unknown B", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventServiceProvider", 8, "Service provider", CALYPSO_FINAL_TYPE_SERVICE_PROVIDER),
            make_calypso_final_element("EventUnknownC", 16, "Unknown C", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventRouteNumber", 16, "Route number", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element("EventUnknownD", 16, "Unknown D", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element("EventUnknownE", 16, "Unknown E", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventContractPointer", 5, "Contract pointer", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_bitmap_element(
                "EventData",
                7,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "EventFirstStampDate", 14, "First stamp date", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "EventFirstStampTime", 11, "First stamp time", CALYPSO_FINAL_TYPE_TIME),
                    make_calypso_final_element(
                        "EventDataSimulation", 1, "Simulation", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventUnknownF", 4, "Unknown F", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "EventUnknownG", 4, "Unknown G", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "EventUnknownH", 4, "Unknown H", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "EventUnknownI", 4, "Unknown I", CALYPSO_FINAL_TYPE_NUMBER),
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
            make_calypso_final_element(
                "EnvPayMethod", 11, "Code mode de paiement", CALYPSO_FINAL_TYPE_PAY_METHOD),
            make_calypso_final_element(
                "EnvAuthenticator",
                16,
                "Code de contrôle de l’intégrité des données",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EnvSelectList",
                32,
                "Bitmap de tableau de paramètre multiple",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_container_element(
                "EnvData",
                2,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "EnvDataCardStatus", 1, "Statut de la carte", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EnvData2", 0, "Données complémentaires", CALYPSO_FINAL_TYPE_UNKNOWN),
                }),
        });

    OpusEnvHolderStructure->container->elements[2] = make_calypso_bitmap_element(
        "Holder",
        2,
        (CalypsoElement[]){
            make_calypso_container_element(
                "HolderData",
                5,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "HolderUnknownA", 3, "Unknown A", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "HolderBirthDate", 8, "Birth date", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "HolderUnknownB", 13, "Unknown B", CALYPSO_FINAL_TYPE_NUMBER),
                    make_calypso_final_element(
                        "HolderProfile", 17, "Profile", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "HolderUnknownC", 8, "Unknown C", CALYPSO_FINAL_TYPE_NUMBER),
                }),
            make_calypso_final_element("HolderUnknownD", 8, "Unknown D", CALYPSO_FINAL_TYPE_NUMBER),
        });

    return OpusEnvHolderStructure;
}
