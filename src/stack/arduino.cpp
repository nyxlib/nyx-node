/*--------------------------------------------------------------------------------------------------------------------*/

#include <SPI.h>
#include <Arduino.h>

#include "arduino.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* NETWORK - W5500                                                                                                    */
/*--------------------------------------------------------------------------------------------------------------------*/

static int w5500_spi_ss_pin = 0;

/*--------------------------------------------------------------------------------------------------------------------*/

static void w5500_spi_beg(__UNUSED__ void *spi)
{
    digitalWrite(w5500_spi_ss_pin, LOW);
}

static void w5500_spi_end(__UNUSED__ void *spi)
{
    digitalWrite(w5500_spi_ss_pin, HIGH);
}

static uint8_t w5500_spi_txn(__UNUSED__ void *spi, uint8_t c)
{
    return SPI.transfer(c);
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

void nyx_arduino_init_w5500(struct mg_mgr *mgr, int spi_ss_pin, uint8_t mac[6])
{
    /*----------------------------------------------------------------------------------------------------------------*/

    pinMode(w5500_spi_ss_pin = spi_ss_pin, OUTPUT);

    /*----------------------------------------------------------------------------------------------------------------*/

    w5500_if.mac[0] = mac[0];
    w5500_if.mac[1] = mac[1];
    w5500_if.mac[2] = mac[2];
    w5500_if.mac[3] = mac[3];
    w5500_if.mac[4] = mac[4];
    w5500_if.mac[5] = mac[5];

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
/* CONSOLE                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_arduino_console(char c, __UNUSED__ buff_t params)
{
    if(Serial)
    {
        Serial.print(c);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
