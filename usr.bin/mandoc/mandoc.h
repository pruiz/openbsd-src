/*	$Id: mandoc.h,v 1.69 2014/07/01 22:36:35 schwarze Exp $ */
/*
 * Copyright (c) 2010, 2011 Kristaps Dzonsons <kristaps@bsd.lv>
 * Copyright (c) 2010-2014 Ingo Schwarze <schwarze@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef MANDOC_H
#define MANDOC_H

#define ASCII_NBRSP	 31  /* non-breaking space */
#define	ASCII_HYPH	 30  /* breakable hyphen */
#define	ASCII_BREAK	 29  /* breakable zero-width space */

/*
 * Status level.  This refers to both internal status (i.e., whilst
 * running, when warnings/errors are reported) and an indicator of a
 * threshold of when to halt (when said internal state exceeds the
 * threshold).
 */
enum	mandoclevel {
	MANDOCLEVEL_OK = 0,
	MANDOCLEVEL_RESERVED,
	MANDOCLEVEL_WARNING, /* warnings: syntax, whitespace, etc. */
	MANDOCLEVEL_ERROR, /* input has been thrown away */
	MANDOCLEVEL_FATAL, /* input is borked */
	MANDOCLEVEL_BADARG, /* bad argument in invocation */
	MANDOCLEVEL_SYSERR, /* system error */
	MANDOCLEVEL_MAX
};

/*
 * All possible things that can go wrong within a parse, be it libroff,
 * libmdoc, or libman.
 */
enum	mandocerr {
	MANDOCERR_OK,

	MANDOCERR_WARNING, /* ===== start of warnings ===== */

	/* related to the prologue */
	MANDOCERR_TH_MISSING, /* missing .TH macro, using "unknown 1" */
	MANDOCERR_TITLE_CASE, /* lower case character in document title */
	MANDOCERR_MSEC_BAD, /* unknown manual section: section */
	MANDOCERR_ARCH_BAD, /* unknown manual volume or arch: volume */
	MANDOCERR_DATE_MISSING, /* missing date, using today's date */
	MANDOCERR_DATE_BAD, /* cannot parse date, using it verbatim: date */
	MANDOCERR_PROLOG_ORDER, /* prologue macros out of order: macro */
	MANDOCERR_PROLOG_REP, /* duplicate prologue macro: macro */
	MANDOCERR_PROLOG_BAD, /* incomplete prologue, terminated by: macro */
	MANDOCERR_PROLOG_ONLY, /* skipping prologue macro in body: macro */

	/* related to document structure */
	MANDOCERR_SO, /* .so is fragile, better use ln(1): .so path */
	MANDOCERR_DOC_EMPTY, /* no document body */
	MANDOCERR_SEC_BEFORE, /* content before first section header: macro */
	MANDOCERR_NAMESEC_FIRST, /* first section is not "NAME": title */
	MANDOCERR_NAMESEC_BAD, /* bad NAME section contents: macro */
	MANDOCERR_SEC_ORDER, /* sections out of conventional order: title */
	MANDOCERR_SEC_REP, /* duplicate section title: title */
	MANDOCERR_SEC_MSEC, /* unexpected section: title for ... only */

	/* related to macros and nesting */
	MANDOCERR_MACROOBS, /* skipping obsolete macro */
	MANDOCERR_IGNPAR, /* skipping paragraph macro */
	MANDOCERR_MOVEPAR, /* moving paragraph macro out of list */
	MANDOCERR_IGNNS, /* skipping no-space macro */
	MANDOCERR_SCOPENEST, /* blocks badly nested */
	MANDOCERR_CHILD, /* child violates parent syntax */
	MANDOCERR_NESTEDDISP, /* nested displays are not portable */
	MANDOCERR_SCOPEREP, /* already in literal mode */
	MANDOCERR_LINESCOPE, /* line scope broken */

	/* related to missing macro arguments */
	MANDOCERR_MACROEMPTY, /* skipping empty macro */
	MANDOCERR_ARGCWARN, /* argument count wrong */
	MANDOCERR_DISPTYPE, /* missing display type */
	MANDOCERR_LISTFIRST, /* list type must come first */
	MANDOCERR_NOWIDTHARG, /* tag lists require a width argument */
	MANDOCERR_FONTTYPE, /* missing font type */
	MANDOCERR_WNOSCOPE, /* skipping end of block that is not open */

