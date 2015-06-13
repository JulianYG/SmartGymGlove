/* Communications header file */
#ifndef __COMM
#define __COMM

void setClocks(int master_frequency, int submain_divider);

void setupBT2(int clock, int divider_coarse, int divider_fine);
void startBT2( void );
void stopBT2( void );

void sendData(int data_device, char* data, int packet_size);

#endif
