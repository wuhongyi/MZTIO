A NetTimeSync software triggering API is implemented in nts.c and used
to build an acquisition program in netdaq.c, a fork of startdaq.c that
enhances run mode 0x501 with network triggering.

Status
======

The system successfully synchronizes run start and triggers and
decisions flowing over the network. Each node type tracks the metadata
it needs to match timestamps and decisions. Data storage is simulated
by printing console output on the DAQ to indicate each received accept
time range that matches a sent trigger.

A system with a Ruby DM on a 3.6GHz Windows desktop and two PixieNets
with a split pulser signal can process about 10k pulses per second
(20k total triggers per second at the DM) with a 160us coincidence
window.

Besides console output to indicate status, both node types write log
files for debugging each sent and received message.

Architecture
============

A complete system consists of one or more PixieNet DAQs running netdaq
and any other computer running the DM, currently prototyped as a Ruby
script in the dm folder.

The DM is a standalone server and DAQs connect to it. The programs can
be started in any order. They synchronize run start, complete one run,
and exit.

The communication backbone is built with two zeromq TCP-based socket
pair types. Using TCP guarantees that when an application receives
data, it is in the order sent and nothing sent previously is missing.
ZeroMQ provides a nice API so connections are managed automatically
and we can work with whole messages without marking boundaries in the
stream.

The socket types are chosen for each use based on their defined
semantics. The DM binds a PUB socket to broadcast run synchronization
and trigger accept decisions to all DAQs. The DM binds a PULL socket
to receive trigger metadata from DAQs. DAQs connect SUB and PUSH,
respectively. For detailed information on the socket types, see
[zmq-socket] and the [zguide].

[zmq-socket]: http://api.zeromq.org/4-3:zmq-socket
[zguide]: http://zguide.zeromq.org

Dependencies
============

ZeroMQ
------

Build and install a recent version from [zeromq releases] on each
architecture, i.e. for the PixieNet ARM and for the DM (probably x64).
The current DM and netdaq code was developed with 4.3.2.
[zeromq build commands] shows the build dependencies and steps. Change
the version string and follow along with the commands on a PixieNet
and the DM host. You can then copy lib/libzmq.so to /usr/local/lib on
other PixieNets, along with the executables in perf/.libs for testing
the library. Try `inproc_lat 10 100000` to start simple and validate
the library with one machine. Then run local_lat/remote_lat and
local_thr/remote_thr over pairs of machines to test connections and
benchmark performance.

[zeromq releases]: https://github.com/zeromq/libzmq/releases
[zeromq build commands]: https://gist.github.com/katopz/8b766a5cb0ca96c816658e9407e83d00

Cross-compiling zeromq and PNcode with the Xilinx SDK on Windows
----------------------------------------------------------------

The script below uses Ubuntu on Windows for bash and make. Since I
couldn't get zeromq building with the Xilinx toolchain on Windows, the
cross toolchain is used to build the library. Then the Xilinx
toolchain is used to build PNcode.

```sh
# zeromq deps
ZSRC=$HOME/dev/zmq/zeromq-4.3.2
ZBUILD=$HOME/dev/zmq/arm/build-zmq
ZINST=$HOME/dev/zmq/arm/deps

# Get the target platform's toolchain. This works for PixieNet Zynq 7000
TC=arm-linux-gnueabihf
sudo apt-get install gcc make gcc-$TC g++-$TC binutils-$TC

mkdir -p $ZBUILD
cd $ZBUILD

$ZSRC/configure --prefix=$ZINST --without-docs\
  --host=$TC CC=$TC-gcc CXX=$TC-g++ AR=$TC-ar
rm -rf $ZINST
make clean
make -j8 install

# To build netdaq in the PNcode tree.
SDK=/mnt/e/Xilinx/SDK/2018.3/gnu/aarch32/nt/gcc-arm-linux-gnueabi
DEPS=$HOME/dev/zmq/arm/deps

make CC=$SDK/bin/arm-linux-gnueabihf-gcc.exe CXX=$SDK/bin/arm-linux-gnueabihf-g++.exe INCDIRS=$SDK/arm-linux-gnueabihf\include\c++\7.3.1 ZINST=$DEPS netdaq
```

