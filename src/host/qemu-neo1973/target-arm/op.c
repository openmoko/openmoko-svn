/*
 *  ARM micro operations
 * 
 *  Copyright (c) 2003 Fabrice Bellard
 *  Copyright (c) 2005 CodeSourcery, LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "exec.h"

#define REGNAME r0
#define REG (env->regs[0])
#include "op_template.h"

#define REGNAME r1
#define REG (env->regs[1])
#include "op_template.h"

#define REGNAME r2
#define REG (env->regs[2])
#include "op_template.h"

#define REGNAME r3
#define REG (env->regs[3])
#include "op_template.h"

#define REGNAME r4
#define REG (env->regs[4])
#include "op_template.h"

#define REGNAME r5
#define REG (env->regs[5])
#include "op_template.h"

#define REGNAME r6
#define REG (env->regs[6])
#include "op_template.h"

#define REGNAME r7
#define REG (env->regs[7])
#include "op_template.h"

#define REGNAME r8
#define REG (env->regs[8])
#include "op_template.h"

#define REGNAME r9
#define REG (env->regs[9])
#include "op_template.h"

#define REGNAME r10
#define REG (env->regs[10])
#include "op_template.h"

#define REGNAME r11
#define REG (env->regs[11])
#include "op_template.h"

#define REGNAME r12
#define REG (env->regs[12])
#include "op_template.h"

#define REGNAME r13
#define REG (env->regs[13])
#include "op_template.h"

#define REGNAME r14
#define REG (env->regs[14])
#include "op_template.h"

#define REGNAME r15
#define REG (env->regs[15])
#define SET_REG(x) REG = x & ~(uint32_t)1
#include "op_template.h"

void OPPROTO op_bx_T0(void)
{
  env->regs[15] = T0 & ~(uint32_t)1;
  env->thumb = (T0 & 1) != 0;
}

void OPPROTO op_movl_T0_0(void)
{
    T0 = 0;
}

void OPPROTO op_movl_T0_im(void)
{
    T0 = PARAM1;
}

void OPPROTO op_movl_T0_T1(void)
{
    T0 = T1;
}

void OPPROTO op_movl_T1_im(void)
{
    T1 = PARAM1;
}

void OPPROTO op_mov_CF_T1(void)
{
    env->CF = ((uint32_t)T1) >> 31;
}

void OPPROTO op_movl_T2_im(void)
{
    T2 = PARAM1;
}

void OPPROTO op_addl_T1_im(void)
{
    T1 += PARAM1;
}

void OPPROTO op_addl_T1_T2(void)
{
    T1 += T2;
}

void OPPROTO op_subl_T1_T2(void)
{
    T1 -= T2;
}

void OPPROTO op_addl_T0_T1(void)
{
    T0 += T1;
}

void OPPROTO op_addl_T0_T1_cc(void)
{
    unsigned int src1;
    src1 = T0;
    T0 += T1;
    env->NZF = T0;
    env->CF = T0 < src1;
    env->VF = (src1 ^ T1 ^ -1) & (src1 ^ T0);
}

void OPPROTO op_adcl_T0_T1(void)
{
    T0 += T1 + env->CF;
}

void OPPROTO op_adcl_T0_T1_cc(void)
{
    unsigned int src1;
    src1 = T0;
    if (!env->CF) {
        T0 += T1;
        env->CF = T0 < src1;
    } else {
        T0 += T1 + 1;
        env->CF = T0 <= src1;
    }
    env->VF = (src1 ^ T1 ^ -1) & (src1 ^ T0);
    env->NZF = T0;
    FORCE_RET();
}

#define OPSUB(sub, sbc, res, T0, T1)            \
                                                \
void OPPROTO op_ ## sub ## l_T0_T1(void)        \
{                                               \
    res = T0 - T1;                              \
}                                               \
                                                \
void OPPROTO op_ ## sub ## l_T0_T1_cc(void)     \
{                                               \
    unsigned int src1;                          \
    src1 = T0;                                  \
    T0 -= T1;                                   \
    env->NZF = T0;                              \
    env->CF = src1 >= T1;                       \
    env->VF = (src1 ^ T1) & (src1 ^ T0);        \
    res = T0;                                   \
}                                               \
                                                \
void OPPROTO op_ ## sbc ## l_T0_T1(void)        \
{                                               \
    res = T0 - T1 + env->CF - 1;                \
}                                               \
                                                \
void OPPROTO op_ ## sbc ## l_T0_T1_cc(void)     \
{                                               \
    unsigned int src1;                          \
    src1 = T0;                                  \
    if (!env->CF) {                             \
        T0 = T0 - T1 - 1;                       \
        env->CF = src1 > T1;                    \
    } else {                                    \
        T0 = T0 - T1;                           \
        env->CF = src1 >= T1;                   \
    }                                           \
    env->VF = (src1 ^ T1) & (src1 ^ T0);        \
    env->NZF = T0;                              \
    res = T0;                                   \
    FORCE_RET();                                \
}

OPSUB(sub, sbc, T0, T0, T1)

OPSUB(rsb, rsc, T0, T1, T0)

void OPPROTO op_andl_T0_T1(void)
{
    T0 &= T1;
}

void OPPROTO op_xorl_T0_T1(void)
{
    T0 ^= T1;
}

void OPPROTO op_orl_T0_T1(void)
{
    T0 |= T1;
}

void OPPROTO op_bicl_T0_T1(void)
{
    T0 &= ~T1;
}

void OPPROTO op_notl_T1(void)
{
    T1 = ~T1;
}

void OPPROTO op_logic_T0_cc(void)
{
    env->NZF = T0;
}

void OPPROTO op_logic_T1_cc(void)
{
    env->NZF = T1;
}

#define EIP (env->regs[15])

void OPPROTO op_test_eq(void)
{
    if (env->NZF == 0)
        GOTO_LABEL_PARAM(1);;
    FORCE_RET();
}

void OPPROTO op_test_ne(void)
{
    if (env->NZF != 0)
        GOTO_LABEL_PARAM(1);;
    FORCE_RET();
}

void OPPROTO op_test_cs(void)
{
    if (env->CF != 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_cc(void)
{
    if (env->CF == 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_mi(void)
{
    if ((env->NZF & 0x80000000) != 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_pl(void)
{
    if ((env->NZF & 0x80000000) == 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_vs(void)
{
    if ((env->VF & 0x80000000) != 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_vc(void)
{
    if ((env->VF & 0x80000000) == 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_hi(void)
{
    if (env->CF != 0 && env->NZF != 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_ls(void)
{
    if (env->CF == 0 || env->NZF == 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_ge(void)
{
    if (((env->VF ^ env->NZF) & 0x80000000) == 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_lt(void)
{
    if (((env->VF ^ env->NZF) & 0x80000000) != 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_gt(void)
{
    if (env->NZF != 0 && ((env->VF ^ env->NZF) & 0x80000000) == 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_test_le(void)
{
    if (env->NZF == 0 || ((env->VF ^ env->NZF) & 0x80000000) != 0)
        GOTO_LABEL_PARAM(1);
    FORCE_RET();
}

void OPPROTO op_goto_tb0(void)
{
    GOTO_TB(op_goto_tb0, PARAM1, 0);
}

void OPPROTO op_goto_tb1(void)
{
    GOTO_TB(op_goto_tb1, PARAM1, 1);
}

void OPPROTO op_exit_tb(void)
{
    EXIT_TB();
}

void OPPROTO op_movl_T0_cpsr(void)
{
    T0 = cpsr_read(env);
    FORCE_RET();
}

void OPPROTO op_movl_T0_spsr(void)
{
    T0 = env->spsr;
}

void OPPROTO op_movl_spsr_T0(void)
{
    uint32_t mask = PARAM1;
    env->spsr = (env->spsr & ~mask) | (T0 & mask);
}

void OPPROTO op_movl_cpsr_T0(void)
{
    cpsr_write(env, T0, PARAM1);
    FORCE_RET();
}

void OPPROTO op_mul_T0_T1(void)
{
    T0 = T0 * T1;
}

/* 64 bit unsigned mul */
void OPPROTO op_mull_T0_T1(void)
{
    uint64_t res;
    res = (uint64_t)T0 * (uint64_t)T1;
    T1 = res >> 32;
    T0 = res;
}

