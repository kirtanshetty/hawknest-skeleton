#include <rc.h>
#include <base.h>
#include <membus.h>
#include <timekeeper.h>
#include <mos6502/vmcall.h>
#include <mos6502/mos6502.h>
#include <mos6502/mos6502-instr.h>

#include <string.h>

static const uint8_t instr_cycles[256] = {
	7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

// static inline uint8_t
// read8 (mos6502_t * cpu, uint16_t addr)
// {
// 	return membus_read(cpu->bus, addr);
// }

// static inline void
// write8 (mos6502_t * cpu, uint16_t addr, uint8_t val)
// {
// 	membus_write(cpu->bus, addr, val);
// }

// static inline uint16_t
// read16 (mos6502_t * cpu, uint16_t addr)
// {
// 	uint16_t lo = (uint16_t)read8(cpu, addr);
// 	uint16_t hi = (uint16_t)read8(cpu, addr + 1);
// 	uint16_t val = lo | (uint16_t)(hi << 8);
// 	return val;
// }

// static inline uint16_t
// buggy_read16 (mos6502_t * cpu, uint16_t addr)
// {
// 	uint16_t first = addr;
//     uint16_t msb = addr & 0xff00;
//     uint16_t lsb = ((addr & 0xff) == 0xff) ? 0 : ((addr & 0xff) + 1);
// 	uint16_t secnd = msb | lsb;
// 	uint16_t lo = (uint16_t)read8(cpu, first);
// 	uint16_t hi = (uint16_t)read8(cpu, secnd);
// 	uint16_t val = (uint16_t)(hi << 8) | lo;
// 	return val;
// }

size_t
mos6502_instr_repr (mos6502_t * cpu, uint16_t addr, char * buffer, size_t buflen)
{
	// FILL ME IN

	// Delete this line when you're done
	buffer[0] = 0;
	return 0;
}

mos6502_step_result_t mos6502_adc_0x6d2(mos6502_t * cpu){
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
  if(cpu->a & 0x80) cpu->p.n = 1;
  else cpu->p.n = 0;

  // // set the overflow flag if the value is negative
  // if(cpu->a & 0x80) cpu->p.v = 1;
  // else cpu->p.v = 0;

  // set the zero flag when the result is 0
  if(!cpu->a) cpu->p.z = 1;
  else cpu->p.z = 0;

  DEBUG_PRINT("adc: 2cpu->a %x \n", cpu->a);
  DEBUG_PRINT("2 cpu->a %x \n", cpu->a);
  DEBUG_PRINT("2 cpu->p %x \n", cpu->p);

  DEBUG_PRINT("--> cpu->pc %x \n", 3);
  cpu->pc += 3;
}

mos6502_step_result_t
mos6502_step (mos6502_t * cpu)
{
	uint8_t opcode = read8(cpu, cpu->pc);
	uint16_t addr = 0;
	uint16_t op1 = 0;

	do{
		DEBUG_PRINT("------ opcode %x \n", opcode);
		switch(opcode){
			case 0x9:
				mos6502_ora_0x9(cpu);
				break;

			case 0xA:
				mos6502_asl_0xA(cpu);
				break;

			case 0xD:
				mos6502_ora_0xD(cpu);
				break;

			case 0xE:
				mos6502_asl_0xE(cpu);
				break;

			case 0x18:
				mos6502_clc_0x18(cpu);
				break;

			case 0x25:
				mos6502_and_0x25(cpu);
				break;

			case 0x29:
				mos6502_and_0x29(cpu);
				break;

			case 0x2d:
				mos6502_and_0x2d(cpu);
				break;

			case 0x38:
				mos6502_sec_0x38(cpu);
				break;

			case 0x48:
				mos6502_pha_0x48(cpu);
				break;

			case 0x6d: // adc
				mos6502_adc_0x6d2(cpu);
				break;

			case 0x78: // adc
				mos6502_sei_0x78(cpu);
				break;

			case 0x84: // sta
				mos6502_sty_0x84(cpu);
				break;

			case 0x85: // sta
				mos6502_sta_0x85(cpu);
				break;

			case 0x88:
				mos6502_dey_0x88(cpu);
				break;

			case 0x8A:
				mos6502_txa_0x8a(cpu);
				break;

			case 0x8C: // sta
				mos6502_sty_0x8c(cpu);
				break;

			case 0x8D: // sta
				mos6502_sta_0x8d(cpu);
				break;

			case 0x8E: // sta
				mos6502_stx_0x8e(cpu);
				break;

			case 0x98:
				mos6502_tya_0x98(cpu);
				break;

			case 0x9D: // sta
				mos6502_sta_0x9d(cpu);
				break;

			case 0x99: // sta
				mos6502_sta_0x99(cpu);
				break;

			case 0xA0: // ldy
				mos6502_ldy_0xa0(cpu);
				break;

			case 0xA2: // ldx
				mos6502_ldx_0xa2(cpu);
				break;

			case 0xA8:
				mos6502_tay_0xa8(cpu);
				break;

			case 0xA9: // lda
				mos6502_lda_0xa9(cpu);
				break;

			case 0xAA:
				mos6502_tax_0xaa(cpu);
				break;

			case 0xC8:
				mos6502_iny_0xc8(cpu);
				break;

			case 0xCA:
				mos6502_dex_0xca(cpu);
				break;

			case 0xCE: // lda
				mos6502_dec_0xce(cpu);
				break;

			case 0xD8:
				mos6502_sed_0xd8(cpu);
				break;

			case 0xE6:
				mos6502_inc_0xe6(cpu);
				break;

			case 0xE8:
				mos6502_inx_0xe8(cpu);
				break;

			case 0xEA:
				mos6502_nop_0xea(cpu);
				break;

			case 0xEE:
				mos6502_inc_0xee(cpu);
				break;

			case 0xF8: // sed
				mos6502_sed_0xf8(cpu);
				break;

			case 0x80:
				mos6502_paravirt_0x80(cpu);
				break;

			default:
				DEBUG_PRINT("Uknown istruction opcode %x \n", opcode);
				return mos6502_default(cpu);
				break;
		}

		// DEBUG_PRINT("------ cpu->pc %x \n", cpu->pc);
		// DEBUG_PRINT("------ instr_cycles[opcode] %x \n", instr_cycles[opcode]);
		// cpu->pc += instr_cycles[opcode];
		// DEBUG_PRINT("------ cpu->pc %x \n", cpu->pc);

		opcode = read8(cpu, cpu->pc);
	} while(opcode);



	mos6502_advance_clk(cpu, instr_cycles[opcode]);

	return MOS6502_STEP_RESULT_SUCCESS;
}
