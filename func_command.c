/*
 File        : func_command.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : Description of commands.

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

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <pthread.h>
#include <Xm/XmAll.h>
#include "types.h"
#include "global.h"

void check_Z_N_flags(uint8_t result)
{
  /* check flag Z*/
  if(result==0) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);  

  /* check flag N */
  if(result&0x80) set_sreg_flag(FLAG_N);
  else clear_sreg_flag(FLAG_N);  
}

int get_io_index(int code)
{
  int n;
  
  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(code==controllers[type_micro].io_registers[n].code)
      return n;
  return -1;
}

void adc_command(void)
{
  uint8_t arg1,arg2; 
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=gpr_pointer[list_commands[pc].arg2];
  gpr_pointer[list_commands[pc].arg1]+=arg2;
  if(get_sreg_flag(FLAG_C)) 
    gpr_pointer[list_commands[pc].arg1]++;
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag C */
  if(((arg1&0x80)&(arg2&0x80))|
     ((arg1&0x80)&~(gpr_pointer[list_commands[pc].arg1]&0x80))|
     (~(gpr_pointer[list_commands[pc].arg1]&0x80)&
      (arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);

  /* check flag H */
  if(((arg1&0x08)&(arg2&0x08))|
     ((arg1&0x08)&~(gpr_pointer[list_commands[pc].arg1]&0x8))|
     (~(gpr_pointer[list_commands[pc].arg1]&0x08)&
      (arg2&0x08)))
   set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);  

  /* check flag V */
  if(((arg1&0x80)&(arg2&0x80)&(~(gpr_pointer[list_commands[pc].arg1]&0x80)))|
     ((~(arg1&0x80))&(~(arg2&0x80))&(gpr_pointer[list_commands[pc].arg1]&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);   
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  
  pc++;
  clocks++;
}

void add_command(void)
{
  unsigned char arg1,arg2; 
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=gpr_pointer[list_commands[pc].arg2];
  gpr_pointer[list_commands[pc].arg1]+=arg2;
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag C */
  if(((arg1&0x80)&(arg2&0x80))|
     ((arg1&0x80)&~(gpr_pointer[list_commands[pc].arg1]&0x80))|
     (~(gpr_pointer[list_commands[pc].arg1]&0x80)&
      (arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);  

   /* check flag H */
  if(((arg1&0x08)&(arg2&0x08))|
     ((arg1&0x08)&(gpr_pointer[list_commands[pc].arg1]&0x8))|
     ((gpr_pointer[list_commands[pc].arg1]&0x08)&
      (arg2&0x08)))
    set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);

  /* check flag V */
  if(((arg1&0x80)&(arg2&0x80)&(~(gpr_pointer[list_commands[pc].arg1]&0x80)))|
     ((~(arg1&0x80))&(~(arg2&0x80))&(gpr_pointer[list_commands[pc].arg1]&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);
  
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  
  
  pc++;
  clocks++;
}

void adiw_command(void)
{
  uint16_t reg,old;

  reg=gpr_pointer[list_commands[pc].arg1];
  reg+=gpr_pointer[list_commands[pc].arg1+1]<<8;
  old=reg;
  reg+=list_commands[pc].arg2;
  gpr_pointer[list_commands[pc].arg1]=(uint8_t)reg;
  gpr_pointer[list_commands[pc].arg1+1]=(uint8_t)(reg>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[(list_commands[pc].arg1+1)/8]=
    1<<(list_commands[pc].arg1+1)%8;

  /* check Flag Z */
  if(!reg) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);
  
  /* check Flag N */
  if(reg&0x8000) set_sreg_flag(FLAG_N);
  else clear_sreg_flag(FLAG_N);
  
  /* check Flag C */
  if((~(reg&0x8000))&&(old&0x8000)) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);

  /* check flag V */
  if((reg&0x8000)&&(~(old&0x8000))) set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  

  pc++;
  clocks+=2;
}

void and_command(void)
{
  gpr_pointer[list_commands[pc].arg1]&=gpr_pointer[list_commands[pc].arg2];

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag V */
  clear_sreg_flag(FLAG_V);
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);

  pc++;
  clocks++;
}

void andi_command(void)
{
  gpr_pointer[list_commands[pc].arg1]=
    list_commands[pc].arg2&gpr_pointer[list_commands[pc].arg1];

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag V */
  clear_sreg_flag(FLAG_V);

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 

  pc++;
  clocks++;
}

void asr_command(void)
{
  /* set flag C */
  if(gpr_pointer[list_commands[pc].arg1]&0x01) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);
  
  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg1]>>1;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);

  /* check flag V */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_C))
  set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 

  pc++;
  clocks++;
}

void bclr_command(void)
{
  switch(list_commands[pc].arg1)
    {
    case 7: clear_sreg_flag(FLAG_I); break;
    case 6: clear_sreg_flag(FLAG_T); break;
    case 5: clear_sreg_flag(FLAG_H); break;
    case 4: clear_sreg_flag(FLAG_S); break;
    case 3: clear_sreg_flag(FLAG_V); break;
    case 2: clear_sreg_flag(FLAG_N); break;
    case 1: clear_sreg_flag(FLAG_Z); break;
    case 0: clear_sreg_flag(FLAG_C); break;
    }
  pc++;
  clocks++;
}

void bld_command(void)
{
  if(get_sreg_flag(FLAG_T))
    gpr_pointer[list_commands[pc].arg1]|=1<<list_commands[pc].arg2;
  else 
    gpr_pointer[list_commands[pc].arg1]&=~(1<<list_commands[pc].arg2);

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  pc++;
  clocks++;
}

void brbc_command(void)
{
  if(get_sreg_flag(1<<list_commands[pc].arg1))
    {pc++;clocks++;}
  else
    {
      pc=list_commands[pc].arg2;
      clocks++;
    }
}

void brbs_command(void)
{
  if(get_sreg_flag(1<<list_commands[pc].arg1))
    {
      pc=list_commands[pc].arg2;
      clocks++;
    }
  else {pc++;clocks++;}
}

void bset_command(void)
{
  set_sreg_flag(1<<list_commands[pc].arg1);
  pc++;
  clocks++;
}

void bst_command(void)
{
  if(gpr_pointer[list_commands[pc].arg1]&(1<<list_commands[pc].arg2))
    set_sreg_flag(FLAG_T);
  else clear_sreg_flag(FLAG_T);
  pc++;
  clocks++;
}