	/* related to bad macro arguments */
	MANDOCERR_IGNARGV, /* skipping argument */
	MANDOCERR_ARGVREP, /* duplicate argument */
	MANDOCERR_DISPREP, /* duplicate display type */
	MANDOCERR_LISTREP, /* duplicate list type */
	MANDOCERR_BADATT, /* unknown AT&T UNIX version */
	MANDOCERR_BADBOOL, /* bad Boolean value */
	MANDOCERR_BADFONT, /* unknown font */
	MANDOCERR_BADSTANDARD, /* unknown standard specifier */
	MANDOCERR_BADWIDTH, /* bad width argument */

	/* related to plain text */
	MANDOCERR_NOBLANKLN, /* blank line in non-literal context */
	MANDOCERR_BADTAB, /* tab in non-literal context */
	MANDOCERR_EOLNSPACE, /* end of line whitespace */
	MANDOCERR_BADCOMMENT, /* bad comment style */
	MANDOCERR_BADESCAPE, /* bad escape sequence */
	MANDOCERR_BADQUOTE, /* unterminated quoted string */

	MANDOCERR_ERROR, /* ===== start of errors ===== */

	/* related to equations */
	MANDOCERR_EQNNSCOPE, /* unexpected equation scope closure*/
	MANDOCERR_EQNSCOPE, /* equation scope open on exit */
	MANDOCERR_EQNBADSCOPE, /* overlapping equation scopes */
	MANDOCERR_EQNEOF, /* unexpected end of equation */
	MANDOCERR_EQNSYNT, /* equation syntax error */

	/* related to tables */
	MANDOCERR_TBL, /* bad table syntax */
	MANDOCERR_TBLOPT, /* bad table option */
	MANDOCERR_TBLLAYOUT, /* bad table layout */
	MANDOCERR_TBLNOLAYOUT, /* no table layout cells specified */
	MANDOCERR_TBLNODATA, /* no table data cells specified */
	MANDOCERR_TBLIGNDATA, /* ignore data in cell */
	MANDOCERR_TBLBLOCK, /* data block still open */
	MANDOCERR_TBLEXTRADAT, /* ignoring extra data cells */

	MANDOCERR_ROFFLOOP, /* input stack limit exceeded, infinite loop? */
	MANDOCERR_BADCHAR, /* skipping bad character */
	MANDOCERR_NAMESC, /* escaped character not allowed in a name */
	MANDOCERR_NONAME, /* manual name not yet set */
	MANDOCERR_NOTEXT, /* skipping text before first section header */
	MANDOCERR_MACRO, /* skipping unknown macro */
	MANDOCERR_REQUEST, /* NOT IMPLEMENTED: skipping request */
	MANDOCERR_ARGCOUNT, /* argument count wrong */
	MANDOCERR_STRAYTA, /* skipping column outside column list */
	MANDOCERR_NOSCOPE, /* skipping end of block that is not open */
	MANDOCERR_SCOPEBROKEN, /* missing end of block */
	MANDOCERR_SCOPEEXIT, /* scope open on exit */
	MANDOCERR_UNAME, /* uname(3) system call failed */
	/* FIXME: merge following with MANDOCERR_ARGCOUNT */
	MANDOCERR_NOARGS, /* macro requires line argument(s) */
	MANDOCERR_NOBODY, /* macro requires body argument(s) */
	MANDOCERR_NOARGV, /* macro requires argument(s) */
	MANDOCERR_NUMERIC, /* request requires a numeric argument */
	MANDOCERR_LISTTYPE, /* missing list type */
	MANDOCERR_ARGSLOST, /* line argument(s) will be lost */

	MANDOCERR_FATAL, /* ===== start of fatal errors ===== */

	MANDOCERR_TOOLARGE, /* input too large */
	MANDOCERR_NOTMANUAL, /* not a manual */
	MANDOCERR_COLUMNS, /* column syntax is inconsistent */
	MANDOCERR_BADDISP, /* NOT IMPLEMENTED: .Bd -file */
	MANDOCERR_SYNTARGVCOUNT, /* argument count wrong, violates syntax */
	MANDOCERR_SYNTCHILD, /* child violates parent syntax */
	MANDOCERR_SYNTARGCOUNT, /* argument count wrong, violates syntax */
	MANDOCERR_SO_PATH, /* NOT IMPLEMENTED: .so with absolute path or ".." */
	MANDOCERR_SO_FAIL, /* .so request failed */
	MANDOCERR_NODOCPROLOG, /* no document prologue */
	MANDOCERR_MEM, /* static buffer exhausted */

	/* ===== system errors ===== */

	MANDOCERR_SYSOPEN, /* cannot open file */
	MANDOCERR_SYSSTAT, /* cannot stat file */
	MANDOCERR_SYSREAD, /* cannot read file */

	MANDOCERR_MAX
};

