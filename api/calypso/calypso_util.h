#include <stdbool.h>

#ifndef CALYPSO_UTIL_H
#define CALYPSO_UTIL_H

typedef enum {
    CALYPSO_APP_CONTRACT,
} CalypsoAppType;

typedef enum {
    CALYPSO_FINAL_TYPE_UNKNOWN,
    CALYPSO_FINAL_TYPE_NUMBER,
    CALYPSO_FINAL_TYPE_DATE,
    CALYPSO_FINAL_TYPE_TIME,
    CALYPSO_FINAL_TYPE_PAY_METHOD,
    CALYPSO_FINAL_TYPE_AMOUNT,
    CALYPSO_FINAL_TYPE_SERVICE_PROVIDER,
    CALYPSO_FINAL_TYPE_ZONES,
    CALYPSO_FINAL_TYPE_TARIFF,
    CALYPSO_FINAL_TYPE_NETWORK_ID,
    CALYPSO_FINAL_TYPE_TRANSPORT_TYPE,
    CALYPSO_FINAL_TYPE_CARD_STATUS,
} CalypsoFinalType;

typedef enum {
    CALYPSO_ELEMENT_TYPE_BITMAP,
    CALYPSO_ELEMENT_TYPE_FINAL
} CalypsoElementType;

typedef struct CalypsoFinalElement_t CalypsoFinalElement;
typedef struct CalypsoBitmapElement_t CalypsoBitmapElement;

typedef struct {
    CalypsoElementType type;
    union {
        CalypsoFinalElement* final;
        CalypsoBitmapElement* bitmap;
    };
} CalypsoElement;

struct CalypsoFinalElement_t {
    char key[36];
    int size;
    char label[64];
    CalypsoFinalType final_type;
};

struct CalypsoBitmapElement_t {
    char key[36];
    int size;
    CalypsoElement* elements;
};

typedef struct {
    CalypsoAppType type;
    CalypsoElement* elements;
    int elements_size;
} CalypsoApp;

CalypsoElement make_calypso_final_element(
    const char* key,
    int size,
    const char* label,
    CalypsoFinalType final_type);

CalypsoElement make_calypso_bitmap_element(const char* key, int size, CalypsoElement* elements);

void free_calypso_structure(CalypsoApp* structure);

int* get_bit_positions(const char* binary_string, int* count);

int is_bit_present(int* positions, int count, int bit);

bool is_calypso_node_present(const char* binary_string, const char* key, CalypsoApp* structure);

int get_calypso_node_offset(const char* binary_string, const char* key, CalypsoApp* structure);

int get_calypso_node_size(const char* key, CalypsoApp* structure);

#endif // CALYPSO_UTIL_H