/* need to change for work on 22 bit PC */
void call_command(void)
{
  uint16_t sp;
  
  pc+=2;
  if(type_micro==0)
    {
      stack_90s1200[2]=stack_90s1200[1];
      stack_90s1200[1]=stack_90s1200[0];
      stack_90s1200[0]=pc;
    }
  else
    {
      if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
      else sp=get_port(SPL);
      if(sp<2) {fprintf(stderr,"Error: Stack Pointer not set\n");}
      pc+=2;
      gpr_pointer[sp]=(uint8_t)(pc>>8);
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      gpr_pointer[sp]=(uint8_t)pc;
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
      else set_port(SPL,sp);  
    }
  pc=list_commands[pc-2].arg1;
  clocks+=4;
}

void cbi_command(void)
{
  int n,mask;
  
  for(mask=n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(controllers[type_micro].io_registers[n].address==list_commands[pc].arg1)
      mask=controllers[type_micro].io_registers[n].write_mask;
  if(mask&(1<<list_commands[pc].arg2))
    io_pointer[list_commands[pc].arg1]&=~(1<<list_commands[pc].arg2);
  pc++;
  clocks+=2;
}

void com_command(void)
{
  gpr_pointer[list_commands[pc].arg1]=0xFF-gpr_pointer[list_commands[pc].arg1];

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);

  set_sreg_flag(FLAG_C);
  clear_sreg_flag(FLAG_V);

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 

  pc++;
  clocks++;
}

