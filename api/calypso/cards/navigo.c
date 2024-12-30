#include <stdlib.h>
#include "navigo.h"

CalypsoApp* get_navigo_contract_structure() {
    CalypsoApp* NavigoContractStructure = malloc(sizeof(CalypsoApp));
    if(!NavigoContractStructure) {
        return NULL;
    }

    int app_elements_count = 1;

    NavigoContractStructure->type = CALYPSO_APP_CONTRACT;
    NavigoContractStructure->elements = malloc(app_elements_count * sizeof(CalypsoElement));
    NavigoContractStructure->elements_size = app_elements_count;

    NavigoContractStructure->elements[0] = make_calypso_bitmap_element(
        "Contract",
        20,
        (CalypsoElement[]){
            make_calypso_final_element(
                "ContractNetworkId", 24, "Identification du réseau", CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_final_element(
                "ContractProvider",
                8,
                "Identification de l’exploitant",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "ContractTariff", 16, "Code tarif", CALYPSO_FINAL_TYPE_TARIFF),

            make_calypso_final_element(
                "ContractSerialNumber", 32, "Numéro TCN", CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_bitmap_element(
                "ContractCustomerInfoBitmap",
                2,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractCustomerProfile",
                        6,
                        "Statut du titulaire ou Taux de réduction applicable",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractCustomerNumber",
                        32,
                        "Numéro de client",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                }),

            make_calypso_bitmap_element(
                "ContractPassengerInfoBitmap",
                2,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractPassengerClass",
                        8,
                        "Classe de service des voyageurs",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractPassengerTotal",
                        8,
                        "Nombre total de voyageurs",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                }),

            make_calypso_final_element(
                "ContractVehicleClassAllowed",
                6,
                "Classes de véhicule autorisé",
                CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_final_element(
                "ContractPaymentPointer",
                32,
                "Pointeurs sur les événements de paiement",
                CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_final_element(
                "ContractPayMethod", 11, "Code mode de paiement", CALYPSO_FINAL_TYPE_PAY_METHOD),

            make_calypso_final_element(
                "ContractServices", 16, "Services autorisés", CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_final_element(
                "ContractPriceAmount", 16, "Montant total", CALYPSO_FINAL_TYPE_AMOUNT),

            make_calypso_final_element(
                "ContractPriceUnit", 16, "Code de monnaie", CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_bitmap_element(
                "ContractRestrictionBitmap",
                7,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractRestrictStart", 11, "", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractRestrictEnd", 11, "", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractRestrictDay", 8, "", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractRestrictTimeCode", 8, "", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractRestrictCode",
                        8,
                        "Code de restriction",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractRestrictProduct",
                        16,
                        "Produit de restriction",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractRestrictLocation",
                        16,
                        "Référence du lieu de restriction",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                }),

            make_calypso_bitmap_element(
                "ContractValidityInfoBitmap",
                9,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractValidityStartDate",
                        14,
                        "Date de début de validité",
                        CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "ContractValidityStartTime",
                        11,
                        "Heure de début de validité",
                        CALYPSO_FINAL_TYPE_TIME),
                    make_calypso_final_element(
                        "ContractValidityEndDate",
                        14,
                        "Date de fin de validité",
                        CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "ContractValidityEndTime",
                        11,
                        "Heure de fin de validité",
                        CALYPSO_FINAL_TYPE_TIME),
                    make_calypso_final_element(
                        "ContractValidityDuration",
                        8,
                        "Durée de validité",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractValidityLimiteDate",
                        14,
                        "Date limite de première utilisation",
                        CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "ContractValidityZones",
                        8,
                        "Numéros des zones autorisées",
                        CALYPSO_FINAL_TYPE_ZONES),
                    make_calypso_final_element(
                        "ContractValidityJourneys",
                        16,
                        "Nombre de voyages autorisés",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractPeriodJourneys",
                        16,
                        "Nombre de voyages autorisés par période",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                }),

            make_calypso_bitmap_element(
                "ContractJourneyData",
                8,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractJourneyOrigin",
                        16,
                        "Code lieu d’origine",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyDestination",
                        16,
                        "Code lieu de destination",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyRouteNumbers",
                        16,
                        "Numéros des lignes autorisées",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyRouteVariants",
                        8,
                        "Variantes aux numéros des lignes autorisées",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyRun", 16, "Référence du voyage", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyVia", 16, "Code lieu du via", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyDistance", 16, "Distance", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "ContractJourneyInterchanges",
                        8,
                        "Nombre de correspondances autorisées",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                }),

            make_calypso_bitmap_element(
                "ContractSaleData",
                4,
                (CalypsoElement[]){
                    make_calypso_final_element(
                        "ContractValiditySaleDate", 14, "Date de vente", CALYPSO_FINAL_TYPE_DATE),
                    make_calypso_final_element(
                        "ContractValiditySaleTime", 11, "Heure de vente", CALYPSO_FINAL_TYPE_TIME),
                    make_calypso_final_element(
                        "ContractValiditySaleAgent",
                        8,
                        "Identification de l’exploitant de vente",
                        CALYPSO_FINAL_TYPE_SERVICE_PROVIDER),
                    make_calypso_final_element(
                        "ContractValiditySaleDevice",
                        16,
                        "Identification du terminal de vente",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                }),

            make_calypso_final_element(
                "ContractStatus", 8, "État du contrat", CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_final_element(
                "ContractLoyaltyPoints",
                16,
                "Nombre de points de fidélité",
                CALYPSO_FINAL_TYPE_NUMBER),

            make_calypso_final_element(
                "ContractAuthenticator",
                16,
                "Code de contrôle de l’intégrité des données",
                CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_final_element(
                "ContractData(0..255)", 0, "Données complémentaires", CALYPSO_FINAL_TYPE_UNKNOWN),
        });

    return NavigoContractStructure;
}

CalypsoApp* get_navigo_event_structure() {
    CalypsoApp* NavigoEventStructure = malloc(sizeof(CalypsoApp));
    if(!NavigoEventStructure) {
        return NULL;
    }

    int app_elements_count = 3;

    NavigoEventStructure->type = CALYPSO_APP_CONTRACT;
    NavigoEventStructure->elements = malloc(app_elements_count * sizeof(CalypsoElement));
    NavigoEventStructure->elements_size = app_elements_count;

    NavigoEventStructure->elements[0] = make_calypso_final_element(
        "EventDateStamp", 14, "Date de l’événement", CALYPSO_FINAL_TYPE_DATE);

    NavigoEventStructure->elements[1] = make_calypso_final_element(
        "EventTimeStamp", 11, "Heure de l’événement", CALYPSO_FINAL_TYPE_TIME);

    NavigoEventStructure->elements[2] = make_calypso_bitmap_element(
        "EventBitmap",
        28,
        (CalypsoElement[]){
            make_calypso_final_element(
                "EventDisplayData", 8, "Données pour l’affichage", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element("EventNetworkId", 24, "Réseau", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventCode", 8, "Nature de l’événement", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventResult", 8, "Code Résultat", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventServiceProvider",
                8,
                "Identité de l’exploitant",
                CALYPSO_FINAL_TYPE_SERVICE_PROVIDER),
            make_calypso_final_element(
                "EventNotokCounter", 8, "Compteur événements anormaux", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventSerialNumber",
                24,
                "Numéro de série de l’événement",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventDestination", 16, "Destination de l’usager", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventLocationId", 16, "Lieu de l’événement", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventLocationGate", 8, "Identification du passage", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventDevice", 16, "Identificateur de l’équipement", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventRouteNumber", 16, "Référence de la ligne", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventRouteVariant",
                8,
                "Référence d’une variante de la ligne",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventJourneyRun", 16, "Référence de la mission", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventVehicleId", 16, "Identificateur du véhicule", CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventVehicleClass", 8, "Type de véhicule utilisé", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventLocationType",
                5,
                "Type d’endroit (gare, arrêt de bus), ",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventEmployee", 240, "Code de l’employé impliqué", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventLocationReference",
                16,
                "Référence du lieu de l’événement",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventJourneyInterchanges",
                8,
                "Nombre de correspondances",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventPeriodJourneys", 16, "Nombre de voyage effectué", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventTotalJourneys",
                16,
                "Nombre total de voyage autorisé",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventJourneyDistance", 16, "Distance parcourue", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventPriceAmount",
                16,
                "Montant en jeu lors de l’événement",
                CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventPriceUnit", 16, "Unité de montant en jeu", CALYPSO_FINAL_TYPE_UNKNOWN),
            make_calypso_final_element(
                "EventContractPointer",
                5,
                "Référence du contrat concerné",
                CALYPSO_FINAL_TYPE_NUMBER),
            make_calypso_final_element(
                "EventAuthenticator", 16, "Code de sécurité", CALYPSO_FINAL_TYPE_UNKNOWN),

            make_calypso_bitmap_element(
                "EventData",
                5,
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
                        "EventDataSimulation",
                        1,
                        "Dernière validation (0=normal, 1=dégradé), ",
                        CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventDataTrip", 2, "Tronçon", CALYPSO_FINAL_TYPE_UNKNOWN),
                    make_calypso_final_element(
                        "EventDataRouteDirection", 2, "Sens", CALYPSO_FINAL_TYPE_UNKNOWN),
                }),
        });

    return NavigoEventStructure;
}
