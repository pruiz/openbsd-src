/*	$OpenPackages$ */
/*	$OpenBSD: lowparse.c,v 1.7 2001/05/07 22:57:19 espie Exp $ */

/* low-level parsing functions. */

/*
 * Copyright (c) 1999,2000 Marc Espie.
 *
 * Extensive code changes for the OpenBSD project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE OPENBSD PROJECT AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OPENBSD
 * PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "make.h"
#include "buf.h"
#include "lowparse.h"

#ifdef CLEANUP
static LIST	    fileNames;	/* file names to free at end */
#endif

/* Definitions for handling #include specifications */
typedef struct IFile_ {
    char		*fname; /* name of file */
    unsigned long	lineno; /* line number */
    FILE		*F;	/* open stream */
    char		*str;	/* read from char area */
    char		*ptr;	/* where we are */
    char		*end;	/* don't overdo it */
} IFile;

static IFile	*current;

static LIST	includes;	/* stack of IFiles generated by
				 * #includes */

static IFile *new_ifile(char *, FILE *);
static IFile *new_istring(char *, char *, unsigned long);
static void free_ifile(IFile *);
static void ParseVErrorInternal(char *, unsigned long, int, char *, va_list);
static int newline(void);
static int skiptoendofline(void);
static void ParseFoldLF(Buffer, int);
static int ParseSkipEmptyLines(Buffer);
static int	    fatals = 0;

/*-
 * ParseVErrorInternal	--
 *	Error message abort function for parsing. Prints out the context
 *	of the error (line number and file) as well as the message with
 *	two optional arguments.
 *
 * Side Effects:
 *	"fatals" is incremented if the level is PARSE_FATAL.
 */
/* VARARGS */
static void
#ifdef __STDC__
ParseVErrorInternal(char *cfname, unsigned long clineno, int type, char *fmt,
    va_list ap)
#else
ParseVErrorInternal(va_alist)
	va_dcl
