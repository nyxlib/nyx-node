#!/usr/bin/env python3
# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import os
import sys
import random
import signal
import struct

# noinspection PyTypeChecker
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import nyx_node

########################################################################################################################

def main():

    ####################################################################################################################

    MODE_NOISE = 0
    MODE_DELTA = 1
    MODE_COMB  = 2

    ####################################################################################################################

    run = nyx_node.NyxOnOff.OFF
    mode = MODE_NOISE

    samp_rate = 2000000.0
    frequency = 143050000.0
    power = -30.0

    fft_size = 512

    stop = False

    ####################################################################################################################

    def gen_noise(n: int, mean_db: float) -> list[float]:

        return [mean_db + (random.random() + random.random() - 1.0) * 6.0 for _ in range(n)]

    ####################################################################################################################

    def gen_delta(n: int, mean_db: float) -> list[float]:

        spectrum = gen_noise(n, mean_db)

        spectrum[n // 2] += 20.0

        return spectrum

    ####################################################################################################################

    def gen_comb(n: int, mean_db: float) -> list[float]:

        spectrum = gen_noise(n, mean_db)

        step = max(n // 8, 1)

        for index in range(step // 2, n, step):

            spectrum[index] += 20.0

        return spectrum

    ####################################################################################################################

    def get_spectrum() -> list[float]:

        n = min(fft_size, 4096)

        if mode == MODE_NOISE:

            return gen_noise(n, power)

        if mode == MODE_DELTA:

            return gen_delta(n, power)

        return gen_comb(n, power)

    ####################################################################################################################

    run_prop = nyx_node.NyxSwitchProp(
        'run',
        'Run',
        nyx_node.NyxOnOff.OFF,
    )

    @run_prop.on
    def on_run_changed(new_value, _old_value):

        nonlocal run

        run = new_value

        return True

    ####################################################################################################################

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

    @mode_noise_prop.on
    def on_mode_noise_changed(new_value, _old_value):

        nonlocal mode

        if new_value == nyx_node.NyxOnOff.ON:

            mode = MODE_NOISE

        return True

    ####################################################################################################################

    mode_delta_prop = nyx_node.NyxSwitchProp(
        'mode_delta',
        'Dirac delta',
        nyx_node.NyxOnOff.OFF,
    )

    @mode_delta_prop.on
    def on_mode_delta_changed(new_value, _old_value):

        nonlocal mode

        if new_value == nyx_node.NyxOnOff.ON:

            mode = MODE_DELTA

        return True

    ####################################################################################################################

    mode_comb_prop = nyx_node.NyxSwitchProp(
        'mode_comb',
        'Dirac comb',
        nyx_node.NyxOnOff.OFF,
    )

    @mode_comb_prop.on
    def on_mode_comb_changed(new_value, _old_value):

        nonlocal mode

        if new_value == nyx_node.NyxOnOff.ON:

            mode = MODE_COMB

        return True

    ####################################################################################################################

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
        samp_rate,
    )

    @samp_rate_prop.on
    def on_samp_rate_changed(new_value, _old_value):

        nonlocal samp_rate

        samp_rate = new_value

        return True

    ####################################################################################################################

    frequency_prop = nyx_node.NyxNumberDoubleProp(
        'frequency',
        'Frequency [Hz]',
        '%.0f',
        1000000.0,
        2000000000.0,
        1000.0,
        frequency,
    )

    @frequency_prop.on
    def on_frequency_changed(new_value, _old_value):

        nonlocal frequency

        frequency = new_value

        return True

    ####################################################################################################################

    power_prop = nyx_node.NyxNumberDoubleProp(
        'power',
        'Power (dB)',
        '%.1f',
        -150.0,
        20.0,
        1.0,
        power,
    )

    @power_prop.on
    def on_power_changed(new_value, _old_value):

        nonlocal power

        power = new_value

        return True

    ####################################################################################################################

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
        fft_size,
    )

    @fft_size_prop.on
    def on_fft_size_changed(new_value, _old_value):

        nonlocal fft_size

        fft_size = new_value

        return True

    ####################################################################################################################

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

    with nyx_node.NyxNode(
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
    ) as node:

        ################################################################################################################

        @node.on_timer(50)
        def on_timer():

            if run != nyx_node.NyxOnOff.ON:

                return

            spectrum = get_spectrum()

            stream_vector.stream_pub([
                struct.pack('=f', samp_rate),
                struct.pack('=f', frequency),
                struct.pack(f'={len(spectrum)}f', *spectrum),
            ])

        ################################################################################################################

        @node.on_mqtt(nyx_node.NyxMQTTEvent.OPEN)
        def on_mqtt_open():

            node.mqtt_sub('demo/exit')

        ################################################################################################################

        @node.on_mqtt(nyx_node.NyxMQTTEvent.MSG)
        def on_mqtt_msg(topic, _message):

            nonlocal stop

            if topic == 'demo/exit':

                stop = True

        ####################################################################################################################

        stop = False

        def signal_handler(_signo, _frame):

            nonlocal stop

            stop = True

        signal.signal(signal.SIGINT, signal_handler)

        ################################################################################################################

        while not stop:

            node.poll(25)

    ####################################################################################################################

    print('Bye.')

    return 0

########################################################################################################################

if __name__ == '__main__':

    sys.exit(main())

########################################################################################################################