Notes for attempting to build zeromq with the Xilinx SDK:
- set ZSRC to a relative path. Rooted WSL paths break Xilinx gcc finding source files.
- set ZBUILD under /mnt/* (native Windows processes can't write to the WSL volume)
- put the SDK gnu\aarch32\nt\gcc-arm-linux-gnueabi\bin on the PATH
- set CC, CXX, and AR on the configure command to the full Xilinx EXEs
  as in the PNcode make command.

I got stuck with ld.exe not finding libzmq.so because the configure
script generated the makefile to symlink the filename to the versioned
shared library (libzmq.so.5.x.x).

Compilation
===========

Netdaq is compiled by running `make` or `make netdaq` in the PNcode
folder. It compiles fine on the PixieNet itself.

It's also feasible to cross-compile using the [Xilinx SDK] in Ubuntu
on Windows. See `PNcode/script/build.sh`. `deploy.sh` scripts the
build and copying the executable to the PixieNets.

ZeroMQ is harder to cross-compile. I couldn't get it built with the
Xilinx toolchain, so I ended up using the ARM toolchain available in
the Ubuntu 18.04 package system for that.

[Xilinx SDK]: https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools.html

Acquisition
===========

Running a system requires starting netdaq in one or more PixieNets'
shells and running a DM. The DM runs as a standalone server. The
PixieNets need to know the DM's IP address.

To run a system with PixieNets with PTP (PTP hardware/firmware and
settings.ini AUX_CTRL bit 3 set), see `PNcode/script/run-ptp.rb`. The
script sets up PTP enable in the near future and starts all the
processes, and blocks on DM completion.

To run a system without PTP, see `PNcode/script/run-remote.sh`.

Other useful scripts
====================

PNcode/script contains scripts used while developing netdaq and dm.
There is overlap with scripts Hostcode.

Most scripts shell commands to two remote PixieNets and assume
hostnames pixie-net and pixie-net-b are configured in ~/.ssh/config
with hostnames and key files.

* runtime.sh: overwrite the REQ_RUNTIME setting on remote PixieNets.
* pmode.sh: overwrite the RUN_TYPE setting on remote PixieNets.
* copy-ptp.sh: helper to sync /var/www/ptp-mii-tool binaries from one
  PixieNet to the other.

Performance Analysis
====================

DAQ IO
------

Currently netdaq is single-threaded, so it occasionally hangs for
10-100ms flushing spectrum and log outputs to the SD card. A delay of
this magnitude in one DAQ while another DAQ continues to send
timestamps often causes triggers to be expired by the DM if a short DM
buffer time is used. Experience indicates even the Ruby rbtree scales
well with number of items inserted, so increasing the buffer size is a
good workaround; with C it should be very fast and limited only by
memory. Still, to support extreme throughput and use less memory, a
better design would refactor the loop into component threads, pass
messages between the threads, and perform all file IO on separate
threads.

As a first step, create a logger task consisting of a thread, an
inproc PULL socket (bind), and a simple loop that receives from the
socket and writes logs to a file. The main thread would start the
logger and connect to a PUSH socket before proceeding with the
acquisition. Other threads could send to the logger, as well. Messages can
be simple strings sent with s_send from zhelpers.h. All of this could
be wrapped in a simple logger API similar to nts.c.

Periodic statistics should also be written in the background. This
probably makes sense as a separate thread. Only the main thread would
send to it, so an inproc PAIR socket would suffice. Messages could be
sent using multipart messages or frames in two parts, one to identify
the file (some ID or filename) and the second with the contents to
append (maybe a third part or a tag character in the first part to
specify append or create).

netdaq probably writes too much to stdout, given all the status
characters written for ring buffer operations. These could be removed
or hidden behind compiler flags, especially if a better designed and
better tested ring buffer implementation is used. 

I don't know if it would help or not to dedicate a thread to polling
DM decisions and managing the NTS metadata buffers. The IO itself is
automatically multithreaded by zeromq, so it may not be so bad to have
the main thread polling for triggers, sending metadata (enqueueing,
technically), polling for decisions (reading already queued data from
the socket), and storing and searching local buffers.

Answering the last question may require profiling the code. To do this
we will need profiling tools on the PixieNet. The Linux perf tool is
part of the kernel tree and needs to be built for the device. Wolfgang
has a Zynq kernel compilation setup. Here are some notes on compiling
perf tools:
- https://www.toradex.com/community/questions/18041/how-can-i-build-linux-tools-for-a-customized-kerne.html
- http://jireren.github.io/blog/2016/09/19/Compile-Perf/

Ruby DM
-------

The Ruby code is fairly well optimized for what it is. It may or may
not help to linearize the matching logic that is currently done with
two each iterators. Interestingly, while doing many runs of five
seconds, setting the buffer as large as five seconds appeared to incur
no performance penalty. At high rates, Ruby integer comparison shows
up in the profiler (counting both indexes and scans), but it was
not a bottleneck.

But it's already evident in the profiler that the zeromq reads and
writes are problematic as the system slows down. The best approach for
performance and customer usability is to port the DM to C. I recommend
looking at tries or radix trees as replacements for the RB-tree used
in the Ruby code. See [nedtries], the same author's RB-tree library,
and [rax] (Redis's radix tree) for some nice APIs that should have the
performance characteristics we need.

[nedtries]: https://github.com/ned14/nedtries
[rax]: https://github.com/antirez/rax

DAQ buffers
-----------

The nts.c API encapsulates a simple hand-rolled ring buffer, besides
the core network communication and timestamp matching/storing logic.
The ring buffer should be replaced with a robust library, such as
[utringbuffer]. If profiling indicates scanning the buffer is a
bottleneck, try to binary search the ring buffer instead of linear
scanning, or replace it with a trie or RB-tree. Originally I thought
the linear scan would be fast, but I'm not sure. If larger buffers are
needed to buffer enough data at high rates, and if the buffer is
continuously full because not all triggers are accepted, it could
become slow.

The ring buffer size is controlled by constant NTS_MAX_WAIT in nts.h.
This should be parameterized so the user can tune it from netdaq.

[utringbuffer]: http://troydhanson.github.io/uthash/utringbuffer.html

Network chattiness
------------------

Networking bottlenecks may appear on the slower DAQs, especially when
all four channels are used. If multiple channels hit, they could be
encoded into one message somehow or simply sent as multipart messages,
one part per channel hit.

Serializing messages with Protocol Buffers or some other binary
encoding library would reduce the size of the integers sent over the
wire.

A trick would be to set the module ID on the DAQ side of trigger send
socket as the socket identity (see [zmq-setsockopt]) and let the DM
examine that ID instead of sending it every time. This may not be
worth complicating the zeromq calls, though.

[zmq-setsockopt]: http://api.zeromq.org/4-3:zmq-setsockopt

A note on interthread communication
-----------------------------------

The philosophy of zeromq applications is to avoid shared memory access
and instead use a less errorprone approach of sending messages through
sockets. zeromq inproc sockets are essentially lockfree queues, so
they are generally fast enough for most purposes. Comparing a one-off
benchmark using Boost lockfree queues and monitoring a condition
variable (as used in the acquire program) with zeromq inproc sockets ,
both performed about the same on the PixieNet, about 250,000 1000 byte
messages per second. If no condition variable was awaited in the Boost
case, it sped up to 440,000 messages per second. With 30 byte
messages, message throughput was higher with both methods. With
inproc, PUSH/PULL and PAIR socket types performed the same. So inproc
sockets perform well and are a good option to avoid a dependency on
another queue library. An advantage is they integrate nicely with
zeromq polling loops.
