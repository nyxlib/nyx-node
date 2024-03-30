/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <signal.h>

#include "../src/indi_base.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static int volatile s_signo = 0;

static void signal_handler(int signo)
{
    s_signo = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_memory_initialize();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(indi_validation_initialize() == false)
    {
        printf("Error initializing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *def1 = indi_switch_def_new("button1_on", "Turn ON", INDI_ONOFF_ON);
    indi_dict_t *def2 = indi_switch_def_new("button2_off", "Turn OFF", INDI_ONOFF_OFF);

    indi_dict_t *defs[] = {def1, def2, NULL};

    indi_dict_t *switch_vector = indi_switch_def_vector_new(
        "Telescope Simulator",
        "button1",
        INDI_STATE_OK,
        INDI_PERM_RW,
        INDI_RULE_AT_MOST_ONE,
        defs,
        NULL
    );

    indi_dict_t *vector_list[] = {switch_vector, NULL};

    /*----------------------------------------------------------------------------------------------------------------*/

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    indi_node_t *node = indi_node_init("mqtt://localhost:1883", NULL, NULL, "TOTO", vector_list, 3000, true, true);

    while(s_signo == 0)
    {
        indi_node_pool(node, 1000);
    }

    indi_node_free(node, true);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(indi_validation_finalize() == false)
    {
        printf("Error finalizing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_err:
    indi_memory_finalize();

    /*----------------------------------------------------------------------------------------------------------------*/

    printf("Bye.\n");

    /*----------------------------------------------------------------------------------------------------------------*/

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
