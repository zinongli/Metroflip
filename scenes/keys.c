#include "../metroflip_i.h"
#include "keys.h"
#include <bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>
#include <nfc/nfc.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <string.h>

#define TAG "keys_check"

const MfClassicKeyPair troika_1k_keys[16] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xfbf225dc5d58},
    {.a = 0xa82607b01c0d, .b = 0x2910989b6880},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x73068f118c13, .b = 0x2b7f3253fac5},
    {.a = 0xfbc2793d540b, .b = 0xd3a297dc2698},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0xae3d65a3dad4, .b = 0x0f1c63013dba},
    {.a = 0xa73f5dc1d333, .b = 0xe35173494a81},
    {.a = 0x69a32f1c2f19, .b = 0x6b8bd9860763},
    {.a = 0x9becdf3d9273, .b = 0xf8493407799d},
    {.a = 0x08b386463229, .b = 0x5efbaecef46b},
    {.a = 0xcd4c61c26e3d, .b = 0x31c7610de3b0},
    {.a = 0xa82607b01c0d, .b = 0x2910989b6880},
    {.a = 0x0e8f64340ba4, .b = 0x4acec1205d75},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
};

const MfClassicKeyPair troika_4k_keys[40] = {
    {.a = 0xEC29806D9738, .b = 0xFBF225DC5D58}, //1
    {.a = 0xA0A1A2A3A4A5, .b = 0x7DE02A7F6025}, //2
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //3
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //4
    {.a = 0x73068F118C13, .b = 0x2B7F3253FAC5}, //5
    {.a = 0xFBC2793D540B, .b = 0xD3A297DC2698}, //6
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //7
    {.a = 0xAE3D65A3DAD4, .b = 0x0F1C63013DBA}, //8
    {.a = 0xA73F5DC1D333, .b = 0xE35173494A81}, //9
    {.a = 0x69A32F1C2F19, .b = 0x6B8BD9860763}, //10
    {.a = 0x9BECDF3D9273, .b = 0xF8493407799D}, //11
    {.a = 0x08B386463229, .b = 0x5EFBAECEF46B}, //12
    {.a = 0xCD4C61C26E3D, .b = 0x31C7610DE3B0}, //13
    {.a = 0xA82607B01C0D, .b = 0x2910989B6880}, //14
    {.a = 0x0E8F64340BA4, .b = 0x4ACEC1205D75}, //15
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //16
    {.a = 0x6B02733BB6EC, .b = 0x7038CD25C408}, //17
    {.a = 0x403D706BA880, .b = 0xB39D19A280DF}, //18
    {.a = 0xC11F4597EFB5, .b = 0x70D901648CB9}, //19
    {.a = 0x0DB520C78C1C, .b = 0x73E5B9D9D3A4}, //20
    {.a = 0x3EBCE0925B2F, .b = 0x372CC880F216}, //21
    {.a = 0x16A27AF45407, .b = 0x9868925175BA}, //22
    {.a = 0xABA208516740, .b = 0xCE26ECB95252}, //23
    {.a = 0xCD64E567ABCD, .b = 0x8F79C4FD8A01}, //24
    {.a = 0x764CD061F1E6, .b = 0xA74332F74994}, //25
    {.a = 0x1CC219E9FEC1, .b = 0xB90DE525CEB6}, //26
    {.a = 0x2FE3CB83EA43, .b = 0xFBA88F109B32}, //27
    {.a = 0x07894FFEC1D6, .b = 0xEFCB0E689DB3}, //28
    {.a = 0x04C297B91308, .b = 0xC8454C154CB5}, //29
    {.a = 0x7A38E3511A38, .b = 0xAB16584C972A}, //30
    {.a = 0x7545DF809202, .b = 0xECF751084A80}, //31
    {.a = 0x5125974CD391, .b = 0xD3EAFB5DF46D}, //32
    {.a = 0x7A86AA203788, .b = 0xE41242278CA2}, //33
    {.a = 0xAFCEF64C9913, .b = 0x9DB96DCA4324}, //34
    {.a = 0x04EAA462F70B, .b = 0xAC17B93E2FAE}, //35
    {.a = 0xE734C210F27E, .b = 0x29BA8C3E9FDA}, //36
    {.a = 0xD5524F591EED, .b = 0x5DAF42861B4D}, //37
    {.a = 0xE4821A377B75, .b = 0xE8709E486465}, //38
    {.a = 0x518DC6EEA089, .b = 0x97C64AC98CA4}, //39
    {.a = 0xBB52F8CCE07F, .b = 0x6B6119752C70}, //40
};

