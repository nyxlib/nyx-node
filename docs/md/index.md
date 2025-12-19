[![][Build Status img]][Build Status]
[![][License_mongoose img]][License_mongoose]
[![][License_arduino img]][License_arduino]

<a href="https://lpsc.in2p3.fr/" target="_blank"><img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_lpsc.svg" alt="LPSC" height="72" /></a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://www.in2p3.fr/" target="_blank"><img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_in2p3.svg" alt="IN2P3" height="72" /></a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://www.univ-grenoble-alpes.fr/" target="_blank"><img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_uga.svg" alt="UGA" height="72" /></a>

# Overview

The Nyx project introduces a protocol, backward-compatible with [INDI 1.7](docs/specs/INDI.pdf) (and [indiserver](http://docs.indilib.org/indiserver/)), for controlling scientific hardware.

It enhances INDI by supporting multiple independent nodes, each embedding its own protocol stack. Nodes communicate using JSON over [MQTT](https://mqtt.org/) for slow control, and through a dedicated streaming system for real-time visualization. An alternative INDI compatibility mode, based on XML over TCP, is also supported. This architecture provides flexibility and scalability for distributed systems.

The core library is written in C11 and targets POSIX, Windows, and Arduino environments, with no external dependencies; [Zlib](https://zlib.net/) is only required when compression is enabled.<br />Its low footprint makes Nyx suitable for deployment on microcontrollers (ESP32, Cortex-M, …).

## Typical architectures

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/refs/heads/main/docs/img/nyx-std.drawio.svg" style="width: 600px;" />
</div> 

> Standard Nyx architecture.

<hr />

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/refs/heads/main/docs/img/nyx-indiserver.drawio.svg" style="width: 600px;" />
</div>

> Any Nyx driver can be loaded by indiserver, via TCP, and used by an INDI 1.7-compliant client.

## Author

[Jérôme ODIER](https://annuaire.in2p3.fr/4121-4467/jerome-odier) ([CNRS/LPSC](http://lpsc.in2p3.fr/))

[Build Status]:https://github.com/nyxlib/nyx-node/actions/workflows/deploy.yml
[Build Status img]:https://github.com/nyxlib/nyx-node/actions/workflows/deploy.yml/badge.svg

[License_mongoose]:https://www.gnu.org/licenses/gpl-2.0.txt
[License_mongoose img]:https://img.shields.io/badge/License-GPL_2.0_only_(mongoose_layer)-blue.svg

[License_arduino]:https://www.gnu.org/licenses/lgpl-3.0.txt
[License_arduino img]:https://img.shields.io/badge/License-LGPL_3.0_or_later_(arduino_layer)-blue.svg
