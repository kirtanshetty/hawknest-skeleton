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
  DEBUG_PRINT("__st_basic reg %x \n", *reg);
  membus_write(cpu->bus, addr, *reg);

  // DEBUG_PRINT("--> cpu->pc %x \n", 3);

  // uint8_t temp = membus_read(cpu->bus, addr);

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

  DEBUG_PRINT("__ld_basic *reg %x \n", *reg);
  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __transfer_reg(mos6502_t * cpu, uint8_t* src, uint8_t* des){
  *des = *src;

  cpu->pc += 1;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __reg_inc(mos6502_t * cpu, uint8_t * reg) {
  uint16_t op = *reg;
  op += 1;

  DEBUG_PRINT("*op = %x \n", op);
  DEBUG_PRINT("*reg = %x \n", *reg);

  DEBUG_PRINT("cpu->p = %x \n", cpu->p);

  if(op & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  if(op & 0x0080) cpu->p.n = 1;
  else cpu->p.n = 0;

  // if(op > 0xFF) {
  //   DEBUG_PRINT("setting carry for %x, = %x \n", op, cpu->p);
  //   cpu->p.c = 1;
  // }
  // else {
  //   DEBUG_PRINT("unsetting carry for %x, = %x \n", op, cpu->p);
  //   cpu->p.c = 0;
  // }

  *reg += 1;

  DEBUG_PRINT("op = %x \n", op);
  DEBUG_PRINT("cpu->p = %x \n", cpu->p);
  DEBUG_PRINT("*reg = %x \n", *reg);

  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __reg_dec(mos6502_t * cpu, uint8_t * reg) {
  uint16_t op = *reg;
  op -= 1;

  if(op & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  if(op & 0x0080) cpu->p.n = 1;
  else cpu->p.n = 0;

  *reg -= 1;

  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __cmp_reg_ind(mos6502_t * cpu, uint8_t* reg){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t op = *reg - membus_read(cpu->bus, addr);

  if(op & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  if(op & 0x0080) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(op > 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __cmp_reg_imm(mos6502_t * cpu, uint8_t* reg){
  uint8_t op = read8(cpu, cpu->pc + 1);
  uint16_t tmp = *reg - op;

  DEBUG_PRINT("cmp: tmp %x \n", tmp);

  DEBUG_PRINT("cmp: cpu->p %x \n", cpu->p);

  if(tmp & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  DEBUG_PRINT("cmp: cpu->p %x \n", cpu->p);

  if(*reg >= op) cpu->p.c = 1;
  else cpu->p.c = 0;

  DEBUG_PRINT("cmp: cpu->p %x \n", cpu->p);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t __branch_on_status(mos6502_t * cpu, uint8_t flag){
  uint8_t addr = read16(cpu, cpu->pc + 1);
  uint16_t next_addr = (0x00FF & cpu->pc) + addr;

  if(next_addr > 0xFF)
    next_addr &= 0x00FF;

  next_addr = next_addr + (cpu->pc & 0xFF00);

  DEBUG_PRINT("__branch: cpu->pc %x", cpu->pc);
  DEBUG_PRINT("__branch: addr %x", addr);
  DEBUG_PRINT("__branch: next_addr %x", next_addr);

  if(flag)
    cpu->pc = next_addr + 2;
  else
    cpu->pc += 2;

  DEBUG_PRINT("__branch: cpu->pc %x", cpu->pc);

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

// void mos6502::StackPush(uint8_t byte)
// {
//   Write(0x0100 + sp, byte);
//   if(sp == 0x00) sp = 0xFF;
//   else sp--;
// }

mos6502_step_result_t mos6502_jsr_0x20(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);

  // cpu->pc -= 1;

  DEBUG_PRINT("jsr: b cpu->pc %x \n", cpu->pc);

  membus_write(cpu->bus, 0x0100 + cpu->sp, (cpu->pc >> 8) & 0xFF);
  if(cpu->sp == 0x00) cpu->sp = 0xFF;
  else cpu->sp -= 1;
  membus_write(cpu->bus, 0x0100 + cpu->sp, (cpu->pc + 2) & 0xFF);
  if(cpu->sp == 0x00) cpu->sp = 0xFF;
  else cpu->sp -= 1;


  cpu->pc = addr;

  DEBUG_PRINT("jsr: a cpu->pc %x \n", cpu->pc);

  // cpu->pc += 1;
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

mos6502_step_result_t mos6502_plp_0x28(mos6502_t * cpu){
  cpu->sp += 1;
  cpu->p.val = membus_read(cpu->bus, STACK_START + cpu->sp);

  cpu->pc += 1;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_and_0x29(mos6502_t * cpu){
  cpu->a &= read8(cpu, cpu->pc + 1);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_rol_0x2a(mos6502_t * cpu){
  uint16_t temp = cpu->a;

  DEBUG_PRINT("--> cpu->a %x \n", cpu->a);

  temp <<= 1;

  if (cpu->p.c) temp |= 0x01;

  if(temp > 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;
  temp &= 0xFF;

  if(temp & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(temp) cpu->p.z = 0;
  else cpu->p.z = 1;

  cpu->a = temp;
  // uint8_t temp = 0;
  // temp = cpu->a << 1;

  // if(temp) cpu->a |= 1;
  // else cpu->a |= 0;

  cpu->pc += 1;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_bit_0x2c(mos6502_t * cpu){
  uint16_t op = read16(cpu, cpu->pc + 1);
  uint16_t temp = cpu->a & op;

  cpu->p.z = temp;
  cpu->p.n = op & (1 << 7);
  cpu->p.v = op & (1 << 6);

  cpu->pc += 3;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_and_0x2d(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);

  DEBUG_PRINT("--> addr %x \n", addr);

  cpu->a &= membus_read(cpu->bus, addr);
  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_rol_0x2e(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t temp = membus_read(cpu->bus, addr);

  DEBUG_PRINT("--> cpu->a %x \n", cpu->a);

  temp <<= 1;

  DEBUG_PRINT("--> temp %x \n", temp);

  if (cpu->p.c) temp |= 0x01;

  DEBUG_PRINT("--> temp %x \n", temp);

  if((temp & 0x00FF) >= 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;

  temp &= 0xFF;

  if(temp & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(temp) cpu->p.z = 0;
  else cpu->p.z = 1;

  membus_write(cpu->bus, addr, temp);

  cpu->pc += 3;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sec_0x38(mos6502_t * cpu){
  cpu->p.c = 1;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_lsr_0x46(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint8_t op = membus_read(cpu->bus, addr);

  cpu->p.c = op & 1;
  op >>= 8;

  if((op & 0x00FF) >= 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;

  op &= 0xFF;

  if(op & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(op) cpu->p.z = 0;
  else cpu->p.z = 1;

  // cpu->p.v = ( ~(cpu->a ^ op) ) & (cpu->a^ op) & 0x80;
  // cpu->a = op2;

  // cpu->pc--;
  membus_write(cpu->bus, addr, op);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;

  // uint16_t addr = write_result(cpu, inst.mode);
  // DEBUG_PRINT("addr: %04x, val: %04x", addr, operand);
  // write8(cpu, addr, operand);
}

mos6502_step_result_t mos6502_pha_0x48(mos6502_t * cpu){
  // DEBUG_PRINT("--> cpu->sp %u \n", cpu->sp);
  // // op1 = cpu->a;

  // DEBUG_PRINT("--> cpu->sp %x \n", cpu->sp);
  // DEBUG_PRINT("--> cpu->a %x \n", cpu->a);
  // // DEBUG_PRINT("--> addr %x \n", addr);
  // // membus_write(cpu->bus, 0x0100 | cpu->sp, op1);
  membus_write(cpu->bus, STACK_START + cpu->sp, cpu->a);

  cpu->sp -= 1;
  cpu->pc += 1;
  if(cpu->sp == 0x00) cpu->sp = 0xFF;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_jmp_0x4c(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  DEBUG_PRINT("--> addr %x \n", addr);
  cpu->pc = addr;
  DEBUG_PRINT("--> cpu->pc %x \n", cpu->pc);

  // cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_eor_0x4d(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  // operand = read8(cpu, operand);
  DEBUG_PRINT("cpu->a %x", cpu->y);
  // DEBUG_PRINT("op %x", op);

  cpu->a = cpu->a ^ membus_read(cpu->bus, addr);
  cpu->p.c = (cpu->a == 0);

  if(cpu->a & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_bvc_0x50(mos6502_t * cpu){
  return __branch_on_status(cpu, !cpu->p.v);
}

mos6502_step_result_t mos6502_eor_0x59(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);

  uint8_t op = membus_read(cpu->bus, addr + cpu->y);
  // operand = read8(cpu, operand);
  DEBUG_PRINT("eor: cpu->a %x", cpu->a);
  DEBUG_PRINT("eor: cpu->y %x", cpu->y);
  DEBUG_PRINT("eor: op %x", op);
  // DEBUG_PRINT("op %x", op);
  cpu->a = cpu->a ^ op;

  cpu->p.c = (cpu->a == 0);
  // set_flag(N, cpu.ac & (1 << 7));

  // if(cpu->a) cpu->p.z = 0;
  // else cpu->p.z = 1;

  if(cpu->a & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_rts_0x60(mos6502_t * cpu){
  if(cpu->sp == 0xFF) cpu->sp = 0x00;
  else cpu->sp++;

  uint8_t addr1 = membus_read(cpu->bus, 0x0100 + cpu->sp);

  if(cpu->sp == 0xFF) cpu->sp = 0x00;
  else cpu->sp++;

  uint8_t addr2 = membus_read(cpu->bus, 0x0100 + cpu->sp);
  cpu->pc = ((addr2 << 8) | addr1) + 1;
  DEBUG_PRINT("rt: cpu->sp: %x", cpu->sp);

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_pla_0x68(mos6502_t * cpu){
  cpu->sp += 1;
  cpu->a = membus_read(cpu->bus, STACK_START + cpu->sp);
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_adc_0x69(mos6502_t * cpu){
  uint8_t op2 = read8(cpu, cpu->pc + 1);

  DEBUG_PRINT("adc: op2 %x \n", op2);
  DEBUG_PRINT("adc: cpu->a %x \n", cpu->a);

  uint16_t op = cpu->a + op2 + (cpu->p.c ? 1 : 0);

  DEBUG_PRINT("adc: op %x \n", op);

  cpu->p.v = (~(cpu->a ^ op2)) & (cpu->a ^ op) & 0x80;

  // if(!(cpu->a & temp) & (cpu->a & op) & 0x0080) cpu->p.v = 1;
  // if(!((cpu->a ^ op) & MSB_VAL) && ((cpu->a ^ op) & MSB_VAL)) cpu->p.v = 1;
  // else cpu->p.v = 0;

  if(op & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  // if(!(cpu->a & MSB_VAL) && op & MSB_VAL) cpu->p.n = 1;
  if(op & 0x0080) cpu->p.n = 1;
  else cpu->p.n = 0;

  DEBUG_PRINT("adc: cpu->a %x \n", cpu->a);

  if(op > 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;

  cpu->a = op;

  DEBUG_PRINT("adc: cpu->p %x \n", cpu->p);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_ror_0x6A(mos6502_t * cpu){
  uint16_t temp = cpu->a;

  if(cpu->p.c) temp |= 0x100;

  if(temp & 0x01) cpu->p.c = 1;
  else cpu->p.c = 0;

  temp = temp >> 1;

  temp &= 0xFF;

  if(temp & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(0x00FF & temp) cpu->p.z = 0;
  else cpu->p.z = 1;

  cpu->a = temp;

  cpu->pc += 1;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_ror_0x66(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t temp = membus_read(cpu->bus, addr);

  if(cpu->p.c) temp |= 0x100;

  if(temp & 0x01) cpu->p.c = 1;
  else cpu->p.c = 0;

  temp = temp >> 1;

  temp &= 0xFF;

  if(temp & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(0x00FF & temp) cpu->p.z = 0;
  else cpu->p.z = 1;

  membus_write(cpu->bus, addr, temp);

  cpu->pc += 1;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_jmp_0x6c(mos6502_t * cpu){
  uint16_t indr_addr = read16(cpu, cpu->pc + 1);

  DEBUG_PRINT("--> indr_addr %x \n", indr_addr);
  // uint16_t addr = membus_read(cpu->bus, indr_addr);
  uint8_t addr = membus_read(cpu->bus, indr_addr);
  indr_addr = (indr_addr & 0x00FF) == 0x00FF ? indr_addr & 0xFF00 : indr_addr + 1;
  uint8_t addr2 =  membus_read(cpu->bus, indr_addr);

  DEBUG_PRINT("--> addr1 %x \n", addr);
  DEBUG_PRINT("--> addr2 %x \n", addr2);
  cpu->pc = addr2;
  cpu->pc <<= 8;
  cpu->pc |= addr;
  DEBUG_PRINT("--> cpu->pc %x \n", cpu->pc);

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_adc_0x6d(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint8_t temp = membus_read(cpu->bus, addr);

  DEBUG_PRINT("adc: temp %x \n", temp);
  DEBUG_PRINT("adc: cpu->a %x \n", cpu->a);

  uint16_t op = cpu->a + temp + (cpu->p.c ? 1 : 0);

  DEBUG_PRINT("adc: op %x \n", op);

  // if(!(cpu->a & temp) & (cpu->a & op) & 0x0080) cpu->p.v = 1;
  if(!((cpu->a ^ temp) & MSB_VAL) && ((cpu->a ^ op) & MSB_VAL)) cpu->p.v = 1;
  else cpu->p.v = 0;

  if(op & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  // if(!(cpu->a & MSB_VAL) && op & MSB_VAL) cpu->p.n = 1;
  if(op & 0x0080) cpu->p.n = 1;
  else cpu->p.n = 0;

  DEBUG_PRINT("adc: cpu->a %x \n", cpu->a);

  if(op > 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;

  cpu->a = op;

  DEBUG_PRINT("adc: cpu->p %x \n", cpu->p);

  cpu->pc += 3;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_bvs_0x70(mos6502_t * cpu) {
  __branch_on_status(cpu, cpu->p.v);
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
  DEBUG_PRINT("sta_0x85 %x", cpu->a);
  membus_write(cpu->bus, addr, cpu->a);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_stx_0x86(mos6502_t * cpu){
  uint8_t addr = read8(cpu, cpu->pc + 1);

  // DEBUG_PRINT("__st_basic reg %x \n", *reg);

  membus_write(cpu->bus, addr, cpu->x);

  // DEBUG_PRINT("--> cpu->pc %x \n", 3);

  // uint8_t temp = membus_read(cpu->bus, addr);

  // DEBUG_PRINT("--> membus_read(cpu->bus, addr) %x \n", temp);
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

mos6502_step_result_t mos6502_stx_0x8e(mos6502_t * cpu){
  return __st_basic(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_bcc_0x90(mos6502_t * cpu){
  return __branch_on_status(cpu, !cpu->p.c);
}

mos6502_step_result_t mos6502_sta_0x95(mos6502_t * cpu){
  uint8_t addr = 0;

  addr = read8(cpu, cpu->pc + 1);

  membus_write(cpu->bus, addr + cpu->x, cpu->a);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_stx_0x96(mos6502_t * cpu){
  uint8_t addr = 0;

  addr = read8(cpu, cpu->pc + 1);

  membus_write(cpu->bus, addr == 0xFF ? cpu->y - 1 : addr + cpu->y, cpu->x);

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_bcs_0xb0(mos6502_t * cpu){
  return __branch_on_status(cpu, cpu->p.c);
}

mos6502_step_result_t mos6502_beq_0xf0(mos6502_t * cpu){
  return __branch_on_status(cpu, cpu->p.z);
}

mos6502_step_result_t mos6502_bne_0xd0(mos6502_t * cpu){
  return __branch_on_status(cpu, !cpu->p.z);
}

mos6502_step_result_t mos6502_bpl_0x10(mos6502_t * cpu){
  return __branch_on_status(cpu, !cpu->p.n);
}

mos6502_step_result_t mos6502_bmi_0x30(mos6502_t * cpu){
  return __branch_on_status(cpu, cpu->p.n);
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

mos6502_step_result_t mos6502_txs_0x9A(mos6502_t * cpu){
  cpu->sp = cpu->x;
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_ldy_0xa0(mos6502_t * cpu){
  return __ld_basic(cpu, &cpu->y);
}

mos6502_step_result_t mos6502_lda_0xa1(mos6502_t * cpu){
  DEBUG_PRINT("cpu->x = %x", cpu->x);
  DEBUG_PRINT("cpu->y = %x", cpu->y);
  DEBUG_PRINT("cpu->a = %x", cpu->a);

  cpu->a = cpu->y;

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_ldx_0xa2(mos6502_t * cpu){
  return __ld_basic(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_lda_0xa5(mos6502_t * cpu){
  uint8_t addr = read8(cpu, cpu->pc + 1);
  uint16_t op = membus_read(cpu->bus, addr);
  cpu->a = op;

  // set the zero flag when the result is 0
  if(!cpu->a) cpu->p.z = 1;
  else cpu->p.z = 0;

  // set the negative flag if the value is negative
  if(cpu->a & MSB_VAL) cpu->p.n = 1;
  else cpu->p.n = 0;

  DEBUG_PRINT("__ld_basic addr %x \n", addr);
  DEBUG_PRINT("__ld_basic cpu->a %x \n", cpu->a);
  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
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

mos6502_step_result_t mos6502_lda_0xad(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  DEBUG_PRINT("lda: addr %x", addr);
  DEBUG_PRINT("lda: cpu->x %x", cpu->x);
  uint16_t op = membus_read(cpu->bus, addr + cpu->x);

  DEBUG_PRINT("lda: op %x", op);
  cpu->a = op;

  // set the zero flag when the result is 0
  if(!cpu->a) cpu->p.z = 1;
  else cpu->p.z = 0;

  // set the negative flag if the value is negative
  if(cpu->a & MSB_VAL) cpu->p.n = 1;
  else cpu->p.n = 0;

  // DEBUG_PRINT("--> cpu->pc %x \n", 2);
  cpu->pc += 3;
  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_lda_0xb1(mos6502_t * cpu){
  cpu->a = cpu->x;

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_lda_0xb5(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t op = membus_read(cpu->bus, addr + cpu->x);

  cpu->a = op;

  // set the zero flag when the result is 0
  if(!cpu->a) cpu->p.z = 1;
  else cpu->p.z = 0;

  // set the negative flag if the value is negative
  if(cpu->a & MSB_VAL) cpu->p.n = 1;
  else cpu->p.n = 0;

  DEBUG_PRINT("--> cpu->a %x \n", cpu->a);
  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_tsx_0xba(mos6502_t * cpu){
  DEBUG_PRINT("--> cpu->sp %x \n", cpu->sp);

  cpu->x = cpu->sp;
  cpu->pc += 1;

  cpu->p.n = 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_iny_0xc8(mos6502_t * cpu){
  return __reg_inc(cpu, &cpu->y);
}

mos6502_step_result_t mos6502_cmp_0xc9(mos6502_t * cpu){
  return __cmp_reg_imm(cpu, &cpu->a);
}

mos6502_step_result_t mos6502_dex_0xca(mos6502_t * cpu){
  return __reg_dec(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_cmy_0xcc(mos6502_t * cpu){
  return __cmp_reg_ind(cpu, &cpu->y);
}

mos6502_step_result_t mos6502_cmp_0xcd(mos6502_t * cpu){
  return __cmp_reg_ind(cpu, &cpu->a);
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

mos6502_step_result_t mos6502_cpx_0xe0(mos6502_t * cpu){
  uint8_t op1 = read8(cpu, cpu->pc + 1);
  uint16_t op = cpu->x - op1;

  if(op & 0x00FF) cpu->p.z = 0;
  else cpu->p.z = 1;

  if(op & 0x0080) cpu->p.n = 1;
  else cpu->p.n = 0;

  cpu->p.c = cpu->x >= op1;

  // if(op > 0xFF) cpu->p.c = 1;
  // else cpu->p.c = 0;

  cpu->pc += 2;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_sbc_0xe5(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);

  uint8_t op = membus_read(cpu->bus, addr);
  op = op ^ 0x00FF;

  uint16_t op2 = cpu->a + op + cpu->p.c;

  if((op2 & 0x00FF) >= 0xFF) cpu->p.c = 1;
  else cpu->p.c = 0;

  op2 &= 0xFF;

  if(op2 & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  if(op2) cpu->p.z = 0;
  else cpu->p.z = 1;

  cpu->p.v = ( ~(cpu->a ^ op) ) & (cpu->a^ op2) & 0x80;
  cpu->a = op2;

  cpu->pc += 2;
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
  DEBUG_PRINT("inx: cpu->pc %x", cpu->pc);

  return __reg_inc(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_nop_0xea(mos6502_t * cpu){
  cpu->pc += 1;

  return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t mos6502_cpx_0xec(mos6502_t * cpu){
  return __cmp_reg_ind(cpu, &cpu->x);
}

mos6502_step_result_t mos6502_sbc_0xed(mos6502_t * cpu){
  uint16_t addr = read16(cpu, cpu->pc + 1);
  uint16_t op = membus_read(cpu->bus, addr);

  op = cpu->a - membus_read(cpu->bus, addr) - (cpu->p.c ? 0 : 1);

  if(op & MSB_VAL) cpu->p.c = 0;
  else cpu->p.c = 1;

  if(op & 0x0F) cpu->p.z = 0;
  else cpu->p.z = 1;

  DEBUG_PRINT("sbc: cpu->a %x \n", cpu->a);
  DEBUG_PRINT("sbc: op %x \n", op);
  DEBUG_PRINT("sbc: cpu->p.c %x \n", cpu->p.c);

  cpu->a = op;

  DEBUG_PRINT("sbc: cpu->a %x \n", cpu->a);
  cpu->pc += 3;

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
