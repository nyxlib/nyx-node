# Protocol

This page describes the **Nyx protocol**, a thin and backward-compatible overlay on the **INDI protocol**. INDI was
originally designed for astronomical hardware but is quite generic. Nyx preserves the INDI property/state model and
message semantics while modernizing transport and serialization. By default, Nyx exchanges **JSON** over an **MQTT
broker**, with a 1:1 mapping to INDI’s XML messages. It can also speak the original **INDI XML** directly over
**TCP** for strict compatibility.

In addition, Nyx introduces an **additional low-latency streaming system**, based on **Redis**, to deliver data
to multiple clients.

## INDI protocol

This section summarizes version 1.7 of the INDI protocol. For the normative specification (message grammar,
semantics, etc.), see the official [INDI.pdf](https://github.com/nyxlib/nyx-node/blob/main/docs/specs/INDI.pdf).

### Purpose and model

INDI is a small, stateless, XML-based protocol to control devices through **properties**. A device exposes
named properties (vectors of elements) and a client reads and changes them. Messages are **asynchronous**:
there is no strict request/response pairing and participants must accept any valid message at any time.
Malformed or unexpected input should be ignored rather than negotiated, which avoids protocol deadlocks
and eases broadcasting and routing.

### Discovery (introspection)

A client begins by asking a device to describe itself. The client may request all devices, all properties
of one device, or one specific property. Devices answer with *definitions* that fully describe each property
and its elements.

```xml
<!-- Client → Device: ask for properties -->
<getProperties version="1.7" device="Mount"/>

<!-- Device → Client: define a number vector with two elements -->
<defNumberVector device="Mount" name="EQUATORIALJ2000_COORD" state="Idle" perm="rw" timeout="50" label="J2000 Equatorial">
  <defNumber name="RA"  label="RA H:M:S"  format="%11.8m" min="0"  max="24">0</defNumber>
  <defNumber name="Dec" label="Dec D:M:S" format="%9.6m"  min="-90" max="90">0</defNumber>
</defNumberVector>
```

Definitions exist for text, number, switch, light, and BLOB vectors (`defTextVector`, `defNumberVector`,
`defSwitchVector`, `defLightVector`, `defBLOBVector`). Each element has a `name` and optional `label`. Numbers
add display `format`, `min`, `max`, and `step`. Switches can announce a UI `rule` such as `OneOfMany`, `AtMostOne`
or `AnyOfMany`.

### State, permissions, and timeouts

Every property carries a **state** among `Idle`, `Ok`, `Busy`, and `Alert`. Devices should also send human-readable
messages and timestamps alongside state changes. Permissions are hints for clients: text and number vectors can be
`ro` (read-only), `wo` (write-only), or `rw` (read-write); switches can be `ro` or `rw`; lights are `ro`. The
`timeout` value, in seconds, expresses the worst-case duration for the device to accomplish a change, allowing
clients to reason about progress and failure.

```xml
<setLightVector device="Building" name="Security" state="Alert" timestamp="2002-03-13T16:06:20">
  <oneLight name="Dock">Alert</oneLight>
</setLightVector>
```

### Changing values (Client → Device)

To change a property, the client sends a *target* message and immediately considers the property **Busy** until the
device reports completion. For numbers and text, the client should send all elements; other types may carry only
the changed members.

```xml
<!-- Set two numbers atomically -->
<newNumberVector device="Mount" name="EQUATORIALJ2000_COORD">
  <oneNumber name="RA">10:20:30</oneNumber>
  <oneNumber name="Dec">-04:05:06</oneNumber>
</newNumberVector>

<!-- Select one binning option; the device enforces OneOfMany -->
<newSwitchVector device="Camera" name="Binning">
  <oneSwitch name="Two">On</oneSwitch>
</newSwitchVector>
```

### Reporting values (Device → Client)

Devices report current values with `setXXXVector` messages and may include a new `state`, `timeout`, and a
timestamped `message`. For rapidly changing values, devices should avoid flooding slower clients.

```xml
<setSwitchVector device="Camera" name="Binning" state="Ok" timestamp="2002-03-13T16:04:02" message="Binning 2:1 selected">
  <oneSwitch name="One">Off</oneSwitch>
  <oneSwitch name="Two">On</oneSwitch>
</setSwitchVector>
```

### Transferring binary data (BLOBs)

BLOB elements carry base64-encoded payloads with a `format` hint (e.g., `.fits` or `.fits.z`) and a decoded size.
Clients control BLOB flow per connection with `enableBLOB`: `Never` disables all `setBLOB` messages on that
connection, `Also` permits `setBLOB` messages alongside normal INDI traffic, and `Only` restricts the connection
to `setBLOB` messages exclusively.

```xml
<!-- Client opts in to receive BLOBs from this device -->
<enableBLOB device="Wonder Cam">Also</enableBLOB>

<!-- Device sends a BLOB update -->
<setBLOBVector device="Wonder Cam" name="Image" state="Ok" timestamp="2002-03-13T16:05:00">
  <oneBLOB name="CCD1" size="16777216" format=".fits">BASE64…</oneBLOB>
</setBLOBVector>
```

### Messages and deletions

Devices can send human-oriented `message` elements, optionally tied to a device, to narrate progress or issues.
They can also announce that a property—or an entire device—is no longer available with `delProperty`.

```xml
<message device="Camera" timestamp="2002-03-13T16:06:20" message="TEC is approaching target temperature"/>

<delProperty device="FilterWheel" name="Filters" timestamp="2002-03-13T16:07:00" message="Wheel disconnected"/>
```

### Snooping other devices

Any device—or client—may subscribe to messages about other devices and properties by issuing `getProperties`
with `device` and optional `name`. From that point, matching `defXXX` and `setXXX` traffic is mirrored to
the requester, enabling coordination and higher-level behaviors.

```xml
<!-- Start snooping the Environment.Now property -->
<getProperties device="Environment" name="Now"/>
```

## Nyx protocol

### XML → JSON mapping

Nyx mirrors INDI’s XML as JSON with a minimal, lossless mapping: the element name is stored under `"<>"`, attributes
use an `"@"` prefix, text content goes under `"$"`, and child elements appear in a `"children"` array (order
preserved). Values are strings by design; clients parse numbers where needed.

**Example — Client → Device (ask for properties):**
```json
{
  "<>": "getProperties",
  "@version": "1.7",
  "@device": "Mount"
}
```

**Example — Device → Client (define a number vector):**
```json
{
  "<>": "defNumberVector",
  "@device": "Mount",
  "@name": "EQUATORIALJ2000_COORD",
  "@state": "Idle",
  "@perm": "rw",
  "@timeout": "50",
  "@label": "J2000 Equatorial",
  "children": [
    {
      "<>": "defNumber",
      "@name": "RA",
      "@label": "RA H:M:S",
      "@format": "%11.8m",
      "@min": "0",
      "@max": "24",
      "$": "0"
    },
    {
      "<>": "defNumber",
      "@name": "Dec",
      "@label": "Dec D:M:S",
      "@format": "%9.6m",
      "@min": "-90",
      "@max": "90",
      "$": "0"
    }
  ]
}
```

### Stream vectors

Nyx introduces **Stream Vectors** to declare metadata that describes a vector of streams. The **stream payloads
themselves are not carried in JSON messages**: they are delivered out‑of‑band via a low‑latency **Redis**
cache/streaming server to multiple clients. This streaming layer is specific to Nyx and not part of INDI.

### Nyx message grammar

The schema below describes the message structures for both the INDI protocol and its Nyx overlay . Nyx-specific
extensions are explicitly noted.

\include ./docs/specs/nyx.xsd
