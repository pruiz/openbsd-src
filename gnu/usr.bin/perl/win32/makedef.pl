#!../miniperl

# Written: 10 April 1996 Gary Ng (71564.1743@compuserve.com)

# Create the export list for perl.
# Needed by WIN32 for creating perl.dll
# based on perl_exp.SH in the main perl distribution directory

# This simple program relys on 'global.sym' being up to date
# with all of the global symbols that a dynamicly link library
# might want to access.

# There is some symbol defined in global.sym and interp.sym
# that does not present in the WIN32 port but there is no easy
# way to find them so I just put a exception list here

my $CCTYPE = shift || "MSVC";

$skip_sym=<<'!END!OF!SKIP!';
Perl_SvIV
Perl_SvNV
Perl_SvTRUE
Perl_SvUV
Perl_block_type
Perl_sv_pvn
Perl_additem
Perl_cast_ulong
Perl_check_uni
Perl_checkcomma
Perl_chsize
Perl_ck_aelem
Perl_cryptseen
Perl_cx_dump
Perl_deb
Perl_deb_growlevel
Perl_debop
Perl_debprofdump
Perl_debstack
Perl_debstackptrs
Perl_do_ipcctl
Perl_do_ipcget
Perl_do_msgrcv
Perl_do_msgsnd
Perl_do_semop
Perl_do_shmio
Perl_doeval
Perl_dofindlabel
Perl_dopoptoeval
Perl_dump_eval
Perl_dump_fds
Perl_dump_form
Perl_dump_gv
Perl_dump_mstats
Perl_dump_op
Perl_dump_packsubs
Perl_dump_pm
Perl_dump_sub
Perl_expectterm
Perl_fetch_gv
Perl_fetch_io
Perl_force_ident
Perl_force_next
Perl_force_word
Perl_hv_stashpv
Perl_intuit_more
Perl_know_next
Perl_modkids
Perl_mstats
Perl_my_bzero
Perl_my_htonl
Perl_my_ntohl
Perl_my_swap
Perl_my_chsize
Perl_newXSUB
Perl_no_fh_allowed
Perl_no_op
Perl_nointrp
Perl_nomem
Perl_pp_cswitch
Perl_pp_entersubr
Perl_pp_evalonce
Perl_pp_interp
Perl_pp_map
Perl_pp_nswitch
Perl_q
Perl_reall_srchlen
Perl_regdump
Perl_regfold
Perl_regmyendp
Perl_regmyp_size
Perl_regmystartp
Perl_regnarrate
Perl_regprop
Perl_same_dirent
Perl_saw_return
Perl_scan_const
Perl_scan_formline
Perl_scan_heredoc
Perl_scan_ident
Perl_scan_inputsymbol
Perl_scan_pat
Perl_scan_prefix
Perl_scan_str
Perl_scan_subst
Perl_scan_trans
Perl_scan_word
Perl_setenv_getix
Perl_skipspace
Perl_sublex_done
Perl_sublex_start
Perl_sv_peek
Perl_sv_ref
Perl_sv_setptrobj
Perl_timesbuf
Perl_too_few_arguments
Perl_too_many_arguments
Perl_unlnk
Perl_wait4pid
Perl_watch
Perl_yyname
Perl_yyrule
allgvs
curblock
curcsv
lastretstr
mystack_mark
perl_init_ext
perl_requirepv
stack
statusvalue_vms
Perl_safexcalloc
Perl_safexmalloc
Perl_safexfree
Perl_safexrealloc
Perl_my_memcmp
Perl_my_memset
Perl_cshlen
Perl_cshname
!END!OF!SKIP!

# All symbols have a Perl_ prefix because that's what embed.h
# sticks in front of them.


print "LIBRARY Perl\n";
print "DESCRIPTION 'Perl interpreter, export autogenerated'\n";
print "CODE LOADONCALL\n";
print "DATA LOADONCALL NONSHARED MULTIPLE\n";
print "EXPORTS\n";

open (GLOBAL, "<../global.sym") || die "failed to open global.sym" . $!;
while (<GLOBAL>) {
	my $symbol;
	next if (!/^[A-Za-z]/);
	next if (/_amg[ \t]*$/);
	$symbol = "Perl_$_";
    	next if ($skip_sym =~ m/$symbol/m);
	emit_symbol($symbol);
}
close(GLOBAL);

