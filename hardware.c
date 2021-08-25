/*
 File        : hardware.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : simulatoin of the hardware devices.

 Copyright notice:

 avr_simulator - A GNU/Linux simulator for the Atmel AVR series
 of microcontrollers. Copyright (C) 2001 Sergey Uvarov

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 Sergiy Uvarov

 E-mail : colonel@ff.dsu.dp.ua
*/

#define CK1    1<<0
#define CK8    1<<1
#define CK32   1<<2
#define CK64   1<<3 
#define CK128  1<<4
#define CK256  1<<5
#define CK1024 1<<6 

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <libintl.h>
#include <Xm/XmAll.h>
#include <inttypes.h>
#include "types.h"
#include "global.h"

void     set_flag(int f);
void     clear_flag(int f);
int      get_flag(int f);
int      check_mask_and_flag(int m,int f);
int      get_pin(int p);
void     set_pin(int p);
void     clear_pin(int p);
uint8_t  add_port(int p,uint8_t add);
uint8_t  get_port(int port);
void     set_port(int p,uint8_t value);
void     set_port16(int low,int hi,uint16_t value);
uint16_t get_port16(int low,int hi);
uint16_t add_port16(int low,int hi,uint16_t value);
void     set_sreg_flag(int f);
void     clear_sreg_flag(int f);
int      get_sreg_flag(int f);

static void compare_set_pin(int mode,int pin);
static void pwm_compare_set_pin(int mode,int pin,int add);
static void fpwm_compare_match_set_pin(int mode,int pin);
static void fpwm_compare_top_set_pin(int mode,int pin);
static void set_icr1(void);
static void set_icr3(void);

int reset_int();
int int0_int();
int int1_int(); 
int int2_int();
int int3_int();
int int4_int();
int int5_int();
int int6_int();
int int7_int();
int pcint0_int();
int pcint1_int();
int timer3_capt_int();
int timer3_compa_int();
int timer3_compb_int();
int timer3_compc_int();
int timer3_ovf_int();
int timer2_comp_int();
int timer2_ovf_int();
int timer1_capt_int();
int timer1_compa_int();
int timer1_compb_int();
int timer1_compc_int();
int timer1_ovf_int();
int timer0_comp_int();
int timer0_ovf_int();
int spi_stc_int();
int uart0_rx_int();
int uart0_udre_int();
int uart0_tx_int();
int uart1_rx_int();
int uart1_udre_int();
int uart1_tx_int();
int adc_int();
int ee_ready_int();
int analog_comp_int();
int twsi_int();
int spm_rdy_int();

typedef int (*Func_int)(int) ;

Func_int func_int[MAX_CODE_INT]={
  reset_int,
  int0_int,
  int1_int, 
  int2_int,
  int3_int,
  int4_int,
  int5_int,
  int6_int,
  int7_int,
  pcint0_int,
  pcint1_int,
  timer3_capt_int,
  timer3_compa_int,
  timer3_compb_int,
  timer3_compc_int,
  timer3_ovf_int,
  timer2_comp_int,
  timer2_ovf_int,
  timer1_capt_int,
  timer1_compa_int,
  timer1_compb_int,
  timer1_compc_int,
  timer1_ovf_int,
  timer0_comp_int,
  timer0_ovf_int,
  spi_stc_int,
  uart0_rx_int,
  uart0_udre_int,
  uart0_tx_int,
  uart1_rx_int,
  uart1_udre_int,
  uart1_tx_int,
  adc_int,
  ee_ready_int,
  analog_comp_int,
  twsi_int,
  spm_rdy_int
};

int prescaler_value0,prescaler_result0,prescaler_value1,prescaler_result1,
  prescaler_value2,prescaler_result2,prescaler_value3,prescaler_result3,
  watchdog_prescaler=1;
double eeprom_wat=0.0025;    /* EERROM write access time in Sec. */
int ex_ifr;                  /* hardware external interrupt flag register */

static Pin *pin;
static IO_port *io_port;
static Flag *flag;

Ports ports[6];


/* clean SRAM and EEPROM */
void clean_memory(void)
{
  int n;

  memset(gpr_pointer,0,controllers[type_micro].size_gpr+
         controllers[type_micro].size_io_reg+
         controllers[type_micro].size_sram);
  memset(eeprom_pointer,0,controllers[type_micro].size_eeprom);
  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    io_pointer[controllers[type_micro].io_registers[n].address]=
      controllers[type_micro].io_registers[n].init_value;
}

void prescaler0(void)
{
  prescaler_result0=0;
  
  if(ISFLAG(PSR10_FLAG)) if(get_flag(PSR10_FLAG))
    {prescaler_value0=prescaler_value1=0;clear_flag(PSR10_FLAG);}
  if(ISFLAG(PSR0_FLAG)) if(get_flag(PSR0_FLAG))
    {prescaler_value0=0;clear_flag(PSR0_FLAG);}
  if(ISFLAG(PSR310_FLAG)) if(get_flag(PSR310_FLAG))
    {
      prescaler_value0=prescaler_value1=prescaler_value3=0;
      clear_flag(PSR310_FLAG);
    }

  prescaler_value0++;

  prescaler_result0|=CK1;
  if(prescaler_value0%8) return;
  prescaler_result0|=CK8;

  if(prescaler_value0%32) return;
  prescaler_result0|=CK32;

  if(prescaler_value0%64) return;
  prescaler_result0|=CK64;
  
  if(prescaler_value0%128) return;
  prescaler_result0|=CK128;

  if(prescaler_value0%256) return;  
  prescaler_result0|=CK256;

  if(prescaler_value0%1024) return;
  prescaler_result0|=CK1024;
  prescaler_value0=0;
 
}

void timer_counter0(void)
{
  int cs=0,prescaler_changed=0,com=0,pwm_mode=0,bottom,max,port;
  static int t0_pin=0,clear_tcnt=0,add=1,ocr=0;

  if(!clocks) {t0_pin=clear_tcnt=0;add=1;}

  if(!ISPORT(TCCR0)) return;
  
  if(get_flag(CS00_FLAG)) cs|=1<<0;
  if(get_flag(CS01_FLAG)) cs|=1<<1;
  if(get_flag(CS02_FLAG)) cs|=1<<2;

  switch(cs)
    {
    case 0: return;  
    case 1: prescaler_changed=prescaler_result0&CK1; break;
    case 2: prescaler_changed=prescaler_result0&CK8; break;
    case 3:
      if(ISPIN(T0_PIN)) prescaler_changed=prescaler_result0&CK64;
      else prescaler_changed=prescaler_result0&CK32;
      break;
    case 4:
      if(ISPIN(T0_PIN)) prescaler_changed=prescaler_result0&CK256;
      else prescaler_changed=prescaler_result0&CK64;
      break;
    case 5:
      if(ISPIN(T0_PIN)) prescaler_changed=prescaler_result0&CK1024;
      else prescaler_changed=prescaler_result0&CK128;
      break;        
    case 6:
      if(ISPIN(T0_PIN))
        {
          if(t0_pin&&!get_pin(T0_PIN)) prescaler_changed=1;
          t0_pin=get_pin(T0_PIN);
        }
      else prescaler_changed=prescaler_result0&CK256;
      break;
    case 7:  
      if(ISPIN(T0_PIN))
        {
          if(!t0_pin&&get_pin(T0_PIN)) prescaler_changed=1;
          t0_pin=get_pin(T0_PIN);
        }
      else prescaler_changed=prescaler_result0&CK1024;
      break;
    }
  
  
  if(!ISPORT(OCR0)) goto exit;

  /* get Compare mode */
  if(get_flag(COM00_FLAG)) com|=1<<0;
  if(get_flag(COM01_FLAG)) com|=1<<1;
  
  /* get PWM mode */
  if(ISFLAG(WGM00_FLAG)&&ISFLAG(WGM01_FLAG))
    {
      if(get_flag(WGM00_FLAG)) pwm_mode|=1<<0;
      if(get_flag(WGM01_FLAG)) pwm_mode|=1<<1;
    }
  else
    {
      if(ISFLAG(PWM0_FLAG)) if(get_flag(PWM0_FLAG)) pwm_mode|=1<<0;
      if(ISFLAG(CTC0_FLAG)) if(get_flag(CTC0_FLAG)) pwm_mode|=1<<1;
    }

  port=get_port(TCNT0);
  max=(port==0xFF);
  bottom=(port==0);

  switch(pwm_mode){
  case 2:  /* CTC mode */
    ocr=get_port(OCR0);
    if(prescaler_changed&&port==ocr) {clear_tcnt=1;set_flag(OCF0_FLAG);}
    goto normal_mode;
  case 0:  /* Normal mode */
    ocr=get_port(OCR0);
  normal_mode:
    add=1;
    if(prescaler_changed)
      {
	if(port==ocr) {compare_set_pin(com,OC0_PIN);set_flag(OCF0_FLAG);}
	if(max) set_flag(TOV0_FLAG);
      }
    else
      {
	if(ISFLAG(FOC0_FLAG)) if(get_flag(FOC0_FLAG)) 
	  {clear_flag(FOC0_FLAG);compare_set_pin(com,OC0_PIN);}
      }
    break;
  case 1: /* PWM, Phase correct */
    if(prescaler_changed)
      {
	if(port==ocr)
	  {set_flag(OCF0_FLAG);pwm_compare_set_pin(com,OC0_PIN,add);}
	
	if(max) {add=-1;ocr=get_port(OCR0);}
	if(bottom) {add=1;set_flag(TOV0_FLAG);}
      }
    break;
  case 3:  /* Fast PWM Mode */
    add=1;
    if(prescaler_changed)
      {
	if(bottom) fpwm_compare_top_set_pin(com,OC0_PIN);
	
	if(port==ocr)
	  {set_flag(OCF0_FLAG);fpwm_compare_match_set_pin(com,OC0_PIN);}
	
	if(max) {ocr=get_port(OCR0);set_flag(TOV0_FLAG);}
      }
   }
  
 exit:
  if(prescaler_changed)
    {
      if(clear_tcnt) {set_port(TCNT0,0);clear_tcnt=0;}
      else add_port(TCNT0,add);
    }
}