const uint8_t SMARTRIDER_STANDARD_KEYS[3][6] = {
    {0x20, 0x31, 0xD1, 0xE5, 0x7A, 0x3B},
    {0x4C, 0xA6, 0x02, 0x9F, 0x94, 0x73},
    {0x19, 0x19, 0x53, 0x98, 0xE3, 0x2F}};

const MfClassicKeyPair charliecard_1k_keys[16] = {
    {.a = 0x3060206F5B0A, .b = 0xF1B9F5669CC8},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x3A09594C8587, .b = 0x62387B8D250D},
    {.a = 0xF238D78FF48F, .b = 0x9DC282D46217},
    {.a = 0xAFD0BA94D624, .b = 0x92EE4DC87191},
    {.a = 0xB35A0E4ACC09, .b = 0x756EF55E2507},
    {.a = 0x447AB7FD5A6B, .b = 0x932B9CB730EF},
    {.a = 0x1F1A0A111B5B, .b = 0xAD9E0A1CA2F7},
    {.a = 0xD58023BA2BDC, .b = 0x62CED42A6D87},
    {.a = 0x2548A443DF28, .b = 0x2ED3B15E7C0F},
};

const MfClassicKeyPair bip_1k_keys[16] = {
    {.a = 0x3a42f33af429, .b = 0x1fc235ac1309},
    {.a = 0x6338a371c0ed, .b = 0x243f160918d1},
    {.a = 0xf124c2578ad0, .b = 0x9afc42372af1},
    {.a = 0x32ac3b90ac13, .b = 0x682d401abb09},
    {.a = 0x4ad1e273eaf1, .b = 0x067db45454a9},
    {.a = 0xe2c42591368a, .b = 0x15fc4c7613fe},
    {.a = 0x2a3c347a1200, .b = 0x68d30288910a},
    {.a = 0x16f3d5ab1139, .b = 0xf59a36a2546d},
    {.a = 0x937a4fff3011, .b = 0x64e3c10394c2},
    {.a = 0x35c3d2caee88, .b = 0xb736412614af},
    {.a = 0x693143f10368, .b = 0x324f5df65310},
    {.a = 0xa3f97428dd01, .b = 0x643fb6de2217},
    {.a = 0x63f17a449af0, .b = 0x82f435dedf01},
    {.a = 0xc4652c54261c, .b = 0x0263de1278f3},
    {.a = 0xd49e2826664f, .b = 0x51284c3686a6},
    {.a = 0x3df14c8000a1, .b = 0x6a470d54127c},
};

const MfClassicKeyPair metromoney_1k_keys[16] = {
    {.a = 0x2803BCB0C7E1, .b = 0x4FA9EB49F75E},
    {.a = 0x9C616585E26D, .b = 0xD1C71E590D16},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0x9C616585E26D, .b = 0xA160FCD5EC4C},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0x112233445566, .b = 0x361A62F35BC9},
    {.a = 0x112233445566, .b = 0x361A62F35BC9},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
};

