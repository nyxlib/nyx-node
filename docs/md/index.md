[![][Build Status img]][Build Status]
[![][License_mongoose img]][License_mongoose]
[![][License_arduino img]][License_arduino]

<a href="https://lpsc.in2p3.fr/" target="_blank"><img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_lpsc.svg" alt="LPSC" height="72" /></a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://www.in2p3.fr/" target="_blank"><img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_in2p3.svg" alt="IN2P3" height="72" /></a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://www.univ-grenoble-alpes.fr/" target="_blank"><img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_uga.svg" alt="UGA" height="72" /></a>

## Overview

The `Nyx Node` project introduces a protocol, backward-compatible with [INDI 1.7](specs/INDI.pdf), for controlling
astronomical hardware. It enhances INDI by supporting multiple independent nodes, each with its own embedded protocol
stack. The nodes can communicate via an [MQTT](https://mqtt.org/) broker and a [Redis](https://redis.io/) cache (low
latency streams) or directly over TCP, offering flexibility and scalability for distributed systems.

The `Nyx Node` library is written in C99 and requires only a POSIX or Arduino environment, with no external
dependencies except optional ZLib support on POSIX. Its low footprint makes it suitable for deployment on
microcontrollers (ESP, Cortex-M, ...).

<div style="text-align: center;">
    <img src="https://nyxlib.org/user/pages/01.home/_callout/diagram.png" style="width:600px;" />
</div> 

## Author

[Jérôme ODIER](https://annuaire.in2p3.fr/4121-4467/jerome-odier) ([CNRS/LPSC](http://lpsc.in2p3.fr/))

## Example

\include ./test/test2.c

[Build Status]:https://github.com/nyxlib/nyx-node/actions/workflows/deploy.yml
[Build Status img]:https://github.com/nyxlib/nyx-node/actions/workflows/deploy.yml/badge.svg

[License_mongoose]:https://www.gnu.org/licenses/gpl-2.0.txt
[License_mongoose img]:https://img.shields.io/badge/License-GPL_2.0_only_(mongoose_layer)-blue.svg

[License_arduino]:https://www.gnu.org/licenses/lgpl-3.0.txt
[License_arduino img]:https://img.shields.io/badge/License-LGPL_3.0_or_later_(arduino_layer)-blue.svg