void prescaler2(void)
{
  prescaler_result2=0;
  
  prescaler_value2++;

  prescaler_result2|=CK1;
  if(prescaler_value2%8) return;
  prescaler_result2|=CK8;

  if(prescaler_value2%32) return;
  prescaler_result2|=CK32;

  if(prescaler_value2%64) return;
  prescaler_result2|=CK64;
  
  if(prescaler_value2%128) return;
  prescaler_result2|=CK128;

  if(prescaler_value2%256) return;  
  prescaler_result2|=CK256;

  if(prescaler_value2%1024) return;
  prescaler_result2|=CK1024;
  prescaler_value2=0;
 
}

void timer_counter2(void)
{
  int cs=0,prescaler_changed=0,com=0,pwm_mode=0,bottom,max,port;
  static int t2_pin=0,clear_tcnt=0,add=1,ocr=0;

  if(!clocks) {t2_pin=clear_tcnt=0;add=1;}

  if(!ISPORT(TCCR2)) return;
  
  if(get_flag(CS20_FLAG)) cs|=1<<0;
  if(get_flag(CS21_FLAG)) cs|=1<<1;
  if(get_flag(CS22_FLAG)) cs|=1<<2;

  switch(cs)
    {
    case 0: return;  
    case 1: prescaler_changed=prescaler_result2&CK1; break;
    case 2: prescaler_changed=prescaler_result2&CK8; break;
    case 3:
      if(ISPIN(T2_PIN)) prescaler_changed=prescaler_result2&CK64;
      else prescaler_changed=prescaler_result2&CK32;
      break;
    case 4:
      if(ISPIN(T2_PIN)) prescaler_changed=prescaler_result2&CK256;
      else prescaler_changed=prescaler_result2&CK64;
      break;
    case 5:
      if(ISPIN(T2_PIN)) prescaler_changed=prescaler_result2&CK1024;
      else prescaler_changed=prescaler_result2&CK128;
      break;        
    case 6:
      if(ISPIN(T2_PIN))
        {
          if(t2_pin&&!get_pin(T2_PIN)) prescaler_changed=1;
          t2_pin=get_pin(T2_PIN);
        }
      else prescaler_changed=prescaler_result2&CK256;
      break;
    case 7:  
      if(ISPIN(T2_PIN))
        {
          if(!t2_pin&&get_pin(T2_PIN)) prescaler_changed=1;
          t2_pin=get_pin(T2_PIN);
        }
      else prescaler_changed=prescaler_result2&CK1024;
      break;
    }
  
  
  if(!ISPORT(OCR2)) goto exit;

  /* get Compare mode */
  if(get_flag(COM20_FLAG)) com|=1<<0;
  if(get_flag(COM21_FLAG)) com|=1<<1;
  
  /* get PWM mode */
  if(ISFLAG(WGM20_FLAG)&&ISFLAG(WGM21_FLAG))
    {
      if(get_flag(WGM20_FLAG)) pwm_mode|=1<<0;
      if(get_flag(WGM21_FLAG)) pwm_mode|=1<<1;
    }
  else
    {
      if(ISFLAG(PWM2_FLAG)) if(get_flag(PWM2_FLAG)) pwm_mode|=1<<0;
      if(ISFLAG(CTC2_FLAG)) if(get_flag(CTC2_FLAG)) pwm_mode|=1<<1;
    }

  port=get_port(TCNT2);
  max=(port==0xFF);
  bottom=(port==0);

  switch(pwm_mode){
  case 2:  /* CTC mode */
    ocr=get_port(OCR2);
    if(prescaler_changed&&port==ocr) {clear_tcnt=1;set_flag(OCF2_FLAG);}
    goto normal_mode;
  case 0:  /* Normal mode */
    ocr=get_port(OCR2);
  normal_mode:
    add=1;
    if(prescaler_changed)
      {
	if(port==ocr) {compare_set_pin(com,OC2_PIN);set_flag(OCF2_FLAG);}
	if(max) set_flag(TOV2_FLAG);
      }
    else
      {
	if(ISFLAG(FOC2_FLAG)) if(get_flag(FOC2_FLAG)) 
	  {clear_flag(FOC2_FLAG);compare_set_pin(com,OC2_PIN);}
      }
    break;
  case 1: /* PWM, Phase correct */
    if(prescaler_changed)
      {
	if(port==ocr)
	  {set_flag(OCF2_FLAG);pwm_compare_set_pin(com,OC2_PIN,add);}
	
	if(max) {add=-1;ocr=get_port(OCR2);}
	if(bottom) {add=1;set_flag(TOV2_FLAG);}
      }
    break;
  case 3:  /* Fast PWM Mode */
    add=1;
    if(prescaler_changed)
      {
	if(bottom) fpwm_compare_top_set_pin(com,OC2_PIN);
	
	if(port==ocr)
	  {set_flag(OCF2_FLAG);fpwm_compare_match_set_pin(com,OC2_PIN);}
	
	if(max) {ocr=get_port(OCR2);set_flag(TOV2_FLAG);}
      }
   }
  
 exit:
  if(prescaler_changed)
    {
      if(clear_tcnt) {set_port(TCNT2,0);clear_tcnt=0;}
      else add_port(TCNT2,add);
    }
}


void prescaler1(void)
{
  prescaler_result1=0;
  prescaler_value1++;

  prescaler_result1|=CK1;
  if(prescaler_value1%8) return;
  prescaler_result1|=CK8;

  if(prescaler_value1%32) return;
  prescaler_result1|=CK32;

  if(prescaler_value1%64) return;
  prescaler_result1|=CK64;
  
  if(prescaler_value1%128) return;
  prescaler_result1|=CK128;

  if(prescaler_value1%256) return;  
  prescaler_result1|=CK256;

  if(prescaler_value1%1024) return;
  prescaler_result1|=CK1024;
  prescaler_value1=0;
 
}

