/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../src/nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static int volatile s_signo = 0;

static void signal_handler(int signo)
{
    s_signo = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void on_callback(struct nyx_object_s *def_vector, bool modified)
{
    printf("ON button %d, modified: %s\n", nyx_switch_def_get((nyx_dict_t *) def_vector), modified ? "true" : "false");
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void off_callback(struct nyx_object_s *def_vector, bool modified)
{
    printf("OFF button %d, modified: %s\n", nyx_switch_def_get((nyx_dict_t *) def_vector), modified ? "true" : "false");

    if(modified)
    {
        nyx_node_send_message(def_vector->node, "Telescope Simulator", "Hello World!");
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_set_log_level(NYX_LOG_LEVEL_DEBUG);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_initialize();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(nyx_validation_initialize() == false)
    {
        printf("Error initializing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_opts_t opt = {
        .group = "Test"
    };

    nyx_dict_t *def1 = nyx_switch_def_new("button1_on", "Turn ON", NYX_ONOFF_ON);
    nyx_dict_t *def2 = nyx_switch_def_new("button2_off", "Turn OFF", NYX_ONOFF_OFF);

    nyx_dict_t *defs1[] = {def1, def2, NULL};

    nyx_dict_t *switch_vector1 = nyx_switch_def_vector_new(
        "Telescope Simulator",
        "foo",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_AT_MOST_ONE,
        defs1,
        &opt
    );

    def1->base.in_callback = on_callback;
    def2->base.in_callback = off_callback;

    nyx_dict_t *def3 = nyx_switch_def_new("foo_on", "Foo ON", NYX_ONOFF_ON);
    nyx_dict_t *def4 = nyx_switch_def_new("foo_off", "Foo OFF", NYX_ONOFF_OFF);

    nyx_dict_t *defs2[] = {def3, def4, NULL};

    nyx_dict_t *switch_vector2 = nyx_switch_def_vector_new(
        "Telescope Simulator",
        "bar",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_AT_MOST_ONE,
        defs2,
        &opt
    );

    nyx_dict_t *def5 = nyx_number_def_new("qux1", "Qux 1", "%.1f", 0.0, 1.0, 0.1, 0.5);
    nyx_dict_t *def6 = nyx_number_def_new("qux2", "Qux 2", "%.1f", 0.0, 1.0, 0.1, 0.5);

    nyx_dict_t *defs3[] = {def5, def6, NULL};

    nyx_dict_t *number_vector1 = nyx_number_def_vector_new(
        "Telescope Simulator",
        "qux",
        NYX_STATE_OK,
        NYX_PERM_RW,
        defs3,
        &opt
    );

    nyx_dict_t *vector_list[] = {switch_vector1, switch_vector2, number_vector1, NULL};

    /*----------------------------------------------------------------------------------------------------------------*/

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    nyx_node_t *node = nyx_node_initialize(
        "tcp://0.0.0.0:7625",
        getenv("MQTT_URL"),
        getenv("MQTT_USERNAME"),
        getenv("MQTT_PASSWORD"),
        "TOTO",
        vector_list,
        NULL,
        3000,
        true,
        true
    );

    while(s_signo == 0)
    {
        nyx_node_poll(node, 1000);
    }

    nyx_node_finalize(node, true);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(nyx_validation_finalize() == false)
    {
        printf("Error finalizing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_err:
    nyx_memory_finalize();

    /*----------------------------------------------------------------------------------------------------------------*/

    printf("Bye.\n");

    /*----------------------------------------------------------------------------------------------------------------*/

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