# also add symbols from interp.sym
# They are only needed if -DMULTIPLICITY is not set but it
# doesn't hurt to include them anyway.
# these don't have Perl prefix

open (INTERP, "<../interp.sym") || die "failed to open interp.sym" . $!;
while (<INTERP>) {
	my $symbol;
	next if (!/^[A-Za-z]/);
	next if (/_amg[ \t]*$/);
	$symbol = $_;
    	next if ($skip_sym =~ m/$symbol/m);
	#print "\t$symbol";
	emit_symbol("Perl_" . $symbol);
}

#close(INTERP);

while (<DATA>) {
	my $symbol;
	next if (!/^[A-Za-z]/);
	next if (/^#/);
	$symbol = $_;
    	next if ($skip_sym =~ m/^$symbol/m);
	emit_symbol($symbol);
}

sub emit_symbol {
	my $symbol = shift;
	chomp $symbol;
	if ($CCTYPE eq "BORLAND") {
		# workaround Borland quirk by exporting both the straight
		# name and a name with leading underscore.  Note the
		# alias *must* come after the symbol itself, if both
		# are to be exported. (Linker bug?)
		print "\t_$symbol\n";
		print "\t$symbol = _$symbol\n";
	}
	else {
		# for binary coexistence, export both the symbol and
		# alias with leading underscore
		print "\t$symbol\n";
		print "\t_$symbol = $symbol\n";
	}
}

1;
__DATA__
# extra globals not included above.
perl_init_i18nl10n
perl_init_ext
perl_alloc
perl_construct
perl_destruct
perl_free
perl_parse
perl_run
perl_get_sv
perl_get_av
perl_get_hv
perl_get_cv
perl_call_argv
perl_call_pv
perl_call_method
perl_call_sv
perl_require_pv
perl_eval_pv
perl_eval_sv
boot_DynaLoader
win32_errno
win32_environ
win32_stdin
win32_stdout
win32_stderr
win32_ferror
win32_feof
win32_strerror
win32_fprintf
win32_printf
win32_vfprintf
win32_vprintf
win32_fread
win32_fwrite
win32_fopen
win32_fdopen
win32_freopen
win32_fclose
win32_fputs
win32_fputc
win32_ungetc
win32_getc
win32_fileno
win32_clearerr
win32_fflush
win32_ftell
win32_fseek
win32_fgetpos
win32_fsetpos
win32_rewind
win32_tmpfile
win32_abort
win32_fstat
win32_stat
win32_pipe
win32_popen
win32_pclose
win32_setmode
win32_lseek
win32_tell
win32_dup
win32_dup2
win32_open
win32_close
win32_eof
win32_read
win32_write
win32_spawnvp
win32_mkdir
win32_rmdir
win32_chdir
win32_flock
win32_execvp
win32_htons
win32_ntohs
win32_htonl
win32_ntohl
win32_inet_addr
win32_inet_ntoa
win32_socket
win32_bind
win32_listen
win32_accept
win32_connect
win32_send
win32_sendto
win32_recv
win32_recvfrom
win32_shutdown
win32_ioctlsocket
win32_setsockopt
win32_getsockopt
win32_getpeername
win32_getsockname
win32_gethostname
win32_gethostbyname
win32_gethostbyaddr
win32_getprotobyname
win32_getprotobynumber
win32_getservbyname
win32_getservbyport
win32_select
win32_endhostent
win32_endnetent
win32_endprotoent
win32_endservent
win32_getnetent
win32_getnetbyname
win32_getnetbyaddr
win32_getprotoent
win32_getservent
win32_sethostent
win32_setnetent
win32_setprotoent
win32_setservent
win32_getenv
win32_perror
win32_setbuf
win32_setvbuf
win32_flushall
win32_fcloseall
win32_fgets
win32_gets
win32_fgetc
win32_putc
win32_puts
win32_getchar
win32_putchar
win32_malloc
win32_calloc
win32_realloc
win32_free
win32stdio
Perl_win32_init
RunPerl
SetIOSubSystem
GetIOSubSystem
