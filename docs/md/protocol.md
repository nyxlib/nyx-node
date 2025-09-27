# Nyx Protocol

Each command between **Client** and **Device** specifies a **Device** name and **Property** name.
The **Device** name serves as a logical grouping of several **Properties**. **Property** names must be
unique per **Device**, and a **Server** must report unique **Device** names to any one **Client**.

The **INDI protocol** does not have the notion of query and response. A sender does not save context
when it sends a command and wait for a specific response. A command may be sent for which a
complementary command back is likely but all **INDI participants** must always be prepared to receive
any command at any time. There is no notion of meta-errors such as illegally formatted commands,
inappropriate commands or problems with the underlying communication mechanism. The proper response to
all unusual or unexpected input is expressly to ignore any such problems (although some form of logging
outside the scope of INDI might be judicious). With these rules the **INDI protocol** is small and
simple; defines-away the possibility of deadlocks at the protocol level; automatically accommodates
broadcasting; permits flexible and transparent routing; and eliminates the need for complex sequencing
and synchronization.

When a **Client** first starts up it knows nothing about the **Devices** and **Properties** it will
control. It begins by connecting to a **Device** or **indiserver** and sending the **getProperties**
command. This includes the protocol version and may include the name of a specific **Device** and
**Property** if it is known by some other means. If no device is specified, then all devices are
reported; if no name is specified, then all properties for the given device are reported. The **Device**
then sends back one **deftype** element for each matching **Property** it offers for control, limited
to the **Properties** of the specified **Device** if included. The **deftype** element shall always
include all members of the vector for each **Property**.

Note again that by sending a request for **Property** definitions the **Client** is not then waiting
specifically for these definitions in reply. Nor is the **Device** obligated to supply these
definitions in any order or particular time frame. The **Client** may learn of some **Properties**
soon and perhaps others much later. The **Client** may also see messages for **Properties** about which
it is as yet unaware in which case the **Client** silently ignores the message. Thus a **Client** must
have the ability to dynamically expand its collection of **Properties** at any time and gracefully
ignore unexpected information.

To inform a **Device** of new target values for a **Property**, a **Client** sends one **newtype**
element. The **Client** must send all members of **Number** and **Text** vectors, or may send just the
members that change for other types. Before it does so, the **Client** sets its notion of the state of
the **Property** to **Busy** and leaves it until told otherwise by the **Device**. A **Client** does
not poll to see whether the current reported values happen to agree with what it last commanded and set
its state back to **Ok** on its own. This policy allows the **Device** to decide how close is close
enough. The **Device** will eventually send a state of **Ok** if and when the new values for the
**Property** have been successfully accomplished according to the **Device’s** criteria, or send back
**Alert** if they can not be accomplished with a message explaining why.

To inform a **Client** of new current values for a **Property** and their state, a **Device** sends one
**settype** element. It is only required to send those members of the vector that have changed. In the
case of **Properties** whose values change rapidly, the **Device** must insure that communication of
the new values are not sent so often as to saturate the connection with the **Client**. For example, a
socket implementation might send a new value only if writing to the socket descriptor would not block
the process.

In order to allow for the likelyhood of requiring special efficiency considerations in the design of
**Clients** to handle **BLOB Properties**, the element **enableBLOB** allows a **Client** to specify
whether **setBLOB** elements will arrive on a given **INDI connection**. The **Client** may send this
element with a value of **Only**, **Also** or **Never**. The default setting upon making a new
connection is **Never** which means **setBLOB** elements will never be sent over said connection. If
the **Client** sends **Only**, from then on only **setBLOB** elements shall be sent to the **Client**
on said connection. If the **Client** sends **Also**, then all other elements may be sent as well. A
**Client** may send the value of **Never** at any time to return to the default condition. This flow
control facility allows a **Client** the opportunity, for example, to open a separate connection and
create a separate processing thread dedicated to handling **BLOB** data. This behavior is only to be
implemented in intermediate **INDI server** processes; individual **Device drivers** shall disregard
**enableBLOB** and send all elements at will.

A **Device** may send out a **message** either as part of another command or by itself. When sent alone
a **message** may be associated with a specific **Device** or just to the **Client** in general.
**Messages** are meant for human readers and should be sent by a **Device** whenever any significant
event occurs or target is reached. The **INDI protocol** syntax provides a means for a **Device** to
time stamp each **message** and is encouraged to do so to maintain consistent time across all its
**Clients**. If the **Device** does not include a time stamp for some reason (such as if it is a very
simple device without local time-keeping capability) the **Client** is encouraged to add its own time
stamp.

A **Device** may tell a **Client** a given **Property** is no longer available by sending
**delProperty**. If the command specifies only a **Device** without a **Property**, the **Client** must
assume all the **Properties** for that **Device**, and indeed the **Device** itself, are no longer
available.

One **Device** may snoop the **Properties** of another **Device** by sending the **getProperties**
command. The command may specify one **Device** and one **Property**, or all **Properties** for a
**Device** or even all **Devices**, depending on whether the optional **device** and **name**
attributes are given. Once specified, all messages from the matching **Devices** and **Properties**
will be copied to the requesting **Device** as well.