static bool charliecard_verify(Nfc* nfc) {
    bool verified = false;
    FURI_LOG_I(TAG, "verifying charliecard..");

    do {
        const uint8_t verify_sector = 1;
        const uint8_t verify_block = mf_classic_get_first_block_num_of_sector(verify_sector) + 1;
        FURI_LOG_I(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(
            charliecard_1k_keys[verify_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, verify_block, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Failed to read block %u: %d", verify_block, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

bool bip_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 0;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_I(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(bip_1k_keys[0].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

        if(error != MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool metromoney_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t ticket_sector_number = 1;
        const uint8_t ticket_block_number =
            mf_classic_get_first_block_num_of_sector(ticket_sector_number) + 1;
        FURI_LOG_D(TAG, "Verifying sector %u", ticket_sector_number);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(
            metromoney_1k_keys[ticket_sector_number].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error = mf_classic_poller_sync_auth(
            nfc, ticket_block_number, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", ticket_block_number, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool smartrider_authenticate_and_read(
    Nfc* nfc,
    uint8_t sector,
    const uint8_t* key,
    MfClassicKeyType key_type,
    MfClassicBlock* block_data) {
    MfClassicKey mf_key;
    memcpy(mf_key.data, key, 6);
    uint8_t block = mf_classic_get_first_block_num_of_sector(sector);

    if(mf_classic_poller_sync_auth(nfc, block, &mf_key, key_type, NULL) != MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Authentication failed for sector %d key type %d", sector, key_type);
        return false;
    }

    if(mf_classic_poller_sync_read_block(nfc, block, &mf_key, key_type, block_data) !=
       MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Read failed for sector %d", sector);
        return false;
    }

    return true;
}

static bool smartrider_verify(Nfc* nfc) {
    furi_assert(nfc);
    MfClassicBlock block_data;

    for(int i = 0; i < 3; i++) {
        if(!smartrider_authenticate_and_read(
               nfc,
               i * 6,
               SMARTRIDER_STANDARD_KEYS[i],
               i % 2 == 0 ? MfClassicKeyTypeA : MfClassicKeyTypeB,
               &block_data) ||
           memcmp(block_data.data, SMARTRIDER_STANDARD_KEYS[i], 6) != 0) {
            FURI_LOG_D(TAG, "Authentication or key mismatch for key %d", i);
            return false;
        }
    }

    FURI_LOG_I(TAG, "SmartRider card verified");
    return true;
}

static bool troika_get_card_config(TroikaCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->data_sector = 11;
        config->keys = troika_1k_keys;
    } else if(type == MfClassicType4k) {
        config->data_sector = 8; // Further testing needed
        config->keys = troika_4k_keys;
    } else {
        success = false;
    }

    return success;
}

static bool troika_verify_type(Nfc* nfc, MfClassicType type) {
    bool verified = false;

    do {
        TroikaCardConfig cfg = {};
        if(!troika_get_card_config(&cfg, type)) break;

        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(cfg.data_sector);
        FURI_LOG_D(TAG, "Verifying sector %lu", cfg.data_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(cfg.keys[cfg.data_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }
        FURI_LOG_D(TAG, "Verify success!");
        verified = true;
    } while(false);

    return verified;
}

static bool troika_verify(Nfc* nfc) {
    return troika_verify_type(nfc, MfClassicType1k) || troika_verify_type(nfc, MfClassicType4k);
}

CardType determine_card_type(Nfc* nfc) {
    FURI_LOG_I(TAG, "checking keys..");
    UNUSED(bip_verify);

    if(bip_verify(nfc)) {
        return CARD_TYPE_METROMONEY;
    } else if(metromoney_verify(nfc)) {
        return CARD_TYPE_METROMONEY;
    } else if(smartrider_verify(nfc)) {
        return CARD_TYPE_SMARTRIDER;
    } else if(troika_verify(nfc)) {
        return CARD_TYPE_TROIKA;
    } else if(charliecard_verify(nfc)) {
        return CARD_TYPE_CHARLIECARD;
    } else {
        FURI_LOG_I(TAG, "its unknown");
        return CARD_TYPE_UNKNOWN;
    }
}
