[![][Build Status img]][Build Status]
[![][License img]][License]

<a href="http://lpsc.in2p3.fr/" target="_blank">
	<img src="./images/logo_lpsc.svg" alt="LPSC" height="72" />
</a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="http://www.in2p3.fr/" target="_blank">
	<img src="./images/logo_in2p3.svg" alt="IN2P3" height="72" />
</a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="http://www.univ-grenoble-alpes.fr/" target="_blank">
	<img src="./images/logo_uga.svg" alt="UGA" height="72" />
</a>

# Nyx Node

The `Nyx Node` project introduces a new protocol, backward-compatible with [INDI 1.7](./spec/INDI.pdf), for controlling
astronomical hardware. It enhances INDI by supporting multiple independent nodes, each with its own embedded server.
These nodes can communicate directly over TCP or via an MQTT broker, providing greater flexibility and scalability for
distributed systems.

The `Nyx Node` library is written in C99 and requires no dependencies. It compiles within any UNIX or arduino
environment.

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
* https://nyxlib.org/node/

Developer
=========

* [Jérôme ODIER](https://annuaire.in2p3.fr/4121-4467/jerome-odier) ([CNRS/LPSC](http://lpsc.in2p3.fr/))

[Build Status]:https://gitlab.in2p3.fr/lpsc-kid/nyx-node/-/commits/main
[Build Status img]:https://gitlab.in2p3.fr/lpsc-kid/nyx-node/badges/main/pipeline.svg

[License]:https://www.gnu.org/licenses/lgpl-3.0.txt
[License img]:https://img.shields.io/badge/license-LGPL_3.0_or_later-blue.svg
