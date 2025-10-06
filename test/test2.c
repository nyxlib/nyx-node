/*--------------------------------------------------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include "../src/nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static int volatile s_signo = 0;

static void signal_handler(int signo)
{
    s_signo = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool def1_callback(nyx_dict_t *vector, nyx_dict_t *def, int new_value, int old_value)
{
    printf("ON button %s, modified: %s\n", nyx_onoff_to_str(new_value), old_value != new_value ? "true" : "false");

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool def2_callback(nyx_dict_t *vector, nyx_dict_t *def, int new_value, int old_value)
{
    printf("OFF button %s, modified: %s\n", nyx_onoff_to_str(new_value), old_value != new_value ? "true" : "false");

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void switch_vector1_callback(nyx_dict_t *vector, bool modified)
{
    nyx_node_send_message(vector->base.node, "Test", modified ? "`vector1` modified"
                                                                            : "`vector1` unmodified"
    );
}


/*--------------------------------------------------------------------------------------------------------------------*/

int cnt = 0;

bool state = false;

nyx_dict_t *def11 = NULL;
nyx_dict_t *def12 = NULL;

void *timer_thread(void *arg)
{
    struct timespec req = {0, 100000000}; // 100 ms

    while(s_signo == 0)
    {
        nanosleep(&req, NULL);

        if(cnt++ == 20)
        {
            if(state)
            {
                nyx_light_def_set(def11, NYX_STATE_OK);
                nyx_light_def_set(def12, NYX_STATE_ALERT);
            }
            else
            {
                nyx_light_def_set(def11, NYX_STATE_ALERT);
                nyx_light_def_set(def12, NYX_STATE_OK);
            }

            state = !state;

            cnt = 0;
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_set_log_level(NYX_LOG_LEVEL_DEBUG);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_initialize();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_opts_t opt = {
        .group = "Test"
    };

    nyx_dict_t *def1 = nyx_switch_def_new("button1_a", "A", NYX_ONOFF_ON);
    nyx_dict_t *def2 = nyx_switch_def_new("button2_b", "B", NYX_ONOFF_OFF);

    nyx_dict_t *defs1[] = {def1, def2, NULL};

    nyx_dict_t *switch_vector1 = nyx_switch_def_vector_new(
        "Test",
        "button_vector1",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_ONE_OF_MANY,
        defs1,
        &opt
    );

    def1->base.in_callback._int = def1_callback;
    def2->base.in_callback._int = def2_callback;

    switch_vector1->base.in_callback._vector = switch_vector1_callback;

    nyx_dict_t *def3 = nyx_switch_def_new("button3_a", "A", NYX_ONOFF_ON);
    nyx_dict_t *def4 = nyx_switch_def_new("button4_b", "B", NYX_ONOFF_OFF);

    nyx_dict_t *defs2[] = {def3, def4, NULL};

    nyx_dict_t *switch_vector2 = nyx_switch_def_vector_new(
        "Test",
        "button_vector2",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_AT_MOST_ONE,
        defs2,
        &opt
    );

    nyx_dict_t *def5 = nyx_switch_def_new("button3_a", "A", NYX_ONOFF_ON);
    nyx_dict_t *def6 = nyx_switch_def_new("button4_b", "B", NYX_ONOFF_OFF);

    nyx_dict_t *defs3[] = {def5, def6, NULL};

    nyx_dict_t *switch_vector3 = nyx_switch_def_vector_new(
        "Test",
        "button_vector3",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_ANY_OF_MANY,
        defs3,
        &opt
    );

    nyx_dict_t *def7 = nyx_number_def_new_double("number_a", "Number A", "%.1f", 0.0, 1.0, 0.1, 0.5);
    nyx_dict_t *def8 = nyx_number_def_new_double("number_b", "Number B", "%.1f", 0.0, 1.0, 0.1, 0.5);

    nyx_dict_t *defs4[] = {def7, def8, NULL};

    nyx_dict_t *number_vector1 = nyx_number_def_vector_new(
        "Test",
        "number_vector",
        NYX_STATE_OK,
        NYX_PERM_RW,
        defs4,
        &opt
    );

    nyx_dict_t *def9 = nyx_text_def_new("text_a", "Text A", "A");
    nyx_dict_t *def10 = nyx_text_def_new("text_b", "Text B", "B");

    nyx_dict_t *defs5[] = {def9, def10, NULL};

    nyx_dict_t *text_vector1 = nyx_text_def_vector_new(
        "Test",
        "text_vector",
        NYX_STATE_OK,
        NYX_PERM_RW,
        defs5,
        &opt
    );

    def11 = nyx_light_def_new("light_a", "Light A", NYX_STATE_OK);
    def12 = nyx_light_def_new("light_b", "Light B", NYX_STATE_ALERT);

    nyx_dict_t *defs6[] = {def11, def12, NULL};

    nyx_dict_t *light_vector1 = nyx_light_def_vector_new(
        "Test",
        "light_vector",
        NYX_STATE_OK,
        defs6,
        &opt
    );

    nyx_dict_t *vector_list[] = {
        switch_vector1,
        switch_vector2,
        switch_vector3,
        number_vector1,
        text_vector1,
        light_vector1,
        NULL,
    };

    /*----------------------------------------------------------------------------------------------------------------*/

    pthread_t tid;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    nyx_node_t *node = nyx_node_initialize(
        "TOTO",
        vector_list,
        "tcp://0.0.0.0:7625",
        getenv("MQTT_URL"),
        getenv("MQTT_USERNAME"),
        getenv("MQTT_PASSWORD"),
        NULL,
        getenv("REDIS_URL"),
        getenv("REDIS_USERNAME"),
        getenv("REDIS_PASSWORD"),
        3000,
        true
    );

    pthread_create(&tid, NULL, timer_thread, NULL);

    while(s_signo == 0)
    {
        nyx_node_poll(node, 1000);
    }

    pthread_join(tid, NULL);

    nyx_node_finalize(node, true);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_finalize();

    /*----------------------------------------------------------------------------------------------------------------*/

    printf("Bye.\n");

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