void timer_counter1(void)
{
  int cs=0,prescaler_changed=0,coma=0,comb=0,comc=0,pwm_mode=0,
    bottom,top,max,port;
  static int t1_pin=0,clear_tcnt=0,add=1;
  static uint16_t ocr1a,ocr1b,ocr1c;

  if(!clocks) {t1_pin=clear_tcnt=0;add=1;}

  if(!ISPORT(TCCR1A)) return;
  
  if(get_flag(CS10_FLAG)) cs|=1<<0;
  if(get_flag(CS11_FLAG)) cs|=1<<1;
  if(get_flag(CS12_FLAG)) cs|=1<<2;

  switch(cs)
    {
    case 0: return;  
    case 1: prescaler_changed=prescaler_result1&CK1; break;
    case 2: prescaler_changed=prescaler_result1&CK8; break;
    case 3: prescaler_changed=prescaler_result1&CK64;break;
    case 4: prescaler_changed=prescaler_result1&CK256;break;
    case 5: prescaler_changed=prescaler_result1&CK1024;break;        
    case 6:
      if(ISPIN(T1_PIN))
        {
          if(t1_pin&&!get_pin(T1_PIN)) prescaler_changed=1;
          t1_pin=get_pin(T1_PIN);
        }
      break;
    case 7:  
      if(ISPIN(T1_PIN))
        {
          if(!t1_pin&&get_pin(T1_PIN)) prescaler_changed=1;
          t1_pin=get_pin(T1_PIN);
        }
      break;
    }
  
  port=get_port16(TCNT1L,TCNT1H);
  max=(port==0xFFFF);
  bottom=(port==0);
  
  /* get PWM mode */
  if(ISFLAG(WGM10_FLAG)) if(get_flag(WGM10_FLAG)) pwm_mode|=1<<0;
  if(ISFLAG(WGM11_FLAG)) if(get_flag(WGM11_FLAG)) pwm_mode|=1<<1;
  if(ISFLAG(WGM12_FLAG)) if(get_flag(WGM12_FLAG)) pwm_mode|=1<<2;
  if(ISFLAG(WGM13_FLAG)) if(get_flag(WGM13_FLAG)) pwm_mode|=1<<3;

  if(ISFLAG(PWM10_FLAG)) if(get_flag(PWM10_FLAG)) pwm_mode|=1<<0;
  if(ISFLAG(PWM11_FLAG)) if(get_flag(PWM11_FLAG)) pwm_mode|=1<<1;
  if(ISFLAG(CTC1_FLAG))  if(get_flag(CTC1_FLAG)&&!pwm_mode) pwm_mode|=1<<2;

  /* get Compare Mode */
  if(ISFLAG(COM1A0_FLAG)) if(get_flag(COM1A0_FLAG)) coma|=1<<0;
  if(ISFLAG(COM1A1_FLAG)) if(get_flag(COM1A1_FLAG)) coma|=1<<1;

  if(ISFLAG(COM1B0_FLAG)) if(get_flag(COM1B0_FLAG)) comb|=1<<0;
  if(ISFLAG(COM1B1_FLAG)) if(get_flag(COM1B1_FLAG)) comb|=1<<1;

  if(ISFLAG(COM1C0_FLAG)) if(get_flag(COM1C0_FLAG)) comc|=1<<0;
  if(ISFLAG(COM1C1_FLAG)) if(get_flag(COM1C1_FLAG)) comc|=1<<1;

  switch(pwm_mode){
  case 4:  /* CTC, OCR1A */
    set_icr1();
    if(prescaler_changed) 
      if(port==get_port16(OCR1AL,OCR1AH)) {clear_tcnt=1;set_flag(OCF1A_FLAG);}
    goto normal_mode;
  case 12: /* CTC, ICR1 */
    if(prescaler_changed)
      if(port==get_port16(ICR1L,ICR1H)) {clear_tcnt=1;set_flag(ICF1_FLAG);}
    goto normal_mode;
  case 0: /* Normal mode */
    set_icr1();
  normal_mode:
    add=1;
    /* Updating Compare Registers */
    ocr1a=get_port16(OCR1AL,OCR1AH);
    if(ISPORT(OCR1BL)) ocr1b=get_port16(OCR1BL,OCR1BH);
    if(ISPORT(OCR1CL)) ocr1c=get_port16(OCR1CL,OCR1CH);
    
    if(prescaler_changed)
      {
	if(max) set_flag(TOV1_FLAG);
	
	/* Compare A*/
	if(port==ocr1a)
	  {set_flag(OCF1A_FLAG);compare_set_pin(coma,OC1A_PIN);}
	
	/* Compare B */
	if(ISPORT(OCR1BL))
	  if(port==ocr1b)
	    {set_flag(OCF1B_FLAG);compare_set_pin(comb,OC1B_PIN);}
	
	/* Compare C */
	if(ISPORT(OCR1CL))
	  if(port==ocr1c)
	    {set_flag(OCF1C_FLAG);compare_set_pin(comc,OC1C_PIN);}
	
      }
    else
      {
	if(ISFLAG(FOC1A_FLAG)) if(get_flag(FOC1A_FLAG)) 
	  {clear_flag(FOC1A_FLAG);compare_set_pin(coma,OC1A_PIN);}
	if(ISFLAG(FOC1B_FLAG)) if(get_flag(FOC1B_FLAG)) 
	  {clear_flag(FOC1B_FLAG);compare_set_pin(comb,OC1B_PIN);}
	if(ISFLAG(FOC1C_FLAG)) if(get_flag(FOC1C_FLAG)) 
	    {clear_flag(FOC1C_FLAG);compare_set_pin(comc,OC1C_PIN);}
      }
    break;

  case 10: /* PWM, Phase Correct, ICR */
    if(!prescaler_changed) break;
    if((top=(port==get_port16(ICR1L,ICR1H)))) set_flag(ICF1_FLAG);
    goto phase_correct_2;

  case 11: /* PWM, Phase Correct, OCR */
    set_icr1();
    if(!prescaler_changed) break;
    if((top=(port==ocr1a))) set_flag(OCF1A_FLAG);
  phase_correct_2:
   
    /* Compare A */
    if(port==ocr1a)
      if(coma==1) 
	{if(get_pin(OC1A_PIN)) clear_pin(OC1A_PIN); else set_pin(OC1A_PIN);}
    goto phase_correct;

  case 1: /* PWM, Phase Correct, 8-bit */
    set_icr1();
    if(!prescaler_changed) break;
    top=(port==0xFF);
    goto phase_correct;

  case 2: /* PWM, Phase Correct, 9-bit */
    set_icr1();
    if(!prescaler_changed) break;
    top=(port==0x1FF);
    goto phase_correct;

  case 3: /* PWM, Phase Correct, 10-bit */
    set_icr1();
    if(!prescaler_changed) break;
    top=(port==0x3FF);

    phase_correct:
    if(bottom) {set_flag(TOV1_FLAG);add=1;}
    if(top) 
      {
	add=-1;
	/* Updating Compare Registers */
	ocr1a=get_port16(OCR1AL,OCR1AH);
	if(ISPORT(OCR1BL)) ocr1b=get_port16(OCR1BL,OCR1BH);
	if(ISPORT(OCR1CL)) ocr1c=get_port16(OCR1CL,OCR1CH);
      }
    
    /* Compare A*/
    if(port==ocr1a) 
      {set_flag(OCF1A_FLAG);pwm_compare_set_pin(coma,OC1A_PIN,add);}
    
    /* Compare B */
    if(ISPORT(OCR1BL))
      if(port==ocr1b)
      {set_flag(OCF1B_FLAG);pwm_compare_set_pin(comb,OC1B_PIN,add);}
    
    /* Compare C */
    if(ISPORT(OCR1CL))
      if(port==ocr1c)
	{set_flag(OCF1C_FLAG);pwm_compare_set_pin(comc,OC1C_PIN,add);}
    break;

  case 5: /* Fast PWM, 8-bit */
    set_icr1();
    if(!prescaler_changed) break;
    top=(port==0xFF);
    goto fpwm_mode;

  case 6: /* Fast PWM, 9-bit */
    set_icr1();
    if(!prescaler_changed) break;
    top=(port==0x1FF);
    goto fpwm_mode;

  case 7: /* Fast PWM, 10-bit */
    set_icr1();
    if(!prescaler_changed) break;
    top=(port==0x3FF);

  fpwm_mode:
    if(top)
      {
	set_flag(TOV1_FLAG);
	fpwm_compare_top_set_pin(coma,OC1A_PIN);
	fpwm_compare_top_set_pin(comb,OC1B_PIN);
	fpwm_compare_top_set_pin(comc,OC1C_PIN);
	ocr1a=get_port16(OCR1AL,OCR1AH);
	if(ISPORT(OCR1BL)) ocr1b=get_port16(OCR1BL,OCR1BH);
	if(ISPORT(OCR1CL)) ocr1c=get_port16(OCR1CL,OCR1CH);
	clear_tcnt=1;
      }

    /* Compare A*/
    if(port==ocr1a)
      {set_flag(OCF1A_FLAG);fpwm_compare_match_set_pin(coma,OC1A_PIN);}
    
    /* Compare B */
    if(ISPORT(OCR1BL))
      if(port==ocr1b)
      {set_flag(OCF1B_FLAG);fpwm_compare_match_set_pin(comb,OC1B_PIN);}
    
    /* Compare C */
    if(ISPORT(OCR1CL))
      if(port==ocr1c)
	{set_flag(OCF1C_FLAG);fpwm_compare_match_set_pin(comc,OC1C_PIN);}
    break;

  case 8: /* PWM, Phase and Frequency Correct, ICR */
    if(!prescaler_changed) break;
    if((top=(port==get_port16(ICR1L,ICR1H)))) set_flag(ICF1_FLAG);
    goto phase_fre_correct;
  case 9: /* PWM, Phase and Frequency Correct, OCR */
    set_icr1();
    if(!prescaler_changed) break;
    if((top=(port==ocr1a))) set_flag(OCF1A_FLAG);
    phase_fre_correct: 
    if(top) add=-1;
    if(bottom) 
      {
	add=1;set_flag(TOV1_FLAG);
	ocr1a=get_port16(OCR1AL,OCR1AH);
	if(ISPORT(OCR1BL)) ocr1b=get_port16(OCR1BL,OCR1BH);
	if(ISPORT(OCR1CL)) ocr1c=get_port16(OCR1CL,OCR1CH);
      }

    /* Compare A */
    if(port==ocr1a)
      {
	set_flag(OCF1A_FLAG);
	if(coma==1) 
	  {if(get_pin(OC1A_PIN)) clear_pin(OC1A_PIN); else set_pin(OC1A_PIN);}
	pwm_compare_set_pin(coma,OC1A_PIN,add);
      }

    /* Compare B */
    if(ISPORT(OCR1BL))
      if(port==ocr1b)
      {set_flag(OCF1B_FLAG);pwm_compare_set_pin(comb,OC1B_PIN,add);}
    
    /* Compare C */
    if(ISPORT(OCR1CL))
      if(port==ocr1c)
	{set_flag(OCF1C_FLAG);pwm_compare_set_pin(comc,OC1C_PIN,add);}
    break;

  case 13: break;
  case 14: /* Fast PWM, ICR */
    if(!prescaler_changed) break;
    if((top=(port==get_port16(ICR1L,ICR1H)))) set_flag(ICF1_FLAG);
    goto fpwm_mode;
  case 15: /* Fast PWM, OCR */
    set_icr1();
    if(!prescaler_changed) break;  
    if((top=(port==ocr1a))) set_flag(OCF1A_FLAG);
    
    /* Compare A */
    if(port==ocr1a)
      if(coma==1) 
	{if(get_pin(OC1A_PIN)) clear_pin(OC1A_PIN); else set_pin(OC1A_PIN);}
    goto fpwm_mode;
  }

 if(prescaler_changed) 
    {
      if(clear_tcnt) {set_port16(TCNT1L,TCNT1H,0);clear_tcnt=0;} 
      else port=add_port16(TCNT1L,TCNT1H,add);
    }
}

void prescaler3(void)
{
  prescaler_result3=0;
  prescaler_value3++;

  prescaler_result3|=CK1;
  if(prescaler_value3%8) return;
  prescaler_result3|=CK8;

  if(prescaler_value3%32) return;
  prescaler_result3|=CK32;

  if(prescaler_value3%64) return;
  prescaler_result3|=CK64;
  
  if(prescaler_value3%128) return;
  prescaler_result3|=CK128;

  if(prescaler_value1%256) return;  
  prescaler_result3|=CK256;

  if(prescaler_value3%1024) return;
  prescaler_result3|=CK1024;
  prescaler_value3=0;
 
}

