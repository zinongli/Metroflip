#include "../metroflip_i.h"
#include "felica.h"

bool suica_verify(FelicaPoller* felica_poller) {
    uint8_t blocks[1] = {0x00};
    FelicaPollerReadCommandResponse* rx_resp;
    felica_poller_read_blocks(felica_poller, 1, blocks, SUICA_SERVICE_CODE, &rx_resp);
    
    return (rx_resp->SF1 + rx_resp->SF2) == 0;
}

bool octopus_verify(FelicaPoller* felica_poller) {
    uint8_t blocks[1] = {0x00};
    FelicaPollerReadCommandResponse* rx_resp;
    felica_poller_read_blocks(felica_poller, 1, blocks, OCTOPUS_SERVICE_CODE, &rx_resp);
    
    return (rx_resp->SF1 + rx_resp->SF2) == 0;
}
