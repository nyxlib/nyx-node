/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../src/indi_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static int volatile s_signo = 0;

static void signal_handler(int signo)
{
    s_signo = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void on_callback(struct indi_object_s *def_vector)
{
    printf("ON button %d\n", indi_switch_def_get((indi_dict_t *) def_vector));
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void off_callback(struct indi_object_s *def_vector)
{
    printf("OFF button %d\n", indi_switch_def_get((indi_dict_t *) def_vector));
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

    indi_opts_t opt = {
        .group = "Test"
    };

    indi_dict_t *def1 = indi_switch_def_new("button1_on", "Turn ON", INDI_ONOFF_ON);
    indi_dict_t *def2 = indi_switch_def_new("button2_off", "Turn OFF", INDI_ONOFF_OFF);

    indi_dict_t *defs1[] = {def1, def2, NULL};

    indi_dict_t *switch_vector1 = indi_switch_def_vector_new(
        "Telescope Simulator",
        "foo",
        INDI_STATE_OK,
        INDI_PERM_RW,
        INDI_RULE_AT_MOST_ONE,
        defs1,
        &opt
    );

    def1->base.out_callback = on_callback;
    def2->base.out_callback = off_callback;

    indi_dict_t *def3 = indi_switch_def_new("foo_on", "Foo ON", INDI_ONOFF_ON);
    indi_dict_t *def4 = indi_switch_def_new("foo_off", "Foo OFF", INDI_ONOFF_OFF);

    indi_dict_t *defs2[] = {def3, def4, NULL};

    indi_dict_t *switch_vector2 = indi_switch_def_vector_new(
        "Telescope Simulator",
        "bar",
        INDI_STATE_OK,
        INDI_PERM_RW,
        INDI_RULE_AT_MOST_ONE,
        defs2,
        &opt
    );

    indi_dict_t *def5 = indi_number_def_new("qux1", "Qux 1", "%.1f", 0.0, 1.0, 0.1, 0.5);
    indi_dict_t *def6 = indi_number_def_new("qux2", "Qux 2", "%.1f", 0.0, 1.0, 0.1, 0.5);

    indi_dict_t *defs3[] = {def5, def6, NULL};

    indi_dict_t *number_vector1 = indi_number_def_vector_new(
        "Telescope Simulator",
        "qux",
        INDI_STATE_OK,
        INDI_PERM_RW,
        defs3,
        &opt
    );

    indi_dict_t *vector_list[] = {switch_vector1, switch_vector2, number_vector1, NULL};

    /*----------------------------------------------------------------------------------------------------------------*/

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    indi_node_t *node = indi_node_init(
        "tcp://0.0.0.0:7625",
        getenv("MQTT_URL"),
        getenv("MQTT_USERNAME"),
        getenv("MQTT_PASSWORD"),
        "TOTO",
        vector_list,
        3000,
        true,
        true
    );

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