void timer_counter3(void)
{
  int cs=0,prescaler_changed=0,coma=0,comb=0,comc=0,pwm_mode=0,
    bottom,top,max,port;
  static int t3_pin=0,clear_tcnt=0,add=1,max_value=0xFFFF;
  static uint16_t ocr3a,ocr3b,ocr3c;

  if(!clocks) {t3_pin=clear_tcnt=0;add=1;}

  if(!ISPORT(TCCR3A)) return;
  
  if(get_flag(CS30_FLAG)) cs|=1<<0;
  if(get_flag(CS31_FLAG)) cs|=1<<1;
  if(get_flag(CS32_FLAG)) cs|=1<<2;

  switch(cs)
    {
    case 0: return;  
    case 1: prescaler_changed=prescaler_result3&CK1; break;
    case 2: prescaler_changed=prescaler_result3&CK8; break;
    case 3: prescaler_changed=prescaler_result3&CK64;break;
    case 4: prescaler_changed=prescaler_result3&CK256;break;
    case 5: prescaler_changed=prescaler_result3&CK1024;break;        
    case 6:
      if(ISPIN(T3_PIN))
        {
          if(t3_pin&&!get_pin(T3_PIN)) prescaler_changed=1;
          t3_pin=get_pin(T3_PIN);
        }
      break;
    case 7:  
      if(ISPIN(T3_PIN))
        {
          if(!t3_pin&&get_pin(T3_PIN)) prescaler_changed=1;
          t3_pin=get_pin(T3_PIN);
        }
      break;
    }
  
  if(clear_tcnt) {set_port16(TCNT3L,TCNT3H,0);clear_tcnt=0;} 

  port=get_port16(TCNT3L,TCNT3H);
  if(prescaler_changed) 
    {
      if(port==max_value) {set_port16(TCNT3L,TCNT3H,0);port=0;}
      else port=add_port16(TCNT3L,TCNT3H,add);
    }
  max=(port==0xFFFF);
  bottom=(port==0);
  
  /* get PWM mode */
  if(ISFLAG(WGM30_FLAG)) if(get_flag(WGM30_FLAG)) pwm_mode|=1<<0;
  if(ISFLAG(WGM31_FLAG)) if(get_flag(WGM31_FLAG)) pwm_mode|=1<<1;
  if(ISFLAG(WGM32_FLAG)) if(get_flag(WGM32_FLAG)) pwm_mode|=1<<2;
  if(ISFLAG(WGM33_FLAG)) if(get_flag(WGM33_FLAG)) pwm_mode|=1<<3;

  /* get Compare Mode */
  if(ISFLAG(COM3A0_FLAG)) if(get_flag(COM3A0_FLAG)) coma|=1<<0;
  if(ISFLAG(COM3A1_FLAG)) if(get_flag(COM3A1_FLAG)) coma|=1<<1;

  if(ISFLAG(COM3B0_FLAG)) if(get_flag(COM3B0_FLAG)) comb|=1<<0;
  if(ISFLAG(COM3B1_FLAG)) if(get_flag(COM3B1_FLAG)) comb|=1<<1;

  if(ISFLAG(COM3C0_FLAG)) if(get_flag(COM3C0_FLAG)) comc|=1<<0;
  if(ISFLAG(COM3C1_FLAG)) if(get_flag(COM3C1_FLAG)) comc|=1<<1;

  switch(pwm_mode){
  case 4:  /* CTC, OCR1A */
    set_icr3();
    if(prescaler_changed) 
      if(port==get_port16(OCR3AL,OCR3AH)) {clear_tcnt=1;set_flag(OCF3A_FLAG);}
    goto normal_mode;
  case 12: /* CTC, ICR1 */
    if(prescaler_changed)
      if(port==get_port16(ICR3L,ICR3H)) {clear_tcnt=1;set_flag(ICF3_FLAG);}
    goto normal_mode;
  case 0: /* Normal mode */
    set_icr3();
  normal_mode:
    add=1;
    max_value=0xFFFF;
    /* Updating Compare Registers */
    ocr3a=get_port16(OCR3AL,OCR3AH);
    if(ISPORT(OCR3BL)) ocr3b=get_port16(OCR3BL,OCR3BH);
    if(ISPORT(OCR3CL)) ocr3c=get_port16(OCR3CL,OCR3CH);
    
    if(prescaler_changed)
      {
	if(bottom) set_flag(TOV3_FLAG);
	
	/* Compare A*/
	if(port==ocr3a)
	  {set_flag(OCF3A_FLAG);compare_set_pin(coma,OC3A_PIN);}
	
	/* Compare B */
	if(ISPORT(OCR3BL))
	  if(port==ocr3b)
	    {set_flag(OCF3B_FLAG);compare_set_pin(comb,OC3B_PIN);}
	
	/* Compare C */
	if(ISPORT(OCR3CL))
	  if(port==ocr3c)
	    {set_flag(OCF3C_FLAG);compare_set_pin(comc,OC3C_PIN);}
	
      }
    else
      {
	if(ISFLAG(FOC3A_FLAG)) if(get_flag(FOC3A_FLAG)) 
	  {clear_flag(FOC3A_FLAG);compare_set_pin(coma,OC3A_PIN);}
	if(ISFLAG(FOC3B_FLAG)) if(get_flag(FOC3B_FLAG)) 
	  {clear_flag(FOC3B_FLAG);compare_set_pin(comb,OC3B_PIN);}
	if(ISFLAG(FOC3C_FLAG)) if(get_flag(FOC3C_FLAG)) 
	    {clear_flag(FOC3C_FLAG);compare_set_pin(comc,OC3C_PIN);}
      }
    break;

  case 10: /* PWM, Phase Correct, ICR */
    max_value=0xFFFF;
    if(!prescaler_changed) break;

    if((top=(port==get_port16(ICR3L,ICR3H)))) set_flag(ICF3_FLAG);
    goto phase_correct_2;
  case 11: /* PWM, Phase Correct, OCR */
    max_value=0xFFFF;
    set_icr3();
    if(!prescaler_changed) break;

    if((top=(port==ocr3a))) set_flag(OCF3A_FLAG);
  phase_correct_2:
   
    /* Compare A */
    if(port==ocr3a)
      if(coma==1) 
	{if(get_pin(OC3A_PIN)) clear_pin(OC3A_PIN); else set_pin(OC3A_PIN);}
    goto phase_correct;
    break;
  case 1: /* PWM, Phase Correct, 8-bit */
    max_value=0xFFFF;
    set_icr3();
    if(!prescaler_changed) break;

    top=(port==0xFF);
    goto phase_correct;
  case 2: /* PWM, Phase Correct, 9-bit */
    set_icr3();
    max_value=0xFFFF;
    if(!prescaler_changed) break;

    top=(port==0x1FF);
    goto phase_correct;
  case 3: /* PWM, Phase Correct, 10-bit */
    set_icr3();
    max_value=0xFFFF;
    if(!prescaler_changed) break;

    top=(port==0x3FF);
    phase_correct:
    if(bottom) {set_flag(TOV3_FLAG);add=1;}
    if(top) 
      {
	add=-1;
	/* Updating Compare Registers */
	ocr3a=get_port16(OCR3AL,OCR3AH);
	if(ISPORT(OCR3BL)) ocr3b=get_port16(OCR3BL,OCR3BH);
	if(ISPORT(OCR3CL)) ocr3c=get_port16(OCR3CL,OCR3CH);
      }
    
    /* Compare A*/
    if(port==ocr3a) 
      {set_flag(OCF3A_FLAG);pwm_compare_set_pin(coma,OC3A_PIN,add);}
    
    /* Compare B */
    if(ISPORT(OCR3BL))
      if(port==ocr3b)
      {set_flag(OCF3B_FLAG);pwm_compare_set_pin(comb,OC3B_PIN,add);}
    
    /* Compare C */
    if(ISPORT(OCR3CL))
      if(port==ocr3c)
	{set_flag(OCF3C_FLAG);pwm_compare_set_pin(comc,OC3C_PIN,add);}
    break;
  case 5: /* Fast PWM, 8-bit */
    max_value=0xFF;
    set_icr3();
    if(!prescaler_changed) break;

    top=(port==max_value);
    goto fpwm_mode;
  case 6: /* Fast PWM, 9-bit */
    max_value=0x1FF;
    set_icr3();
    if(!prescaler_changed) break;

    top=(port==max_value);
    goto fpwm_mode;
  case 7: /* Fast PWM, 10-bit */
    max_value=0x3FF;
    set_icr3();
    if(!prescaler_changed) break;

    top=(port==max_value);
  fpwm_mode:
    if(top)
      {
	set_flag(TOV3_FLAG);
	fpwm_compare_top_set_pin(coma,OC3A_PIN);
	fpwm_compare_top_set_pin(comb,OC3B_PIN);
	fpwm_compare_top_set_pin(comc,OC3C_PIN);
	ocr3a=get_port16(OCR3AL,OCR3AH);
	if(ISPORT(OCR3BL)) ocr3b=get_port16(OCR3BL,OCR3BH);
	if(ISPORT(OCR3CL)) ocr3c=get_port16(OCR3CL,OCR3CH);
      }

    /* Compare A*/
    if(port==ocr3a)
      {set_flag(OCF3A_FLAG);fpwm_compare_match_set_pin(coma,OC3A_PIN);}
    
    /* Compare B */
    if(ISPORT(OCR3BL))
      if(port==ocr3b)
      {set_flag(OCF3B_FLAG);fpwm_compare_match_set_pin(comb,OC3B_PIN);}
    
    /* Compare C */
    if(ISPORT(OCR3CL))
      if(port==ocr3c)
	{set_flag(OCF3C_FLAG);fpwm_compare_match_set_pin(comc,OC3C_PIN);}
    break;

  case 8: /* PWM, Phase and Frequency Correct, ICR */
    if(!prescaler_changed) break;
    if((top=(port==get_port16(ICR3L,ICR3H)))) set_flag(ICF3_FLAG);
    goto phase_fre_correct;
  case 9: /* PWM, Phase and Frequency Correct, OCR */
    set_icr3();
    if(!prescaler_changed) break;

    if((top=(port==ocr3a))) set_flag(OCF3A_FLAG);
    phase_fre_correct: 
    if(top) add=-1;
    if(bottom) 
      {
	add=1;set_flag(TOV3_FLAG);
	ocr3a=get_port16(OCR3AL,OCR3AH);
	if(ISPORT(OCR3BL)) ocr3b=get_port16(OCR3BL,OCR3BH);
	if(ISPORT(OCR3CL)) ocr3c=get_port16(OCR3CL,OCR3CH);
      }

    /* Compare A */
    if(port==ocr3a)
      {
	set_flag(OCF3A_FLAG);
	if(coma==1) 
	  {if(get_pin(OC3A_PIN)) clear_pin(OC3A_PIN); else set_pin(OC3A_PIN);}
	pwm_compare_set_pin(coma,OC3A_PIN,add);
      }

    /* Compare B */
    if(ISPORT(OCR3BL))
      if(port==ocr3b)
      {set_flag(OCF3B_FLAG);pwm_compare_set_pin(comb,OC3B_PIN,add);}
    
    /* Compare C */
    if(ISPORT(OCR3CL))
      if(port==ocr3c)
	{set_flag(OCF3C_FLAG);pwm_compare_set_pin(comc,OC3C_PIN,add);}
    break;

  case 13: break;
  case 14: /* Fast PWM, ICR */
    max_value=get_port16(ICR3L,ICR3H);
    if(!prescaler_changed) break;
    if((top=(port==max_value))) set_flag(ICF3_FLAG);
    goto fpwm_mode;
  case 15: /* Fast PWM, OCR */
    max_value=ocr3a;
    set_icr3();
    if(!prescaler_changed) break;
  
    if((top=(port==max_value))) set_flag(OCF3A_FLAG);
    
    /* Compare A */
    if(port==ocr3a)
      if(coma==1) 
	{if(get_pin(OC3A_PIN)) clear_pin(OC3A_PIN); else set_pin(OC3A_PIN);}
    goto fpwm_mode;
  }
}

