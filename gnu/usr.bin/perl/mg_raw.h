/* -*- buffer-read-only: t -*-
 *
 *    mg_raw.h
 * !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
 * This file is built by regen/mg_vtable.pl.
 * Any changes made here will be lost!
 */

    { '\0', "want_vtbl_sv | PERL_MAGIC_READONLY_ACCEPTABLE",
      "/* sv '\\0' Special scalar variable */" },
    { '#', "want_vtbl_arylen | PERL_MAGIC_VALUE_MAGIC",
      "/* arylen '#' Array length ($#ary) */" },
    { '%', "magic_vtable_max | PERL_MAGIC_VALUE_MAGIC",
      "/* rhash '%' extra data for restricted hashes */" },
    { '&', "magic_vtable_max",
      "/* proto '&' my sub prototype CV */" },
    { '.', "want_vtbl_pos | PERL_MAGIC_VALUE_MAGIC",
      "/* pos '.' pos() lvalue */" },
    { ':', "magic_vtable_max | PERL_MAGIC_VALUE_MAGIC",
      "/* symtab ':' extra data for symbol tables */" },
    { '<', "want_vtbl_backref | PERL_MAGIC_READONLY_ACCEPTABLE | PERL_MAGIC_VALUE_MAGIC",
      "/* backref '<' for weak ref data */" },
    { '@', "magic_vtable_max | PERL_MAGIC_VALUE_MAGIC",
      "/* arylen_p '@' to move arylen out of XPVAV */" },
    { 'B', "want_vtbl_regexp | PERL_MAGIC_READONLY_ACCEPTABLE | PERL_MAGIC_VALUE_MAGIC",
      "/* bm 'B' Boyer-Moore (fast string search) */" },
    { 'c', "want_vtbl_ovrld",
      "/* overload_table 'c' Holds overload table (AMT) on stash */" },
    { 'D', "want_vtbl_regdata",
      "/* regdata 'D' Regex match position data (@+ and @- vars) */" },
    { 'd', "want_vtbl_regdatum",
      "/* regdatum 'd' Regex match position data element */" },
    { 'E', "want_vtbl_env",
      "/* env 'E' %ENV hash */" },
    { 'e', "want_vtbl_envelem",
      "/* envelem 'e' %ENV hash element */" },
    { 'f', "want_vtbl_regexp | PERL_MAGIC_READONLY_ACCEPTABLE | PERL_MAGIC_VALUE_MAGIC",
      "/* fm 'f' Formline ('compiled' format) */" },
    { 'g', "want_vtbl_mglob | PERL_MAGIC_READONLY_ACCEPTABLE | PERL_MAGIC_VALUE_MAGIC",
      "/* regex_global 'g' m//g target */" },
    { 'H', "want_vtbl_hints",
      "/* hints 'H' %^H hash */" },
    { 'h', "want_vtbl_hintselem",
      "/* hintselem 'h' %^H hash element */" },
    { 'I', "want_vtbl_isa",
      "/* isa 'I' @ISA array */" },
    { 'i', "want_vtbl_isaelem",
      "/* isaelem 'i' @ISA array element */" },
    { 'k', "want_vtbl_nkeys | PERL_MAGIC_VALUE_MAGIC",
      "/* nkeys 'k' scalar(keys()) lvalue */" },
    { 'L', "magic_vtable_max",
      "/* dbfile 'L' Debugger %_<filename */" },
    { 'l', "want_vtbl_dbline",
      "/* dbline 'l' Debugger %_<filename element */" },
    { 'o', "want_vtbl_collxfrm | PERL_MAGIC_VALUE_MAGIC",
      "/* collxfrm 'o' Locale transformation */" },
    { 'P', "want_vtbl_pack | PERL_MAGIC_VALUE_MAGIC",
      "/* tied 'P' Tied array or hash */" },
    { 'p', "want_vtbl_packelem",
      "/* tiedelem 'p' Tied array or hash element */" },
    { 'q', "want_vtbl_packelem",
      "/* tiedscalar 'q' Tied scalar or handle */" },
    { 'r', "want_vtbl_regexp | PERL_MAGIC_READONLY_ACCEPTABLE | PERL_MAGIC_VALUE_MAGIC",
      "/* qr 'r' precompiled qr// regex */" },
    { 'S', "magic_vtable_max",
      "/* sig 'S' %SIG hash */" },
    { 's', "want_vtbl_sigelem",
      "/* sigelem 's' %SIG hash element */" },
    { 't', "want_vtbl_taint | PERL_MAGIC_VALUE_MAGIC",
      "/* taint 't' Taintedness */" },
    { 'U', "want_vtbl_uvar",
      "/* uvar 'U' Available for use by extensions */" },
    { 'V', "magic_vtable_max | PERL_MAGIC_VALUE_MAGIC",
      "/* vstring 'V' SV was vstring literal */" },
    { 'v', "want_vtbl_vec | PERL_MAGIC_VALUE_MAGIC",
      "/* vec 'v' vec() lvalue */" },
    { 'w', "want_vtbl_utf8 | PERL_MAGIC_VALUE_MAGIC",
      "/* utf8 'w' Cached UTF-8 information */" },
    { 'x', "want_vtbl_substr | PERL_MAGIC_VALUE_MAGIC",
      "/* substr 'x' substr() lvalue */" },
    { 'y', "want_vtbl_defelem | PERL_MAGIC_VALUE_MAGIC",
      "/* defelem 'y' Shadow \"foreach\" iterator variable / smart parameter vivification */" },
    { ']', "want_vtbl_checkcall | PERL_MAGIC_VALUE_MAGIC",
      "/* checkcall ']' inlining/mutation of call to this CV */" },
    { '~', "magic_vtable_max",
      "/* ext '~' Available for use by extensions */" },

/* ex: set ro: */
