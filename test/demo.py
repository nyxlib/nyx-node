# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import os
import sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import nyx_node

########################################################################################################################

def main():

    ####################################################################################################################

    run_prop = nyx_node.NyxSwitchProp(
        'run',
        'Run',
        nyx_node.NyxOnOff.OFF,
    )

    run_vector = nyx_node.NyxSwitchVector(
        'Demo',
        'run',
        nyx_node.NyxState.OK,
        nyx_node.NyxPerm.RW,
        nyx_node.NyxRule.AT_MOST_ONE,
        [run_prop],
        group = 'Demo',
    )

    ####################################################################################################################

    mode_noise_prop = nyx_node.NyxSwitchProp(
        'mode_noise',
        'Noise only',
        nyx_node.NyxOnOff.ON,
    )

    mode_delta_prop = nyx_node.NyxSwitchProp(
        'mode_delta',
        'Dirac delta',
        nyx_node.NyxOnOff.OFF,
    )

    mode_comb_prop = nyx_node.NyxSwitchProp(
        'mode_comb',
        'Dirac comb',
        nyx_node.NyxOnOff.OFF,
    )

    mode_vector = nyx_node.NyxSwitchVector(
        'Demo',
        'signal_mode',
        nyx_node.NyxState.OK,
        nyx_node.NyxPerm.RW,
        nyx_node.NyxRule.ONE_OF_MANY,
        [
            mode_noise_prop,
            mode_delta_prop,
            mode_comb_prop,
        ],
        group = 'Demo',
    )

    ####################################################################################################################

    samp_rate_prop = nyx_node.NyxNumberDoubleProp(
        'samp_rate',
        'Sample rate [Hz]',
        '%.0f',
        1000.0,
        50000000.0,
        1000.0,
        2000000.0,
    )

    frequency_prop = nyx_node.NyxNumberDoubleProp(
        'frequency',
        'Frequency [Hz]',
        '%.0f',
        1000000.0,
        2000000000.0,
        1000.0,
        143050000.0,
    )

    power_prop = nyx_node.NyxNumberDoubleProp(
        'power',
        'Power (dB)',
        '%.1f',
        -150.0,
        20.0,
        1.0,
        -30.0,
    )

    signal_vector = nyx_node.NyxNumberVector(
        'Demo',
        'signal_params',
        nyx_node.NyxState.OK,
        nyx_node.NyxPerm.RW,
        [
            samp_rate_prop,
            frequency_prop,
            power_prop,
        ],
        group = 'Demo',
    )

    ####################################################################################################################

    fft_size_prop = nyx_node.NyxNumberUIntProp(
        'fft_size',
        'FFT size',
        '%u',
        1,
        4096,
        1,
        512,
    )

    fft_vector = nyx_node.NyxNumberVector(
        'Demo',
        'fft_params',
        nyx_node.NyxState.OK,
        nyx_node.NyxPerm.RW,
        [fft_size_prop],
        group = 'Demo',
    )

    ####################################################################################################################

    stream_samp_rate_prop = nyx_node.NyxStreamProp(
        'samp_rate',
        'Sample rate [Hz]',
    )

    stream_frequency_prop = nyx_node.NyxStreamProp(
        'frequency',
        'Frequency [Hz]',
    )

    stream_samples_prop = nyx_node.NyxStreamProp(
        'samples',
        'Samples',
    )

    stream_vector = nyx_node.NyxStreamVector(
        'Demo',
        'spectrum',
        nyx_node.NyxState.OK,
        [
            stream_samp_rate_prop,
            stream_frequency_prop,
            stream_samples_prop,
        ],
        group = 'Demo',
    )

    ####################################################################################################################

    node = nyx_node.NyxNode(
        'NYX_DEMO_PY',
        [
            mode_vector,
            run_vector,
            signal_vector,
            fft_vector,
            stream_vector,
        ],
        'tcp://0.0.0.0:7625',
        os.getenv('MQTT_URL'),
        os.getenv('STREAM_URL'),
        os.getenv('MQTT_USERNAME'),
        os.getenv('MQTT_PASSWORD'),
        3000,
        True,
    )

    ####################################################################################################################

    try:

        while True:

            node.poll(25)

    except KeyboardInterrupt:

        pass

    finally:

        node.finalize()

    ####################################################################################################################

    print('Bye.')

    return 0

########################################################################################################################

if __name__ == '__main__':

    sys.exit(main())

########################################################################################################################