static void compare_set_pin(int mode,int pin)
{
  switch(mode)
    { 
    case 1:                /* Toggle output pin */ 
      if(get_pin(pin)) clear_pin(pin);
      else set_pin(pin);
      break;
    case 2: clear_pin(pin);break;  /* Clear output pin */
    case 3: set_pin(pin);break;    /* Set output pin */
    }
}

static void pwm_compare_set_pin(int mode,int pin,int add)
{
  switch(mode)
    {
    case 2:
      if(add==1) clear_pin(pin);
      else set_pin(pin);
      break;
    case 3:
      if(add==-1) clear_pin(pin);
      else set_pin(pin);
      break;
    }
}

static void fpwm_compare_match_set_pin(int mode,int pin)
{
  switch(mode){
  case 2: clear_pin(pin); break;
  case 3: set_pin(pin);  
  }
}

static void fpwm_compare_top_set_pin(int mode,int pin)
{
  switch(mode){
  case 2: set_pin(pin); break;
  case 3: clear_pin(pin);  
  }
}

static void set_icr1(void)
{
  int pin,capture=0;
  static int old_pin=0,count;

  if(ISFLAG(ACIC_FLAG)&&get_flag(ACIC_FLAG)) pin=get_flag(ACO_FLAG);
  else pin=get_pin(IC1_PIN);
  
  if(ISFLAG(ICNC1_FLAG)&&get_flag(ICNC1_FLAG))
    { /* Noise Canceler is enabled */
      if(old_pin!=pin) count=0;
      if(get_flag(ICES1_FLAG)) {if(!old_pin&&pin) count=1;}
      else {if(old_pin&&!pin) count=1;}
      if(old_pin==pin&&count) if(count++==4) {capture=1;count=0;}
    }
  else
    {
      if(get_flag(ICES1_FLAG)) {if(!old_pin&&pin) capture=1;}
      else {if(old_pin&&!pin) capture=1;}
    }
  
  if(capture)
    {
      set_port16(ICR1L,ICR1H,get_port16(TCNT1L,TCNT1H));
      set_flag(ICF1_FLAG);
    }
  old_pin=pin;
}

static void set_icr3(void)
{
  int pin,capture=0;
  static int old_pin=0,count;

  pin=get_pin(IC3_PIN);
  
  if(ISFLAG(ICNC3_FLAG)&&get_flag(ICNC3_FLAG))
    { /* Noise Canceler is enabled */
      if(old_pin!=pin) count=0;
      if(get_flag(ICES3_FLAG)) {if(!old_pin&&pin) count=1;}
      else {if(old_pin&&!pin) count=1;}
      if(old_pin==pin&&count) if(count++==4) {capture=1;count=0;}
    }
  else
    {
      if(get_flag(ICES3_FLAG)) {if(!old_pin&&pin) capture=1;}
      else {if(old_pin&&!pin) capture=1;}
    }
  
  if(capture)
    {
      set_port16(ICR3L,ICR3H,get_port16(TCNT3L,TCNT3H));
      set_flag(ICF3_FLAG);
    }
  old_pin=pin;
}

void analog_comparator(void)
{
  int mode=0,mux_mode=0,pin;
  static int aco_old=0,aco;

  if(!ISPORT(ACSR)) return;

  if(get_flag(ACD_FLAG)) return;

  if(ISFLAG(ACME_FLAG)&&get_flag(ACME_FLAG)&&!get_flag(ADEN_FLAG))
    {
      if(get_flag(MUX0_FLAG)) mux_mode|=1<<0;
      if(get_flag(MUX1_FLAG)) mux_mode|=1<<1;
      if(get_flag(MUX2_FLAG)) mux_mode|=1<<2;
      
      switch(mux_mode){
      case 0: pin=ADC0_PIN;break;
      case 1: pin=ADC1_PIN;break;
      case 2: pin=ADC2_PIN;break;
      case 3: pin=ADC3_PIN;break;
      case 4: pin=ADC4_PIN;break;
      case 5: pin=ADC5_PIN;break;
      case 6: pin=ADC6_PIN;break;
      case 7: pin=ADC7_PIN;break;
      }
      
      if(get_pin(AIN0_PIN)&&!get_pin(pin)) {aco=1;set_flag(ACO_FLAG);}
      else {clear_flag(ACO_FLAG);aco=0;}
    }
  else
    {
      if(get_pin(AIN0_PIN)&&!get_pin(AIN1_PIN)) {aco=1;set_flag(ACO_FLAG);}
      else {clear_flag(ACO_FLAG);aco=0;}
    }

  /* get mode */
  if(get_flag(ACIS0_FLAG)) mode|=1<<0;
  if(get_flag(ACIS1_FLAG)) mode|=1<<1;

  switch(mode){
  case 0: 
    if(aco_old!=aco) set_flag(ACI_FLAG);
    break;
  case 2:
    if(aco_old&&!aco) set_flag(ACI_FLAG);
    break;
  case 3:
    if(!aco_old&&aco) set_flag(ACI_FLAG);
    break;
  }
  aco_old=aco;
}

void save_port(int portr,int ddr,int pinr,FILE *file)
{
  uint8_t pin_value;
  char str_c[10],value_hex[5];

  if(ISPORT(portr)) return;
  
  pin_value=get_port(portr)&get_port(ddr);
  set_port(pinr,pin_value);
  
  if(file==NULL) return;

  sprintf(str_c,"%d",clocks); 
  fill_zero(str_c,8);
  sprintf(value_hex,"%X",pin_value);
  fill_zero(value_hex,2);
  fprintf(file,"%s:0x%s\n",str_c,value_hex);
  fflush(file); 
  
}

void save_ports(void)
{
  int n;
  static FILE * files[6]={NULL};

  if(clocks==0)  /* init */
    {  
      for(n=0;n<6;n++)
	{
	  if(files[n]!=NULL) fclose(files[n]);      
	  files[n]=fopen(data_port_log[n].filename,"w");
	}
    }
  
  save_port(PORTA,DDRA,PINA,files[0]);
}

/*  void watchdog_timer(void) */
/*  { */
/*    static int wdtoe; */
/*    int wdtcr,flag; */

/*    if(index_ports[WDTCR]==-1) return; */
  
/*    wdtcr=io_port[index_ports[WDTCR]].address; */

/*    if(((int)((double)wtd_freq*clocks/frequency)%(2048*1024))!= */
/*       ((int)((double)wtd_freq*(clocks-1)/frequency)%(2048*1024))) */
/*      watchdog_prescaler++; */
  
  /*  printf("WatchDog: %d\n",watchdog_prescaler); */
/* if((io_pointer[wdtcr]&0x08)==0) return;*/ /* Watchdog isn't running */
  
  /*  if(io_port[index_ports[WDTCR]].write_mask!=0x0F&& */
/*       !(io_pointer[wdtcr]&0x10)&&flag==1) */
      /* watchdog is running */ 
/*      {if(clocks_wdt>watchdog_prescaler) watchdog_prescaler++;} */
/*    else */
      /* watchdog is stopped */ 
