#ifndef DEFS_H__INCLUDED
#define DEFS_H__INCLUDED

#define mask(wid) ( (2^(wid)) - 1 )

#define reg(base,off) ( *((volatile uint32_t *) ((base) + (offset))) )

#define set(base,reg,wid,pos,value) (\
  reg((base),(reg)) = (\
      ( reg((base),(reg)) & ~(mask(wid) << (pos)) )\
    | ( ((value) & mask(wid)) << (pos)) )\
  )\
)

#define get(base,reg,wid,pos) (\
  ( (reg((base),(reg)) >> (pos)) & mask(wid) )\
)

#endif//DEFS_H__INCLUDED
