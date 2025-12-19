[![][Build Status img]][Build Status]
[![][Quality Gate Status img]][Quality Gate Status]
[![][License_mongoose img]][License_mongoose]
[![][License_arduino img]][License_arduino]

<div>
    <a href="http://lpsc.in2p3.fr/" target="_blank">
        <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_lpsc.svg" height="72"></a>
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
    <a href="http://www.in2p3.fr/" target="_blank">
        <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_in2p3.svg" height="72"></a>
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
    <a href="http://www.univ-grenoble-alpes.fr/" target="_blank">
        <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/logo_uga.svg" height="72"></a>
</div>

# Nyx Node

The Nyx project introduces a protocol, backward-compatible with [INDI 1.7](docs/specs/INDI.pdf) (and [indiserver](http://docs.indilib.org/indiserver/)), for controlling scientific hardware.

It enhances INDI by supporting multiple independent nodes, each embedding its own protocol stack. Nodes communicate using JSON over [MQTT](https://mqtt.org/) for slow control, and through a dedicated streaming system for real-time visualization. An alternative INDI compatibility mode, based on XML over TCP, is also supported. This architecture provides flexibility and scalability for distributed systems.

The core library is written in C11 and targets POSIX, Windows, and Arduino environments, with no external dependencies; [Zlib](https://zlib.net/) is only required when compression is enabled.<br />Its low footprint makes Nyx suitable for deployment on microcontrollers (ESP32, Cortex-M, …).

# Typical architecture

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/refs/heads/main/docs/img/nyx-std.drawio.svg" style="width: 600px;" />
</div> 

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/refs/heads/main/docs/img/nyx-indiserver.drawio.svg" style="width: 600px;" />
</div> 

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/refs/heads/main/docs/img/nyx-indi-bridge.drawio.svg" style="width: 600px;" />
</div> 

# Build instructions

```bash
mkdir build
cd build

cmake ..
make
sudo make install
```

# Home page and documentation

Home page:
* https://nyxlib.org/

Documentation:
* https://nyxlib.org/documentation/

# Developer

* [Jérôme ODIER](https://annuaire.in2p3.fr/4121-4467/jerome-odier) ([CNRS/LPSC](http://lpsc.in2p3.fr/))

# A bit of classical culture

In Greek mythology, Nyx is the goddess and personification of the night. She is one of the primordial deities, born
from Chaos at the dawn of creation.

Mysterious and powerful, Nyx dwells in the deepest shadows of the cosmos, from where she gives birth to many other
divine figures, including Hypnos (Sleep) and Thanatos (Death).

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/refs/heads/main/docs/img/nyx.png" style="width: 600px;" />
</div>

[Build Status]:https://github.com/nyxlib/nyx-node/actions/workflows/deploy.yml
[Build Status img]:https://github.com/nyxlib/nyx-node/actions/workflows/deploy.yml/badge.svg

[Quality Gate Status]:https://sonarcloud.io/summary/new_code?id=nyxlib_nyx-node
[Quality Gate Status img]:https://sonarcloud.io/api/project_badges/measure?project=nyxlib_nyx-node&metric=alert_status

[License_mongoose]:https://www.gnu.org/licenses/gpl-2.0.txt
[License_mongoose img]:https://img.shields.io/badge/License-GPL_2.0_only_(mongoose_layer)-blue.svg

[License_arduino]:https://www.gnu.org/licenses/lgpl-3.0.txt
[License_arduino img]:https://img.shields.io/badge/License-LGPL_3.0_or_later_(arduino_layer)-blue.svg