struct	tbl_opts {
	char		  tab; /* cell-separator */
	char		  decimal; /* decimal point */
	int		  linesize;
	int		  opts;
#define	TBL_OPT_CENTRE	 (1 << 0)
#define	TBL_OPT_EXPAND	 (1 << 1)
#define	TBL_OPT_BOX	 (1 << 2)
#define	TBL_OPT_DBOX	 (1 << 3)
#define	TBL_OPT_ALLBOX	 (1 << 4)
#define	TBL_OPT_NOKEEP	 (1 << 5)
#define	TBL_OPT_NOSPACE	 (1 << 6)
	int		  cols; /* number of columns */
};

/*
 * The head of a table specifies all of its columns.  When formatting a
 * tbl_span, iterate over these and plug in data from the tbl_span when
 * appropriate, using tbl_cell as a guide to placement.
 */
struct	tbl_head {
	int		  ident; /* 0 <= unique id < cols */
	int		  vert; /* width of preceding vertical line */
	struct tbl_head	 *next;
	struct tbl_head	 *prev;
};

enum	tbl_cellt {
	TBL_CELL_CENTRE, /* c, C */
	TBL_CELL_RIGHT, /* r, R */
	TBL_CELL_LEFT, /* l, L */
	TBL_CELL_NUMBER, /* n, N */
	TBL_CELL_SPAN, /* s, S */
	TBL_CELL_LONG, /* a, A */
	TBL_CELL_DOWN, /* ^ */
	TBL_CELL_HORIZ, /* _, - */
	TBL_CELL_DHORIZ, /* = */
	TBL_CELL_MAX
};

/*
 * A cell in a layout row.
 */
struct	tbl_cell {
	struct tbl_cell	 *next;
	int		  vert; /* width of preceding vertical line */
	enum tbl_cellt	  pos;
	size_t		  spacing;
	int		  flags;
#define	TBL_CELL_TALIGN	 (1 << 0) /* t, T */
#define	TBL_CELL_BALIGN	 (1 << 1) /* d, D */
#define	TBL_CELL_BOLD	 (1 << 2) /* fB, B, b */
#define	TBL_CELL_ITALIC	 (1 << 3) /* fI, I, i */
#define	TBL_CELL_EQUAL	 (1 << 4) /* e, E */
#define	TBL_CELL_UP	 (1 << 5) /* u, U */
#define	TBL_CELL_WIGN	 (1 << 6) /* z, Z */
	struct tbl_head	 *head;
};

/*
 * A layout row.
 */
struct	tbl_row {
	struct tbl_row	 *next;
	struct tbl_cell	 *first;
	struct tbl_cell	 *last;
	int		  vert; /* trailing vertical line */
};

enum	tbl_datt {
	TBL_DATA_NONE, /* has no data */
	TBL_DATA_DATA, /* consists of data/string */
	TBL_DATA_HORIZ, /* horizontal line */
	TBL_DATA_DHORIZ, /* double-horizontal line */
	TBL_DATA_NHORIZ, /* squeezed horizontal line */
	TBL_DATA_NDHORIZ /* squeezed double-horizontal line */
};

/*
 * A cell within a row of data.  The "string" field contains the actual
 * string value that's in the cell.  The rest is layout.
 */
struct	tbl_dat {
	struct tbl_cell	 *layout; /* layout cell */
	int		  spans; /* how many spans follow */
	struct tbl_dat	 *next;
	char		 *string; /* data (NULL if not TBL_DATA_DATA) */
	enum tbl_datt	  pos;
};

enum	tbl_spant {
	TBL_SPAN_DATA, /* span consists of data */
	TBL_SPAN_HORIZ, /* span is horizontal line */
	TBL_SPAN_DHORIZ /* span is double horizontal line */
};

/*
 * A row of data in a table.
 */
struct	tbl_span {
	struct tbl_opts	 *opts;
	struct tbl_head	 *head;
	struct tbl_row	 *layout; /* layout row */
	struct tbl_dat	 *first;
	struct tbl_dat	 *last;
	int		  line; /* parse line */
	int		  flags;
#define	TBL_SPAN_FIRST	 (1 << 0)
#define	TBL_SPAN_LAST	 (1 << 1)
	enum tbl_spant	  pos;
	struct tbl_span	 *next;
};

enum	eqn_boxt {
	EQN_ROOT, /* root of parse tree */
	EQN_TEXT, /* text (number, variable, whatever) */
	EQN_SUBEXPR, /* nested `eqn' subexpression */
	EQN_LIST, /* subexpressions list */
	EQN_MATRIX /* matrix subexpression */
};

