/* memcmp -- compare two memory regions.
   This function is in the public domain.  */

/*

@deftypefn Supplemental int memcmp (const void *@var{x}, const void *@var{y}, size_t @var{count})

Compares the first @var{count} bytes of two areas of memory.  Returns
zero if they are the same, a value less than zero if @var{x} is
lexically less than @var{y}, or a value greater than zero if @var{x}
is lexically greater than @var{y}.  Note that lexical order is determined
as if comparing unsigned char arrays.

@end deftypefn

*/

#include <ansidecl.h>
#ifdef __STDC__
#include <stddef.h>
#else
#define size_t unsigned long
#endif

int
DEFUN(memcmp, (str1, str2, count),
      const PTR str1 AND const PTR str2 AND size_t count)
{
  register unsigned char *s1 = (unsigned char*)str1;
  register unsigned char *s2 = (unsigned char*)str2;

  while (count-- > 0)
    {
      if (*s1++ != *s2++)
	  return s1[-1] < s2[-1] ? -1 : 1;
    }
  return 0;
}

