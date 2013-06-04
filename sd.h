#ifndef __SD_H__
#define __SD_H__

#include <stdint.h>

struct sd_syscmd {
    const uint8_t cmd[2];
    const uint32_t flags;
    const char *description;
    int(*handle_cmd)(void *state, int arg);
};

/* Command as it travels over the wire in binary mode */
struct sd_cmd {
    uint8_t cmd[2]; /* `\                   */                
                    /*    > Network packet  */
    uint32_t arg;   /* ,/                   */                
                                                              
    struct sd_syscmd *syscmd;                                 
};                                                            


int net_write_data(void *state, void *pkt, int pkt_size);
int sd_get_elapsed(void *state, long *real_sec, long *real_nsec);
int fpga_ticks(void *state);

int pkt_send_error(void *state, uint32_t code, char *msg);
int pkt_send_nand_cycle_fpga(void *state, uint32_t fpga_counter, uint8_t data, uint8_t ctrl, uint8_t unk[2]);
int pkt_send_nand_cycle(void *state, uint8_t data, uint8_t ctrl, uint8_t unk[2]);
int pkt_send_sd_data(void *state, uint8_t *block);
int pkt_send_sd_cmd_arg(void *state, uint8_t regnum, uint8_t val);
int pkt_send_sd_cmd_arg_fpga(void *state, uint32_t fpga_counter, uint8_t regnum, uint8_t val);
int pkt_send_sd_response(void *state, uint8_t byte);
int pkt_send_sd_response_fpga(void *state, uint32_t fpga_counter, uint8_t byte);
int pkt_send_sd_cid(void *state, uint8_t cid[16]);
int pkt_send_sd_csd(void *state, uint8_t csd[16]);
int pkt_send_buffer_offset(void *state, uint8_t buffertype, uint32_t offset);
int pkt_send_buffer_contents(void *state, uint8_t buffertype, uint8_t *buffer)
;
int pkt_send_command(void *state, struct sd_cmd *cmd);
int pkt_send_reset(void *state);
int pkt_send_buffer_drain(void *state, uint8_t start_stop);
int pkt_send_hello(void *state);


#endif // __SD_H__