void cp_command(void)
{
  unsigned int result,arg1,arg2;
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=gpr_pointer[list_commands[pc].arg2];
  result=arg1-arg2;
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|((arg2&0x80)&(result&0x80))|
     ((result&0x80)&~(arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C); 
    
  
  check_Z_N_flags(result);
  
  /* check flag V */
  if(((arg1&0x80)&~(arg2&0x80)&~(result&0x80))|
     (~(arg1&0x80)&(arg2&0x80)&(result&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V); 
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 
  
  /* check flag H */
  if(((arg1&0x08)&(arg2&0x08))|((arg2&0x08)&~(result&0x8))|
     (~(result&0x08)&(arg1&0x08)))
   set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H); 

  pc++;
  clocks++;
}

void cpc_command(void)
{
  unsigned int result,arg1,arg2;
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=gpr_pointer[list_commands[pc].arg2];
  result=arg1-arg2;
  if(get_sreg_flag(FLAG_C))
    result--;
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|((arg2&0x80)&(result&0x80))|
     ((result&0x80)&~(arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C); 

  check_Z_N_flags(result);
  
  /* check flag V */
  if(((arg1&0x80)&~(arg2&0x80)&~(result&0x80))|
     (~(arg1&0x80)&(arg2&0x80)&(result&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V); 

  /* check flag S */
 if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 

  /* check flag H */
  if((~(arg1&0x08)&(arg2&0x08))|((arg2&0x08)&(result&0x8))|
     ((result&0x08)&~(arg1&0x08)))
     set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H); 
  pc++;
  clocks++;
}

void cpi_command(void)
{
 unsigned int result,arg1,arg2;
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=list_commands[pc].arg2;
  result=arg1-arg2;
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|((arg2&0x80)&(result&0x80))|
     ((result&0x80)&~(arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C); 
  
  check_Z_N_flags(result);
  
  /* check flag V */
  if(((arg1&0x80)&~(arg2&0x80)&~(result&0x80))|
     (~(arg1&0x80)&(arg2&0x80)&(result&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V); 

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 
  
  /* check flag H */
  if((~(arg1&0x08)&(arg2&0x08))|((arg2&0x08)&(result&0x8))|
     ((result&0x08)&~(arg1&0x08)))
    set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H); 
  pc++;
  clocks++; 
}

void cpse_command(void)
{

  if(gpr_pointer[list_commands[pc].arg1]!=gpr_pointer[list_commands[pc].arg2])
    {
      pc++;
      clocks++; 
    }
  else
    {
      clocks+=list_commands[pc+1].num_words+1;
      pc+=list_commands[pc+1].num_words+1;
    }
}

void dec_command(void)
{
  /* check flag V */
  if(gpr_pointer[list_commands[pc].arg1]==0x80) set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V); 
  
  gpr_pointer[list_commands[pc].arg1]--;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 
  clocks++;
  pc++;
}

void elpm1_command(void)
{
  int address;
  uint16_t code;

  if(!ISPORT(RAMPZ))
    {
      puts("This command doesn't support by this device.");
      return;
    }
  
  address=gpr_pointer[30];
  address+=gpr_pointer[31]<<8;
  address+=get_port(RAMPZ)<<16;

  code=list_commands[address>>1].code;

  if(address&1)
    gpr_pointer[0]=code>>8;
  else 
    gpr_pointer[0]=code;

  /* set bit, it means that field's value will be update */
  mem_file_changed[0]=mem_file_changed[0]|1;

  pc++;
  clocks+=3;
}

void elpm2_command(void)
{
  int address;
  uint16_t code;

  if(!ISPORT(RAMPZ))
    {
      puts("This command doesn't support by this device.");
      return;
    }
  
  address=gpr_pointer[30];
  address+=gpr_pointer[31]<<8;
  address+=get_port(RAMPZ)<<16;

  code=list_commands[address>>1].code;

  if(address&1)
    gpr_pointer[list_commands[pc].arg1]=code>>8;
  else 
    gpr_pointer[list_commands[pc].arg1]=code;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  pc++;
  clocks+=3;
}

void elpm3_command(void)
{
  int address;
  uint16_t code;

  if(!ISPORT(RAMPZ))
    {
      puts("This command doesn't support by this device.");
      return;
    }
  
  address=gpr_pointer[30];
  address+=gpr_pointer[31]<<8;
  address+=get_port(RAMPZ)<<16;

  code=list_commands[address>>1].code;

  if(address&1)
    gpr_pointer[list_commands[pc].arg1]=code>>8;
  else 
    gpr_pointer[list_commands[pc].arg1]=code;

  address++;
  gpr_pointer[30]=(uint8_t)address;
  gpr_pointer[31]=(uint8_t)(address>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[31/8]|=(1<<31%8)|(1<<30%8);
  
  if((address>>16)!=get_port(RAMPZ))
    set_port(RAMPZ,address>>16);
  
  pc++;
  clocks+=3;
}

void eor_command(void)
{
  gpr_pointer[list_commands[pc].arg1]^=gpr_pointer[list_commands[pc].arg2];

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);

  /* clear FLAG_V */
  clear_sreg_flag(FLAG_V); 

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S); 
  pc++;
  clocks++;
}

void fmul_command(void)
{
  return;
}

void fmuls_command(void)
{
  return;
}

void fmulsu_command(void)
{
  return;
}

void icall_command(void)
{
  uint16_t sp;
  
  if(type_micro==0)
    {
      stack_90s1200[2]=stack_90s1200[1];
      stack_90s1200[1]=stack_90s1200[0];
      stack_90s1200[0]=pc+1;
    }
  else
    {
      if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
      else sp=get_port(SPL);
      if(sp<2) {fprintf(stderr,"Error: Stack Pointer not set\n");}
      
      gpr_pointer[sp]=(uint8_t)(pc>>8);
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      gpr_pointer[sp]=(uint8_t)pc;
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
      else set_port(SPL,sp);  
    }

  pc=gpr_pointer[30]+(gpr_pointer[31]<<8);
  clocks+=3;
}

void ijmp_command(void)
{
  pc=gpr_pointer[30]+(gpr_pointer[31]<<8);
  clocks+=2;
}

void in_command(void)
{
  int n;
  IO_port *port=NULL;
  uint16_t value;

  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(list_commands[pc].arg2==controllers[type_micro].io_registers[n].address)
      {port=&controllers[type_micro].io_registers[n];break;}

  if(port==NULL)
    {
      fprintf(stderr,"Port not defined, address: 0x%X\n",
	      list_commands[pc].arg2);
      pc++;
      clocks++;
      return;
    }

  for(n=0;port16_rd[n].code!=-1;n++)
    if(port16_rd[n].code==port->code) break;
  
  if(port16_rd[n].code==-1)
    {
      gpr_pointer[list_commands[pc].arg1]=io_pointer[list_commands[pc].arg2];
      mem_file_changed[list_commands[pc].arg1/8]=1<<list_commands[pc].arg1%8;
    }
  else
    {
      switch(port16_rd[n].type){
      case LOW:
	value=get_port16(port16_rd[n].code,port16_rd[n-1].code);
	gpr_pointer[list_commands[pc].arg1]=(uint8_t)value;
	*port16_rd[n].temp=(uint8_t)(value>>8);
	mem_file_changed[list_commands[pc].arg1/8]=1<<list_commands[pc].arg1%8;
	break;
      case HIGH: 
	gpr_pointer[list_commands[pc].arg1]=*port16_rd[n].temp;
	mem_file_changed[list_commands[pc].arg1/8]=1<<list_commands[pc].arg1%8;

      }
    }

  pc++;
  clocks++;
}

void inc_command(void)
{
  /* check Flag V */
  if(gpr_pointer[list_commands[pc].arg1]==0x7F) set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);
  
  gpr_pointer[list_commands[pc].arg1]++;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);

  pc++;
  clocks++;
}

void jmp_command(void)
{
  pc=list_commands[pc].arg1;
  clocks++;
}

/* add useing RAMPX register for >64K data memory*/
void ld_x_command(void)
{
  int x=gpr_pointer[26];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) x+=gpr_pointer[27]<<8;
  if(x<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[x];
  else fprintf(stderr,"Error: X register don't point to data memory\n");

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  pc++;
  clocks+=2;
}

void ld_x_plus_command(void)
{
  int x=gpr_pointer[26];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) x+=gpr_pointer[27]<<8;
  if(x<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[x];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
  
  x++;
  gpr_pointer[26]=(uint8_t)x;
  if(size>0xFF) gpr_pointer[27]=(uint8_t)(x>>8);
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[26/8]|=(1<<26%8)|(1<<27%8);
  
  pc++;
  clocks+=2;
}

void ld_minus_x_command(void)
{
  int x=gpr_pointer[26];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) x+=gpr_pointer[27]<<8;
  x--;
  if(x<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[x];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
  
  gpr_pointer[26]=(uint8_t)x;
  if(size>0xFF) gpr_pointer[27]=(uint8_t)(x>>8);
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[26/8]|=(1<<26%8)|(1<<27%8);
  
  pc++;
  clocks+=2;
}

void ld_y_command(void)
{
  int y=gpr_pointer[28];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  if(y<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[y];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
 
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;  
  
  pc++;
  clocks+=2;
}

void ld_y_plus_command(void)
{
  int y=gpr_pointer[28];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  if(y<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[y];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
  
  y++;
  gpr_pointer[28]=(uint8_t)y;
  if(size>0xFF) gpr_pointer[29]=(uint8_t)(y>>8);
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[28/8]|=(1<<28%8)|(1<<29%8);
  
  pc++;
  clocks+=2;
}

void ld_minus_y_command(void)
{
  int y=gpr_pointer[28];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  y--;
  if(y<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[y];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
  
  gpr_pointer[28]=(uint8_t)y;
  if(size>0xFF) gpr_pointer[29]=(uint8_t)(y>>8);
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[28/8]|=(1<<28%8)|(1<<29%8);

  pc++;
  clocks+=2;
}

void ld_z_command(void)
{
  int z=gpr_pointer[30];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) z+=gpr_pointer[30]<<8;
  if(z<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[z];
  else fprintf(stderr,"Error: X register don't point to data memory\n");

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  pc++;
  clocks+=2;
}

void ld_z_plus_command(void)
{
  int z=gpr_pointer[30];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) z+=gpr_pointer[31]<<8;
  if(z<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[z];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
  
  z++;
  gpr_pointer[30]=(uint8_t)z;
  if(size>0xFF) gpr_pointer[31]=(uint8_t)(z>>8);
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[30/8]|=(1<<30%8)|(1<<31%8);

  pc++;
  clocks+=2;
}

void ld_minus_z_command(void)
{
  int z=gpr_pointer[30];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) z+=gpr_pointer[31]<<8;
  z--;
  if(z<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[z];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
  
  gpr_pointer[30]=(uint8_t)z;
  if(size>0xFF) gpr_pointer[31]=(uint8_t)(z>>8);
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[30/8]|=(1<<30%8)|(1<<31%8);
  
  pc++;
  clocks+=2;
}

void ldd_y_command(void)
{
  int y=gpr_pointer[28];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  y+=list_commands[pc].arg2;
  if(y<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[y];
  else fprintf(stderr,"Error: X register don't point to data memory\n");
 
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8; 

  pc++;
  clocks+=2;
}

void ldd_z_command(void)
{
  int z=gpr_pointer[30];
  int size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  if(size>0xFF) z+=gpr_pointer[30]<<8;
  z+=list_commands[pc].arg2;
  if(z<size) gpr_pointer[list_commands[pc].arg1]=gpr_pointer[z];
  else fprintf(stderr,"Error: X register don't point to data memory\n");

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  pc++;
  clocks+=2;
}

void ldi_command(void)
{
  gpr_pointer[list_commands[pc].arg1]=list_commands[pc].arg2;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  pc++;
  clocks++;
}

void lds_command(void)
{
  if(list_commands[pc].arg2<controllers[type_micro].size_gpr+
     controllers[type_micro].size_io_reg+controllers[type_micro].size_sram)
     gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg2];
  else fprintf(stderr,"Error: argument don't point to data memory\n");

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  pc+=2;
  clocks+=2;
}

void lpm_command(void)
{
  int address;
  uint16_t code;
  
  address=gpr_pointer[30];
  address+=gpr_pointer[31]<<8;
  
  code=list_commands[address>>1].code;

  if(address&1) gpr_pointer[0]=code>>8;
  else gpr_pointer[0]=code;

  /* set bit, it means that field's value will be update */
  mem_file_changed[0]|=1;

  pc++;
  clocks+=3;
}

void lpm_z_command(void)
{
  int address;
  uint16_t code;
  
  address=gpr_pointer[30];
  address+=gpr_pointer[31]<<8;
  
  code=list_commands[address>>1].code;

  if(address&1) gpr_pointer[list_commands[pc].arg1]=code>>8;
  else gpr_pointer[list_commands[pc].arg1]=code;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  pc++;
  clocks+=3;
}

void lpm_z_plus_command(void)
{
  int address;
  uint16_t code;
  
  address=gpr_pointer[30];
  address+=gpr_pointer[31]<<8;
  
  code=list_commands[address>>1].code;

  if(address&1) gpr_pointer[list_commands[pc].arg1]=code>>8;
  else gpr_pointer[list_commands[pc].arg1]=code;

  address ++;
  gpr_pointer[30]=(uint8_t)address;
  gpr_pointer[31]=(uint8_t)(address>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[30/8]|=(1<<30%8)|(1<<31%8);

  pc++;
  clocks+=3;
}

void lsr_command(void)
{
  /* check Flag C */
  if(gpr_pointer[list_commands[pc].arg1]&0x1) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);

  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg1]>>1;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  /* check Flag Z */
  if(gpr_pointer[list_commands[pc].arg1]==0) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);
  
  clear_sreg_flag(FLAG_N);

  /* check flag V */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_C)) set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);

  pc++;
  clocks++;
}

