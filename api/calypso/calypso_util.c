#include <stdlib.h>
#include <string.h>
#include "calypso_util.h"

CalypsoElement make_calypso_final_element(
    const char* key,
    int size,
    const char* label,
    CalypsoFinalType final_type) {
    CalypsoElement final_element = {};

    final_element.type = CALYPSO_ELEMENT_TYPE_FINAL;
    final_element.final = malloc(sizeof(CalypsoFinalElement));
    final_element.final->size = size;
    final_element.final->final_type = final_type;
    strncpy(final_element.final->key, key, 36);
    strncpy(final_element.final->label, label, 64);

    return final_element;
}

CalypsoElement make_calypso_bitmap_element(const char* key, int size, CalypsoElement* elements) {
    CalypsoElement bitmap_element = {};

    bitmap_element.type = CALYPSO_ELEMENT_TYPE_BITMAP;
    bitmap_element.bitmap = malloc(sizeof(CalypsoBitmapElement));
    bitmap_element.bitmap->size = size;
    bitmap_element.bitmap->elements = malloc(size * sizeof(CalypsoElement));
    for(int i = 0; i < size; i++) {
        bitmap_element.bitmap->elements[i] = elements[i];
    }
    strncpy(bitmap_element.bitmap->key, key, 36);

    return bitmap_element;
}

void free_calypso_element(CalypsoElement* element) {
    if(element->type == CALYPSO_ELEMENT_TYPE_FINAL) {
        free(element->final);
    } else {
        for(int i = 0; i < element->bitmap->size; i++) {
            free_calypso_element(&element->bitmap->elements[i]);
        }
        free(element->bitmap->elements);
        free(element->bitmap);
    }
}

void free_calypso_structure(CalypsoApp* structure) {
    for(int i = 0; i < structure->elements_size; i++) {
        free_calypso_element(&structure->elements[i]);
    }
    free(structure->elements);
    free(structure);
}

int* get_bit_positions(const char* binary_string, int* count) {
    int length = strlen(binary_string);
    int* positions = malloc(length * sizeof(int));
    int pos_index = 0;

    for(int i = 0; i < length; i++) {
        if(binary_string[length - 1 - i] == '1') {
            positions[pos_index++] = i;
        }
    }

    *count = pos_index;
    return positions;
}

int is_bit_present(int* positions, int count, int bit) {
    for(int i = 0; i < count; i++) {
        if(positions[i] == bit) {
            return 1;
        }
    }
    return 0;
}

bool is_calypso_subnode_present(
    const char* binary_string,
    const char* key,
    CalypsoBitmapElement* bitmap) {
    char bit_slice[bitmap->size + 1];
    strncpy(bit_slice, binary_string, bitmap->size);
    bit_slice[bitmap->size] = '\0';
    int count = 0;
    int* positions = get_bit_positions(bit_slice, &count);
    int offset = bitmap->size;
    for(int i = 0; i < count; i++) {
        CalypsoElement* element = &bitmap->elements[positions[i]];
        if(element->type == CALYPSO_ELEMENT_TYPE_FINAL) {
            if(strcmp(element->final->key, key) == 0) {
                free(positions);
                return true;
            }
            offset += element->final->size;
        } else {
            if(strcmp(element->bitmap->key, key) == 0) {
                free(positions);
                return true;
            }
            int sub_binary_string_size = element->bitmap->size;
            char bit_slice[sub_binary_string_size + 1];
            strncpy(bit_slice, binary_string, sub_binary_string_size);
            bit_slice[sub_binary_string_size] = '\0';
            if(is_calypso_subnode_present(binary_string + offset, key, element->bitmap)) {
                free(positions);
                return true;
            }
            offset += element->bitmap->size;
        }
    }
    free(positions);
    return false;
}

