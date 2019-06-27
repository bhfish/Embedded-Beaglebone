#ifndef _UDP_H_
#define _UDP_H_

//Parse received UDP message and match with our commands
int udp_cmd_parse(char* rx_buffer, char* tx_buffer, int* num);

//The main task of UDP module. Listen on socket port 12345 
//receive commands and reply
void udp_task(void);

//Initialization
void udp_init(void);

#endif