/*      wdtoe=0; */
    
  
/*    if(io_pointer[wdtcr]&0x10)*/  /* attempt to stop watchdog */
/*      { */
/*        if(wdtoe>4) */
/*  	{ */
/*  	*/  /* clear flag 'wdtoe' after 4 cycles */ 
/*  	  io_pointer[wdtcr]=io_pointer[wdtcr]&0xEF; */
/*  	  wdtoe=controllers[type_micro].size_gpr; */
/*  	  mem_file_changed[(wdtcr+wdtoe)/8]|=1<<(wdtcr+wdtoe)%8; */
/*  	  wdtoe=0; */
/*  	} */
/*        wdtoe++; */
/*      } */
  
/*    if(!watchdog_prescaler) return; */
/*    flag=0; */
/*  */  /* reset condition */ 
/*    switch(io_pointer[wdtcr]&0x7) */
/*      { */
/*      case 0: */
/*        if((watchdog_prescaler%(16*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 1: */
/*        if((watchdog_prescaler%(32*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 2: */
/*        if((watchdog_prescaler%(64*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 3: */
/*        if((watchdog_prescaler%(128*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 4: */
/*        if((watchdog_prescaler%(256*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 5: */
/*        if((watchdog_prescaler%(512*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 6: */
/*        if((watchdog_prescaler%(1024*1024))==0) */
/*  	flag=1; */
/*        break; */
/*      case 7: */
/*        if((watchdog_prescaler%(2048*1024))==0) */
/*  	watchdog_prescaler=0; */
/*  	flag=1; */
/*        break; */
/*      } */

/*    if(flag) */
/*      { */
/*        update_main(); */
/*        show_info(_("WatchDog Reset")); */
/*        pthread_kill(thread,SIGSTOP); */
/*        if(wacthdog_timeout==UNPROGRAMMED) */
/*  	{ */
/*  	  clocks+=(int)(((double)frequency*16*1024)/wtd_freq); */
/*  	  watchdog_prescaler+=16*1024+1; */
/*  	} */
/*        else */
/*  	{ */
/*  	  clocks+=(int)(((double)frequency*1024)/wtd_freq); */
/*  	  watchdog_prescaler+=1024; */
/*  	} */
/*        watchdog_prescaler=pc=0; */
/*      } */
/*  } */

/*  void eeprom_access(void) */
/*  { */
/*    static int attempt,time=-1; */
/*    int eedr,eecr,eecr_mask; */
/*    Word eear; */
  
/*    eear.word=0; */
/*    eear.byte[0]=io_pointer[io_port[index_ports[EEARL]].address]; */
/*    if(index_ports[EEARH]!=-1) */
/*      { */
/*        eear.byte[1]=io_pointer[io_port[index_ports[EEARH]].address]; */
/*      } */
/*    eecr=io_port[index_ports[EECR]].address; */
/*    eecr_mask=io_port[index_ports[EECR]].write_mask; */
/*    eedr=io_port[index_ports[EEDR]].address; */

/*    if((io_pointer[eecr]&0x03)==1) */
/*    */  /* read EEPROM data */ 
/*      { */
/*        if((type_micro>1&&type_micro<6)||type_micro>7) */
/*  	clocks+=4; */
/*        else*/ /* operation of reading EEPROM halts CPU for two/four cycles */ 
/*  	clocks+=2;  */
/*        io_pointer[eecr]=io_pointer[eecr]&0xFE; */
/*        if(eear.word<controllers[type_micro].size_eeprom) */
/*  	io_pointer[eedr]=eeprom_pointer[eear.word]; */
/*        else */
/*  	{puts("EEPROM address more then its size.");} */
/*        eecr+=controllers[type_micro].size_gpr; */
/*        mem_file_changed[eecr/8]|=1<<eecr%8; */
/*        eedr+=controllers[type_micro].size_gpr; */
/*        mem_file_changed[eedr/8]|=1<<eedr%8; */
/*        return; */
/*      } */

/*    if((io_pointer[eecr]&0x2)==0) {time=-1;}; */

/*    if(eecr_mask==0x03&&(io_pointer[eecr]&0x03)==0x02&&time==-1) */
/*   */   /* write EEPROM, for AT90S1200 */ 
/*      { */
/*        clocks+=2;*/ /* operation of writing EEPROM halts CPU for two cycles */ 
/*        time=clocks; */
/*        return; */
/*      } */

/*    if((eecr_mask&0x07)==0x07) */
/*      { */
/*        if(io_pointer[eecr]&0x4) */
/*  */	/* attempt to do writing operation */ 
/*  	{ */
/*  	  attempt++; */
/*  	  if(attempt>3) */ /* clear flag 'EEMWE' after 4 cycles */  
/*  	    { */
/*  	      io_pointer[eecr]=io_pointer[eecr]&0xFB; */
/*  	      attempt=0; */
/*  	      eecr+=controllers[type_micro].size_gpr; */
/*  	      mem_file_changed[eecr/8]|=1<<eecr%8; */
/*  	      return; */
/*  	    } */
/*  	} */
/*        if((io_pointer[eecr]&0x7)==0x6&&time==-1) */
/*  */ 	/* write data to EEPROM*/ 
/*  	{ */
/*  	  clocks+=2;*/ /* operation of writing EEPROM halts CPU for two cycles */ 
/*  	  time=clocks; */
/*  	} */
/*        if(!(io_pointer[eecr]&0x7)) */
/*  	attempt=0; */
/*      } */

/*    if(time!=-1) */
/*      { */ /* waiting until the write access time is elapsed then do write */ 
/*        if(eeprom_wat<=1.0/frequency*(clocks-time)) */
/*  	{ */
/*  	  io_pointer[eecr]&=0xFD; */
/*  	  if(eear.word<controllers[type_micro].size_eeprom) */
/*  	    { */
/*  	      eeprom_pointer[eear.word]=io_pointer[eedr]; */
/*  	      update_eeprom_window(); */
/*  	    } */
/*  	  else */
/*  	    {puts("EEPROM address more then its size.");} */
/*  	  if(eecr_mask&0x08) */ /* generate interrupt */ 
/*  	    io_pointer[eecr]|=0x08; */
/*  	  eecr+=controllers[type_micro].size_gpr; */
/*  	  mem_file_changed[eecr/8]|=1<<eecr%8; */
/*  	  time=-1; */
/*  	} */
/*        return; */
/*      } */
/*  } */

int interrupt(void)
{
  uint16_t sp;
  int address,n;
  
  if(!get_sreg_flag(FLAG_I)) return 0;
  
  address=-1;
 
  for(n=1;controllers[type_micro].interrupts[n]!=-1&&address==-1;n++)
    address=func_int[controllers[type_micro].interrupts[n]](n);

  if(address==-1) return 0;

  if(!type_micro)
    {
      stack_90s1200[2]=stack_90s1200[1];
      stack_90s1200[1]=stack_90s1200[0];
      stack_90s1200[0]=pc;
    }
  else
    {
      if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
      else sp=get_port(SPL);
      if(sp<2) {fprintf(stderr,"Error: Stack Pointer not set\n"); return 0;}
 
      gpr_pointer[sp]=(uint8_t)(pc>>8);
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      gpr_pointer[sp]=(uint8_t)pc;
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
      else set_port(SPL,sp); 
    }
  pc=address;
  clocks+=2; 
  clear_sreg_flag(FLAG_I);
  num_call++;

  return 1;
}
  
/*  void load_ports() */
/*  { */
/*    static FILE * files[6]={NULL}; */
/*    static int read_clock[6],read_value[6]; */
/*    int n,shift; */

/*    if(clocks==0) */ /* init */ 
/*      { */
/*        for(n=0;n<6;n++) */
/*  	{ */
/*  	  if(ports[n].pins!=-1) */
/*  	    { */
/*  	      if(files[n]!=NULL) fclose(files[n]); */
/*  	      if((files[n]=fopen(data_port_sti[n].filename,"r"))!=NULL) */
/*  		read_clock[n]=0; */
/*  	      else */
/*  		read_clock[n]=-1; */
/*  	    } */
/*  	  else */
/*  	    read_clock[n]=-1; */
/*  	} */
/*      } */
  
/*    for(n=0;n<6;n++) */
/*      if(!read_clock[n]) */
/*        if((fscanf(files[n],"%d:%x",&read_clock[n],&read_value[n])==EOF)) */
/*  	read_clock[n]=-1; */
  
/*    for(n=0;n<6;n++) */
/*      if(clocks==read_clock[n]) */
/*        { */
/*  	io_pointer[ports[n].pins]=read_value[n]; */
/*  	shift=controllers[type_micro].size_gpr; */
/*  	mem_file_changed[(ports[n].pins+shift)/8]|=1<<(ports[n].pins+shift)%8; */
/*  	read_clock[n]=0; */
/*        } */
/*  } */

/*  void save_ports() */
/*  { */
/*    static FILE * files[6]={NULL}; */
/*    static int write_value[6]; */
/*    int n,shift; */
/*    char buf[80],buf1[10],buf2[4]; */
  

  
/*    for(n=0;n<6;n++) */
/*      if(ports[n].data!=-1) */
/*        { */
/*  	if(ports[n].direct!=-1) */
/*  	  { */
/*  	    io_pointer[ports[n].pins]=(io_pointer[ports[n].pins]& */
/*  				       ~io_pointer[ports[n].direct])| */
/*  	      (io_pointer[ports[n].data]&io_pointer[ports[n].direct]); */
/*  	    shift=controllers[type_micro].size_gpr; */
/*  	    mem_file_changed[(ports[n].pins+shift)/8]|= */
/*  	      1<<(ports[n].pins+shift)%8; */
/*  	  } */
	
