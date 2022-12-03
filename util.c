////////////////////////////////////////////////
// utility functions
//
// 2020-04-02 pds    initial cut
//

#include <stdint.h>  // for uint32_t
#include <stddef.h>  // for size_t


void tostr (char *str, size_t len, uint32_t val)
{
  int i;
  for (i=1; i<=len; i++)
  {
    str[len - i] = (char) ((val % 10UL) + '0');
    val /= 10;
  }

  str[i - 1] = '\0';
}

size_t strlen (char *str)
{
  size_t len=0;
  while (str[len] != '\0')
  {
    len++;
  }
  return len;
}
