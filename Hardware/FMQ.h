#ifndef __FMQ_H
#define __FMQ_H 
#include "sys.h"

void mfq_Init(void);
void fmq(float temp_one);
void beep_on(void);
void beep_off(void);

void beep_alarm(uint8_t is_open, uint8_t cnt);

#define BEEP PBout(0)	// BEEP,·äÃùÆ÷½Ó¿Ú
#endif