/*  	if((ports[n].direct==-1&&io_pointer[ports[n].data]!=write_value[n])|| */
/*  	   (ports[n].direct!=-1&&io_pointer[ports[n].pins]!=write_value[n])) */
/*  	  { */
	    
/*  	    if(ports[n].direct!=-1) */
/*  	      write_value[n]=io_pointer[ports[n].pins]; */
/*  	    else */
/*  	      write_value[n]=io_pointer[ports[n].data]; */
	    
/*  	    if(files[n]!=NULL) */
/*  	      { */
/*  		sprintf(buf1,"%d",clocks); */
/*  		fill_zero(buf1,8); */
/*  		sprintf(buf2,"%X",write_value[n]); */
/*  		fill_zero(buf2,2); */
/*  		sprintf(buf,"%s:%s\n",buf1,buf2); */
/*  		fputs(buf,files[n]);  */
/*  		fflush(files[n]);  */
/*  	      } */
/*  	  } */
	
/*        } */
/*  } */


/*  void external_interrupts(void) */
/*  { */
/*    int gifr,mcucr,int_flag; */
/*    static int int_old[8]; */
 /* previous values of pins*/ 
  
/*    if(index_ports[GIFR]!=-1) gifr=io_port[index_ports[GIFR]].address; */
/*    else gifr=-1; */
/*    if(index_ports[MCUCR]!=-1) mcucr=io_port[index_ports[MCUCR]].address; */
/*    else mcucr=-1; */
  
/*    int_flag=0; */
    /* external interrupt 0 */ 
/*    if(index_pins[INT0_PIN]!=-1) */
/*      { */
/*        switch(io_pointer[mcucr]&0x03) */
	  /* check config EXT_INT0*/ 
/*  	{ */
/*  	case 0x00: */ /* low level */ 
/*  	  if(!(io_pointer[pin[index_pins[INT0_PIN]].pins_address]& */
/*  	       pin[index_pins[INT0_PIN]].mask)) */
/*  	    int_flag=1; */
/*  	  break; */
/*  	case 0x01: */ /* reserved */ 
/*  	  break; */
/*  	case 0x02: */ /* falling edge */ 
/*  	  if(int_old[0]&&!(io_pointer[pin[index_pins[INT0_PIN]].pins_address]& */
/*  			   pin[index_pins[INT0_PIN]].mask)) */
/*  	    int_flag=1; */
/*  	  break; */
/*  	case 0x03: */ /* rising edge */ 
/*  	  if(!int_old[0]&&(io_pointer[pin[index_pins[INT0_PIN]].pins_address]& */
/*  			   pin[index_pins[INT0_PIN]].mask)) */
/*  	    int_flag=1; */
/*  	  break; */
/*  	} */
/*        int_old[0]=io_pointer[pin[index_pins[INT0_PIN]].pins_address]& */
/*  	pin[index_pins[INT0_PIN]].mask; */
      
/*        if(int_flag) */
/*  	{ */
/*  	  if(index_ports[GIFR]!=-1) */
/*  	    { */
/*  	      gifr=io_port[index_ports[GIFR]].address; */
/*  	      io_pointer[gifr]|=0x40; */
/*  	      gifr+=controllers[type_micro].size_gpr; */
/*  	      mem_file_changed[gifr/8]|=1<<gifr%8; */
/*  	    } */
/*  	  else  */
/*  	    { */
/*  	      ex_ifr|=0x01; */
/*  	    } */
/*  	} */
/*      } */

/*    int_flag=0; */
    /* external interrupt 1 */ 
/*    if(index_pins[INT1_PIN]!=-1) */
/*      { */
/*        switch((io_pointer[mcucr]&0x0C)>>2) */ /* check config EXT_INT0*/ 
/*  	{ */
/*  	case 0x00: */ /* low level */ 
/*  	  if(!(io_pointer[pin[index_pins[INT1_PIN]].pins_address]& */
/*  	       pin[index_pins[INT1_PIN]].mask)) */
/*  	    int_flag=1; */
/*  	  break; */
/*  	case 0x01: */ /* reserved */ 
/*  	  break; */
/*  	case 0x02: */ /* falling edge */ 
/*  	  if(int_old[1]&&!(io_pointer[pin[index_pins[INT1_PIN]].pins_address]& */
/*  			   pin[index_pins[INT1_PIN]].mask)) */
/*  	    int_flag=1; */
/*  	  break; */
/*  	case 0x03: */
 /* rising edge */ 
/*  	  if(!int_old[1]&&(io_pointer[pin[index_pins[INT1_PIN]].pins_address]& */
/*  			   pin[index_pins[INT1_PIN]].mask)) */
/*  	    int_flag=1; */
/*  	  break; */
/*  	} */
/*        int_old[1]=io_pointer[pin[index_pins[INT1_PIN]].pins_address]& */
/*  	pin[index_pins[INT1_PIN]].mask; */
      
/*        if(int_flag) */
/*  	{ */
/*  	  if(index_ports[GIFR]!=-1) */
/*  	    { */
/*  	      gifr=io_port[index_ports[GIFR]].address; */
/*  	      io_pointer[gifr]|=0x80; */
/*  	      gifr+=controllers[type_micro].size_gpr; */
/*  	      mem_file_changed[gifr/8]|=1<<gifr%8; */
/*  	    } */
/*  	  else  */
/*  	    { */
/*  	      ex_ifr|=0x02; */
/*  	    } */
/*  	}       */
/*      } */
/*  } */

void hardware_init(void)
{
  io_port=controllers[type_micro].io_registers;
  pin=controllers[type_micro].pins;
  flag=controllers[type_micro].flags;
  prescaler_value0=prescaler_value1=prescaler_value2=prescaler_value3=0;
}

void hardware(void)
{ 
/*   load_ports(); */
  prescaler0();
  timer_counter0();
  prescaler1();
  timer_counter1();
  prescaler2();
  timer_counter2();
  prescaler3();
  timer_counter3();
/*    eeprom_access(); */
  analog_comparator(); 
  /*    external_interrupts(); */
/*    save_ports(); */
/*    watchdog_timer(); */
}

/* function to detecting interrupt*/
int reset_int(int addr)
{
  return -1;
}

int int0_int(int addr)
{
  if(!ISFLAG(INT0_FLAG)) return -1;
  if(get_flag(INT0_FLAG))
    {
      if(ISFLAG(INTF0_FLAG))
	{
	  if(get_flag(INTF0_FLAG))
	    {
	      clear_flag(INTF0_FLAG);
	      return addr;
	    }
	  return -1;
	}
      if(!ISFLAG(ISC00_FLAG)) /* interrupt when pin is low-level, mega103 */
	{
	  if(!get_pin(INT0_PIN)) return addr;
	  return -1;
	}
      if(ex_ifr&(1<<0))
	{
	  ex_ifr&=~(1<<0);
	  return addr;
	}
    }
  return -1;
}

int int1_int(int addr)
{ 
  if(!ISFLAG(INT1_FLAG)) return -1;
  if(get_flag(INT1_FLAG))
    {
      if(ISFLAG(INTF1_FLAG))
	{
	  if(get_flag(INTF1_FLAG))
	    {
	      clear_flag(INTF1_FLAG);
	      return addr;
	    }
	  return -1;
	}
      if(!ISFLAG(ISC10_FLAG)) /* interrupt when pin is low-level, mega103 */
	{
	  if(!get_pin(INT1_PIN)) return addr;
	  return -1;
	}
      if(ex_ifr&(1<<1))
	{
	  ex_ifr&=~(1<<1);
	  return addr;
	}
    }
  return -1;
}


int int2_int(int addr)
{
  if(!ISFLAG(INT2_FLAG)) return -1;
  if(get_flag(INT2_FLAG))
    {
      if(ISFLAG(INTF2_FLAG))
	{
	  if(get_flag(INTF2_FLAG))
	    {
	      clear_flag(INTF2_FLAG);
	      return addr;
	    }
	  return -1;
	}
      if(!ISFLAG(ISC20_FLAG)) /* interrupt when pin is low-level, mega103 */
	{
	  if(!get_pin(INT2_PIN)) return addr;
	  return -1;
	}
    }
  return -1;
}

int int3_int(int addr)
{
  if(!ISFLAG(INT3_FLAG)) return -1;
  if(get_flag(INT3_FLAG))
    {
      if(ISFLAG(INTF3_FLAG))
	{
	  if(get_flag(INTF3_FLAG))
	    {
	      clear_flag(INTF3_FLAG);
	      return addr;
	    }
	  return -1;
	}
      if(!ISFLAG(ISC30_FLAG)) /* interrupt when pin is low-level, mega103 */
	{
	  if(!get_pin(INT3_PIN)) return addr;
	  return -1;
	}
    }
  
  return -1;
}

int int4_int(int addr)
{
  if(check_mask_and_flag(INT4_FLAG,INTF4_FLAG)) return addr;
  return -1;
}

int int5_int(int addr)
{
  if(check_mask_and_flag(INT5_FLAG,INTF5_FLAG)) return addr;
  return -1;
}

int int6_int(int addr)
{
  if(check_mask_and_flag(INT6_FLAG,INTF6_FLAG)) return addr;
  return -1;
}

int int7_int(int addr)
{
  if(check_mask_and_flag(INT7_FLAG,INTF7_FLAG)) return addr;
  return -1;
}

int pcint0_int()
{
  return -1;
}

int pcint1_int()
{
  return -1;
}

int timer3_capt_int(int addr)
{
  if(check_mask_and_flag(TICIE3_FLAG,ICF3_FLAG)) return addr;
  return -1;
}