/* 64 bit signed mul */
void OPPROTO op_imull_T0_T1(void)
{
    uint64_t res;
    res = (int64_t)((int32_t)T0) * (int64_t)((int32_t)T1);
    T1 = res >> 32;
    T0 = res;
}

/* 48 bit signed mul, top 32 bits */
void OPPROTO op_imulw_T0_T1(void)
{
  uint64_t res;
  res = (int64_t)((int32_t)T0) * (int64_t)((int32_t)T1);
  T0 = res >> 16;
}

void OPPROTO op_addq_T0_T1(void)
{
    uint64_t res;
    res = ((uint64_t)T1 << 32) | T0;
    res += ((uint64_t)(env->regs[PARAM2]) << 32) | (env->regs[PARAM1]);
    T1 = res >> 32;
    T0 = res;
}

void OPPROTO op_addq_lo_T0_T1(void)
{
    uint64_t res;
    res = ((uint64_t)T1 << 32) | T0;
    res += (uint64_t)(env->regs[PARAM1]);
    T1 = res >> 32;
    T0 = res;
}

void OPPROTO op_logicq_cc(void)
{
    env->NZF = (T1 & 0x80000000) | ((T0 | T1) != 0);
}

/* memory access */

#define MEMSUFFIX _raw
#include "op_mem.h"

#if !defined(CONFIG_USER_ONLY)
#define MEMSUFFIX _user
#include "op_mem.h"
#define MEMSUFFIX _kernel
#include "op_mem.h"
#endif

/* shifts */

/* T1 based */

void OPPROTO op_shll_T1_im(void)
{
    T1 = T1 << PARAM1;
}

void OPPROTO op_shrl_T1_im(void)
{
    T1 = (uint32_t)T1 >> PARAM1;
}

void OPPROTO op_shrl_T1_0(void)
{
    T1 = 0;
}

void OPPROTO op_sarl_T1_im(void)
{
    T1 = (int32_t)T1 >> PARAM1;
}

void OPPROTO op_sarl_T1_0(void)
{
    T1 = (int32_t)T1 >> 31;
}

void OPPROTO op_rorl_T1_im(void)
{
    int shift;
    shift = PARAM1;
    T1 = ((uint32_t)T1 >> shift) | (T1 << (32 - shift));
}

void OPPROTO op_rrxl_T1(void)
{
    T1 = ((uint32_t)T1 >> 1) | ((uint32_t)env->CF << 31);
}

/* T1 based, set C flag */
void OPPROTO op_shll_T1_im_cc(void)
{
    env->CF = (T1 >> (32 - PARAM1)) & 1;
    T1 = T1 << PARAM1;
}

void OPPROTO op_shrl_T1_im_cc(void)
{
    env->CF = (T1 >> (PARAM1 - 1)) & 1;
    T1 = (uint32_t)T1 >> PARAM1;
}

void OPPROTO op_shrl_T1_0_cc(void)
{
    env->CF = (T1 >> 31) & 1;
    T1 = 0;
}

void OPPROTO op_sarl_T1_im_cc(void)
{
    env->CF = (T1 >> (PARAM1 - 1)) & 1;
    T1 = (int32_t)T1 >> PARAM1;
}

void OPPROTO op_sarl_T1_0_cc(void)
{
    env->CF = (T1 >> 31) & 1;
    T1 = (int32_t)T1 >> 31;
}

void OPPROTO op_rorl_T1_im_cc(void)
{
    int shift;
    shift = PARAM1;
    env->CF = (T1 >> (shift - 1)) & 1;
    T1 = ((uint32_t)T1 >> shift) | (T1 << (32 - shift));
}

void OPPROTO op_rrxl_T1_cc(void)
{
    uint32_t c;
    c = T1 & 1;
    T1 = ((uint32_t)T1 >> 1) | ((uint32_t)env->CF << 31);
    env->CF = c;
}

/* T2 based */
void OPPROTO op_shll_T2_im(void)
{
    T2 = T2 << PARAM1;
}

void OPPROTO op_shrl_T2_im(void)
{
    T2 = (uint32_t)T2 >> PARAM1;
}

void OPPROTO op_shrl_T2_0(void)
{
    T2 = 0;
}

void OPPROTO op_sarl_T2_im(void)
{
    T2 = (int32_t)T2 >> PARAM1;
}

void OPPROTO op_sarl_T2_0(void)
{
    T2 = (int32_t)T2 >> 31;
}

void OPPROTO op_rorl_T2_im(void)
{
    int shift;
    shift = PARAM1;
    T2 = ((uint32_t)T2 >> shift) | (T2 << (32 - shift));
}

void OPPROTO op_rrxl_T2(void)
{
    T2 = ((uint32_t)T2 >> 1) | ((uint32_t)env->CF << 31);
}

/* T1 based, use T0 as shift count */

void OPPROTO op_shll_T1_T0(void)
{
    int shift;
    shift = T0 & 0xff;
    if (shift >= 32)
        T1 = 0;
    else
        T1 = T1 << shift;
    FORCE_RET();
}

void OPPROTO op_shrl_T1_T0(void)
{
    int shift;
    shift = T0 & 0xff;
    if (shift >= 32)
        T1 = 0;
    else
        T1 = (uint32_t)T1 >> shift;
    FORCE_RET();
}

void OPPROTO op_sarl_T1_T0(void)
{
    int shift;
    shift = T0 & 0xff;
    if (shift >= 32)
        shift = 31;
    T1 = (int32_t)T1 >> shift;
}

void OPPROTO op_rorl_T1_T0(void)
{
    int shift;
    shift = T0 & 0x1f;
    if (shift) {
        T1 = ((uint32_t)T1 >> shift) | (T1 << (32 - shift));
    }
    FORCE_RET();
}

/* T1 based, use T0 as shift count and compute CF */

void OPPROTO op_shll_T1_T0_cc(void)
{
    int shift;
    shift = T0 & 0xff;
    if (shift >= 32) {
        if (shift == 32)
            env->CF = T1 & 1;
        else
            env->CF = 0;
        T1 = 0;
    } else if (shift != 0) {
        env->CF = (T1 >> (32 - shift)) & 1;
        T1 = T1 << shift;
    }
    FORCE_RET();
}

void OPPROTO op_shrl_T1_T0_cc(void)
{
    int shift;
    shift = T0 & 0xff;
    if (shift >= 32) {
        if (shift == 32)
            env->CF = (T1 >> 31) & 1;
        else
            env->CF = 0;
        T1 = 0;
    } else if (shift != 0) {
        env->CF = (T1 >> (shift - 1)) & 1;
        T1 = (uint32_t)T1 >> shift;
    }
    FORCE_RET();
}