void mov_command(void)
{
  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg2];

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  pc++;
  clocks++;  
}

void movw_command(void)
{
  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg2];
  gpr_pointer[list_commands[pc].arg1+1]=gpr_pointer[list_commands[pc].arg2+1];

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  mem_file_changed[(list_commands[pc].arg1+1)/8]|=
    1<<(list_commands[pc].arg1+1)%8;
  pc++;
  clocks++;
}

void mul_command(void)
{
  uint16_t result;

  result=gpr_pointer[list_commands[pc].arg1]*
    gpr_pointer[list_commands[pc].arg2];
  gpr_pointer[0]=(uint8_t)result;
  gpr_pointer[1]=(uint8_t)(result>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[0]|=0x03;
  
  /* check Flag Z */
  if(!result) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);
   
  /* check Flag C*/
  if(result&0x8000) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C); 
  
  pc++;
  clocks+=2;
}

void muls_command(void)
{
  int16_t result;
  
  result=((int8_t)gpr_pointer[list_commands[pc].arg1])*
    ((int8_t)gpr_pointer[list_commands[pc].arg2]);
  gpr_pointer[0]=(uint8_t)result;
  gpr_pointer[1]=(uint8_t)(result>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[0]|=0x03;
  
  /* check Flag Z */
  if(!result) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);
    
  /* check Flag C*/
  if(result&0x8000) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);
  
  pc++;
  clocks+=2;
}

void mulsu_command(void)
{
  int16_t result;
  
  result=((int8_t)gpr_pointer[list_commands[pc].arg1])*
    (gpr_pointer[list_commands[pc].arg2]);
  gpr_pointer[0]=(uint8_t)result;
  gpr_pointer[1]=(uint8_t)(result>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[0]|=0x03;
  
  /* check Flag Z */
  if(!result) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);

  /* check Flag C*/
  if(result&0x8000) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);
  
  pc++;
  clocks+=2;
}

void neg_command(void)
{
  uint8_t old;
  
  old=gpr_pointer[list_commands[pc].arg1];
  gpr_pointer[list_commands[pc].arg1]=0-old;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  /* check Flag C */
  if(gpr_pointer[list_commands[pc].arg1]) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);

  /* check Flag V */
  if(gpr_pointer[list_commands[pc].arg1]==0x80) set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);

  /* check Flag H */
  if((old&0x08)!=(gpr_pointer[list_commands[pc].arg1]&0x08))
    set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);

  pc++;
  clocks++; 
}

void nop_command(void)
{
  pc++;
  clocks++;
}

void or_command(void)
{
  gpr_pointer[list_commands[pc].arg1]|=gpr_pointer[list_commands[pc].arg2];
  
  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);

  clear_sreg_flag(FLAG_V);

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);
  
  pc++;
  clocks++;
}

void ori_command(void)
{
  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg1]|
    list_commands[pc].arg2;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);

  clear_sreg_flag(FLAG_V);

  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);
  
  pc++;
  clocks++;
}

void out_command(void)
{
  int n,address;
  IO_port *port=NULL;
  
  
  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(list_commands[pc].arg1==controllers[type_micro].io_registers[n].address)
      {port=&controllers[type_micro].io_registers[n];break;}

  if(port==NULL)
    {
      fprintf(stderr,"Port not defined, address: 0x%X\n",
	      list_commands[pc].arg1);
      pc++;
      clocks++;
      return;
    }

  for(n=0;port16_wr[n].code!=-1;n++)
    if(port16_wr[n].code==port->code) break;
  
  if(port16_wr[n].code==-1)
    {
      io_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg2]&
	port->write_mask;
      address=list_commands[pc].arg1+controllers[type_micro].size_gpr;
      mem_file_changed[address/8]=1<<address%8;
    }
  else
    {
      switch(port16_wr[n].type){
      case LOW:
	set_port16(port16_wr[n].code,port16_wr[n-1].code,
		   gpr_pointer[list_commands[pc].arg2]|*port16_wr[n].temp<<8);
	break;
      case HIGH: *port16_wr[n].temp=gpr_pointer[list_commands[pc].arg2];
      }
    }
  pc++;
  clocks++;
}

void pop_command(void)
{
  uint16_t sp;
  
  if(!type_micro)
    {
      puts("This command doesn't support by this device.");
      return;
    } 
  if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
  else sp=get_port(SPL);
  /*  if(sp<0) {fprintf(stderr,"Error: Stack Pointer not set\n");} */
  sp++;
  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[sp];
  if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
  else set_port(SPL,sp);  

  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  pc++;
  clocks+=2;
}