int timer3_compa_int(int addr)
{
  if(check_mask_and_flag(OCIE3A_FLAG,OCF3A_FLAG)) return addr;
  return -1;
}

int timer3_compb_int(int addr)
{
  if(check_mask_and_flag(OCIE3B_FLAG,OCF3B_FLAG)) return addr;
  return -1;
}

int timer3_compc_int(int addr)
{
  if(check_mask_and_flag(OCIE3C_FLAG,OCF3C_FLAG)) return addr;
  return -1;
}

int timer3_ovf_int(int addr)
{
  if(check_mask_and_flag(TOIE3_FLAG,TOV3_FLAG)) return addr;
  return -1;
}

int timer2_comp_int(int addr)
{
  if(check_mask_and_flag(OCIE2_FLAG,OCF2_FLAG)) return addr;
  return -1;
}

int timer2_ovf_int(int addr)
{
  if(check_mask_and_flag(TOIE2_FLAG,TOV2_FLAG)) return addr;
  return -1;
}

int timer1_capt_int(int addr)
{
  if(check_mask_and_flag(TICIE1_FLAG,ICF1_FLAG)) return addr;
  return -1;
}

int timer1_compa_int(int addr)
{
  if(check_mask_and_flag(OCIE1A_FLAG,OCF1A_FLAG)) return addr;
  return -1;
}

int timer1_compb_int(int addr)
{
  if(check_mask_and_flag(OCIE1B_FLAG,OCF1B_FLAG)) return addr;
  return -1;
}

int timer1_compc_int(int addr)
{
  if(check_mask_and_flag(OCIE1C_FLAG,OCF1C_FLAG)) return addr;
  return -1;
}

int timer1_ovf_int(int addr)
{
  if(check_mask_and_flag(TOIE1_FLAG,TOV1_FLAG)) return addr;
  return -1;
}

int timer0_comp_int(int addr)
{
  if(check_mask_and_flag(OCIE0_FLAG,OCF0_FLAG)) return addr;
  return -1;
}

int timer0_ovf_int(int addr)
{
  if(check_mask_and_flag(TOIE0_FLAG,TOV0_FLAG)) return addr;
  return -1;
}

int spi_stc_int(int addr)
{
  if(check_mask_and_flag(SPIE_FLAG,SPIF_FLAG)) return addr;
  return -1;
}

int uart0_rx_int(int addr)
{
  if(!ISFLAG(RXCIE0_FLAG)) return -1;
  
  if(get_flag(RXCIE0_FLAG)&&get_flag(RXC0_FLAG)) return addr;
  return -1;
}

int uart0_udre_int(int addr)
{
  if(!ISFLAG(UDRIE0_FLAG)) return -1;
  
  if(get_flag(UDRIE0_FLAG)&&get_flag(UDRE0_FLAG)) return addr;
  return -1;
}

int uart0_tx_int(int addr)
{  
  if(check_mask_and_flag(TXCIE0_FLAG,TXC0_FLAG)) return addr;
  return -1;
}

int uart1_rx_int(int addr)
{
  if(!ISFLAG(RXCIE1_FLAG)) return -1;
  
  if(get_flag(RXCIE1_FLAG)&&get_flag(RXC1_FLAG)) return addr;
  return -1;
}

int uart1_udre_int(int addr)
{
  if(!ISFLAG(UDRIE1_FLAG)) return -1;
  
  if(get_flag(UDRIE1_FLAG)&&get_flag(UDRE1_FLAG)) return addr;
  return -1;
}

int uart1_tx_int(int addr)
{  
  if(check_mask_and_flag(TXCIE1_FLAG,TXC1_FLAG)) return addr;
  return -1;
}

int adc_int(int addr)
{
  if(check_mask_and_flag(ADIE_FLAG,ADIF_FLAG)) return addr;
  return -1;
}

int ee_ready_int(int addr)
{
  if(!ISFLAG(EERIE_FLAG)) return -1;
  
  if(get_flag(EERIE_FLAG)&&get_flag(EEWE_FLAG)==0) return addr;
  return -1;
}

int analog_comp_int(int addr)
{
  if(check_mask_and_flag(ACIE_FLAG,ACI_FLAG)) return addr;
  return -1;
}

int twsi_int(int addr)
{
  if(!ISFLAG(TWIE_FLAG)) return -1;

  if(get_flag(TWIE_FLAG)&&get_flag(TWINT_FLAG)) return addr;
  return -1;
}

int spm_rdy_int(int addr)
{
  return -1;
}

void set_flag(int f)
{
  int index=index_flags[f];
  int port;

  if(index==-1) {fprintf(stderr,"Error: flag not found. Flag: %d\n",f);return;}
  port=io_port[index_ports[flag[index].port_code]].address;
  io_pointer[port]|=flag[index].flag_mask;

  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

void clear_flag(int f)
{
  int index=index_flags[f];
  int port;
  port=io_port[index_ports[flag[index].port_code]].address;
  if(index==-1) {fprintf(stderr,"Error: flag not found. Flag: %d\n",f);return;}
  io_pointer[port]&=~flag[index].flag_mask;

  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

int get_flag(int f)
{
  int index=index_flags[f];
  if(index==-1)
    {fprintf(stderr,"Error: flag not found. Flag: %d\n",f);return -1;}
  return io_pointer[io_port[index_ports[flag[index].port_code]].address]&flag[index].flag_mask;
}

int check_mask_and_flag(int m,int f)
{
  if(!ISFLAG(m)) return 0;
  
  if(get_flag(m)&&get_flag(f))
    {
      clear_flag(f);
      return 1;
    }
  return 0;
}

int get_pin(int p)
{
  int index=index_pins[p];
  if(index==-1){fprintf(stderr,"Error: pin not found. Pin: %d\n",p);return -1;}
  return io_pointer[pin[index].pins_address]&pin[index].mask;
}

void set_pin(int p)
{
  int index=index_pins[p];
  int port;

  if(index==-1) {fprintf(stderr,"Error: pin not found. Pin: %d\n",p);return;}
  port=pin[index].data_address;
  io_pointer[port]|=pin[index].mask;

  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

void clear_pin(int p)
{
  int index=index_pins[p];
  int port;

  if(index==-1) {fprintf(stderr,"Error: pin not found. Pin: %d\n",p);return;}
  port=pin[index].data_address;
  io_pointer[port]&=~pin[index].mask;

  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

uint8_t get_port(int port)
{
  int index=index_ports[port];
  if(index==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[port].name);
      return 0;
    }

  return io_pointer[io_port[index].address];
}


uint8_t add_port(int p,uint8_t add)
{
  int port=index_ports[p];
  uint8_t result;
  if(port==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[port].name);
      return 0;
    }
  port=io_port[port].address;
  result=(io_pointer[port]+=add);
    
  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;

  return result;
}

void set_port(int p,uint8_t value)
{
  int port=index_ports[p];
  if(port==-1)
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[port].name);
      return;
    }
  port=io_port[port].address;
  io_pointer[port]=value;

  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

uint16_t get_port16(int low,int hi)
{
  uint16_t result;
  int index_low=index_ports[low],index_hi=index_ports[hi];
  if(index_low==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[low].name);
      return 0;
    }
  if(index_hi==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[hi].name);
      return 0;
    }

  result=io_pointer[io_port[index_low].address]+
    (io_pointer[io_port[index_hi].address]<<8);
  return result;
}

void set_port16(int low,int hi,uint16_t value)
{
  int port_low=index_ports[low],port_hi=index_ports[hi];
  if(port_low==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[low].name);
      return;
    }
  if(port_hi==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[hi].name);
      return;
    }
  port_low=io_port[port_low].address;
  port_hi=io_port[port_hi].address;
  io_pointer[port_low]=(uint8_t)value;
  io_pointer[port_hi]=(uint8_t)(value>>8);
  
  port_low+=controllers[type_micro].size_gpr;
  mem_file_changed[port_low/8]|=1<<port_low%8;
  port_hi+=controllers[type_micro].size_gpr;
  mem_file_changed[port_hi/8]|=1<<port_hi%8;
}

uint16_t add_port16(int low,int hi,uint16_t value)
{
  uint16_t result;
  int port_low=index_ports[low],port_hi=index_ports[hi];

  if(port_low==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[low].name);
      return 0;
    }
  if(port_hi==-1) 
    {
      fprintf(stderr,"Error: port not found. Port: %s\n",
	      descr_io_ports[hi].name);
      return 0;
    }
  port_low=io_port[port_low].address;
  port_hi=io_port[port_hi].address;
  result=io_pointer[port_low]+(io_pointer[port_hi]<<8);
  result+=value;
  io_pointer[port_low]=(uint8_t)result;
  io_pointer[port_hi]=(uint8_t)(result>>8);
  
  port_low+=controllers[type_micro].size_gpr;
  mem_file_changed[port_low/8]|=1<<port_low%8;
  port_hi+=controllers[type_micro].size_gpr;
  mem_file_changed[port_hi/8]|=1<<port_hi%8;
  return result;
}

void set_sreg_flag(int f)
{
  int port=io_port[index_ports[SREG]].address;
  io_pointer[port]|=f;
  
  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

void clear_sreg_flag(int f)
{
  int port=io_port[index_ports[SREG]].address;
  io_pointer[port]&=~f;
  
  port+=controllers[type_micro].size_gpr;
  mem_file_changed[port/8]|=1<<port%8;
}

int get_sreg_flag(int f)
{
  if(io_pointer[io_port[index_ports[SREG]].address]&f) return 1;
  return 0;
}
     