void OPPROTO op_sarl_T1_T0_cc(void)
{
    int shift;
    shift = T0 & 0xff;
    if (shift >= 32) {
        env->CF = (T1 >> 31) & 1;
        T1 = (int32_t)T1 >> 31;
    } else {
        env->CF = (T1 >> (shift - 1)) & 1;
        T1 = (int32_t)T1 >> shift;
    }
    FORCE_RET();
}

void OPPROTO op_rorl_T1_T0_cc(void)
{
    int shift1, shift;
    shift1 = T0 & 0xff;
    shift = shift1 & 0x1f;
    if (shift == 0) {
        if (shift1 != 0)
            env->CF = (T1 >> 31) & 1;
    } else {
        env->CF = (T1 >> (shift - 1)) & 1;
        T1 = ((uint32_t)T1 >> shift) | (T1 << (32 - shift));
    }
    FORCE_RET();
}

/* misc */
void OPPROTO op_clz_T0(void)
{
    int count;
    for (count = 32; T0 > 0; count--)
        T0 = T0 >> 1;
    T0 = count;
    FORCE_RET();
}

void OPPROTO op_sarl_T0_im(void)
{
    T0 = (int32_t)T0 >> PARAM1;
}

/* Sign/zero extend */
void OPPROTO op_sxth_T0(void)
{
  T0 = (int16_t)T0;
}

void OPPROTO op_sxth_T1(void)
{
  T1 = (int16_t)T1;
}

void OPPROTO op_sxtb_T1(void)
{
    T1 = (int8_t)T1;
}

void OPPROTO op_uxtb_T1(void)
{
    T1 = (uint8_t)T1;
}

void OPPROTO op_uxth_T1(void)
{
    T1 = (uint16_t)T1;
}

void OPPROTO op_sxtb16_T1(void)
{
    uint32_t res;
    res = (uint16_t)(int8_t)T1;
    res |= (uint32_t)(int8_t)(T1 >> 16) << 16;
    T1 = res;
}

void OPPROTO op_uxtb16_T1(void)
{
    uint32_t res;
    res = (uint16_t)(uint8_t)T1;
    res |= (uint32_t)(uint8_t)(T1 >> 16) << 16;
    T1 = res;
}

#define SIGNBIT (uint32_t)0x80000000
/* saturating arithmetic  */
void OPPROTO op_addl_T0_T1_setq(void)
{
  uint32_t res;

  res = T0 + T1;
  if (((res ^ T0) & SIGNBIT) && !((T0 ^ T1) & SIGNBIT))
      env->QF = 1;

  T0 = res;
  FORCE_RET();
}

void OPPROTO op_addl_T0_T1_saturate(void)
{
  uint32_t res;

  res = T0 + T1;
  if (((res ^ T0) & SIGNBIT) && !((T0 ^ T1) & SIGNBIT)) {
      env->QF = 1;
      if (T0 & SIGNBIT)
          T0 = 0x80000000;
      else
          T0 = 0x7fffffff;
  }
  else
    T0 = res;
  
  FORCE_RET();
}

void OPPROTO op_subl_T0_T1_saturate(void)
{
  uint32_t res;

  res = T0 - T1;
  if (((res ^ T0) & SIGNBIT) && ((T0 ^ T1) & SIGNBIT)) {
      env->QF = 1;
      if (T0 & SIGNBIT)
          T0 = 0x80000000;
      else
          T0 = 0x7fffffff;
  }
  else
    T0 = res;
  
  FORCE_RET();
}

void OPPROTO op_double_T1_saturate(void)
{
  int32_t val;

  val = T1;
  if (val >= 0x40000000) {
      T1 = 0x7fffffff;
      env->QF = 1;
  } else if (val <= (int32_t)0xc0000000) {
      T1 = 0x80000000;
      env->QF = 1;
  } else {
      T1 = val << 1;
  }
  FORCE_RET();
}

/* thumb shift by immediate */
void OPPROTO op_shll_T0_im_thumb(void)
{
    int shift;
    shift = PARAM1;
    if (shift != 0) {
	env->CF = (T1 >> (32 - shift)) & 1;
	T0 = T0 << shift;
    }
    env->NZF = T0;
    FORCE_RET();
}

void OPPROTO op_shrl_T0_im_thumb(void)
{
    int shift;

    shift = PARAM1;
    if (shift == 0) {
	env->CF = ((uint32_t)shift) >> 31;
	T0 = 0;
    } else {
	env->CF = (T0 >> (shift - 1)) & 1;
	T0 = T0 >> shift;
    }
    env->NZF = T0;
    FORCE_RET();
}

void OPPROTO op_sarl_T0_im_thumb(void)
{
    int shift;

    shift = PARAM1;
    if (shift == 0) {
	T0 = ((int32_t)T0) >> 31;
	env->CF = T0 & 1;
    } else {
	env->CF = (T0 >> (shift - 1)) & 1;
	T0 = ((int32_t)T0) >> shift;
    }
    env->NZF = T0;
    FORCE_RET();
}

/* exceptions */

void OPPROTO op_swi(void)
{
    env->exception_index = EXCP_SWI;
    cpu_loop_exit();
}

void OPPROTO op_undef_insn(void)
{
    env->exception_index = EXCP_UDEF;
    cpu_loop_exit();
}

void OPPROTO op_debug(void)
{
    env->exception_index = EXCP_DEBUG;
    cpu_loop_exit();
}

void OPPROTO op_wfi(void)
{
    env->exception_index = EXCP_HLT;
    env->halted = 1;
    cpu_loop_exit();
}

void OPPROTO op_bkpt(void)
{
    env->exception_index = EXCP_BKPT;
    cpu_loop_exit();
}

/* VFP support.  We follow the convention used for VFP instrunctions:
   Single precition routines have a "s" suffix, double precision a
   "d" suffix.  */

#define VFP_OP(name, p) void OPPROTO op_vfp_##name##p(void)

#define VFP_BINOP(name) \
VFP_OP(name, s)             \
{                           \
    FT0s = float32_ ## name (FT0s, FT1s, &env->vfp.fp_status);    \
}                           \
VFP_OP(name, d)             \
{                           \
    FT0d = float64_ ## name (FT0d, FT1d, &env->vfp.fp_status);    \
}
VFP_BINOP(add)
VFP_BINOP(sub)
VFP_BINOP(mul)
VFP_BINOP(div)
#undef VFP_BINOP

#define VFP_HELPER(name)  \
VFP_OP(name, s)           \
{                         \
    do_vfp_##name##s();    \
}                         \
VFP_OP(name, d)           \
{                         \
    do_vfp_##name##d();    \
}
VFP_HELPER(abs)
VFP_HELPER(sqrt)
VFP_HELPER(cmp)
VFP_HELPER(cmpe)
#undef VFP_HELPER

/* XXX: Will this do the right thing for NANs.  Should invert the signbit
   without looking at the rest of the value.  */
VFP_OP(neg, s)
{
    FT0s = float32_chs(FT0s);
}

VFP_OP(neg, d)
{
    FT0d = float64_chs(FT0d);
}

VFP_OP(F1_ld0, s)
{
    union {
        uint32_t i;
        float32 s;
    } v;
    v.i = 0;
    FT1s = v.s;
}

VFP_OP(F1_ld0, d)
{
    union {
        uint64_t i;
        float64 d;
    } v;
    v.i = 0;
    FT1d = v.d;
}

/* Helper routines to perform bitwise copies between float and int.  */
static inline float32 vfp_itos(uint32_t i)
{
    union {
        uint32_t i;
        float32 s;
    } v;

    v.i = i;
    return v.s;
}