void push_command(void)
{
  uint16_t sp;
  
  if(!type_micro)
    {
      puts("This command doesn't support by this device.");
      return;
    } 
  if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
  else sp=get_port(SPL);
  if(sp<1) {fprintf(stderr,"Error: Stack Pointer not set\n");}
  gpr_pointer[sp]=gpr_pointer[list_commands[pc].arg1];
  sp--;
  if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
  else set_port(SPL,sp);  
  mem_file_changed[sp/8]|=1<<sp%8;

  pc++;
  clocks+=2;
}

void rcall_command(void)
{
  uint16_t sp;
  
  if(type_micro==0)
    {
      stack_90s1200[2]=stack_90s1200[1];
      stack_90s1200[1]=stack_90s1200[0];
      stack_90s1200[0]=pc+1;
    }
  else
    {
      if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
      else sp=get_port(SPL);
      if(sp<2) 
	{
	  fprintf(stderr,"Error: Stack Pointer not set\n");
	  return;
	}
      pc++;
      gpr_pointer[sp]=(uint8_t)(pc>>8);
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      gpr_pointer[sp]=(uint8_t)pc;
      mem_file_changed[sp/8]|=1<<sp%8;
      sp--;
      if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
      else set_port(SPL,sp);       
    }
  pc+=list_commands[pc].arg1+1;
  clocks+=3;
}

/* need to change for work on 22 bit PC */
void ret_command(void)
{
  uint16_t sp;

  if(type_micro==0)
    {
      pc=stack_90s1200[0];
      stack_90s1200[0]=stack_90s1200[1];
      stack_90s1200[1]=stack_90s1200[2];
    }
  else
    {
      if(ISPORT(SPH)) sp=get_port16(SPL,SPH);
      else sp=get_port(SPL);
 /*       if(sp<2) {fprintf(stderr,"Error: Stack Pointer not set\n");} */
      sp++;
      pc=gpr_pointer[sp];
      sp++;
      pc+=gpr_pointer[sp]<<8;
      if(ISPORT(SPH)) set_port16(SPL,SPH,sp);
      else set_port(SPL,sp);       
    }
  clocks+=4;
}

void reti_command(void)
{
  ret_command();
  set_sreg_flag(FLAG_I);
}

void rjmp_command(void)
{
  pc=list_commands[pc].arg1+pc+1;
  clocks+=2;
}

void ror_command(void)
{
  int flag;

  flag=get_sreg_flag(FLAG_C);
  if(gpr_pointer[list_commands[pc].arg1]&0x01) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);
    
  gpr_pointer[list_commands[pc].arg1]=gpr_pointer[list_commands[pc].arg1]>>1;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  if(flag) gpr_pointer[list_commands[pc].arg1]|=0x80;
  else gpr_pointer[list_commands[pc].arg1]&=0x7F;
  pc++;
  clocks++;
}

