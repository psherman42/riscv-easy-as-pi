#ifndef CLOCK_H__INCLUDED
#define CLOCK_H__INCLUDED

#define MTIME 0x0200BFF8

enum prci_clk_mode_t { PRCI_INT, PRCI_EXT, PRCI_EXT_DIR };

uint32_t clock_init (enum prci_clk_mode_t clk_mode);

#endif//CLOCK_H__INCLUDED
