/* This file is is generated by a shell script.  DO NOT EDIT! */

/* emulate the original gld for the given sh
   Copyright 1991, 1993, 1995, 2000 Free Software Foundation, Inc.
   Written by Steve Chamberlain steve@cygnus.com

This file is part of GLD, the Gnu Linker.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#define TARGET_IS_sh

#include "libiberty.h"
#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"

#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"

#include "ldexp.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"

static void gldsh_before_parse PARAMS ((void));
static char *gldsh_get_script PARAMS ((int *isfile));

static void
gldsh_before_parse()
{
#ifndef TARGET_			/* I.e., if not generic.  */
  ldfile_output_architecture = bfd_arch_sh;
#endif /* not TARGET_ */
}

static char *
gldsh_get_script(isfile)
     int *isfile;
{			     
  *isfile = 0;

  if (link_info.relocateable == true && config.build_constructors == true)
    return
concat(
"OUTPUT_FORMAT(\"coff-sh\")\n\
OUTPUT_ARCH(sh)\n\
MEMORY\n\
{\n\
  ram : o = 0x1000, l = 512k\n\
}\n\
 "," SECTIONS\n\
{\n\
 ","  .text :\n\
  {\n\
    *(.text)\n\
    *(.strings)\n\
  } \n\
  .tors :\n\
  {\n\
    ___ctors = . ;\n\
    *(.ctors)\n\
    ___ctors_end = . ;\n\
    ___dtors = . ;\n\
    *(.dtors)\n\
    ___dtors_end = . ;\n\
  } \n\
 ","  .data :\n\
  {\n\
    *(.data)\n\
  } \n\
 "," .bss :\n\
  {\n\
    *(.bss)\n\
    *(COMMON)\n\
  } \n\
 "," .stack   :\n\
  {\n\
    *(.stack)\n\
  } \n\
 "," .stab 0  :\n\
  {\n\
    *(.stab)\n\
  }\n\
 "," .stabstr 0  :\n\
  {\n\
    *(.stabstr)\n\
  }\n\
}\n\n", NULL)
  ; else if (link_info.relocateable == true) return
concat (
"OUTPUT_FORMAT(\"coff-sh\")\n\
OUTPUT_ARCH(sh)\n\
 "," MEMORY\n\
{\n\
  ram : o = 0x1000, l = 512k\n\
}\n\
 "," SECTIONS\n\
{\n\
 ","  .text :\n\
  {\n\
    *(.text)\n\
    *(.strings)\n\
  } \n\
 ","   .tors :\n\
  {\n\
    ___ctors = . ;\n\
    *(.ctors)\n\
    ___ctors_end = . ;\n\
    ___dtors = . ;\n\
    *(.dtors)\n\
    ___dtors_end = . ;\n\
  } \n\
 ","   .data :\n\
  {\n\
    *(.data)\n\
  } \n\
 ","   .bss :\n\
  {\n\
    *(.bss)\n\
    *(COMMON)\n\
  } \n\
 ","   .stack   :\n\
  {\n\
    *(.stack)\n\
  } \n\
 ","   .stab 0  :\n\
  {\n\
    *(.stab)\n\
  }\n\
 ","   .stabstr 0  :\n\
  {\n\
    *(.stabstr)\n\
  }\n\
}\n\n", NULL)
  ; else if (!config.text_read_only) return
concat (
"OUTPUT_FORMAT(\"coff-sh\")\n\
OUTPUT_ARCH(sh)\n\
MEMORY\n\
{\n\
  ram : o = 0x1000, l = 512k\n\
}\n\
SECTIONS\n\
{\n\
 ","   .text :\n\
  {\n\
    *(.text)\n\
    *(.strings)\n\
     _etext = . ; \n\
  }  > ram\n\
 ","   .tors :\n\
  {\n\
    ___ctors = . ;\n\
    *(.ctors)\n\
    ___ctors_end = . ;\n\
    ___dtors = . ;\n\
    *(.dtors)\n\
    ___dtors_end = . ;\n\
  }  > ram\n\
 ","   .data :\n\
  {\n\
    *(.data)\n\
     _edata = . ; \n\
  }  > ram\n\
 ","   .bss :\n\
  {\n\
     _bss_start = . ; \n\
    *(.bss)\n\
    *(COMMON)\n\
     _end = . ;  \n\
  }  > ram\n\
 ","   .stack  0x30000   :\n\
  {\n\
     _stack = . ; \n\
    *(.stack)\n\
  }  > ram\n\
 ","   .stab 0 (NOLOAD) :\n\
  {\n\
    *(.stab)\n\
  }\n\
 ","   .stabstr 0 (NOLOAD) :\n\
  {\n\
    *(.stabstr)\n\
  }\n\
}\n\n", NULL)
  ; else if (!config.magic_demand_paged) return
concat (
"OUTPUT_FORMAT(\"coff-sh\")\n\
OUTPUT_ARCH(sh)\n\
MEMORY\n\
{\n\
  ram : o = 0x1000, l = 512k\n\
}\n\
SECTIONS\n\
{\n\
 ","   .text :\n\
  {\n\
    *(.text)\n\
    *(.strings)\n\
     _etext = . ; \n\
  }  > ram\n\
 ","   .tors :\n\
  {\n\
    ___ctors = . ;\n\
    *(.ctors)\n\
    ___ctors_end = . ;\n\
    ___dtors = . ;\n\
    *(.dtors)\n\
    ___dtors_end = . ;\n\
  }  > ram\n\
 ","   .data :\n\
  {\n\
    *(.data)\n\
     _edata = . ; \n\
  }  > ram\n\
 ","   .bss :\n\
  {\n\
     _bss_start = . ; \n\
    *(.bss)\n\
    *(COMMON)\n\
     _end = . ;  \n\
  }  > ram\n\
 ","   .stack  0x30000   :\n\
  {\n\
     _stack = . ; \n\
    *(.stack)\n\
  }  > ram\n\
 ","   .stab 0 (NOLOAD) :\n\
  {\n\
    *(.stab)\n\
  }\n\
 ","   .stabstr 0 (NOLOAD) :\n\
  {\n\
    *(.stabstr)\n\
  }\n\
}\n\n", NULL)
  ; else return
concat (
"OUTPUT_FORMAT(\"coff-sh\")\n\
OUTPUT_ARCH(sh)\n\
MEMORY\n\
{\n\
  ram : o = 0x1000, l = 512k\n\
}\n\
SECTIONS\n\
{\n\
 ","   .text :\n\
  {\n\
    *(.text)\n\
    *(.strings)\n\
     _etext = . ; \n\
  }  > ram\n\
 ","   .tors :\n\
  {\n\
    ___ctors = . ;\n\
    *(.ctors)\n\
    ___ctors_end = . ;\n\
    ___dtors = . ;\n\
    *(.dtors)\n\
    ___dtors_end = . ;\n\
  }  > ram\n\
 ","   .data :\n\
  {\n\
    *(.data)\n\
     _edata = . ; \n\
  }  > ram\n\
 ","   .bss :\n\
  {\n\
     _bss_start = . ; \n\
    *(.bss)\n\
    *(COMMON)\n\
     _end = . ;  \n\
  }  > ram\n\
 ","   .stack  0x30000   :\n\
  {\n\
     _stack = . ; \n\
    *(.stack)\n\
  }  > ram\n\
 ","   .stab 0 (NOLOAD) :\n\
  {\n\
    *(.stab)\n\
  }\n\
 ","   .stabstr 0 (NOLOAD) :\n\
  {\n\
    *(.stabstr)\n\
  }\n\
}\n\n", NULL)
; }

struct ld_emulation_xfer_struct ld_sh_emulation = 
{
  gldsh_before_parse,
  syslib_default,
  hll_default,
  after_parse_default,
  after_open_default,
  after_allocation_default,
  set_output_arch_default,
  ldemul_default_target,
  before_allocation_default,
  gldsh_get_script,
  "sh",
  "coff-sh"
};
