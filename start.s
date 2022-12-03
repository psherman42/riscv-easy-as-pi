################################################
##
## x
## 2020-04-02 pds    initial cut
##

.section .text

###############################################
##
## entry point main reset vector
##

.globl _start
_start:
  lui a1, 0x80004
  addi sp, a1, -4


  li a0, 0x98989898 # unlock and disable addr
  csrrc x0, pmpcfg0, a0
  csrrc x0, pmpcfg1, a0
  csrrc x0, pmpcfg2, a0
  csrrc x0, pmpcfg3, a0
  li a0, 07070707 # enable R, W, X
  csrrs x0, pmpcfg0, a0
  csrrs x0, pmpcfg1, a0
  csrrs x0, pmpcfg2, a0
  csrrs x0, pmpcfg3, a0

  li a0, 0x00001aaa # disable MPP, SPP. MPIE, SPIE, MIE, SIE
  csrrc x0, mstatus, a0

  #li a0, 0x1800 # MPP=3
  #csrw mstatus, a0

  csrw mip, zero
  csrw mie, zero

  .option push
pcrel_addr:
  auipc gp, %hi(pcrel_addr)
  addi gp, gp, %lo(pcrel_addr)
  .option pop

  jal main
  j .

