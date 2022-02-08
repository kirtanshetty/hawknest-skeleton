#include <mos6502/vmcall.h>
#include <mos6502/mos6502.h>
#include <mos6502/mos6502-instr.h>

/* --------------------- Helper Functions --------------------- */

mos6502_step_result_t __sta_index_write(mos6502_t * cpu, uint8_t index){
  uint16_t addr = 0;

  addr = read16(cpu, cpu->pc + 1);
  membus_write(cpu->bus, addr + index, cpu->a);

  // DEBUG_PRINT("--> cpu->pc %x \n", 3);
  cpu->pc += 3;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __st_basic(mos6502_t * cpu, uint8_t* reg){
  uint16_t addr = 0;

  addr = read16(cpu, cpu->pc + 1);
  // DEBUG_PRINT("--> reg %x \n", *reg);
  membus_write(cpu->bus, addr, *reg);

  // DEBUG_PRINT("--> cpu->pc %x \n", 3);

  uint8_t temp = membus_read(cpu->bus, addr);

  // DEBUG_PRINT("--> membus_read(cpu->bus, addr) %x \n", temp);
  cpu->pc += 3;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __ld_basic(mos6502_t * cpu, uint8_t* reg){
  *reg = read8(cpu, cpu->pc + 1);

  // set the zero flag when the result is 0
  if(!*reg) cpu->p.z = 1;
  else cpu->p.z = 0;

  // set the negative flag if the value is negative
  if(*reg & MSB_VAL) cpu->p.n = 1;
  else cpu->p.n = 0;

  // DEBUG_PRINT("--> cpu->pc %x \n", 2);
  cpu->pc += 2;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __transfer_reg(mos6502_t * cpu, uint8_t* src, uint8_t* des){
  *des = *src;

  cpu->pc += 1;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __reg_inc(mos6502_t * cpu, uint8_t * reg) {
  *reg += 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __reg_dec(mos6502_t * cpu, uint8_t * reg) {
  *reg -= 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

/* --------------------- ISA Instruction Set --------------------- */

mos6502_step_result_t mos6502_ora_0x9(mos6502_t * cpu){
  cpu->a |= read8(cpu, cpu->pc + 1);
  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_asl_0xA(mos6502_t * cpu){
  cpu->a = cpu->a << 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_ora_0xD(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint8_t temp = membus_read(cpu->bus, addr);

  DEBUG_PRINT("--> addr %x \n", addr);
  DEBUG_PRINT("--> cpu->a %x \n", cpu->a);
  DEBUG_PRINT("--> temp %x \n", temp);
  DEBUG_PRINT("--> temp | cpu->a %x \n", temp | cpu->a);

  cpu->a |= temp;
  // membus_write(cpu->bus, addr, temp | cpu->a);

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_asl_0xE(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t temp = membus_read(cpu->bus, addr);

  if(temp & MSB_VAL)
    cpu->p.c = 1;
  else
    cpu->p.c = 0;

  membus_write(cpu->bus, addr, temp << 1);
  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_clc_0x18(mos6502_t * cpu){
  cpu->p.c = 0;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_and_0x25(mos6502_t * cpu){
  uint16_t addr = read8(cpu, cpu->pc + 1);

  // DEBUG_PRINT("--> addr %x \n", addr);
  // DEBUG_PRINT("--> cpu->a %x \n", cpu->a);
  // DEBUG_PRINT("--> membus_read(cpu->bus, addr) %x \n", membus_read(cpu->bus, addr));

  cpu->a &= membus_read(cpu->bus, addr);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_and_0x29(mos6502_t * cpu){
  cpu->a &= read8(cpu, cpu->pc + 1);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_and_0x2d(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);

  DEBUG_PRINT("--> addr %x \n", addr);

  cpu->a &= membus_read(cpu->bus, addr);
  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sec_0x38(mos6502_t * cpu){
  cpu->p.c = 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_pha_0x48(mos6502_t * cpu){
  DEBUG_PRINT("--> cpu->sp %u \n", cpu->sp);
  // op1 = cpu->a;

  DEBUG_PRINT("--> cpu->sp %x \n", cpu->sp);
  DEBUG_PRINT("--> cpu->a %x \n", cpu->a);
  // DEBUG_PRINT("--> addr %x \n", addr);
  // membus_write(cpu->bus, 0x0100 | cpu->sp, op1);
  membus_write(cpu->bus, STACK_START | cpu->sp, cpu->a);
  // membus_set_write_handler(cpu->bus, 0, void * obj, size_t offset, void * handler);

  cpu->sp -= 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_adc_0x6d(mos6502_t * cpu){
  uint16_t addr = 0;
  uint16_t temp = 0;

  addr = read16(cpu, cpu->pc + 1);

  DEBUG_PRINT("adc: addr %x \n", addr);
  DEBUG_PRINT("adc: cpu->bus %p \n", cpu->bus);
  // DEBUG_PRINT("adc: membus_read(cpu->bus, addr) %x \n", membus_read(cpu->bus, addr));
  DEBUG_PRINT("adc: cpu->a %x \n", cpu->a);

  temp = membus_read(cpu->bus, addr) + cpu->a;

  DEBUG_PRINT("adc: temp %x \n", temp);

  if(temp & 0x100){
   cpu->p.c = 1;
  }
  else {
   cpu->p.c = 0;
  }

  DEBUG_PRINT("adc: temp %x \n", temp);

  cpu->a = membus_read(cpu->bus, addr) + cpu->a; //cpu->p.c

  // set the negative flag if the value is negative
  if(cpu->a & MSB_VAL) cpu->p.n = 1;
  else cpu->p.n = 0;

  // // set the overflow flag if the value is negative
  // if(cpu->a & MSB_VAL) cpu->p.v = 1;
  // else cpu->p.v = 0;

  // set the zero flag when the result is 0
  if(!cpu->a) cpu->p.z = 1;
  else cpu->p.z = 0;

  DEBUG_PRINT("adc: 2cpu->a %x \n", cpu->a);
  DEBUG_PRINT("2 cpu->a %x \n", cpu->a);
  DEBUG_PRINT("2 cpu->p %x \n", cpu->p);

  DEBUG_PRINT("--> cpu->pc %x \n", 3);
  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sei_0x78(mos6502_t * cpu){
  cpu->p.i = 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sty_0x84(mos6502_t * cpu){
  uint16_t addr = 0;

  addr = read8(cpu, cpu->pc + 1);
  membus_write(cpu->bus, addr, cpu->y);

  DEBUG_PRINT("--> cpu->pc %x \n", 2);
  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sta_0x85(mos6502_t * cpu){
  uint16_t addr = 0;

  addr = read8(cpu, cpu->pc + 1);
  membus_write(cpu->bus, addr, cpu->a);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_dey_0x88(mos6502_t * cpu){
  return __reg_dec(cpu, &cpu->y);
}

mos6502_step_result_t mos6502_txa_0x8a(mos6502_t * cpu){
  return __transfer_reg(cpu, &cpu->x, &cpu->a);
}

mos6502_step_result_t mos6502_sty_0x8c(mos6502_t * cpu){
  uint16_t addr = 0;

  addr = read16(cpu, cpu->pc + 1);
  membus_write(cpu->bus, addr, cpu->y);

  DEBUG_PRINT("--> cpu->pc %x \n", 3);
  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sta_0x8d(mos6502_t * cpu){
  return __st_basic(cpu, &cpu->a);
}

mos6502_step_result_t  mos6502_stx_0x8e(mos6502_t * cpu){
  return __st_basic(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_tya_0x98(mos6502_t * cpu){
  return __transfer_reg(cpu, &cpu->y, &cpu->a);
}

mos6502_step_result_t mos6502_sta_0x9d(mos6502_t * cpu){
  return __sta_index_write(cpu, cpu->x);
}

mos6502_step_result_t mos6502_sta_0x99(mos6502_t * cpu){
  return __sta_index_write(cpu, cpu->y);
}

mos6502_step_result_t mos6502_ldy_0xa0(mos6502_t * cpu){
  return __ld_basic(cpu, &cpu->y);
}

mos6502_step_result_t mos6502_ldx_0xa2(mos6502_t * cpu){
  return __ld_basic(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_tay_0xa8(mos6502_t * cpu){
  return __transfer_reg(cpu, &cpu->a, &cpu->y);
}

mos6502_step_result_t mos6502_lda_0xa9(mos6502_t * cpu){
  return __ld_basic(cpu, &cpu->a);
}

mos6502_step_result_t mos6502_tax_0xaa(mos6502_t * cpu){
  return __transfer_reg(cpu, &cpu->a, &cpu->x);
}

mos6502_step_result_t mos6502_iny_0xc8(mos6502_t * cpu){
  return __reg_inc(cpu, &cpu->y);
}

mos6502_step_result_t mos6502_dex_0xca(mos6502_t * cpu){
  return __reg_dec(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_dec_0xce(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t temp = membus_read(cpu->bus, addr);

  membus_write(cpu->bus, addr, temp - 1);

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sed_0xd8(mos6502_t * cpu){
  cpu->p.d = 0;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_inc_0xe6(mos6502_t * cpu){
  uint8_t addr = read8(cpu, cpu->pc + 1);
  uint8_t temp = membus_read(cpu->bus, addr);

  membus_write(cpu->bus, addr, temp + 1);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_inx_0xe8(mos6502_t * cpu){
  return __reg_inc(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_nop_0xea(mos6502_t * cpu){
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_inc_0xee(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t temp = membus_read(cpu->bus, addr);

  membus_write(cpu->bus, addr, temp + 1);

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sed_0xf8(mos6502_t * cpu){
  cpu->p.d = 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_paravirt_0x80(mos6502_t * cpu){
  uint8_t operand1 = 0;

  operand1 = read8(cpu, cpu->pc + 1);
  cpu->pc += 2;
  handle_vmcall(cpu, operand1);

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_default(mos6502_t * cpu){
  return MOS6502_STEP_RESULT_ILLEGAL_INSTRUCTION;
}
