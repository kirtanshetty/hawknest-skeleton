// #ifndef M_INSTR_H
// #define M_INSTR_H

// #include <mos6502/vmcall.h>
#include <mos6502/mos6502.h>

#define MSB_VAL 0x80
#define STACK_START 0x0100

static inline uint8_t
read8 (mos6502_t * cpu, uint16_t addr)
{
 return membus_read(cpu->bus, addr);
}

static inline void
write8 (mos6502_t * cpu, uint16_t addr, uint8_t val)
{
 membus_write(cpu->bus, addr, val);
}

static inline uint16_t
read16 (mos6502_t * cpu, uint16_t addr)
{
 uint16_t lo = (uint16_t)read8(cpu, addr);
 uint16_t hi = (uint16_t)read8(cpu, addr + 1);
 uint16_t val = lo | (uint16_t)(hi << 8);
 return val;
}

static inline uint16_t
buggy_read16 (mos6502_t * cpu, uint16_t addr)
{
 uint16_t first = addr;
    uint16_t msb = addr & 0xff00;
    uint16_t lsb = ((addr & 0xff) == 0xff) ? 0 : ((addr & 0xff) + 1);
 uint16_t secnd = msb | lsb;
 uint16_t lo = (uint16_t)read8(cpu, first);
 uint16_t hi = (uint16_t)read8(cpu, secnd);
 uint16_t val = (uint16_t)(hi << 8) | lo;
 return val;
}

mos6502_step_result_t mos6502_adc_0x6d(mos6502_t * cpu);

mos6502_step_result_t mos6502_sta_0x85(mos6502_t * cpu);

mos6502_step_result_t mos6502_sta_0x8d(mos6502_t * cpu);

mos6502_step_result_t mos6502_lda_0xa9(mos6502_t * cpu);

mos6502_step_result_t mos6502_paravirt_0x80(mos6502_t * cpu);

mos6502_step_result_t mos6502_default(mos6502_t * cpu);

// #endif