static inline uint32_t vfp_stoi(float32 s)
{
    union {
        uint32_t i;
        float32 s;
    } v;

    v.s = s;
    return v.i;
}

/* Integer to float conversion.  */
VFP_OP(uito, s)
{
    FT0s = uint32_to_float32(vfp_stoi(FT0s), &env->vfp.fp_status);
}

VFP_OP(uito, d)
{
    FT0d = uint32_to_float64(vfp_stoi(FT0s), &env->vfp.fp_status);
}

VFP_OP(sito, s)
{
    FT0s = int32_to_float32(vfp_stoi(FT0s), &env->vfp.fp_status);
}

VFP_OP(sito, d)
{
    FT0d = int32_to_float64(vfp_stoi(FT0s), &env->vfp.fp_status);
}

/* Float to integer conversion.  */
VFP_OP(toui, s)
{
    FT0s = vfp_itos(float32_to_uint32(FT0s, &env->vfp.fp_status));
}

VFP_OP(toui, d)
{
    FT0s = vfp_itos(float64_to_uint32(FT0d, &env->vfp.fp_status));
}

VFP_OP(tosi, s)
{
    FT0s = vfp_itos(float32_to_int32(FT0s, &env->vfp.fp_status));
}

VFP_OP(tosi, d)
{
    FT0s = vfp_itos(float64_to_int32(FT0d, &env->vfp.fp_status));
}

/* TODO: Set rounding mode properly.  */
VFP_OP(touiz, s)
{
    FT0s = vfp_itos(float32_to_uint32_round_to_zero(FT0s, &env->vfp.fp_status));
}

VFP_OP(touiz, d)
{
    FT0s = vfp_itos(float64_to_uint32_round_to_zero(FT0d, &env->vfp.fp_status));
}

VFP_OP(tosiz, s)
{
    FT0s = vfp_itos(float32_to_int32_round_to_zero(FT0s, &env->vfp.fp_status));
}

VFP_OP(tosiz, d)
{
    FT0s = vfp_itos(float64_to_int32_round_to_zero(FT0d, &env->vfp.fp_status));
}

/* floating point conversion */
VFP_OP(fcvtd, s)
{
    FT0d = float32_to_float64(FT0s, &env->vfp.fp_status);
}

VFP_OP(fcvts, d)
{
    FT0s = float64_to_float32(FT0d, &env->vfp.fp_status);
}

/* Get and Put values from registers.  */
VFP_OP(getreg_F0, d)
{
  FT0d = *(float64 *)((char *) env + PARAM1);
}

VFP_OP(getreg_F0, s)
{
  FT0s = *(float32 *)((char *) env + PARAM1);
}

VFP_OP(getreg_F1, d)
{
  FT1d = *(float64 *)((char *) env + PARAM1);
}

VFP_OP(getreg_F1, s)
{
  FT1s = *(float32 *)((char *) env + PARAM1);
}

VFP_OP(setreg_F0, d)
{
  *(float64 *)((char *) env + PARAM1) = FT0d;
}

VFP_OP(setreg_F0, s)
{
  *(float32 *)((char *) env + PARAM1) = FT0s;
}

void OPPROTO op_vfp_movl_T0_fpscr(void)
{
    do_vfp_get_fpscr ();
}

void OPPROTO op_vfp_movl_T0_fpscr_flags(void)
{
    T0 = env->vfp.xregs[ARM_VFP_FPSCR] & (0xf << 28);
}

void OPPROTO op_vfp_movl_fpscr_T0(void)
{
    do_vfp_set_fpscr();
}

void OPPROTO op_vfp_movl_T0_xreg(void)
{
    T0 = env->vfp.xregs[PARAM1];
}

void OPPROTO op_vfp_movl_xreg_T0(void)
{
    env->vfp.xregs[PARAM1] = T0;
}

/* Move between FT0s to T0  */
void OPPROTO op_vfp_mrs(void)
{
    T0 = vfp_stoi(FT0s);
}

void OPPROTO op_vfp_msr(void)
{
    FT0s = vfp_itos(T0);
}

/* Move between FT0d and {T0,T1} */
void OPPROTO op_vfp_mrrd(void)
{
    CPU_DoubleU u;
    
    u.d = FT0d;
    T0 = u.l.lower;
    T1 = u.l.upper;
}

void OPPROTO op_vfp_mdrr(void)
{
    CPU_DoubleU u;
    
    u.l.lower = T0;
    u.l.upper = T1;
    FT0d = u.d;
}

/* Copy the most significant bit of T0 to all bits of T1.  */
void OPPROTO op_signbit_T1_T0(void)
{
    T1 = (int32_t)T0 >> 31;
}

void OPPROTO op_movl_cp_T0(void)
{
    helper_set_cp(env, PARAM1, T0);
    FORCE_RET();
}

void OPPROTO op_movl_T0_cp(void)
{
    T0 = helper_get_cp(env, PARAM1);
    FORCE_RET();
}

void OPPROTO op_movl_cp15_T0(void)
{
    helper_set_cp15(env, PARAM1, T0);
    FORCE_RET();
}

void OPPROTO op_movl_T0_cp15(void)
{
    T0 = helper_get_cp15(env, PARAM1);
    FORCE_RET();
}

/* Access to user mode registers from privileged modes.  */
void OPPROTO op_movl_T0_user(void)
{
    int regno = PARAM1;
    if (regno == 13) {
        T0 = env->banked_r13[0];
    } else if (regno == 14) {
        T0 = env->banked_r14[0];
    } else if ((env->uncached_cpsr & 0x1f) == ARM_CPU_MODE_FIQ) {
        T0 = env->usr_regs[regno - 8];
    } else {
        T0 = env->regs[regno];
    }
    FORCE_RET();
}


void OPPROTO op_movl_user_T0(void)
{
    int regno = PARAM1;
    if (regno == 13) {
        env->banked_r13[0] = T0;
    } else if (regno == 14) {
        env->banked_r14[0] = T0;
    } else if ((env->uncached_cpsr & 0x1f) == ARM_CPU_MODE_FIQ) {
        env->usr_regs[regno - 8] = T0;
    } else {
        env->regs[regno] = T0;
    }
    FORCE_RET();
}

void OPPROTO op_movl_T2_T0(void)
{
    T2 = T0;
}

void OPPROTO op_movl_T0_T2(void)
{
    T0 = T2;
}

/* iwMMXt support.  */

#define M1	env->iwmmxt.regs[PARAM1]

/* iwMMXt macros extracted from GNU gdb.  */

/* Set the SIMD wCASF flags for 8, 16, 32 or 64-bit operations.  */
#define SIMD8_SET( v, n, b)	((v != 0) << ((((b) + 1) * 4) + (n)))
#define SIMD16_SET(v, n, h)	((v != 0) << ((((h) + 1) * 8) + (n)))
#define SIMD32_SET(v, n, w)	((v != 0) << ((((w) + 1) * 16) + (n)))
#define SIMD64_SET(v, n)	((v != 0) << (32 + (n)))
/* Flags to pass as "n" above.  */
#define SIMD_NBIT	-1
#define SIMD_ZBIT	-2
#define SIMD_CBIT	-3
#define SIMD_VBIT	-4
/* Various status bit macros.  */
#define NBIT8(x)	((x) & 0x80)
#define NBIT16(x)	((x) & 0x8000)
#define NBIT32(x)	((x) & 0x80000000)
#define NBIT64(x)	((x) & 0x8000000000000000ULL)
#define ZBIT8(x)	(((x) & 0xff) == 0)
#define ZBIT16(x)	(((x) & 0xffff) == 0)
#define ZBIT32(x)	(((x) & 0xffffffff) == 0)
#define ZBIT64(x)	(x == 0)
/* Sign extension macros.  */
#define EXTEND8H(a)	((a) & 0x80 ? ((a) | 0xff00) : (a))
#define EXTEND8(a)	((a) & 0x80 ? ((a) | 0xffffff00) : (a))
#define EXTEND16(a)	((a) & 0x8000 ? ((a) | 0xffff0000) : (a))
#define EXTEND32(a)	((a) & 0x80000000ULL ? \
    ((a) | 0xffffffff00000000ULL) : (a))