bool is_calypso_node_present(const char* binary_string, const char* key, CalypsoApp* structure) {
    int offset = 0;
    for(int i = 0; i < structure->elements_size; i++) {
        if(structure->elements[i].type == CALYPSO_ELEMENT_TYPE_FINAL) {
            if(strcmp(structure->elements[i].final->key, key) == 0) {
                return true;
            }
            offset += structure->elements[i].final->size;
        } else {
            if(strcmp(structure->elements[i].bitmap->key, key) == 0) {
                return true;
            }
            int sub_binary_string_size = structure->elements[i].bitmap->size;
            char bit_slice[sub_binary_string_size + 1];
            strncpy(bit_slice, binary_string, sub_binary_string_size);
            bit_slice[sub_binary_string_size] = '\0';
            if(is_calypso_subnode_present(
                   binary_string + offset, key, structure->elements[i].bitmap)) {
                return true;
            }
            offset += structure->elements[i].bitmap->size;
        }
    }
    return false;
}

int get_calypso_subnode_offset(
    const char* binary_string,
    const char* key,
    CalypsoBitmapElement* bitmap,
    bool* found) {
    char bit_slice[bitmap->size + 1];
    strncpy(bit_slice, binary_string, bitmap->size);
    bit_slice[bitmap->size] = '\0';

    int count = 0;
    int* positions = get_bit_positions(bit_slice, &count);

    int count_offset = bitmap->size;
    for(int i = 0; i < count; i++) {
        CalypsoElement element = bitmap->elements[positions[i]];
        if(element.type == CALYPSO_ELEMENT_TYPE_FINAL) {
            if(strcmp(element.final->key, key) == 0) {
                *found = true;
                free(positions);
                return count_offset;
            }
            count_offset += element.final->size;
        } else {
            if(strcmp(element.bitmap->key, key) == 0) {
                *found = true;
                free(positions);
                return count_offset;
            }
            count_offset += get_calypso_subnode_offset(
                binary_string + count_offset, key, element.bitmap, found);
            if(*found) {
                free(positions);
                return count_offset;
            }
        }
    }
    free(positions);
    return count_offset;
}

int get_calypso_node_offset(const char* binary_string, const char* key, CalypsoApp* structure) {
    int count = 0;
    bool found = false;
    for(int i = 0; i < structure->elements_size; i++) {
        if(structure->elements[i].type == CALYPSO_ELEMENT_TYPE_FINAL) {
            if(strcmp(structure->elements[i].final->key, key) == 0) {
                return count;
            }
            count += structure->elements[i].final->size;
        } else {
            if(strcmp(structure->elements[i].bitmap->key, key) == 0) {
                return count;
            }
            int sub_binary_string_size = structure->elements[i].bitmap->size;
            char bit_slice[sub_binary_string_size + 1];
            strncpy(bit_slice, binary_string + count, sub_binary_string_size);
            bit_slice[sub_binary_string_size] = '\0';
            count += get_calypso_subnode_offset(
                binary_string + count, key, structure->elements[i].bitmap, &found);
            if(found) {
                return count;
            }
        }
    }
    return 0;
}

int get_calypso_subnode_size(const char* key, CalypsoElement* element) {
    if(element->type == CALYPSO_ELEMENT_TYPE_FINAL) {
        if(strcmp(element->final->key, key) == 0) {
            return element->final->size;
        }
    } else {
        if(strcmp(element->bitmap->key, key) == 0) {
            return element->bitmap->size;
        }
        for(int i = 0; i < element->bitmap->size; i++) {
            int size = get_calypso_subnode_size(key, &element->bitmap->elements[i]);
            if(size != 0) {
                return size;
            }
        }
    }
    return 0;
}

int get_calypso_node_size(const char* key, CalypsoApp* structure) {
    for(int i = 0; i < structure->elements_size; i++) {
        if(structure->elements[i].type == CALYPSO_ELEMENT_TYPE_FINAL) {
            if(strcmp(structure->elements[i].final->key, key) == 0) {
                return structure->elements[i].final->size;
            }
        } else {
            if(strcmp(structure->elements[i].bitmap->key, key) == 0) {
                return structure->elements[i].bitmap->size;
            }
            for(int j = 0; j < structure->elements[i].bitmap->size; j++) {
                int size =
                    get_calypso_subnode_size(key, &structure->elements[i].bitmap->elements[j]);
                if(size != 0) {
                    return size;
                }
            }
        }
    }
    return 0;
}
