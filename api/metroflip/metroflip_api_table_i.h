#include "metroflip_api.h"
#include "../../metroflip_i.h"

/* 
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 * TBD: automatically generate this table from app's header files
 */
static constexpr auto metroflip_api_table = sort(create_array_t<sym_entry>(
    // metroflip stuff
    API_METHOD(metroflip_exit_widget_callback, void, (GuiButtonType, InputType, void*)),
    API_METHOD(metroflip_save_widget_callback, void, (GuiButtonType, InputType, void*)),
    API_METHOD(metroflip_delete_widget_callback, void, (GuiButtonType, InputType, void*)),
    API_METHOD(metroflip_app_blink_start, void, (Metroflip*)),
    API_METHOD(metroflip_app_blink_stop, void, (Metroflip*)),
    API_METHOD(bit_slice_to_dec, int, (const char*, int, int)),
    API_METHOD(byte_to_binary, void, (uint8_t, char*)),
    API_VARIABLE(read_file, uint8_t[5]),
    API_VARIABLE(apdu_success, uint8_t[2]),
    API_VARIABLE(select_app, uint8_t[8]),
    API_METHOD(mf_classic_key_cache_load, bool, (MfClassicKeyCache*, const uint8_t*, size_t)),
    API_METHOD(
        mf_classic_key_cache_get_next_key,
        bool,
        (MfClassicKeyCache*, uint8_t*, MfClassicKey*, MfClassicKeyType*)),
    API_METHOD(mf_classic_key_cache_reset, void, (MfClassicKeyCache*)),
    API_METHOD(
        manage_keyfiles,
        KeyfileManager,
        (char[], const uint8_t*, size_t, MfClassicKeyCache*, uint64_t, uint64_t)),
    API_METHOD(uid_to_string, void, (const uint8_t*, size_t, char*, size_t)),
    API_METHOD(
        handle_keyfile_case,
        void,
        (Metroflip*, const char*, const char*, FuriString*, char[])),

    // calypso
    API_METHOD(get_calypso_node_offset, int, (const char*, const char*, CalypsoApp*)),
    API_METHOD(get_network_string, const char*, (CALYPSO_CARD_TYPE)),
    API_METHOD(metroflip_back_button_widget_callback, void, (GuiButtonType, InputType, void*)),
    API_METHOD(metroflip_next_button_widget_callback, void, (GuiButtonType, InputType, void*)),
    API_METHOD(is_calypso_node_present, bool, (const char*, const char*, CalypsoApp*)),
    API_METHOD(get_calypso_node_size, int, (const char*, CalypsoApp*)),
    API_METHOD(free_calypso_structure, void, (CalypsoApp*)),
    API_METHOD(guess_card_type, CALYPSO_CARD_TYPE, (int, int)),

    // intercode
    API_METHOD(get_intercode_structure_env_holder, CalypsoApp*, ()),
    API_METHOD(get_intercode_structure_contract, CalypsoApp*, ()),
    API_METHOD(get_intercode_structure_event, CalypsoApp*, ()),
    API_METHOD(get_intercode_structure_counter, CalypsoApp*, ()),

    // navigo
    API_METHOD(show_navigo_event_info, void, (NavigoCardEvent*, NavigoCardContract*, FuriString*)),
    API_METHOD(show_navigo_special_event_info, void, (NavigoCardSpecialEvent*, FuriString*)),
    API_METHOD(show_navigo_contract_info, void, (NavigoCardContract*, FuriString*)),
    API_METHOD(show_navigo_environment_info, void, (NavigoCardEnv*, NavigoCardHolder*, FuriString*)),

    // opus
    API_METHOD(get_opus_contract_structure, CalypsoApp*, ()),
    API_METHOD(get_opus_event_structure, CalypsoApp*, ()),
    API_METHOD(get_opus_env_holder_structure, CalypsoApp*, ()),
    API_METHOD(show_opus_event_info, void, (OpusCardEvent*, OpusCardContract*, FuriString*)),
    API_METHOD(show_opus_contract_info, void, (OpusCardContract*, FuriString*)),
    API_METHOD(show_opus_environment_info, void, (OpusCardEnv*, OpusCardHolder*, FuriString*)),

    // ravkav
    API_METHOD(get_ravkav_contract_structure, CalypsoApp*, ()),
    API_METHOD(get_ravkav_event_structure, CalypsoApp*, ()),
    API_METHOD(get_ravkav_env_holder_structure, CalypsoApp*, ()),
    API_METHOD(show_ravkav_event_info, void, (RavKavCardEvent*, FuriString*)),
    API_METHOD(show_ravkav_contract_info, void, (RavKavCardContract*, FuriString*)),
    API_METHOD(show_ravkav_environment_info, void, (RavKavCardEnv*, FuriString*)),

    API_VARIABLE(I_RFIDDolphinReceive_97x61, Icon),
    API_VARIABLE(I_icon, Icon),

    // Suica
    API_VARIABLE(I_Suica_AsakusaA, Icon),
    API_VARIABLE(I_Suica_BigStar, Icon),
    API_VARIABLE(I_Suica_ChiyodaC, Icon),
    API_VARIABLE(I_Suica_CrackingEgg, Icon),
    API_VARIABLE(I_Suica_DashLine, Icon),
    API_VARIABLE(I_Suica_EmptyArrowDown, Icon),
    API_VARIABLE(I_Suica_EmptyArrowRight, Icon),
    API_VARIABLE(I_Suica_EntrySlider1, Icon),
    API_VARIABLE(I_Suica_EntrySlider2, Icon),
    API_VARIABLE(I_Suica_EntrySlider3, Icon),
    API_VARIABLE(I_Suica_EntryStopL1, Icon),
    API_VARIABLE(I_Suica_EntryStopL2, Icon),
    API_VARIABLE(I_Suica_EntryStopL3, Icon),
    API_VARIABLE(I_Suica_EntryStopR1, Icon),
    API_VARIABLE(I_Suica_EntryStopR2, Icon),
    API_VARIABLE(I_Suica_EntryStopR3, Icon),
    API_VARIABLE(I_Suica_FilledArrowDown, Icon),
    API_VARIABLE(I_Suica_FilledArrowRight, Icon),
    API_VARIABLE(I_Suica_GinzaG, Icon),
    API_VARIABLE(I_Suica_HanzomonZ, Icon),
    API_VARIABLE(I_Suica_HibiyaH, Icon),
    API_VARIABLE(I_Suica_JRLogo, Icon),
    API_VARIABLE(I_Suica_KeikyuKK, Icon),
    API_VARIABLE(I_Suica_KeikyuLogo, Icon),
    API_VARIABLE(I_Suica_MarunouchiHonanchoMb, Icon),
    API_VARIABLE(I_Suica_MarunouchiM, Icon),
    API_VARIABLE(I_Suica_MinusSign0, Icon),
    API_VARIABLE(I_Suica_MinusSign1, Icon),
    API_VARIABLE(I_Suica_MinusSign2, Icon),
    API_VARIABLE(I_Suica_MinusSign3, Icon),
    API_VARIABLE(I_Suica_MinusSign4, Icon),
    API_VARIABLE(I_Suica_MinusSign5, Icon),
    API_VARIABLE(I_Suica_MinusSign6, Icon),
    API_VARIABLE(I_Suica_MinusSign7, Icon),
    API_VARIABLE(I_Suica_MinusSign8, Icon),
    API_VARIABLE(I_Suica_MinusSign9, Icon),
    API_VARIABLE(I_Suica_MitaI, Icon),
    API_VARIABLE(I_Suica_MobileLogo, Icon),
    API_VARIABLE(I_Suica_NambokuN, Icon),
    API_VARIABLE(I_Suica_Nothing, Icon),
    API_VARIABLE(I_Suica_OedoE, Icon),
    API_VARIABLE(I_Suica_PenguinHappyBirthday, Icon),
    API_VARIABLE(I_Suica_PenguinTodaysVIP, Icon),
    API_VARIABLE(I_Suica_PlusSign1, Icon),
    API_VARIABLE(I_Suica_PlusSign2, Icon),
    API_VARIABLE(I_Suica_PlusSign3, Icon),
    API_VARIABLE(I_Suica_PlusStar, Icon),
    API_VARIABLE(I_Suica_QuestionMarkBig, Icon),
    API_VARIABLE(I_Suica_QuestionMarkSmall, Icon),
    API_VARIABLE(I_Suica_RinkaiR, Icon),
    API_VARIABLE(I_Suica_ShinjukuS, Icon),
    API_VARIABLE(I_Suica_ShopPin, Icon),
    API_VARIABLE(I_Suica_SmallStar, Icon),
    API_VARIABLE(I_Suica_StoreFan1, Icon),
    API_VARIABLE(I_Suica_StoreFan2, Icon),
    API_VARIABLE(I_Suica_StoreFrame, Icon),
    API_VARIABLE(I_Suica_StoreLightningHorizontal, Icon),
    API_VARIABLE(I_Suica_StoreLightningVertical, Icon),
    API_VARIABLE(I_Suica_StoreP1Counter, Icon),
    API_VARIABLE(I_Suica_StoreReceiptDashLine, Icon),
    API_VARIABLE(I_Suica_StoreReceiptFrame1, Icon),
    API_VARIABLE(I_Suica_StoreReceiptFrame2, Icon),
    API_VARIABLE(I_Suica_StoreSlidingDoor, Icon),
    API_VARIABLE(I_Suica_TWRLogo, Icon),
    API_VARIABLE(I_Suica_ToeiLogo, Icon),
    API_VARIABLE(I_Suica_TokyoMetroLogo, Icon),
    API_VARIABLE(I_Suica_TokyoMonorailLogo, Icon),
    API_VARIABLE(I_Suica_TozaiT, Icon),
    API_VARIABLE(I_Suica_VendingCan1, Icon),
    API_VARIABLE(I_Suica_VendingCan2, Icon),
    API_VARIABLE(I_Suica_VendingCan3, Icon),
    API_VARIABLE(I_Suica_VendingCan4, Icon),
    API_VARIABLE(I_Suica_VendingFlap1, Icon),
    API_VARIABLE(I_Suica_VendingFlap2, Icon),
    API_VARIABLE(I_Suica_VendingFlap3, Icon),
    API_VARIABLE(I_Suica_VendingFlapHollow, Icon),
    API_VARIABLE(I_Suica_VendingMachine, Icon),
    API_VARIABLE(I_Suica_VendingPage2Full, Icon),
    API_VARIABLE(I_Suica_VendingThankYou, Icon),
    API_VARIABLE(I_Suica_YenKanji, Icon),
    API_VARIABLE(I_Suica_YenSign, Icon),
    API_VARIABLE(I_Suica_YurakuchoY, Icon),
    API_VARIABLE(I_Suica_CardIcon, Icon),
    API_VARIABLE(I_Suica_ShopIcon, Icon),
    API_VARIABLE(I_Suica_VendingIcon, Icon),
    API_VARIABLE(I_Suica_TrainIcon, Icon),
    API_VARIABLE(I_Suica_BdayCakeIcon, Icon),
    API_VARIABLE(I_Suica_UnknownIcon, Icon),
    
    API_METHOD(render_section_header, void, (FuriString*, const char*, uint8_t, uint8_t)),
    API_METHOD(mosgortrans_parse_transport_block, bool, (const MfClassicBlock*, FuriString*)),
    API_VARIABLE(I_WarningDolphinFlip_45x42, Icon),
    API_VARIABLE(I_DolphinDone_80x58, Icon),
    API_VARIABLE(I_DolphinMafia_119x62, Icon)));
