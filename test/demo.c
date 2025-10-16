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

typedef enum demo_mode_e
{
    DEMO_MODE_NOISE = 0,
    DEMO_MODE_DELTA = 1,
    DEMO_MODE_COMB = 2,

} demo_mode_t;

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_onoff_t s_run = NYX_ONOFF_OFF;
static demo_mode_t s_mode = DEMO_MODE_NOISE;

static float s_samp_rate = 2000000.0f;
static float s_frequency = 143050000.0f;
static float s_power = -30.0f;

static unsigned int s_fft_size = 512U;

/*--------------------------------------------------------------------------------------------------------------------*/

static bool run_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, int new_value, __UNUSED__ int old_value)
{
    s_run = (nyx_onoff_t) new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool mode_noise_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, __UNUSED__ int new_value, __UNUSED__ int old_value)
{
    if(new_value == NYX_ONOFF_ON)
    {
        s_mode = DEMO_MODE_NOISE;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool mode_delta_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, __UNUSED__ int new_value, __UNUSED__ int old_value)
{
    if(new_value == NYX_ONOFF_ON)
    {
        s_mode = DEMO_MODE_DELTA;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool mode_comb_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, __UNUSED__ int new_value, __UNUSED__ int old_value)
{
    if(new_value == NYX_ONOFF_ON)
    {
        s_mode = DEMO_MODE_COMB;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool sr_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, double new_value, __UNUSED__ double old_value)
{
    s_samp_rate = (float) new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool freq_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, double new_value, __UNUSED__ double old_value)
{
    s_frequency = (float) new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool power_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, double new_value, __UNUSED__ double old_value)
{
    s_power = (float) new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool fftsize_callback(__UNUSED__ nyx_dict_t *vector, __UNUSED__ nyx_dict_t *def, unsigned int new_value, __UNUSED__ unsigned int old_value)
{
    s_fft_size = new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_dict_t *stream_vector_signal = NULL;

/*--------------------------------------------------------------------------------------------------------------------*/

static void gen_noise(float *dst, size_t n, float mean_db)
{
    for(size_t i = 0; i < n; i++)
    {
        float u = (float) rand() / (float) RAND_MAX;
        float v = (float) rand() / (float) RAND_MAX;

        float w = (u + v - 1.0f) * 6.0f;

        dst[i] = mean_db + w;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void gen_delta(float *dst, size_t n, float mean_db)
{
    gen_noise(dst, n, mean_db);

    dst[n / 2U] += 20.0f;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void gen_comb(float *dst, size_t n, float mean_db)
{
    gen_noise(dst, n, mean_db);

    size_t step = n / 8u;

    for(size_t i = step / 2u; i < n; i += step)
    {
        dst[i] += 20.0f;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void timer_stream(__UNUSED__ void *arg)
{
    if(s_run != NYX_ONOFF_ON)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    static float spectrum[4096];

    const size_t n = (s_fft_size <= 4096U) ? (size_t) s_fft_size
                                           : (size_t) 4096U
    ;

    switch(s_mode)
    {
        case DEMO_MODE_NOISE: gen_noise(spectrum, n, s_power); break;
        case DEMO_MODE_DELTA: gen_delta (spectrum, n, s_power); break;
        default:              gen_comb (spectrum, n, s_power); break;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t sizes[] = {sizeof(s_samp_rate), sizeof(s_frequency), n * sizeof(float)};
    BUFF_t buffs[] = {&s_samp_rate       , &s_frequency       , spectrum         };

    nyx_stream_pub(stream_vector_signal, 100, 3, sizes, buffs);

    /*----------------------------------------------------------------------------------------------------------------*/
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
        .group = "Demo"
    };

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *def_run = nyx_switch_def_new("run", "Run", NYX_ONOFF_OFF);

    def_run->base.in_callback._int = run_callback;

    nyx_dict_t *defs_run[] = {def_run, NULL};

    nyx_dict_t *run_vector = nyx_switch_def_vector_new(
        "Demo",
        "run",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_AT_MOST_ONE,
        defs_run,
        &opt
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *def_mode_noise = nyx_switch_def_new("mode_noise", "Noise only", NYX_ONOFF_ON);
    nyx_dict_t *def_mode_delta = nyx_switch_def_new("mode_delta", "Dirac delta", NYX_ONOFF_OFF);
    nyx_dict_t *def_mode_comb = nyx_switch_def_new("mode_comb", "Dirac comb", NYX_ONOFF_OFF);

    def_mode_noise->base.in_callback._int = mode_noise_callback;
    def_mode_delta->base.in_callback._int = mode_delta_callback;
    def_mode_comb->base.in_callback._int = mode_comb_callback;

    nyx_dict_t *defs_mode[] = {def_mode_noise, def_mode_delta, def_mode_comb, NULL};

    nyx_dict_t *mode_vector = nyx_switch_def_vector_new(
        "Demo",
        "signal_mode",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_ONE_OF_MANY,
        defs_mode,
        &opt
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *samp_rate = nyx_number_def_new_double("samp_rate", "Sample rate [Hz]", "%.0f", 1000.0, 50000000.0, 1000.0, s_samp_rate);
    nyx_dict_t *freq = nyx_number_def_new_double("frequency", "Frequency [Hz]", "%.0f", 1000000.0, 2000000000.0, 1000.0, s_frequency);
    nyx_dict_t *power = nyx_number_def_new_double("power", "Power (dB)", "%.1f", -150.0, 20.0, 0.1, s_power);

    samp_rate->base.in_callback._double = sr_callback;
    freq->base.in_callback._double = freq_callback;
    power->base.in_callback._double = power_callback;

    nyx_dict_t *defs_params1[] = {samp_rate, freq, power, NULL};

    nyx_dict_t *params_vector1 = nyx_number_def_vector_new(
        "Demo",
        "signal_params",
        NYX_STATE_OK,
        NYX_PERM_RW,
        defs_params1,
        &opt
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *fft_size = nyx_number_def_new_uint ("fft_size", "FFT size", "%lu", 1UL, 4096UL, 1UL, s_fft_size);

    fft_size->base.in_callback._uint = fftsize_callback;

    nyx_dict_t *defs_params2[] = {fft_size, NULL};

    nyx_dict_t *params_vector2 = nyx_number_def_vector_new(
        "Demo",
        "fft_params",
        NYX_STATE_OK,
        NYX_PERM_RW,
        defs_params2,
        &opt
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *defs_stream[] = {
        nyx_stream_def_new("samp_rate", "Sample rate [Hz]"),
        nyx_stream_def_new("frequency", "Frequency [Hz]"),
        nyx_stream_def_new("samples", "Samples"),
        NULL,
    };

    stream_vector_signal = nyx_stream_def_vector_new(
        "Demo",
        "spectrum_stream",
        NYX_STATE_OK,
        defs_stream,
        &opt
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *vector_list[] = {
        run_vector,
        mode_vector,
        params_vector1,
        params_vector2,
        stream_vector_signal,
        NULL,
    };

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_t *node = nyx_node_initialize(
        "NYX_DEMO",
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

    /*----------------------------------------------------------------------------------------------------------------*/

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    nyx_node_add_timer(node, 50, timer_stream, NULL);

    while(s_signo == 0)
    {
        nyx_node_poll(node, 25);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_finalize(node, true);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_finalize();

    /*----------------------------------------------------------------------------------------------------------------*/

    printf("Bye.\n");

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