enum	eqn_markt {
	EQNMARK_NONE = 0,
	EQNMARK_DOT,
	EQNMARK_DOTDOT,
	EQNMARK_HAT,
	EQNMARK_TILDE,
	EQNMARK_VEC,
	EQNMARK_DYAD,
	EQNMARK_BAR,
	EQNMARK_UNDER,
	EQNMARK__MAX
};

enum	eqn_fontt {
	EQNFONT_NONE = 0,
	EQNFONT_ROMAN,
	EQNFONT_BOLD,
	EQNFONT_FAT,
	EQNFONT_ITALIC,
	EQNFONT__MAX
};

enum	eqn_post {
	EQNPOS_NONE = 0,
	EQNPOS_OVER,
	EQNPOS_SUP,
	EQNPOS_SUB,
	EQNPOS_TO,
	EQNPOS_FROM,
	EQNPOS__MAX
};

enum	eqn_pilet {
	EQNPILE_NONE = 0,
	EQNPILE_PILE,
	EQNPILE_CPILE,
	EQNPILE_RPILE,
	EQNPILE_LPILE,
	EQNPILE_COL,
	EQNPILE_CCOL,
	EQNPILE_RCOL,
	EQNPILE_LCOL,
	EQNPILE__MAX
};

 /*
 * A "box" is a parsed mathematical expression as defined by the eqn.7
 * grammar.
 */
struct	eqn_box {
	int		  size; /* font size of expression */
#define	EQN_DEFSIZE	  INT_MIN
	enum eqn_boxt	  type; /* type of node */
	struct eqn_box	 *first; /* first child node */
	struct eqn_box	 *last; /* last child node */
	struct eqn_box	 *next; /* node sibling */
	struct eqn_box	 *parent; /* node sibling */
	char		 *text; /* text (or NULL) */
	char		 *left;
	char		 *right;
	enum eqn_post	  pos; /* position of next box */
	enum eqn_markt	  mark; /* a mark about the box */
	enum eqn_fontt	  font; /* font of box */
	enum eqn_pilet	  pile; /* equation piling */
};

/*
 * An equation consists of a tree of expressions starting at a given
 * line and position.
 */
struct	eqn {
	char		 *name; /* identifier (or NULL) */
	struct eqn_box	 *root; /* root mathematical expression */
	int		  ln; /* invocation line */
	int		  pos; /* invocation position */
};

/*
 * Parse options.
 */
#define	MPARSE_MDOC	1  /* assume -mdoc */
#define	MPARSE_MAN	2  /* assume -man */
#define	MPARSE_SO	4  /* honour .so requests */
#define	MPARSE_QUICK	8  /* abort the parse early */

enum	mandoc_esc {
	ESCAPE_ERROR = 0, /* bail! unparsable escape */
	ESCAPE_IGNORE, /* escape to be ignored */
	ESCAPE_SPECIAL, /* a regular special character */
	ESCAPE_FONT, /* a generic font mode */
	ESCAPE_FONTBOLD, /* bold font mode */
	ESCAPE_FONTITALIC, /* italic font mode */
	ESCAPE_FONTBI, /* bold italic font mode */
	ESCAPE_FONTROMAN, /* roman font mode */
	ESCAPE_FONTPREV, /* previous font mode */
	ESCAPE_NUMBERED, /* a numbered glyph */
	ESCAPE_UNICODE, /* a unicode codepoint */
	ESCAPE_NOSPACE, /* suppress space if the last on a line */
	ESCAPE_SKIPCHAR /* skip the next character */
};

typedef	void	(*mandocmsg)(enum mandocerr, enum mandoclevel,
			const char *, int, int, const char *);

struct	mparse;
struct	mchars;
struct	mdoc;
struct	man;

__BEGIN_DECLS

enum mandoc_esc	  mandoc_escape(const char **, const char **, int *);
struct mchars	 *mchars_alloc(void);
void		  mchars_free(struct mchars *);
char		  mchars_num2char(const char *, size_t);
int		  mchars_num2uc(const char *, size_t);
int		  mchars_spec2cp(const struct mchars *,
			const char *, size_t);
const char	 *mchars_spec2str(const struct mchars *,
			const char *, size_t, size_t *);
struct mparse	 *mparse_alloc(int, enum mandoclevel, mandocmsg, char *);
void		  mparse_free(struct mparse *);
void		  mparse_keep(struct mparse *);
enum mandoclevel  mparse_readfd(struct mparse *, int, const char *);
void		  mparse_reset(struct mparse *);
void		  mparse_result(struct mparse *,
			struct mdoc **, struct man **, char **);
const char	 *mparse_getkeep(const struct mparse *);
const char	 *mparse_strerror(enum mandocerr);
const char	 *mparse_strlevel(enum mandoclevel);

__END_DECLS

#endif /*!MANDOC_H*/
