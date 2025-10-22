# Demo

This demo shows how to manually develop a node that exposes a single device — a signal generator with its own properties.

Alternatively, [nyx-demo-c](https://github.com/nyxlib/nyx-demo-c) and [nyx-demo-cpp](https://github.com/nyxlib/nyx-demo-cpp) respectively demonstrate how to develop a similar device in C or  C++, this time using [Nyx-Assistant](https://github.com/nyxlib/nyx-assistant/) and [Nyx-Gen](https://github.com/nyxlib/nyx-gen/) to graphically define the node architecture and automatically generate the code skeleton and glue.

## Interface

Resulting interface as displayed in Nyx-Dashboard:

<div style="text-align: center;">
    <img src="https://raw.githubusercontent.com/nyxlib/nyx-node/main/docs/img/demo.png" style="width: 1250px;" />
</div> 

## Source code

\include ./test/demo.c
