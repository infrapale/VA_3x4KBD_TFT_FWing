#ifndef __RADIO433_H__
#define __RADIO433_H__

#define RADIO433_MAX_MSG_LEN  60
void radio_init(byte rfm_cs_pin, byte rfm_irqn_pin, byte rfm_rst_pin, float rfm_freq);
uint8_t radio_read_msg(char *inp_buf, uint8_t max_len);
void radio_send_msg(char *rf69_msg );


#endif