void OPPROTO op_iwmmxt_movl_T0_T1_wRn(void)
{
    T0 = M1 & ~(uint32_t) 0;
    T1 = M1 >> 32;
}

void OPPROTO op_iwmmxt_movl_wRn_T0_T1(void)
{
    M1 = ((uint64_t) T1 << 32) | T0;
}

void OPPROTO op_iwmmxt_movq_M0_wRn(void)
{
    M0 = M1;
}

void OPPROTO op_iwmmxt_orq_M0_wRn(void)
{
    M0 |= M1;
}

void OPPROTO op_iwmmxt_andq_M0_wRn(void)
{
    M0 &= M1;
}

void OPPROTO op_iwmmxt_xorq_M0_wRn(void)
{
    M0 ^= M1;
}

void OPPROTO op_iwmmxt_maddsq_M0_wRn(void)
{
    M0 = ((
            EXTEND16((M0 >> 0) & 0xffff) * EXTEND16((M1 >> 0) & 0xffff) +
            EXTEND16((M0 >> 16) & 0xffff) * EXTEND16((M1 >> 16) & 0xffff)
        ) & 0xffffffff) | ((
            EXTEND16((M0 >> 32) & 0xffff) * EXTEND16((M1 >> 32) & 0xffff) +
            EXTEND16((M0 >> 48) & 0xffff) * EXTEND16((M1 >> 48) & 0xffff)
        ) << 32);
}

void OPPROTO op_iwmmxt_madduq_M0_wRn(void)
{
    M0 = ((
            ((M0 >> 0) & 0xffff) * ((M1 >> 0) & 0xffff) +
            ((M0 >> 16) & 0xffff) * ((M1 >> 16) & 0xffff)
        ) & 0xffffffff) | ((
            ((M0 >> 32) & 0xffff) * ((M1 >> 32) & 0xffff) +
            ((M0 >> 48) & 0xffff) * ((M1 >> 48) & 0xffff)
        ) << 32);
}

void OPPROTO op_iwmmxt_sadb_M0_wRn(void)
{
#define abs(x) (((x) >= 0) ? x : -x)
#define SADB(SHR) abs((int) ((M0 >> SHR) & 0xff) - (int) ((M1 >> SHR) & 0xff))
    M0 =
        SADB(0) + SADB(8) + SADB(16) + SADB(24) +
        SADB(32) + SADB(40) + SADB(48) + SADB(56);
#undef SADB
}

void OPPROTO op_iwmmxt_sadw_M0_wRn(void)
{
#define SADW(SHR) \
    abs((int) ((M0 >> SHR) & 0xffff) - (int) ((M1 >> SHR) & 0xffff))
    M0 = SADW(0) + SADW(16) + SADW(32) + SADW(48);
#undef SADW
}

void OPPROTO op_iwmmxt_addl_M0_wRn(void)
{
    M0 += env->iwmmxt.regs[PARAM1] & 0xffffffff;
}

void OPPROTO op_iwmmxt_mulsw_M0_wRn(void)
{
#define MULS(SHR) (((( \
        EXTEND16((M0 >> SHR) & 0xffff) * EXTEND16((M1 >> SHR) & 0xffff) \
    ) >> PARAM2) & 0xffff) << SHR)
    M0 = MULS(0) | MULS(16) | MULS(32) | MULS(48);
#undef MULS
}

void OPPROTO op_iwmmxt_muluw_M0_wRn(void)
{
#define MULU(SHR) (((( \
        ((M0 >> SHR) & 0xffff) * ((M1 >> SHR) & 0xffff) \
    ) >> PARAM2) & 0xffff) << SHR)
    M0 = MULU(0) | MULU(16) | MULU(32) | MULU(48);
#undef MULU
}

void OPPROTO op_iwmmxt_macsw_M0_wRn(void)
{
#define MACS(SHR) ( \
        (int32_t) EXTEND16((M0 >> SHR) & 0xffff) * \
        (int32_t) EXTEND16((M1 >> SHR) & 0xffff)) 
    M0 = (int64_t) (MACS(0) + MACS(16) + MACS(32) + MACS(48));
#undef MACS
}

void OPPROTO op_iwmmxt_macuw_M0_wRn(void)
{
#define MACU(SHR) ( \
        (uint32_t) ((M0 >> SHR) & 0xffff) * \
        (uint32_t) ((M1 >> SHR) & 0xffff)) 
    M0 = MACU(0) + MACU(16) + MACU(32) + MACU(48);
#undef MACU
}

void OPPROTO op_iwmmxt_addsq_M0_wRn(void)
{
    M0 = (int64_t) M0 + (int64_t) M1;
}

void OPPROTO op_iwmmxt_adduq_M0_wRn(void)
{
    M0 += M1;
}

void OPPROTO op_iwmmxt_movq_wRn_M0(void)
{
    M1 = M0;
}

void OPPROTO op_iwmmxt_movl_wCx_T0(void)
{
    env->iwmmxt.cregs[PARAM1] = T0;
}

void OPPROTO op_iwmmxt_movl_T0_wCx(void)
{
    T0 = env->iwmmxt.cregs[PARAM1];
}

void OPPROTO op_iwmmxt_movl_T1_wCx(void)
{
    T1 = env->iwmmxt.cregs[PARAM1];
}

void OPPROTO op_iwmmxt_set_mup(void)
{
    env->iwmmxt.cregs[ARM_IWMMXT_wCon] |= 2;
}

void OPPROTO op_iwmmxt_set_cup(void)
{
    env->iwmmxt.cregs[ARM_IWMMXT_wCon] |= 1;
}

void OPPROTO op_iwmmxt_setpsr_nz(void)
{
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        SIMD64_SET((M0 == 0), SIMD_ZBIT) |
        SIMD64_SET((M0 & (1ULL << 63)), SIMD_NBIT);
}

void OPPROTO op_iwmmxt_negq_M0(void)
{
    M0 = ~M0;
}

#define NZBIT8(x, i) \
    SIMD8_SET(NBIT8((x) & 0xff), SIMD_NBIT, i) | \
    SIMD8_SET(ZBIT8((x) & 0xff), SIMD_ZBIT, i)
#define NZBIT16(x, i) \
    SIMD16_SET(NBIT16((x) & 0xffff), SIMD_NBIT, i) | \
    SIMD16_SET(ZBIT16((x) & 0xffff), SIMD_ZBIT, i)
#define NZBIT32(x, i) \
    SIMD32_SET(NBIT32((x) & 0xffffffff), SIMD_NBIT, i) | \
    SIMD32_SET(ZBIT32((x) & 0xffffffff), SIMD_ZBIT, i)
