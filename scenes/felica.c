#include "../metroflip_i.h"
#include "felica.h"

bool suica_verify(const FelicaData* felica_data) {
    return felica_data->pmm.data[1] == SUICA_IC_TYPE_CODE;
}

bool octopus_verify(FelicaPoller* felica_poller) {
    uint8_t blocks[1] = {0x00};
    FelicaPollerReadCommandResponse* rx_resp;
    felica_poller_read_blocks(felica_poller, 1, blocks, OCTOPUS_SERVICE_CODE, &rx_resp);
    
    return (rx_resp->SF1 + rx_resp->SF2) == 0;
}
