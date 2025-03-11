/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(PICO_BOARD)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <hardware/spi.h>

#include "rpi.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* NETWORK - UTILITIES                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

static void get_mac_addr(uint8_t mac[6], uint8_t mac0, uint8_t mac1, STR_t node_id)
{
    uint32_t hash = nyx_hash32(node_id, strlen(node_id), 0xAABBCCDD);

    mac[0] = mac0;
    mac[1] = mac1;
    mac[2] = (hash >> 24) & 0xFF;
    mac[3] = (hash >> 16) & 0xFF;
    mac[4] = (hash >> 8) & 0xFF;
    mac[5] = (hash >> 0) & 0xFF;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* NETWORK - W5500                                                                                                    */
/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(MG_ENABLE_TCPIP) && defined(MG_ENABLE_DRIVER_W5500)
/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_w5500_spi_miso_pin = -1;
int nyx_w5500_spi_mosi_pin = -1;
int nyx_w5500_spi_clk_pin = -1;
int nyx_w5500_spi_cs_pin = -1;

/*--------------------------------------------------------------------------------------------------------------------*/

static void wspi_cs5500_spi_beg(__UNUSED__ void *spi)
{
    gpio_put(nyx_w5500_spi_cs_pin, 0);
}

static void w5500_spi_end(__UNUSED__ void *spi)
{
    gpio_put(nyx_w5500_spi_cs_pin, 1);
}

static uint8_t w5500_spi_txn(__UNUSED__ void *spi, uint8_t c)
{
    uint8_t result = 0;

    spi_write_read_blocking(spi0, &byte, &result, 1);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static struct mg_tcpip_spi w5500_spi = {
    nullptr,
    w5500_spi_beg,
    w5500_spi_end,
    w5500_spi_txn,
};

/*--------------------------------------------------------------------------------------------------------------------*/

static struct mg_tcpip_if w5500_if = {
    .driver = &mg_tcpip_driver_w5500,
    .driver_data = &w5500_spi,
};

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_rpi_init_w5500(struct mg_mgr *mgr, STR_t node_id)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    spi_init(spi0, 500 * 1000);

    gpio_set_function(nyx_w5500_spi_miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(nyx_w5500_spi_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(nyx_w5500_spi_clk_pin, GPIO_FUNC_SPI);

    /*----------------------------------------------------------------------------------------------------------------*/

    gpio_init(nyx_w5500_spi_cs_pin);

    gpio_set_dir(nyx_w5500_spi_cs_pin, GPIO_OUT);

    gpio_put(nyx_w5500_spi_cs_pin, 1);

    /*----------------------------------------------------------------------------------------------------------------*/

    get_mac_addr(w5500_if.mac, 0xEF, 0x02, node_id);

    mg_tcpip_init(mgr, &w5500_if);

    /*----------------------------------------------------------------------------------------------------------------*/

    MG_INFO(("MAC: %M. Waiting for IP...", mg_print_mac, w5500_if.mac));

    while(w5500_if.state != MG_TCPIP_STATE_READY)
    {
        mg_mgr_poll(mgr, 0);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
/* CONSOLE                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_rpi_console(char c, __UNUSED__ buff_t params)
{
    putchar(c);
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