#define NZBIT64(x) \
    SIMD64_SET(NBIT64(x), SIMD_NBIT) | \
    SIMD64_SET(ZBIT64(x), SIMD_ZBIT)
#define IWMMXT_OP_UNPACK(S, SH0, SH1, SH2, SH3)			\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, b_M0_wRn))(void)	\
{								\
    M0 =							\
        (((M0 >> SH0) & 0xff) << 0) | (((M1 >> SH0) & 0xff) << 8) |	\
        (((M0 >> SH1) & 0xff) << 16) | (((M1 >> SH1) & 0xff) << 24) |	\
        (((M0 >> SH2) & 0xff) << 32) | (((M1 >> SH2) & 0xff) << 40) |	\
        (((M0 >> SH3) & 0xff) << 48) | (((M1 >> SH3) & 0xff) << 56);	\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT8(M0 >> 0, 0) | NZBIT8(M0 >> 8, 1) |		\
        NZBIT8(M0 >> 16, 2) | NZBIT8(M0 >> 24, 3) |		\
        NZBIT8(M0 >> 32, 4) | NZBIT8(M0 >> 40, 5) |		\
        NZBIT8(M0 >> 48, 6) | NZBIT8(M0 >> 56, 7);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, w_M0_wRn))(void)	\
{								\
    M0 =							\
        (((M0 >> SH0) & 0xffff) << 0) |				\
        (((M1 >> SH0) & 0xffff) << 16) |			\
        (((M0 >> SH2) & 0xffff) << 32) |			\
        (((M1 >> SH2) & 0xffff) << 48);				\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT8(M0 >> 0, 0) | NZBIT8(M0 >> 16, 1) |		\
        NZBIT8(M0 >> 32, 2) | NZBIT8(M0 >> 48, 3);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, l_M0_wRn))(void)	\
{								\
    M0 =							\
        (((M0 >> SH0) & 0xffffffff) << 0) |			\
        (((M1 >> SH0) & 0xffffffff) << 32);			\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, ub_M0))(void)	\
{								\
    M0 =							\
        (((M0 >> SH0) & 0xff) << 0) |				\
        (((M0 >> SH1) & 0xff) << 16) |				\
        (((M0 >> SH2) & 0xff) << 32) |				\
        (((M0 >> SH3) & 0xff) << 48);				\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |		\
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, uw_M0))(void)	\
{								\
    M0 =							\
        (((M0 >> SH0) & 0xffff) << 0) |				\
        (((M0 >> SH2) & 0xffff) << 32);				\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, ul_M0))(void)	\
{								\
    M0 = (((M0 >> SH0) & 0xffffffff) << 0);			\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] = NZBIT64(M0 >> 0);	\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, sb_M0))(void)	\
{								\
    M0 =							\
        (EXTEND8H((M0 >> SH0) & 0xff) << 0) |			\
        (EXTEND8H((M0 >> SH1) & 0xff) << 16) |			\
        (EXTEND8H((M0 >> SH2) & 0xff) << 32) |			\
        (EXTEND8H((M0 >> SH3) & 0xff) << 48);			\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |		\
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, sw_M0))(void)	\
{								\
    M0 =							\
        (EXTEND16((M0 >> SH0) & 0xffff) << 0) |			\
        (EXTEND16((M0 >> SH2) & 0xffff) << 32);			\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);		\
}								\
void OPPROTO glue(op_iwmmxt_unpack, glue(S, sl_M0))(void)	\
{								\
    M0 = EXTEND32((M0 >> SH0) & 0xffffffff);			\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] = NZBIT64(M0 >> 0);	\
}
IWMMXT_OP_UNPACK(l, 0, 8, 16, 24)
IWMMXT_OP_UNPACK(h, 32, 40, 48, 56)

#define IWMMXT_OP_CMP(SUFF, Tb, Tw, Tl, O)				\
void OPPROTO glue(op_iwmmxt_, glue(SUFF, b_M0_wRn))(void)	\
{								\
    M0 =							\
        CMP(0, Tb, O, 0xff) | CMP(8, Tb, O, 0xff) |		\
        CMP(16, Tb, O, 0xff) | CMP(24, Tb, O, 0xff) |		\
        CMP(32, Tb, O, 0xff) | CMP(40, Tb, O, 0xff) |		\
        CMP(48, Tb, O, 0xff) | CMP(56, Tb, O, 0xff);		\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT8(M0 >> 0, 0) | NZBIT8(M0 >> 8, 1) |		\
        NZBIT8(M0 >> 16, 2) | NZBIT8(M0 >> 24, 3) |		\
        NZBIT8(M0 >> 32, 4) | NZBIT8(M0 >> 40, 5) |		\
        NZBIT8(M0 >> 48, 6) | NZBIT8(M0 >> 56, 7);		\
}								\
void OPPROTO glue(op_iwmmxt_, glue(SUFF, w_M0_wRn))(void)	\
{								\
    M0 = CMP(0, Tw, O, 0xffff) | CMP(16, Tw, O, 0xffff) |	\
        CMP(32, Tw, O, 0xffff) | CMP(48, Tw, O, 0xffff);	\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |		\
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);		\
}								\
void OPPROTO glue(op_iwmmxt_, glue(SUFF, l_M0_wRn))(void)	\
{								\
    M0 = CMP(0, Tl, O, 0xffffffff) |				\
        CMP(32, Tl, O, 0xffffffff);				\
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =			\
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);		\
}
#define CMP(SHR, TYPE, OPER, MASK) ((((TYPE) ((M0 >> SHR) & MASK) OPER \
            (TYPE) ((M1 >> SHR) & MASK)) ? (uint64_t) MASK : 0) << SHR)
IWMMXT_OP_CMP(cmpeq, uint8_t, uint16_t, uint32_t, ==)
IWMMXT_OP_CMP(cmpgts, int8_t, int16_t, int32_t, >)
IWMMXT_OP_CMP(cmpgtu, uint8_t, uint16_t, uint32_t, >)
#undef CMP
#define CMP(SHR, TYPE, OPER, MASK) ((((TYPE) ((M0 >> SHR) & MASK) OPER \
            (TYPE) ((M1 >> SHR) & MASK)) ? M0 : M1) & ((uint64_t) MASK << SHR))
IWMMXT_OP_CMP(mins, int8_t, int16_t, int32_t, <)
IWMMXT_OP_CMP(minu, uint8_t, uint16_t, uint32_t, <)
IWMMXT_OP_CMP(maxs, int8_t, int16_t, int32_t, >)
IWMMXT_OP_CMP(maxu, uint8_t, uint16_t, uint32_t, >)
#undef CMP
#define CMP(SHR, TYPE, OPER, MASK) ((uint64_t) (((TYPE) ((M0 >> SHR) & MASK) \
            OPER (TYPE) ((M1 >> SHR) & MASK)) & MASK) << SHR)
IWMMXT_OP_CMP(subn, uint8_t, uint16_t, uint32_t, -)
IWMMXT_OP_CMP(addn, uint8_t, uint16_t, uint32_t, +)
#undef CMP
/* TODO Signed- and Unsigned-Saturation */
#define CMP(SHR, TYPE, OPER, MASK) ((uint64_t) (((TYPE) ((M0 >> SHR) & MASK) \
            OPER (TYPE) ((M1 >> SHR) & MASK)) & MASK) << SHR)
IWMMXT_OP_CMP(subu, uint8_t, uint16_t, uint32_t, -)
IWMMXT_OP_CMP(addu, uint8_t, uint16_t, uint32_t, +)
IWMMXT_OP_CMP(subs, int8_t, int16_t, int32_t, -)
IWMMXT_OP_CMP(adds, int8_t, int16_t, int32_t, +)
#undef CMP
#undef IWMMXT_OP_CMP