#endif
{
	(void)fprintf(stderr, "\"%s\", line %lu: ", cfname, clineno);
	if (type == PARSE_WARNING)
		(void)fprintf(stderr, "warning: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	if (type == PARSE_FATAL)
		fatals ++;
}

/*-
 * Parse_Error	--
 *	External interface to ParseVErrorInternal; uses the default filename
 *	Line number.
 */
/* VARARGS */
void
#ifdef __STDC__
Parse_Error(int type, char *fmt, ...)
#else
Parse_Error(va_alist)
	va_dcl
#endif
{
	va_list ap;
#ifdef __STDC__
	va_start(ap, fmt);
#else
	int type;		/* Error type (PARSE_WARNING, PARSE_FATAL) */
	char *fmt;

	va_start(ap);
	type = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif

	ParseVErrorInternal(current->fname, current->lineno, type, fmt, ap);
}

static IFile *
new_ifile(name, stream)
    char *name;
    FILE *stream;
{
    IFile *ifile;
#ifdef CLEANUP
    Lst_AtEnd(&fileNames, name);
#endif

    ifile = emalloc(sizeof(*ifile));
    ifile->fname = name;
    /* Naturally enough, we start reading at line 0 */
    ifile->lineno = 0;
    ifile->F = stream;
    ifile->ptr = ifile->end = NULL;
    return ifile;
}

static void
free_ifile(ifile)
    IFile *ifile;
{
    if (ifile->F && fileno(ifile->F) != STDIN_FILENO)
	(void)fclose(ifile->F);
    else
	free(ifile->str);
    /* Note we can't free the file names yet, as they are embedded in GN for
     * error reports. */
    free(ifile);
}

static IFile *
new_istring(str, name, lineno)
    char *str;
    char *name;
    unsigned long lineno;
{
    IFile *ifile;

    ifile = emalloc(sizeof(*ifile));
    /* No malloc, name is always taken from an already existing ifile */
    ifile->fname = name;
    ifile->F = NULL;
    /* Strings are used from for loops... */
    ifile->lineno = lineno;
    ifile->ptr = ifile->str = str;
    ifile->end = str + strlen(str);
    return ifile;
}


/*-
 *---------------------------------------------------------------------
 * Parse_FromString  --
 *	Start Parsing from the given string
 *
 * Side Effects:
 *	A structure is added to the includes Lst and readProc, lineno,
 *	fname and curFILE are altered for the new file
 *---------------------------------------------------------------------
 */
void
Parse_FromString(str, lineno)
    char		*str;
    unsigned long	lineno;
{
    if (DEBUG(FOR))
	(void)fprintf(stderr, "%s\n----\n", str);

    if (current != NULL)
	Lst_AtFront(&includes, current);
    current = new_istring(str, current->fname, lineno);
}


void
Parse_FromFile(name, stream)
    char		*name;
    FILE		*stream;
{
    if (current != NULL)
	Lst_AtFront(&includes, current);
    current = new_ifile(name, stream);
}

/*-
 *---------------------------------------------------------------------
 * Parse_NextFile --
 *	Called when EOF is reached in the current file. If we were reading
 *	an include file, the includes stack is popped and things set up
 *	to go back to reading the previous file at the previous location.
 *
 * Results:
 *	CONTINUE if there's more to do. DONE if not.
 *
 * Side Effects:
 *	The old curFILE, is closed. The includes list is shortened.
 *	lineno, curFILE, and fname are changed if CONTINUE is returned.
 *---------------------------------------------------------------------
 */
Boolean
Parse_NextFile()
{
    IFile *next;

    next = (IFile *)Lst_DeQueue(&includes);
    if (next != NULL) {
	if (current != NULL)
	    free_ifile(current);
	current = next;
	return TRUE;
    } else
	return FALSE;
}

/* guts for ParseReadc. Grab a new line off fgetln when we hit "\n" */
static int
newline()
{
    size_t len;

    if (current->F) {
	current->ptr = fgetln(current->F, &len);
	if (current->ptr) {
	    current->end = current->ptr + len;
	    return *current->ptr++;
	} else {
	    current->end = NULL;
	}
    }
    return EOF;
}

#define ParseReadc()	current->ptr < current->end ? *current->ptr++ : newline()

/* Take advantage of fgetln: we don't have to scan a whole line to skip it.
 */
static int
skiptoendofline()
{
    if (current->F) {
	if (current->end - current->ptr > 1)
	    current->ptr = current->end - 1;
	if (*current->ptr == '\n')
	    return *current->ptr++;
	return EOF;
    } else {
	int c;

	do {
	    c = ParseReadc();
	} while (c != '\n' && c != EOF);
	return c;
    }
}


/* ParseSkipGetLine():
 *	Return the first logical line that starts with '.'
 */
char *
ParseSkipGetLine(linebuf)
    Buffer linebuf;
{
    int c;

	/* If first char isn't dot, skip to end of line, handling \ */
    while ((c = ParseReadc()) != '.') {
	for (;c != '\n'; c = ParseReadc()) {
	    if (c == '\\') {
		c = ParseReadc();
		if (c == '\n')
		    current->lineno++;
	    }
	    if (c == EOF) {
		Parse_Error(PARSE_FATAL, "Unclosed conditional");
		return NULL;
	    }
	}
	current->lineno++;
    }

	/* This is the line we need to copy */
    return ParseGetLine(linebuf, "conditional");
}

/* Grab logical line into linebuf.
 * The first character is already in c. */
static void
ParseFoldLF(linebuf, c)
    Buffer linebuf;
    int c;
{
    for (;;) {
	if (c == '\n') {
	    current->lineno++;
	    break;
	}
	if (c == EOF)
	    break;
	Buf_AddChar(linebuf, c);
	c = ParseReadc();
	while (c == '\\') {
	    c = ParseReadc();
	    if (c == '\n') {
		Buf_AddSpace(linebuf);
		current->lineno++;
		do {
		    c = ParseReadc();
		} while (c == ' ' || c == '\t');
	    } else {
		Buf_AddChar(linebuf, '\\');
		if (c == '\\') {
		    Buf_AddChar(linebuf, '\\');
		    c = ParseReadc();
		}
		break;
	    }
	}
    }
}

/* ParseGetLine:
 *	Simply get line, no parsing beyond \
 */
char *
ParseGetLine(linebuf, type)
    Buffer	linebuf;
    const char	*type;
{
    int c;

    Buf_Reset(linebuf);
    c = ParseReadc();
    if (c == EOF) {
	Parse_Error(PARSE_FATAL, "Unclosed %s", type);
	return NULL;
    }

    /* Handle '\' at beginning of line, since \\n needs special treatment */
    while (c == '\\') {
	c = ParseReadc();
	if (c == '\n') {
	    current->lineno++;
	    do {
		c = ParseReadc();
	    } while (c == ' ' || c == '\t');
	} else {
	    Buf_AddChar(linebuf, '\\');
	    if (c == '\\') {
		Buf_AddChar(linebuf, '\\');
		c = ParseReadc();
	    }
	    break;
	}
    }
    ParseFoldLF(linebuf, c);

    return Buf_Retrieve(linebuf);
}

/* Skip all empty lines, and return the first `useful' character.
 * (This does skip blocks of comments at high speed, which justifies
 * the complexity of the function.)
 */
static int
ParseSkipEmptyLines(linebuf)
    Buffer	linebuf;
{
    int 	c;		/* the current character */

    for (;;) {
	Buf_Reset(linebuf);
	c = ParseReadc();
	/* Strip leading spaces, fold on '\n' */
	if (c == ' ') {
	    do {
		c = ParseReadc();
	    } while (c == ' ' || c == '\t');
	    while (c == '\\') {
		c = ParseReadc();
		if (c == '\n') {
		    current->lineno++;
		    do {
			c = ParseReadc();
		    } while (c == ' ' || c == '\t');
		} else {
		    Buf_AddChar(linebuf, '\\');
		    if (c == '\\') {
			Buf_AddChar(linebuf, '\\');
			c = ParseReadc();
		    }
		    if (c == EOF)
			return '\n';
		    else
			return c;
		}
	    }
	    assert(c != '\t');
	}
	if (c == '#')
	    c = skiptoendofline();
	/* Almost identical to spaces, except this occurs after comments
	 * have been taken care of, and we keep the tab itself.  */
	if (c == '\t') {
	    Buf_AddChar(linebuf, '\t');
	    do {
		c = ParseReadc();
	    } while (c == ' ' || c == '\t');
	    while (c == '\\') {
		c = ParseReadc();
		if (c == '\n') {
		    current->lineno++;
		    do {
			c = ParseReadc();
		    } while (c == ' ' || c == '\t');
		} else {
		    Buf_AddChar(linebuf, '\\');
		    if (c == '\\') {
			Buf_AddChar(linebuf, '\\');
			c = ParseReadc();
		    }
		    if (c == EOF)
			return '\n';
		    else
			return c;
		    return c;
		}
	    }
	}
	if (c == '\n')
	    current->lineno++;
	else
	    return c;
    }
}

/*-
 *---------------------------------------------------------------------
 * ParseReadLine --
 *	Read an entire line from the input file.
 *
 * Results:
 *	A line without a new line, or NULL at EOF.
 *
 * Notes:
 *	Removes beginning and trailing blanks (but keeps first tab).
 *	Updates line numbers, handles escaped newlines, and skip over
 *	uninteresting lines.
 *	All but trivial comments can't be handled at this point, because we
 *	don't know yet which lines are shell commands or not.
 *---------------------------------------------------------------------
 */
char *
ParseReadLine(linebuf)
    Buffer	linebuf;
{
    int 	c;		/* the current character */

    c = ParseSkipEmptyLines(linebuf);

    if (c == EOF)
	return NULL;
    else {
	ParseFoldLF(linebuf, c);
	Buf_KillTrailingSpaces(linebuf);
	return Buf_Retrieve(linebuf);
    }
}

unsigned long
Parse_Getlineno()
{
    return current->lineno;
}

const char *
Parse_Getfilename()
{
    return current->fname;
}

#ifdef CLEANUP
void
LowParse_Init()
{
    Lst_Init(&includes);
    current = NULL;
}

void
LowParse_End()
{
    Lst_Destroy(&includes, NOFREE);	/* Should be empty now */
}
#endif


void
Finish_Errors()
{
    if (current != NULL) {
	free_ifile(current);
	current = NULL;
    }
    if (fatals) {
	fprintf(stderr, "Fatal errors encountered -- cannot continue\n");
	exit(1);
    }
}
