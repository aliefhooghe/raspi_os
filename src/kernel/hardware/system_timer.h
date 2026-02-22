#ifndef SATAN_HARDWARE_SYSTEM_TIMER_H_
#define SATAN_HARDWARE_SYSTEM_TIMER_H_

#define SYS_TIMER_CS_M0  0x00000001u  // System timer match 0 
#define SYS_TIMER_CS_M1  0x00000002u  // System timer match 1 
#define SYS_TIMER_CS_M2  0x00000004u  // System timer match 2 
#define SYS_TIMER_CS_M3  0x00000008u  // System timer match 3 

void system_timer_init(void);

#endif