void OPPROTO op_iwmmxt_avgb_M0_wRn(void)
{
#define AVGB(SHR) ((( \
        ((M0 >> SHR) & 0xff) + ((M1 >> SHR) & 0xff) + PARAM2) >> 1) << SHR)
    M0 =
        AVGB(0) | AVGB(8) | AVGB(16) | AVGB(24) |
        AVGB(32) | AVGB(40) | AVGB(48) | AVGB(56);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        SIMD8_SET(ZBIT8((M0 >> 0) & 0xff), SIMD_ZBIT, 0) |
        SIMD8_SET(ZBIT8((M0 >> 8) & 0xff), SIMD_ZBIT, 1) |
        SIMD8_SET(ZBIT8((M0 >> 16) & 0xff), SIMD_ZBIT, 2) |
        SIMD8_SET(ZBIT8((M0 >> 24) & 0xff), SIMD_ZBIT, 3) |
        SIMD8_SET(ZBIT8((M0 >> 32) & 0xff), SIMD_ZBIT, 4) |
        SIMD8_SET(ZBIT8((M0 >> 40) & 0xff), SIMD_ZBIT, 5) |
        SIMD8_SET(ZBIT8((M0 >> 48) & 0xff), SIMD_ZBIT, 6) |
        SIMD8_SET(ZBIT8((M0 >> 56) & 0xff), SIMD_ZBIT, 7);
#undef AVGB
}

void OPPROTO op_iwmmxt_avgw_M0_wRn(void)
{
#define AVGW(SHR) ((( \
        ((M0 >> SHR) & 0xffff) + ((M1 >> SHR) & 0xffff) + PARAM2) >> 1) << SHR)
    M0 = AVGW(0) | AVGW(16) | AVGW(32) | AVGW(48);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        SIMD16_SET(ZBIT16((M0 >> 0) & 0xffff), SIMD_ZBIT, 0) |
        SIMD16_SET(ZBIT16((M0 >> 16) & 0xffff), SIMD_ZBIT, 1) |
        SIMD16_SET(ZBIT16((M0 >> 32) & 0xffff), SIMD_ZBIT, 2) |
        SIMD16_SET(ZBIT16((M0 >> 48) & 0xffff), SIMD_ZBIT, 3);
#undef AVGW
}

void OPPROTO op_iwmmxt_msadb_M0_wRn(void)
{
    M0 = ((((M0 >> 0) & 0xffff) * ((M1 >> 0) & 0xffff) +
           ((M0 >> 16) & 0xffff) * ((M1 >> 16) & 0xffff)) & 0xffffffff) |
         ((((M0 >> 32) & 0xffff) * ((M1 >> 32) & 0xffff) +
           ((M0 >> 48) & 0xffff) * ((M1 >> 48) & 0xffff)) << 32);
}

void OPPROTO op_iwmmxt_align_M0_T0_wRn(void)
{
    M0 >>= T0 << 3;
    M0 |= M1 << (64 - (T0 << 3));
}

void OPPROTO op_iwmmxt_insr_M0_T0_T1(void)
{
    M0 &= ~((uint64_t) T1 << PARAM1);
    M0 |= (uint64_t) (T0 & T1) << PARAM1;
}

void OPPROTO op_iwmmxt_extrsb_T0_M0(void)
{
    T0 = EXTEND8((M0 >> PARAM1) & 0xff);
}

void OPPROTO op_iwmmxt_extrsw_T0_M0(void)
{
    T0 = EXTEND16((M0 >> PARAM1) & 0xffff);
}

void OPPROTO op_iwmmxt_extru_T0_M0_T1(void)
{
    T0 = (M0 >> PARAM1) & T1;
}

void OPPROTO op_iwmmxt_bcstb_M0_T0(void)
{
    T0 &= 0xff;
    M0 =
        ((uint64_t) T0 << 0) | ((uint64_t) T0 << 8) |
        ((uint64_t) T0 << 16) | ((uint64_t) T0 << 24) |
        ((uint64_t) T0 << 32) | ((uint64_t) T0 << 40) |
        ((uint64_t) T0 << 48) | ((uint64_t) T0 << 56);
}

void OPPROTO op_iwmmxt_bcstw_M0_T0(void)
{
    T0 &= 0xffff;
    M0 =
        ((uint64_t) T0 << 0) | ((uint64_t) T0 << 16) |
        ((uint64_t) T0 << 32) | ((uint64_t) T0 << 48);
}

void OPPROTO op_iwmmxt_bcstl_M0_T0(void)
{
    M0 = ((uint64_t) T0 << 0) | ((uint64_t) T0 << 32);
}

void OPPROTO op_iwmmxt_addcb_M0(void)
{
    M0 =
        ((M0 >> 0) & 0xff) + ((M0 >> 8) & 0xff) +
        ((M0 >> 16) & 0xff) + ((M0 >> 24) & 0xff) +
        ((M0 >> 32) & 0xff) + ((M0 >> 40) & 0xff) +
        ((M0 >> 48) & 0xff) + ((M0 >> 56) & 0xff);
}

void OPPROTO op_iwmmxt_addcw_M0(void)
{
    M0 =
        ((M0 >> 0) & 0xffff) + ((M0 >> 16) & 0xffff) +
        ((M0 >> 32) & 0xffff) + ((M0 >> 48) & 0xffff);
}

void OPPROTO op_iwmmxt_addcl_M0(void)
{
    M0 = (M0 & 0xffffffff) + (M0 >> 32);
}

void OPPROTO op_iwmmxt_msbb_T0_M0(void)
{
    T0 =
        ((M0 >> 7) & 0x01) | ((M0 >> 14) & 0x02) |
        ((M0 >> 21) & 0x04) | ((M0 >> 28) & 0x08) |
        ((M0 >> 35) & 0x10) | ((M0 >> 42) & 0x20) |
        ((M0 >> 49) & 0x40) | ((M0 >> 56) & 0x80);
}

void OPPROTO op_iwmmxt_msbw_T0_M0(void)
{
    T0 =
        ((M0 >> 15) & 0x01) | ((M0 >> 30) & 0x02) |
        ((M0 >> 45) & 0x04) | ((M0 >> 52) & 0x08);
}

void OPPROTO op_iwmmxt_msbl_T0_M0(void)
{
    T0 = ((M0 >> 31) & 0x01) | ((M0 >> 62) & 0x02);
}

void OPPROTO op_iwmmxt_srlw_M0_T0(void)
{
    M0 =
        (((M0 & (0xffffll << 0)) >> T0) & (0xffffll << 0)) |
        (((M0 & (0xffffll << 16)) >> T0) & (0xffffll << 16)) |
        (((M0 & (0xffffll << 32)) >> T0) & (0xffffll << 32)) |
        (((M0 & (0xffffll << 48)) >> T0) & (0xffffll << 48));
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

void OPPROTO op_iwmmxt_srll_M0_T0(void)
{
    M0 =
        ((M0 & (0xffffffffll << 0)) >> T0) |
        ((M0 >> T0) & (0xffffffffll << 32));
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);
}

void OPPROTO op_iwmmxt_srlq_M0_T0(void)
{
    M0 >>= T0;
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] = NZBIT64(M0);
}

