/* Node
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <array>
#include <csignal>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

#include "nyx_node.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

enum class DemoMode
{
    NOISE = 0,
    DELTA = 1,
    COMB = 2,
};

/*--------------------------------------------------------------------------------------------------------------------*/

struct DemoState
{
    nyx::OnOff run = NYX_ONOFF_OFF;
    DemoMode mode = DemoMode::NOISE;

    float samp_rate = 2000000.0f;
    float frequency = 143050000.0f;
    float power = -30.0f;

    unsigned int fft_size = 512U;

    std::mt19937 random;
    std::uniform_real_distribution<float> uniform;

    std::array<float, 4096> spectrum = {};

    DemoState() : random(0), uniform(0.0f, 1.0f)
    {
    }
};

/*--------------------------------------------------------------------------------------------------------------------*/

static volatile std::sig_atomic_t &signal_number()
{
    static volatile std::sig_atomic_t result = 0;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void signal_handler(int signo)
{
    signal_number() = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static DemoState &demo_state()
{
    static DemoState result;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void check(bool value, STR_t message)
{
    if(!value)
    {
        throw nyx::Error(message);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool run_callback(const nyx::Vector &, const nyx::SwitchProp &, nyx::OnOff new_value, nyx::OnOff)
{
    demo_state().run = new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool mode_noise_callback(const nyx::Vector &, const nyx::SwitchProp &, nyx::OnOff new_value, nyx::OnOff)
{
    if(new_value == NYX_ONOFF_ON)
    {
        demo_state().mode = DemoMode::NOISE;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool mode_delta_callback(const nyx::Vector &, const nyx::SwitchProp &, nyx::OnOff new_value, nyx::OnOff)
{
    if(new_value == NYX_ONOFF_ON)
    {
        demo_state().mode = DemoMode::DELTA;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool mode_comb_callback(const nyx::Vector &, const nyx::SwitchProp &, nyx::OnOff new_value, nyx::OnOff)
{
    if(new_value == NYX_ONOFF_ON)
    {
        demo_state().mode = DemoMode::COMB;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool samp_rate_callback(const nyx::Vector &, const nyx::NumberDoubleProp &, double new_value, double)
{
    demo_state().samp_rate = static_cast<float>(new_value);

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool frequency_callback(const nyx::Vector &, const nyx::NumberDoubleProp &, double new_value, double)
{
    demo_state().frequency = static_cast<float>(new_value);

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool power_callback(const nyx::Vector &, const nyx::NumberDoubleProp &, double new_value, double)
{
    demo_state().power = static_cast<float>(new_value);

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool fftsize_callback(const nyx::Vector &, const nyx::NumberUIntProp &, uint32_t new_value, uint32_t)
{
    demo_state().fft_size = new_value;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void gen_noise(DemoState &state, std::array<float, 4096> &dst, size_t n, float mean_db)
{
    for(size_t i = 0; i < n; i++)
    {
        float u = state.uniform(state.random);
        float v = state.uniform(state.random);

        float w = (u + v - 1.0f) * 6.0f;

        dst[i] = mean_db + w;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void gen_delta(DemoState &state, std::array<float, 4096> &dst, size_t n, float mean_db)
{
    gen_noise(state, dst, n, mean_db);

    dst[n / 2U] += 20.0f;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void gen_comb(DemoState &state, std::array<float, 4096> &dst, size_t n, float mean_db)
{
    gen_noise(state, dst, n, mean_db);

    size_t step = n / 8U;

    for(size_t i = step / 2U; i < n; i += step)
    {
        dst[i] += 20.0f;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void timer_stream(DemoState &state, const nyx::StreamVector &stream_vector)
{
    if(state.run != NYX_ONOFF_ON)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    const size_t n = (state.fft_size <= state.spectrum.size()) ? static_cast<size_t>(state.fft_size)
                                                               : state.spectrum.size()
    ;

    switch(state.mode)
    {
        case DemoMode::NOISE: gen_noise(state, state.spectrum, n, state.power); break;
        case DemoMode::DELTA: gen_delta(state, state.spectrum, n, state.power); break;
        default:              gen_comb(state, state.spectrum, n, state.power); break;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    std::array<size_t, 3> sizes = {{sizeof(state.samp_rate), sizeof(state.frequency), n * sizeof(float)}};
    std::array<BUFF_t, 3> buffs = {{&state.samp_rate       , &state.frequency       , state.spectrum.data()}};

    check(stream_vector.stream_pub(sizes.size(), sizes.data(), buffs.data()), "Unable to publish Nyx stream");

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_callback(
    const nyx::NodeRef &node,
    nyx::MQTTEvent event_type,
    size_t topic_size,
    BUFF_t topic_buff,
    size_t,
    BUFF_t
) {
    /**/ if(event_type == NYX_NODE_EVENT_OPEN)
    {
        node.mqtt_sub("demo/exit", 0);
    }
    else if(event_type == NYX_NODE_EVENT_MSG)
    {
        static constexpr STR_t TOPIC = "demo/exit";
        static constexpr size_t TOPIC_SIZE = 9U;

        if(topic_size == TOPIC_SIZE && std::memcmp(topic_buff, TOPIC, TOPIC_SIZE) == 0)
        {
            signal_number() = 1;
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::set_log_level(NYX_LOG_LEVEL_DEBUG);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::Runtime runtime;

    /*----------------------------------------------------------------------------------------------------------------*/

    DemoState &state = demo_state();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::Opts opt("Demo");

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::SwitchProp run_prop("run", "Run", NYX_ONOFF_OFF);

    run_prop.on<run_callback>();

    nyx::SwitchVector run_vector(
        "Demo",
        "run",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_AT_MOST_ONE,
        std::vector<nyx::SwitchProp>(),
        opt
    );

    check(run_vector.add(run_prop), "Unable to add run property");

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::SwitchProp mode_noise_prop("mode_noise", "Noise only", NYX_ONOFF_ON);
    nyx::SwitchProp mode_delta_prop("mode_delta", "Dirac delta", NYX_ONOFF_OFF);
    nyx::SwitchProp mode_comb_prop("mode_comb", "Dirac comb", NYX_ONOFF_OFF);

    mode_noise_prop.on<mode_noise_callback>();
    mode_delta_prop.on<mode_delta_callback>();
    mode_comb_prop.on<mode_comb_callback>();

    nyx::SwitchVector mode_vector(
        "Demo",
        "signal_mode",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_ONE_OF_MANY,
        std::vector<nyx::SwitchProp>(),
        opt
    );

    check(mode_vector.add(mode_noise_prop), "Unable to add noise mode property");
    check(mode_vector.add(mode_delta_prop), "Unable to add delta mode property");
    check(mode_vector.add(mode_comb_prop), "Unable to add comb mode property");

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::NumberDoubleProp samp_rate_prop("samp_rate", "Sample rate [Hz]", "%.0f", 1000.0, 50000000.0, 1000.0, static_cast<double>(state.samp_rate));
    nyx::NumberDoubleProp frequency_prop("frequency", "Frequency [Hz]", "%.0f", 1000000.0, 2000000000.0, 1000.0, static_cast<double>(state.frequency));
    nyx::NumberDoubleProp power_prop("power", "Power (dB)", "%.1f", -150.0, 20.0, 1.0, static_cast<double>(state.power));

    samp_rate_prop.on<samp_rate_callback>();
    frequency_prop.on<frequency_callback>();
    power_prop.on<power_callback>();

    nyx::NumberVector signal_vector(
        "Demo",
        "signal_params",
        NYX_STATE_OK,
        NYX_PERM_RW,
        std::vector<nyx::NumberProp>(),
        opt
    );

    check(signal_vector.add(samp_rate_prop), "Unable to add sample rate property");
    check(signal_vector.add(frequency_prop), "Unable to add frequency property");
    check(signal_vector.add(power_prop), "Unable to add power property");

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::NumberUIntProp fft_size_prop("fft_size", "FFT size", "%u", 1U, 4096U, 1U, static_cast<uint32_t>(state.fft_size));

    fft_size_prop.on<fftsize_callback>();

    nyx::NumberVector fft_vector(
        "Demo",
        "fft_params",
        NYX_STATE_OK,
        NYX_PERM_RW,
        std::vector<nyx::NumberProp>(),
        opt
    );

    check(fft_vector.add(fft_size_prop), "Unable to add FFT size property");

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::StreamProp stream_samp_rate_prop("samp_rate", "Sample rate [Hz]");
    nyx::StreamProp stream_frequency_prop("frequency", "Frequency [Hz]");
    nyx::StreamProp stream_samples_prop("samples", "Samples");

    nyx::StreamVector stream_vector(
        "Demo",
        "spectrum",
        NYX_STATE_OK,
        std::vector<nyx::StreamProp>(),
        opt
    );

    check(stream_vector.add(stream_samp_rate_prop), "Unable to add stream sample rate property");
    check(stream_vector.add(stream_frequency_prop), "Unable to add stream frequency property");
    check(stream_vector.add(stream_samples_prop), "Unable to add stream samples property");

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx::NodeOpts node_opts;

    node_opts.indi_url = "tcp://0.0.0.0:7625";
    node_opts.mqtt_url = std::getenv("MQTT_URL");
    node_opts.nss_url = std::getenv("STREAM_URL");
    node_opts.mqtt_username = std::getenv("MQTT_USERNAME");
    node_opts.mqtt_password = std::getenv("MQTT_PASSWORD");
    node_opts.on_mqtt<mqtt_callback>();
    node_opts.retry_ms = 3000;
    node_opts.enable_xml = true;

    std::vector<nyx::Dict> vectors;

    vectors.emplace_back(run_vector.dict_ptr(), true);
    vectors.emplace_back(mode_vector.dict_ptr(), true);
    vectors.emplace_back(signal_vector.dict_ptr(), true);
    vectors.emplace_back(fft_vector.dict_ptr(), true);
    vectors.emplace_back(stream_vector.dict_ptr(), true);

    nyx::Node node(
        "NYX_DEMO",
        vectors,
        node_opts
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    (void) std::signal(SIGINT, signal_handler);
    (void) std::signal(SIGTERM, signal_handler);

    uint32_t stream_timer_ms = 0;

    while(signal_number() == 0)
    {
        node.poll(25);

        stream_timer_ms += 25;

        if(stream_timer_ms >= 50)
        {
            stream_timer_ms = 0;

            timer_stream(state, stream_vector);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node.close();

    /*----------------------------------------------------------------------------------------------------------------*/

    std::printf("Bye.\n");

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
