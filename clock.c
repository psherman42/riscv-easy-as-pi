////////////////////////////////////////////////
// system clock
//
// 2020-04-02 pds    initial cut
// 2023-06-04 pds    BUG FIX: prci_hfxoscrdy() incorrectly specified prci_hfrosccfg
//

#include <stdint.h>  // for uint32_t
#include <stddef.h>  // for size_t
#include "clock.h"

///////////////////////////////////////////////
//
// low level clock routines
//

#define prci_base 0x10008000

//
// HFROSCCFG
//

#define prci_hfrosccfg    (*(volatile uint32_t *) (prci_base + 0x00))

// 0=div1, 1=div2, ..., 63=div64. 1.125MHz-72MHz
#define prci_hfroscdiv(x)  ( prci_hfrosccfg = (( prci_hfrosccfg & ~(0x3F << 0)) | (( (x) & 0x3F) << 0)) )

#define prci_hfrosctrim(x)  ( prci_hfrosccfg = (( prci_hfrosccfg & ~(0x1F << 16)) | (( (x) & 0x1F) << 16)) )

// 0=dis, 1=ena
#define prci_hfroscen(x)  ( prci_hfrosccfg = (( prci_hfrosccfg & ~(0x1 << 30)) | (( (x) & 0x1) << 30)) )

// 0=not-ready, 1=ready
#define prci_hfroscrdy()  ( ((prci_hfrosccfg >> 31) & 0x1) )

//
// HFXOSCCFG
//

#define prci_hfxosccfg    (*(volatile uint32_t *) (prci_base + 0x04))

// 0=dis, 1=ena
#define prci_hfxoscen(x)  ( prci_hfxosccfg = (( prci_hfxosccfg & ~(0x1 << 30)) | (( (x) & 0x1) << 30)) )

// 0=not-ready, 1=ready
#define prci_hfxoscrdy()  ( ((prci_hfxosccfg >> 31) & 0x1) )  // 2023-06-04 pds. BUG FIX, was prci_hfrosccfg (thanks, @BEforlin)

//
// PLLCFG
//

#define prci_pllcfg       (*(volatile uint32_t *) (prci_base + 0x08))

// 0=div1, 1=div2, 2=div3, 3=div4
#define prci_pllr(x)  ( prci_pllcfg = (( prci_pllcfg & ~(0x7 << 0)) | (( (x) & 0x7) << 0)) )

#define prci_pllf(x)  ( prci_pllcfg = (( prci_pllcfg & ~(0x3F << 4)) | (( (x) & 0x3F) << 4)) )

// 0=na, 1=div2, 2=div4, 3=div8
#define prci_pllq(x)  ( prci_pllcfg = (( prci_pllcfg & ~(0x3 << 10)) | (( (x) & 0x3) << 10)) )

// 0=hfroscclk-drives-hfclk, 1=pll-drives-hfclk
#define prci_pllsel(x)  ( prci_pllcfg = (( prci_pllcfg & ~(0x1 << 16)) | (( (x) & 0x1) << 16)) )

#define prci_pllrefsel(x)  ( prci_pllcfg = (( prci_pllcfg & ~(0x1 << 17)) | (( (x) & 0x1) << 17)) )

#define prci_pllbypass(x)  ( prci_pllcfg = (( prci_pllcfg & ~(0x1 << 18)) | (( (x) & 0x1) << 18)) )

// 0=unlocked, 1=locked. requires 100us to acquire
#define prci_plllock()  ( ((prci_pllcfg >> 31) & 0x1) )

//
// PLLOUTDIV(CFG)
//

#define prci_plloutdivcfg    (*(volatile uint32_t *) (prci_base + 0x0C))

#define prci_plloutdiv(x)  ( prci_plloutdivcfg = (( prci_plloutdivcfg & ~(0x3F << 0)) | (( (x) & 0x3F) << 0)) )

// 0=div, 1=no-div ... MANUAL TYPO BUG? should be one bit (0x1) not six (0x3F)?
#define prci_plloutdivby1(x)  ( prci_plloutdivcfg = (( prci_plloutdivcfg & ~(0x3F << 8)) | (( (x) & 0x3F) << 8)) )

//
// LFROSCCFG
//

#define prci_lfrosccfg    (*(volatile uint32_t *) (prci_base + 0x70))

// 1.5-230 KHz
#define prci_lfroscdiv(x)  ( prci_lfrosccfg = (( prci_lfrosccfg & ~(0x3F << 0)) | (( (x) & 0x3F) << 0)) )

#define prci_lfrosctrim(x)  ( prci_lfrosccfg = (( prci_lfrosccfg & ~(0x1F << 16)) | (( (x) & 0x1F) << 16)) )

// 0=dis, 1=ena
#define prci_lfroscen(x)  ( prci_lfrosccfg = (( prci_lfrosccfg & ~(0x1 << 30)) | (( (x) & 0x1) << 30)) )

// 0=not-ready, 1=ready
#define prci_lfroscrdy()  ( ((prci_lfrosccfg >> 31) & 0x1) )

//
// LFCLKMUX
//

#define prci_lfclkmux     (*(volatile uint32_t *) (prci_base + 0x7C))

// 0=ena, 1=dis
#define prci_lfextclk_sel(x)  ( prci_lfclkmux = (( prci_lfclkmux & ~(0x3F << 8)) | (( (x) & 0x3F) << 8)) )

// status of psdlfaltclk pad
#define prci_lfextclk()  ( ((prci_lfclkmux >> 31) & 0x1) )

//
// PROCMONCFG
//

#define prci_procmoncfg   (*(volatile uint32_t *) (prci_base + 0xF0))
#define prci_procfoo(x)  ( prci_procmoncfg = (( prci_procmoncfg & ~(0x3F << 8)) | (( (x) & 0x3F) << 8)) )
#define prci_procbar()  ( ((prci_procmoncfg >> 31) & 0x1) )

//
// CORECLKCFG (not on the FE310?)
//

#define prci_coreclkcfg   (*(volatile uint32_t *) (prci_base + 0x10))
#define prci_corefoo(x)  ( prci_coreclkcfg = (( prci_procmoncfg & ~(0x3F << 8)) | (( (x) & 0x3F) << 8)) )
#define prci_corebar()  ( ((prci_coreclkcfg >> 31) & 0x1) )


///////////////////////////////////////////////
//
// high level clock routines
//

// select external direct mode
uint32_t clock_init (enum prci_clk_mode_t clk_mode)
{
  uint32_t clk_hz;

  switch(clk_mode)
  {
    case PRCI_INT:
      clk_hz = 16000000;
      break;

    case PRCI_EXT:
      clk_hz = 16000000;
      break;

    case PRCI_EXT_DIR:  // assuming a 16mhz xtal
      prci_hfxoscen( 1 );
      while (prci_hfxoscrdy() != 0x01 );  // timeout return

      prci_pllr( 0x1 );  // R=2
      prci_pllf( 0x1f );  // F=64
      prci_pllq( 0x3 );  // Q=8
      prci_plloutdivby1(1); prci_plloutdiv(0);

      prci_pllrefsel( 1 );
      prci_pllbypass( 1 );
      prci_pllsel(1);

      prci_hfroscen( 0 );
      clk_hz = 16000000;
      break;

    default:
      clk_hz = 0;
      break;
  }

  return clk_hz;
}

