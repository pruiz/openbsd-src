/*	$OpenBSD: rtld_machine.c,v 1.1 2001/09/23 00:14:56 drahn Exp $ */

/*
 * Copyright (c) 1999 Dale Rahn
 * Copyright (c) 2001 Niklas Hallqvist
 * Copyright (c) 2001 Artur Grabowski
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed under OpenBSD by
 *	Dale Rahn.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define _DYN_LOADER

#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/mman.h>

#include <nlist.h>
#include <link.h>

#include "syscall.h"
#include "archdep.h"
#include "resolve.h"

void
_dl_bcopy(void *src, void *dest, int size)
{
	unsigned char *psrc, *pdest;
	int i;
	psrc = src;
	pdest = dest;
	for (i = 0; i < size; i++) {
		pdest[i] = psrc[i];
	}
}

int
_dl_md_reloc(elf_object_t *object, int rel, int relasz)
{
	long	i;
	long	numrela;
	long	fails = 0;
	Elf64_Addr loff;
	Elf64_Rela  *relas;
	load_list_t *llist;

	loff   = object->load_offs;
	numrela = object->Dyn.info[relasz] / sizeof(Elf64_Rela);
	relas = (Elf64_Rela *)(object->Dyn.info[rel]);

	if ((object->status & STAT_RELOC_DONE) || !relas) {
		return(0);
	}

	/*
	 * unprotect some segments if we need it.
	 * XXX - we unprotect waay to much. only the text can have cow
	 *       relocations.
	 */
	if ((rel == DT_REL || rel == DT_RELA)) {
		for (llist = object->load_list; llist != NULL; llist = llist->next) {
			if (!(llist->prot & PROT_WRITE)) {
				_dl_mprotect(llist->start, llist->size,
					llist->prot|PROT_WRITE);
			}
		}
	}

	for (i = 0; i < numrela; i++, relas++) {
		Elf64_Addr *r_addr;
		Elf64_Addr ooff;
		const Elf64_Sym *sym, *this;
		const char *symn;

		r_addr = (Elf64_Addr *)(relas->r_offset + loff);

		if (ELF64_R_SYM(relas->r_info) == 0xffffffff) {
			continue;
		}

		sym = object->dyn.symtab;
		sym += ELF64_R_SYM(relas->r_info);
		symn = object->dyn.strtab + sym->st_name;

		this = NULL;
		switch (ELF64_R_TYPE(relas->r_info)) {
		case R_TYPE(64):
			ooff = _dl_find_symbol(symn, _dl_objects, &this, 0, 1);
			if (this == NULL)
				goto resolve_failed;
			*r_addr += ooff + this->st_value + relas->r_addend;
			break;
		case R_TYPE(RELATIVE):
			/*
			 * There is a lot of unaligned RELATIVE
			 * relocs generated by gcc in the exception handlers.
			 */
			if ((((Elf_Addr) r_addr) & 0x7) != 0) {
				Elf_Addr tmp;
#if 0
_dl_printf("unaligned RELATIVE: %p type: %d %s 0x%lx -> 0x%lx\n", r_addr,
ELF_R_TYPE(relas->r_info), object->load_name, *r_addr, *r_addr+loff);
#endif
				_dl_bcopy(r_addr, &tmp, sizeof(Elf_Addr));
				tmp += loff;
				_dl_bcopy(&tmp, r_addr, sizeof(Elf_Addr));
			} else {
				*r_addr += loff;
			}
			break;
		case R_TYPE(JMP_SLOT):
			ooff = _dl_find_symbol(symn, _dl_objects, &this, 0, 1);
			if (this == NULL)
				goto resolve_failed;
			*r_addr = ooff + this->st_value + relas->r_addend;
			break;
		case R_TYPE(GLOB_DAT):
			ooff = _dl_find_symbol(symn, _dl_objects, &this, 0, 1);
			if (this == NULL)
				goto resolve_failed;
			*r_addr = ooff + this->st_value + relas->r_addend;
			break;
		case R_TYPE(NONE):
			break;
		default:
			_dl_printf("%s:"
				" %s: unsupported relocation '%s' %d at %lx\n",
					_dl_progname, object->load_name, symn,
					ELF64_R_TYPE(relas->r_info), r_addr );
			_dl_exit(1);
		}
		continue;
resolve_failed:
		_dl_printf("%s: %s :can't resolve reference '%s'\n",
			_dl_progname, object->load_name, symn);
		fails++;
	}
	/*
	 * flush ??  - XXX
	__asm __volatile("imb" : : : "memory");
	 */

	/* reprotect the unprotected segments */
	if ((rel == DT_REL || rel == DT_RELA)) {
		for (llist = object->load_list; llist != NULL; llist = llist->next) {
			if (!(llist->prot & PROT_WRITE)) {
				_dl_mprotect(llist->start, llist->size,
					llist->prot);
			}
		}
	}

	return (fails);
}

/*
 * Resolve a symbol at run-time.
 */
void *
_dl_bind(elf_object_t *object, Elf_Word reloff)
{
	Elf_RelA *rela;
	Elf_Addr *addr, ooff;
	const Elf_Sym *sym, *this;
	const char *symn;

	rela = (Elf_RelA *)(object->Dyn.info[DT_JMPREL] + reloff);

	sym = object->dyn.symtab;
	sym += ELF64_R_SYM(rela->r_info);
	symn = object->dyn.strtab + sym->st_name;

	addr = (Elf_Addr *)(object->load_offs + rela->r_offset);
	ooff = _dl_find_symbol(symn, _dl_objects, &this, 0, 1);
	if (this == NULL) {
		_dl_printf("lazy binding failed!\n");
		*((int *)0) = 0;	/* XXX */
	}
	*addr = ooff + this->st_value + rela->r_addend;

	return (void *)*addr;
}

/*
 *	Relocate the Global Offset Table (GOT).
 */
void
_dl_md_reloc_got(elf_object_t *object, int lazy)
{
	Elf_Addr *pltgot;
	extern void _dl_bind_start(void);	/* XXX */

	lazy = 0; /* not yet */
	pltgot = (Elf_Addr *)object->Dyn.info[DT_PLTGOT];

	if (object->obj_type == OBJTYPE_LDR || !lazy || pltgot == NULL) {
		_dl_md_reloc(object, DT_JMPREL, DT_PLTRELSZ);
		return;
	}

#ifdef SUPPORT_LAZY
	if (object->obj_type != OBJTYPE_EXE) {
		int i, size;
		Elf_Addr *addr;
		Elf_RelA *rela;

		size = object->Dyn.info[DT_PLTRELSZ] / sizeof(Elf_RelA);
		rela = (Elf_RelA *)(object->Dyn.info[DT_JMPREL]);

		for (i = 0; i < size; i++) {
			addr = (Elf_Addr *)(object->load_offs + rela[i].r_offset);
			*addr += object->load_offs;
		}
	}

	pltgot[2] = (Elf_Addr)_dl_bind_start;
	pltgot[3] = (Elf_Addr)object;
#endif /* SUPPORT_LAZY */
}