void sbc_command(void)
{
  unsigned char arg1,arg2; 
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=gpr_pointer[list_commands[pc].arg2];
  gpr_pointer[list_commands[pc].arg1]-=arg2;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  if(get_sreg_flag(FLAG_C)) gpr_pointer[list_commands[pc].arg1]--;
 
  if(gpr_pointer[list_commands[pc].arg1]) clear_sreg_flag(FLAG_Z); 
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|
     (~(arg1&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80))|
     ((gpr_pointer[list_commands[pc].arg1]&0x80)&(arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);  
  
  /* check flag N */
  if(gpr_pointer[list_commands[pc].arg1]&0x80)
    set_sreg_flag(FLAG_N);
  else clear_sreg_flag(FLAG_N);  
  
  /* check flag H */
  if((~(arg1&0x08)&(arg2&0x08))|
     ((arg2&0x08)&(gpr_pointer[list_commands[pc].arg1]&0x8))|
     ((gpr_pointer[list_commands[pc].arg1]&0x08)&(arg1&0x08)))
    set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);  
  
  /* check flag V */
  if(((arg1&0x80)&~(arg2&0x80)&(~(gpr_pointer[list_commands[pc].arg1]&0x80)))|
     ((~(arg1&0x80))&(arg2&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);  
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  

  pc++;
  clocks++;
}

void sbci_command(void)
{
  unsigned char arg1,arg2; 
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=list_commands[pc].arg2;
  gpr_pointer[list_commands[pc].arg1]-=arg2;

  /* set bit, it means that field's value will be update */
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  if(get_sreg_flag(FLAG_C)) gpr_pointer[list_commands[pc].arg1]--;
 
  if(gpr_pointer[list_commands[pc].arg1]) clear_sreg_flag(FLAG_Z);
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|
     (~(arg1&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80))|
     ((gpr_pointer[list_commands[pc].arg1]&0x80)&(arg2&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);  

  /* check flag N */
  if(gpr_pointer[list_commands[pc].arg1]&0x80) set_sreg_flag(FLAG_N);
  else clear_sreg_flag(FLAG_N);  
  
  /* check flag H */
  if((~(arg1&0x08)&(arg2&0x08))|
     ((arg2&0x08)&(gpr_pointer[list_commands[pc].arg1]&0x8))|
     ((gpr_pointer[list_commands[pc].arg1]&0x08)&(arg1&0x08)))
    set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);  

  /* check flag V */
  if(((arg1&0x80)&~(arg2&0x80)&(~(gpr_pointer[list_commands[pc].arg1]&0x80)))|
     ((~(arg1&0x80))&(arg2&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  

  pc++;
  clocks++;  
}

void sbi_command(void)
{
  int n,address;

  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(list_commands[pc].arg1==controllers[type_micro].io_registers[n].address)
      {
	io_pointer[list_commands[pc].arg1]=
	  (io_pointer[list_commands[pc].arg1]|(0x1<<list_commands[pc].arg2))&
	  controllers[type_micro].io_registers[n].write_mask;
	break;
      }
  address=list_commands[pc].arg1+controllers[type_micro].size_gpr;
  mem_file_changed[address/8]|=1<<address%8;
  
  pc++;
  clocks+=2;
}

void sbic_command(void)
{
  if(io_pointer[list_commands[pc].arg1]&(1<<list_commands[pc].arg2))
    {
      pc++;
      clocks++; 
    }
  else
    {
      clocks+=list_commands[pc+1].num_words+1;
      pc+=list_commands[pc+1].num_words+1;
    }
  
}

void sbis_command(void)
{
  if(io_pointer[list_commands[pc].arg1]&(1<<list_commands[pc].arg2))
    {
      clocks+=list_commands[pc+1].num_words+1;
      pc+=list_commands[pc+1].num_words+1;
    }
  else
    {
      pc++;
      clocks++; 
    } 
}

void sbiw_command(void)
{
  uint16_t reg,old;

  reg=gpr_pointer[list_commands[pc].arg1];
  reg+=gpr_pointer[list_commands[pc].arg1+1]<<8;
  old=reg;
  reg-=list_commands[pc].arg2;
  gpr_pointer[list_commands[pc].arg1]=(uint8_t)reg;
  gpr_pointer[list_commands[pc].arg1+1]=(uint8_t)(reg>>8);

  /* set bit, it means that field's value will be update */
  mem_file_changed[0]|=0x03;
  
  /* check Flag Z */
  if(!reg) set_sreg_flag(FLAG_Z);
  else clear_sreg_flag(FLAG_Z);  
   

  /* check Flag N */
  if(reg&0x8000) set_sreg_flag(FLAG_N);
  else clear_sreg_flag(FLAG_N);  
    
  
  /* check Flag C */
  if((reg&0x8000)&&(~(old&0x8000))) set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);  
    

  /* check flag V */
  if((old&0x8000)&&(~(reg&0x8000))) set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);  
    
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  

  pc++;
  clocks+=2;
}

void sbrc_command(void)
{
  if(gpr_pointer[list_commands[pc].arg1]&(1<<list_commands[pc].arg2))
    {
      pc++;
      clocks++;
    }
  else
    {
      clocks+=list_commands[pc+1].num_words+1;
      pc+=list_commands[pc+1].num_words+1;
    }
}

void sbrs_command(void)
{
  if(gpr_pointer[list_commands[pc].arg1]&(1<<list_commands[pc].arg2))
    {
      clocks+=list_commands[pc+1].num_words+1;
      pc+=list_commands[pc+1].num_words+1;
    }
  else
    {
      pc++;
      clocks++;
    } 
}

void sleep_command(void)
{
  pc++;
  clocks++;
}

void st_x_command(void)
{
  uint16_t x;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  x=gpr_pointer[26];
  if(size>0xFF) x+=gpr_pointer[27]<<8;

  /* set bit, it means that field's value will be update */
  mem_file_changed[x/8]|=1<<x%8;
  
  if(x>size) {fprintf(stderr,"Error: X points out of memory\n");return;}
  
  if(x<controllers[type_micro].size_gpr||
     (x>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[x]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      x-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(x==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[x]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
    
  pc++;
  clocks+=2;
}

void st_x_plus_command(void)
{
  uint16_t x;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  x=gpr_pointer[26];
  if(size>0xFF) x+=gpr_pointer[27]<<8;
  
  if(x>size) {fprintf(stderr,"Error: X points out of memory\n");return;}
  
  if(x<controllers[type_micro].size_gpr||
     (x>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[x]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      x-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(x==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[x]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  x++;
  gpr_pointer[26]=(uint8_t)x;
  gpr_pointer[27]=(uint8_t)(x>>8);

  mem_file_changed[26/8]|=(1<<26%8)|(1<<27%8);
  mem_file_changed[x/8]|=1<<x%8;
  pc++;
  clocks+=2;  
}

void st_minus_x_command(void)
{
  uint16_t x;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  x=gpr_pointer[26];
  if(size>0xFF) x+=gpr_pointer[27]<<8;
  x--;

  if(x>size) {fprintf(stderr,"Error: X points out of memory\n");return;}
  
  if(x<controllers[type_micro].size_gpr||
     (x>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[x]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      x-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(x==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[x]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  gpr_pointer[26]=(uint8_t)x;
  gpr_pointer[27]=(uint8_t)(x>>8);

  mem_file_changed[26/8]|=(1<<26%8)|(1<<27%8);
  mem_file_changed[x/8]|=1<<x%8;
  pc++;
  clocks+=2; 
}


void st_y_command(void)
{
  uint16_t y;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  y=gpr_pointer[28];
  if(size>0xFF) y+=gpr_pointer[29]<<8;

  /* set bit, it means that field's value will be update */
  mem_file_changed[y/8]|=1<<y%8;
  
  if(y>size) {fprintf(stderr,"Error: Y points out of memory\n");return;}
  
  if(y<controllers[type_micro].size_gpr||
     (y>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[y]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      y-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(y==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[y]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
    
  pc++;
  clocks+=2;
}

void st_y_plus_command(void)
{
  uint16_t y;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  y=gpr_pointer[28];
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  
  if(y>size) {fprintf(stderr,"Error: Y points out of memory\n");return;}
  
  if(y<controllers[type_micro].size_gpr||
     (y>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[y]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      y-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(y==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[y]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  y++;
  gpr_pointer[28]=(uint8_t)y;
  gpr_pointer[29]=(uint8_t)(y>>8);

  mem_file_changed[28/8]|=(1<<28%8)|(1<<29%8);
  mem_file_changed[y/8]|=1<<y%8;
  pc++;
  clocks+=2;
}

void st_minus_y_command(void)
{
  uint16_t y;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  y=gpr_pointer[28];
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  y--;

  if(y>size) {fprintf(stderr,"Error: Y points out of memory\n");return;}
  
  if(y<controllers[type_micro].size_gpr||
     (y>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[y]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      y-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(y==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[y]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  gpr_pointer[28]=(uint8_t)y;
  gpr_pointer[29]=(uint8_t)(y>>8);

  mem_file_changed[28/8]|=(1<<28%8)|(1<<29%8);
  mem_file_changed[y/8]|=1<<y%8;
  pc++;
  clocks+=2;
}

void std_y_commad(void)
{
  uint16_t y;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  y=gpr_pointer[28];
  if(size>0xFF) y+=gpr_pointer[29]<<8;
  y+=list_commands[pc].arg1;
  
 if(y>size) {fprintf(stderr,"Error: X points out of memory\n");return;}
  
  if(y<controllers[type_micro].size_gpr||
     (y>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[y]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      y-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(y==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[y]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  mem_file_changed[y/8]|=1<<y%8;
  pc++;
  clocks+=2;
}

void st_z_command(void)
{
  uint16_t z;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  z=gpr_pointer[30];
  if(size>0xFF) z+=gpr_pointer[31]<<8;
  if(z>size) {fprintf(stderr,"Error: Z points out of memory\n");return;}
  
  if(z<controllers[type_micro].size_gpr||
     (z>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[z]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      z-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(z==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[z]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }

  mem_file_changed[z/8]|=1<<z%8;
  pc++;
  clocks+=2;
}

void st_z_plus_command(void)
{
  uint16_t z;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  z=gpr_pointer[30];
  if(size>0xFF) z+=gpr_pointer[31]<<8;
  
  if(z>size) {fprintf(stderr,"Error: Z points out of memory\n");return;}
  
  if(z<controllers[type_micro].size_gpr||
     (z>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[z]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      z-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(z==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[z]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  z++;
  gpr_pointer[30]=(uint8_t)z;
  gpr_pointer[31]=(uint8_t)(z>>8);

  mem_file_changed[30/8]|=(1<<30%8)|(1<<31%8);
  mem_file_changed[z/8]|=1<<z%8;
  pc++;
  clocks+=2;
}

void st_minus_z_command(void)
{
  
  uint16_t z;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  z=gpr_pointer[30];
  if(size>0xFF) z+=gpr_pointer[31]<<8;
  z--;

  if(z>size) {fprintf(stderr,"Error: Z points out of memory\n");return;}
  
  if(z<controllers[type_micro].size_gpr||
     (z>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[z]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      z-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(z==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[z]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }
  gpr_pointer[30]=(uint8_t)z;
  gpr_pointer[31]=(uint8_t)(z>>8);

  mem_file_changed[30/8]|=(1<<30%8)|(1<<31%8);
  mem_file_changed[z/8]|=1<<z%8;
  pc++;
  clocks+=2; 
}

void std_z_commad(void)
{
  uint16_t z;
  int n,size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  
  z=gpr_pointer[30];
  if(size>0xFF) z+=gpr_pointer[31]<<8;
  z+=list_commands[pc].arg1;

  if(z>size) {fprintf(stderr,"Error: Z points out of memory\n");return;}
  
  if(z<controllers[type_micro].size_gpr||
     (z>=controllers[type_micro].size_gpr+controllers[type_micro].size_io_reg))
    gpr_pointer[z]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      z-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(z==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[z]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
      if(controllers[type_micro].io_registers[n].code==-1)
	fprintf(stderr,"Error: Port doesn't exist\n");
    }

  mem_file_changed[z/8]|=1<<z%8;
  pc++;
  clocks+=2;
}

void sts_command(void)
{
  uint16_t addr,n;

  addr=list_commands[pc].arg1;

  mem_file_changed[addr/8]|=1<<addr%8;

  if(addr<controllers[type_micro].size_gpr+
     controllers[type_micro].size_io_reg+controllers[type_micro].size_sram)
    {fprintf(stderr,"Error: out of memory\n");return;}

  if(addr<controllers[type_micro].size_gpr||
     (addr>=controllers[type_micro].size_gpr+
      controllers[type_micro].size_io_reg))
    gpr_pointer[addr]=gpr_pointer[list_commands[pc].arg2];
  else
    {
      addr-=controllers[type_micro].size_gpr;
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
	if(addr==controllers[type_micro].io_registers[n].address)
	  {
	    io_pointer[addr]=gpr_pointer[list_commands[pc].arg2]&
	      controllers[type_micro].io_registers[n].write_mask;
	    break;
	  }
    }
  
  pc+=2;
  clocks+=2;  
}

void sub_command(void)
{
  unsigned char arg1,arg2; 
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=gpr_pointer[list_commands[pc].arg2];
  gpr_pointer[list_commands[pc].arg1]-=arg2;
  
  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;

  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|
     ((arg2&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80))|
     ((gpr_pointer[list_commands[pc].arg1]&0x80)&~(arg1&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);    

   /* check flag H */
  if(((~(arg1&0x08))&(arg2&0x08))|
     ((arg2&0x08)&(gpr_pointer[list_commands[pc].arg1]&0x8))|
     ((gpr_pointer[list_commands[pc].arg1]&0x08)&(~(arg1&0x08))))
     set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);  

  /* check flag V */
  if(((arg1&0x80)&(~(arg2&0x80))&
      (~(gpr_pointer[list_commands[pc].arg1]&0x80)))|
     ((~(arg1&0x80))&(arg2&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);  
  
  /* check flag S */
  if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  

  pc++;
  clocks++;
}

void subi_command(void)
{
  unsigned char arg1,arg2; 
  
  arg1=gpr_pointer[list_commands[pc].arg1];
  arg2=list_commands[pc].arg2;
  gpr_pointer[list_commands[pc].arg1]-=arg2;

  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  check_Z_N_flags(gpr_pointer[list_commands[pc].arg1]);
  
  /* check flag C */
  if((~(arg1&0x80)&(arg2&0x80))|
     ((arg2&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80))|
     ((gpr_pointer[list_commands[pc].arg1]&0x80)&~(arg1&0x80)))
    set_sreg_flag(FLAG_C);
  else clear_sreg_flag(FLAG_C);

   /* check flag H */
  if(((~(arg1&0x08))&(arg2&0x08))|
     ((arg2&0x08)&(gpr_pointer[list_commands[pc].arg1]&0x8))|
     ((gpr_pointer[list_commands[pc].arg1]&0x08)&(~(arg1&0x08))))
    set_sreg_flag(FLAG_H);
  else clear_sreg_flag(FLAG_H);  

  /* check flag V */
  if(((arg1&0x80)&(~(arg2&0x80))&
      (~(gpr_pointer[list_commands[pc].arg1]&0x80)))|
     ((~(arg1&0x80))&(arg2&0x80)&(gpr_pointer[list_commands[pc].arg1]&0x80)))
    set_sreg_flag(FLAG_V);
  else clear_sreg_flag(FLAG_V);  
  
  /* check flag S */
 if(get_sreg_flag(FLAG_N)^get_sreg_flag(FLAG_V)) set_sreg_flag(FLAG_S);
  else clear_sreg_flag(FLAG_S);  
  pc++;
  clocks++;
}

void swap_command(void)
{
  
  gpr_pointer[list_commands[pc].arg1]=(gpr_pointer[list_commands[pc].arg1]<<4)+
    (gpr_pointer[list_commands[pc].arg1]>>4);

  mem_file_changed[list_commands[pc].arg1/8]|=1<<list_commands[pc].arg1%8;
  
  clocks++;
  pc++;
}

void wdr_command(void)
{
  watchdog_prescaler=0;
  clocks++;
  pc++;
}


int reg_d5(char * mnemocode,int command,int address)
/* 0000000d dddd0000 destantion  r0-r31*/
{

  command=(command&0x01F0)>>4;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int reg_s5(char * mnemocode,int command,int address)
/* 000000d0 0000dddd source  r0-r31    */
{
  command=((command&0x0200)>>5)|(command&0x000F);
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
} 

int const_6(char * mnemocode,int command,int address)
/* 00000000 dd00dddd const 0-63  */
{
  command=((command&0x00C0)>>2)|(command&0x000F);
  sprintf(&mnemocode[strlen(mnemocode)],"%Xh",command);
  return command;
}

int bit_sreg(char * mnemocode,int command,int address)
/* 00000000 0ddd0000  */
{
  char bit;

  command=(command&0x0070)>>4;
  switch(command)
    {
    case 7: bit='I'; break;
    case 6: bit='T'; break;
    case 5: bit='H'; break;
    case 4: bit='S'; break;
    case 3: bit='V'; break;
    case 2: bit='N'; break;
    case 1: bit='Z'; break;
    case 0: bit='C'; break;
    }
  sprintf(&mnemocode[strlen(mnemocode)],"'%c'",bit);
  return command;
}

int bit_sreg2(char * mnemocode,int command,int address)
/* 00000000 00000ddd bld         */
{
  char bit;

  command=command&7;
  switch(command)
    {
    case 7: bit='I'; break;
    case 6: bit='T'; break;
    case 5: bit='H'; break;
    case 4: bit='S'; break;
    case 3: bit='V'; break;
    case 2: bit='N'; break;
    case 1: bit='Z'; break;
    case 0: bit='C'; break;
    }
  sprintf(&mnemocode[strlen(mnemocode)],"'%c'",bit);
  return command;
}

int bit_num2(char * mnemocode,int command,int address)
/* 00000000 00000ddd 0-7         */
{

  command=command&7;
  sprintf(&mnemocode[strlen(mnemocode)],"%d",command);
  return command;
}

int reg_d4(char * mnemocode,int command,int address)
/* 00000000 dddd0000 destantion r16-r31 */
{
  command=((command&0x00F0)>>4)|0x0010;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int reg_s4(char * mnemocode,int command,int address)
/* 00000000 0000dddd source r16-r31     */
{
  command=(command&0x000F)|0x0010;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int reg_s3(char * mnemocode,int command,int address)
/* 00000000 00000ddd r16-r23      */
{
  command=(command&0x0007)+16;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int reg_d3(char * mnemocode,int command,int address)
/* 00000000 0ddd0000 r16-r23      */
{
  command=((command&0x0070)>>4)+16;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int none(char * mnemocode,int command,int address)
{
  return 0;
}

int none_last(char * mnemocode,int command,int address)
{
  mnemocode[strlen(mnemocode)-1]=0;
  return 0;
}

int addr_shift(char * mnemocode,int command,int address)
/* 000000dd ddddd000             */
{
  if(command&0x0200) command=((command&0x03F8)>>3)|(-1<<7);
  else command=(command&0x03F8)>>3;
  command+=address+1;
  sprintf(&mnemocode[strlen(mnemocode)],"%Xh",command);
  return command;
}

int port(char * mnemocode,int command,int address)
/* 00000000 ddddd000 port I/O low*/
{
  int n;
  IO_port *port=NULL;

  command=(command&0x00F8)>>3;
  
  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(command==controllers[type_micro].io_registers[n].address)
      {port=&controllers[type_micro].io_registers[n];break;}
  
  if(port==NULL)
    sprintf(&mnemocode[strlen(mnemocode)],"%Xh",command);
  else strcat(mnemocode,descr_io_ports[port->code].name);
  return command;
}

int port_all(char * mnemocode,int command,int address)
/* 00000dd0 0000dddd port I/O all*/
{
  int n;
  IO_port *port=NULL;

  command=((command&0x0600)>>5)|(command&0x000F);
  
  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    if(command==controllers[type_micro].io_registers[n].address)
      {port=&controllers[type_micro].io_registers[n];break;}
  
  if(port==NULL)
    sprintf(&mnemocode[strlen(mnemocode)],"%Xh",command);
  else strcat(mnemocode,descr_io_ports[port->code].name);
  return command;
}

unsigned char byte(char * mnemocode,int command,int address)
/* 0000dddd 0000dddd             */
{
  command=((command&0x0F00)>>4)|(command&0x000F);
  sprintf(&mnemocode[strlen(mnemocode)],"%Xh",command);
  return (unsigned char)command;
}

int indir_addr(char * mnemocode,int command,int address)
/* 00000000 000000dd X,X+,-X           */
{
  char c;
  switch(command&0x000C)
    {
    case 0xC: c='X'; break;
    case 0x8: c='Y'; break;
    case 0x0: c='Z'; break;
    case 0x4: c='Z'; break;
    }
  command=command&0x0003;
  switch(command)
    {
      case 0:sprintf(&mnemocode[strlen(mnemocode)],"%c",c);
      break;
      case 1:sprintf(&mnemocode[strlen(mnemocode)],"%c+",c);
      break;
      case 2:sprintf(&mnemocode[strlen(mnemocode)],"-%c",c);
      break;
    }
  return command;
}

int indir_shift(char * mnemocode,int command,int address)
/* 00d0dd00 00000ddd 0-63         */
{
  switch(command&0x0008)
    {
    case 8: sprintf(&mnemocode[strlen(mnemocode)],"Y+");break;
    case 0: sprintf(&mnemocode[strlen(mnemocode)],"Z+");break;
    }
  command=((command&0x2000)>>8)|((command&0x0C00)>>7)|(command&0x0007);
  sprintf(&mnemocode[strlen(mnemocode)],"%Xh",command);
  return command;
}

int addr_shift12(char *mnemocode,int command,int address)
/* 0000dddd dddddddd rcall,rjmp  */
{
  int buf,addr;

  if(command&0x0800) buf=command|(-1^0xFFF); 
  else buf=command&0x0FFF;
  /*printf("\ncom: %X\t%X\t%X\n",command,buf,address);*/ /* test line*/
  addr=buf+address+1;
  sprintf(&mnemocode[strlen(mnemocode)],"%Xh",addr);
  return buf;
}

int reg_d2(char * mnemocode,int command,int address)
/* 00000000 00dd0000 r24,r26,r28,r30 */
{
  command=((command&0x0030)>>3)+24;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int put_z(char *mnemocode,int command,int address)
{
  sprintf(&mnemocode[strlen(mnemocode)],"Z");
  return 0;
}

int put_zplus(char *mnemocode,int command,int address)
{
  sprintf(&mnemocode[strlen(mnemocode)],"Z+");
  return 0;
}

int word_d(char *mnemocode,int command,int address)
/* 00000000 dddd0000 r0,r2,...,r30*/
{
  command=(command&0x00F0)>>3;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int word_s(char *mnemocode,int command,int address)
/* 00000000 0000dddd r0,r2,...,r30*/
{
  command=(command&0x000F)<<1;
  sprintf(&mnemocode[strlen(mnemocode)],"r%d",command);
  return command;
}

int addr22(char *mnemocode,int command,int address)
/* use in long call and long jmp to get address */
{
  char buf[10];

  sprintf(buf,"%X",list_commands[address+1].code);
  fill_zero(buf,5);
  strcat(mnemocode,buf);
  return list_commands[address+1].code;
}

int word(char *mnemocode,int command,int address)
/* dddd dddd dddd dddd */
{
  char buf[10];

  sprintf(buf,"%X",command);
  fill_zero(buf,4);
  strcat(mnemocode,buf);
  return command;
}
