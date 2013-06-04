novena-tbram
============

Reads data from the Novena FPGA and stores it in a tbraw file.  As an
initial processing step, it removes impossible command sequences.


RAM format
============

Data is stored in RAM in the following format:

 struct fpga_ram_record {
     uint8_t data;
     uint8_t ctrl;
     uint8_t unk[2];
     uint32_t nsec;
     uint32_t sec;
 } __attribute__((__packed__));

Data pins are stored in-order, with NAND D7 being equivalent to data[7].
NAND values are stored inverted, so they must be flipped before getting
written out.

Control pins are defined with the following mask values:

 enum control_pins {
     NAND_ALE = 1,
     NAND_CLE = 2,
     NAND_WE = 4,
     NAND_RE = 8,
     NAND_CS = 16,
     NAND_RB = 32,
 };

The unk[] values represent the ten "unknown" pins on a Sandisk card.

tbraw format
============

The tbraw format is a series of packets that get emitted by an SD tap
board.  It is a serialized stream, making it suitable for streaming over
the network.  A tbraw file must be post-processed before analysis can take
place.

Each packet in a tbraw file has the following 11-byte header:

 struct pkt_header {
     uint8_t type;
     uint32_t sec;
     uint32_t nsec;
     uint16_t size;
 } __attribute__((__packed__));

The "type" code comes from the following enum:

 enum pkt_type {
     PACKET_UNKNOWN = 0,
     PACKET_ERROR = 1,
     PACKET_NAND_CYCLE = 2,
     PACKET_SD_DATA = 3,
     PACKET_SD_CMD_ARG = 4,
     PACKET_SD_RESPONSE = 5,
     PACKET_SD_CID = 6,
     PACKET_SD_CSD = 7,
     PACKET_BUFFER_OFFSET = 8,
     PACKET_BUFFER_CONTENTS = 9,
     PACKET_COMMAND = 10,
     PACKET_RESET = 11,
     PACKET_BUFFER_DRAIN = 12,
     PACKET_HELLO = 13,
 };

The "size" field determines the size of the entire packet, including the
11-byte header.  Therefore, the smallest possible packet is 11-bytes, and
the "size" field would be equal to "11".

"sec" and "nsec" are two values that log the seconds and nanoseconds
at which point the packet was generated.

Any additional data follows the header.  Additional data varies between
packet types.  See "packet.c" for specifics as to what commands carry what
sort of data.

tbraw conventions
-----------------

As a convention, a tbraw file begins with a PACKET_HELLO packet indicating
the version number, with a timestamp of 0.  This acts as a magic number to
denote a file as being a tbraw file.

Following the hello packet is a PACKET_RESET indicating the SD card began
its reset sequence.

When a tbraw stream is processed into an event stream, all packets are
sorted chronologically according to their header's sec/nsec values.

