#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "sd.h"

struct state {
    int out_fd;         // Target file descriptor
    int in_fd;          // Source file descriptor
    uint32_t sec, nsec; // Used for writing timestamps
};

struct fpga_ram_record {
    uint8_t data;
    uint8_t ctrl;
    uint8_t unk[2];
    uint32_t nsec;
    uint32_t sec;
} __attribute__ ((__packed__));

int net_write_data(void *state, void *pkt, int pkt_size) {
    struct state *st = state;
    return write(st->out_fd, pkt, pkt_size);
}

int sd_get_elapsed(void *state, long *real_sec, long *real_nsec) {
    struct state *st = state;
    *real_sec = st->sec;
    *real_nsec = st->nsec;
    return 0;
}


// Unused in this implementation
int fpga_ticks(void *state) {
    return 0;
}


static int convert_ramfile(struct state *st) {
    struct fpga_ram_record rec;
    int packet_count = 0;
    int invalid_xmas = 0;
    int invalid_ale_cle = 0;
    int invalid_empty = 0;
    int invalid_full = 0;

    pkt_send_hello(st);
    pkt_send_reset(st);
    pkt_send_buffer_drain(st, 1);
    while (read(st->in_fd, &rec, sizeof(rec)) == sizeof(rec)) {
        // Both ALE and CLE set, with WE and RE 0
        if ((rec.ctrl&0x0f) == 0x3) {
            invalid_xmas++;
            continue;
        }
        // Both ALE and CLE set
        if ((rec.ctrl&0x03) == 0x3) {
            invalid_ale_cle++;
            continue;
        }
        // Neither WE nor RE are set
        if ((rec.ctrl&0x0c) == 0xc) {
            invalid_empty++;
            continue;
        }
        // Both WE and RE are set
        if ((rec.ctrl&0x0c) == 0x0) {
            invalid_full++;
            continue;
        }

        st->sec = rec.sec;
        st->nsec = rec.nsec;
        pkt_send_nand_cycle(st, rec.data, rec.ctrl, rec.unk);
        packet_count++;
    }
    pkt_send_buffer_drain(st, 2);
    printf("Converted %d packets\n", packet_count);
    printf("Ignored %d packets with ALE/CLE/!RE/!WE\n", invalid_xmas);
    printf("Ignored %d packets with both ALE and CLE\n", invalid_ale_cle);
    printf("Ignored %d packets with both RE and WE\n", invalid_full);
    printf("Ignored %d packets with no RE or WE\n", invalid_empty);
    return 0;
}


static int print_help(char *name) {
    printf("Usage: %s -r [romfile] -o [outfile]\n", name);
    return 0;
}

int main(int argc, char **argv) {
    struct state st;
    int ch;

    memset(&st, 0, sizeof(st));

    while ((ch = getopt(argc, argv, "hr:o:")) != -1) {
        switch(ch) {
            case 'r':
                st.in_fd = open(optarg, O_RDONLY);
                if (-1 == st.in_fd) {
                    perror("Unable to open ram file");
                    return 3;
                }
                break;

            case 'o':
                st.out_fd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (-1 == st.out_fd) {
                    perror("Unable to open tbraw file");
                    return 2;
                }
                break;

            case 'h':
            default:
                print_help(argv[0]);
                return 1;
        }
    }
    if (!st.in_fd || !st.out_fd) {
        print_help(argv[0]);
        return 1;
    }

    argc -= optind;
    argv += optind;

    return convert_ramfile(&st);
}