void OPPROTO op_iwmmxt_sllw_M0_T0(void)
{
    M0 =
        (((M0 & (0xffffll << 0)) << T0) & (0xffffll << 0)) |
        (((M0 & (0xffffll << 16)) << T0) & (0xffffll << 16)) |
        (((M0 & (0xffffll << 32)) << T0) & (0xffffll << 32)) |
        (((M0 & (0xffffll << 48)) << T0) & (0xffffll << 48));
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

void OPPROTO op_iwmmxt_slll_M0_T0(void)
{
    M0 =
        ((M0 << T0) & (0xffffffffll << 0)) |
        ((M0 & (0xffffffffll << 32)) << T0);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);
}

void OPPROTO op_iwmmxt_sllq_M0_T0(void)
{
    M0 <<= T0;
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] = NZBIT64(M0);
}

void OPPROTO op_iwmmxt_sraw_M0_T0(void)
{
    M0 =
        (((EXTEND16(M0 >> 0) >> T0) & 0xffff) << 0) |
        (((EXTEND16(M0 >> 16) >> T0) & 0xffff) << 16) |
        (((EXTEND16(M0 >> 32) >> T0) & 0xffff) << 32) |
        (((EXTEND16(M0 >> 48) >> T0) & 0xffff) << 48);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

void OPPROTO op_iwmmxt_sral_M0_T0(void)
{
    M0 =
        (((EXTEND32(M0 >> 0) >> T0) & 0xffffffff) << 0) |
        (((EXTEND32(M0 >> 32) >> T0) & 0xffffffff) << 32);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);
}

void OPPROTO op_iwmmxt_sraq_M0_T0(void)
{
    M0 = (int64_t) M0 >> T0;
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] = NZBIT64(M0);
}

void OPPROTO op_iwmmxt_rorw_M0_T0(void)
{
    M0 =
        ((((M0 & (0xffffll << 0)) >> T0) |
          ((M0 & (0xffffll << 0)) << (16 - T0))) & (0xffffll << 0)) |
        ((((M0 & (0xffffll << 16)) >> T0) |
          ((M0 & (0xffffll << 16)) << (16 - T0))) & (0xffffll << 16)) |
        ((((M0 & (0xffffll << 32)) >> T0) |
          ((M0 & (0xffffll << 32)) << (16 - T0))) & (0xffffll << 32)) |
        ((((M0 & (0xffffll << 48)) >> T0) |
          ((M0 & (0xffffll << 48)) << (16 - T0))) & (0xffffll << 48));
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

void OPPROTO op_iwmmxt_rorl_M0_T0(void)
{
    M0 =
        ((M0 & (0xffffffffll << 0)) >> T0) |
        ((M0 >> T0) & (0xffffffffll << 32)) |
        ((M0 << (32 - T0)) & (0xffffffffll << 0)) |
        ((M0 & (0xffffffffll << 32)) << (32 - T0));
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);
}

void OPPROTO op_iwmmxt_rorq_M0_T0(void)
{
    M0 = (M0 >> T0) | (M0 << (64 - T0));
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] = NZBIT64(M0);
}

void OPPROTO op_iwmmxt_shufh_M0_T0(void)
{
    M0 =
        (((M0 >> ((T0 << 4) & 0x30)) & 0xffff) << 0) |
        (((M0 >> ((T0 << 2) & 0x30)) & 0xffff) << 16) |
        (((M0 >> ((T0 << 0) & 0x30)) & 0xffff) << 32) |
        (((M0 >> ((T0 >> 2) & 0x30)) & 0xffff) << 48);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

/* TODO: Unsigned-Saturation */
void OPPROTO op_iwmmxt_packuw_M0_wRn(void)
{
    M0 =
        (((M0 >> 0) & 0xff) << 0) | (((M0 >> 16) & 0xff) << 8) |
        (((M0 >> 32) & 0xff) << 16) | (((M0 >> 48) & 0xff) << 24) |
        (((M1 >> 0) & 0xff) << 32) | (((M1 >> 16) & 0xff) << 40) |
        (((M1 >> 32) & 0xff) << 48) | (((M1 >> 48) & 0xff) << 56);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT8(M0 >> 0, 0) | NZBIT8(M0 >> 8, 1) |
        NZBIT8(M0 >> 16, 2) | NZBIT8(M0 >> 24, 3) |
        NZBIT8(M0 >> 32, 4) | NZBIT8(M0 >> 40, 5) |
        NZBIT8(M0 >> 48, 6) | NZBIT8(M0 >> 56, 7);
}

void OPPROTO op_iwmmxt_packul_M0_wRn(void)
{
    M0 =
        (((M0 >> 0) & 0xffff) << 0) | (((M0 >> 32) & 0xffff) << 16) |
        (((M1 >> 0) & 0xffff) << 32) | (((M1 >> 32) & 0xffff) << 48);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

void OPPROTO op_iwmmxt_packuq_M0_wRn(void)
{
    M0 = (M0 & 0xffffffff) | ((M1 & 0xffffffff) << 32);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);
}

/* TODO: Signed-Saturation */
void OPPROTO op_iwmmxt_packsw_M0_wRn(void)
{
    M0 =
        (((M0 >> 0) & 0xff) << 0) | (((M0 >> 16) & 0xff) << 8) |
        (((M0 >> 32) & 0xff) << 16) | (((M0 >> 48) & 0xff) << 24) |
        (((M1 >> 0) & 0xff) << 32) | (((M1 >> 16) & 0xff) << 40) |
        (((M1 >> 32) & 0xff) << 48) | (((M1 >> 48) & 0xff) << 56);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT8(M0 >> 0, 0) | NZBIT8(M0 >> 8, 1) |
        NZBIT8(M0 >> 16, 2) | NZBIT8(M0 >> 24, 3) |
        NZBIT8(M0 >> 32, 4) | NZBIT8(M0 >> 40, 5) |
        NZBIT8(M0 >> 48, 6) | NZBIT8(M0 >> 56, 7);
}

void OPPROTO op_iwmmxt_packsl_M0_wRn(void)
{
    M0 =
        (((M0 >> 0) & 0xffff) << 0) | (((M0 >> 32) & 0xffff) << 16) |
        (((M1 >> 0) & 0xffff) << 32) | (((M1 >> 32) & 0xffff) << 48);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT16(M0 >> 0, 0) | NZBIT16(M0 >> 16, 1) |
        NZBIT16(M0 >> 32, 2) | NZBIT16(M0 >> 48, 3);
}

void OPPROTO op_iwmmxt_packsq_M0_wRn(void)
{
    M0 = (M0 & 0xffffffff) | ((M1 & 0xffffffff) << 32);
    env->iwmmxt.cregs[ARM_IWMMXT_wCASF] =
        NZBIT32(M0 >> 0, 0) | NZBIT32(M0 >> 32, 1);
}

void OPPROTO op_iwmmxt_muladdsl_M0_T0_T1(void)
{
    M0 += EXTEND32(T0) * EXTEND32(T1);
}

void OPPROTO op_iwmmxt_muladdsw_M0_T0_T1(void)
{
    M0 += EXTEND32(EXTEND16((T0 >> 0) & 0xffff) *
                   EXTEND16((T1 >> 0) & 0xffff));
    M0 += EXTEND32(EXTEND16((T0 >> 16) & 0xffff) *
                   EXTEND16((T1 >> 16) & 0xffff));
}

void OPPROTO op_iwmmxt_muladdswl_M0_T0_T1(void)
{
    M0 += EXTEND32(EXTEND16(T0 & 0xffff) * EXTEND16(T1 & 0xffff));
}
