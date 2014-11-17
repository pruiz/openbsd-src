
BEGIN {
    unless ('A' eq pack('U', 0x41)) {
	print "1..0 # Unicode::Collate cannot pack a Unicode code point\n";
	exit 0;
    }
    unless (0x41 == unpack('U', 'A')) {
	print "1..0 # Unicode::Collate cannot get a Unicode code point\n";
	exit 0;
    }
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..528\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate::Locale;

ok(1);

#########################

my $objJa = Unicode::Collate::Locale->
    new(locale => 'JA', normalization => undef);

ok($objJa->getlocale, 'ja');

$objJa->change(level => 1);

# first ten kanji
ok($objJa->lt("\x{4E9C}", "\x{5516}"));
ok($objJa->lt("\x{5516}", "\x{5A03}"));
ok($objJa->lt("\x{5A03}", "\x{963F}"));
ok($objJa->lt("\x{963F}", "\x{54C0}"));
ok($objJa->lt("\x{54C0}", "\x{611B}"));
ok($objJa->lt("\x{611B}", "\x{6328}"));
ok($objJa->lt("\x{6328}", "\x{59F6}"));
ok($objJa->lt("\x{59F6}", "\x{9022}"));
ok($objJa->lt("\x{9022}", "\x{8475}"));

# last five kanji and undef
ok($objJa->lt("\x{69C7}", "\x{9059}"));
ok($objJa->lt("\x{9059}", "\x{7464}"));
ok($objJa->lt("\x{7464}", "\x{51DC}"));
ok($objJa->lt("\x{51DC}", "\x{7199}"));
ok($objJa->lt("\x{7199}", "\x{4E02}")); # 4E02: UIdeo undef in JIS X 0208
ok($objJa->lt("\x{4E02}", "\x{3400}")); # 3400: Ext.A undef in JIS X 0208

# Ext.B
ok($objJa->lt("\x{20000}", "\x{20001}"));
ok($objJa->lt("\x{20001}", "\x{20002}"));
ok($objJa->lt("\x{20002}", "\x{20003}"));
ok($objJa->lt("\x{20003}", "\x{20004}"));
ok($objJa->lt("\x{20004}", "\x{20005}"));

# 22

$objJa->change(overrideCJK => undef);

ok($objJa->lt("\x{4E00}", "\x{4E01}"));
ok($objJa->lt("\x{4E01}", "\x{4E02}"));
ok($objJa->lt("\x{4E02}", "\x{4E03}"));
ok($objJa->lt("\x{4E03}", "\x{4E04}"));
ok($objJa->lt("\x{4E04}", "\x{4E05}"));

ok($objJa->lt("\x{9F9B}", "\x{9F9C}"));
ok($objJa->lt("\x{9F9C}", "\x{9F9D}"));
ok($objJa->lt("\x{9F9D}", "\x{9F9E}"));
ok($objJa->lt("\x{9F9E}", "\x{9F9F}"));
ok($objJa->lt("\x{9F9F}", "\x{9FA0}"));

# 32

$objJa->change(level => 3);

ok($objJa->eq("\x{3041}", "\x{30A1}"));
ok($objJa->eq("\x{3042}", "\x{30A2}"));
ok($objJa->eq("\x{3043}", "\x{30A3}"));
ok($objJa->eq("\x{3044}", "\x{30A4}"));
ok($objJa->eq("\x{3045}", "\x{30A5}"));
ok($objJa->eq("\x{3046}", "\x{30A6}"));
ok($objJa->eq("\x{3047}", "\x{30A7}"));
ok($objJa->eq("\x{3048}", "\x{30A8}"));
ok($objJa->eq("\x{3049}", "\x{30A9}"));
ok($objJa->eq("\x{304A}", "\x{30AA}"));
ok($objJa->eq("\x{304B}", "\x{30AB}"));
ok($objJa->eq("\x{304C}", "\x{30AC}"));
ok($objJa->eq("\x{304D}", "\x{30AD}"));
ok($objJa->eq("\x{304E}", "\x{30AE}"));
ok($objJa->eq("\x{304F}", "\x{30AF}"));
ok($objJa->eq("\x{3050}", "\x{30B0}"));
ok($objJa->eq("\x{3051}", "\x{30B1}"));
ok($objJa->eq("\x{3052}", "\x{30B2}"));
ok($objJa->eq("\x{3053}", "\x{30B3}"));
ok($objJa->eq("\x{3054}", "\x{30B4}"));
ok($objJa->eq("\x{3055}", "\x{30B5}"));
ok($objJa->eq("\x{3056}", "\x{30B6}"));
ok($objJa->eq("\x{3057}", "\x{30B7}"));
ok($objJa->eq("\x{3058}", "\x{30B8}"));
ok($objJa->eq("\x{3059}", "\x{30B9}"));
ok($objJa->eq("\x{305A}", "\x{30BA}"));
ok($objJa->eq("\x{305B}", "\x{30BB}"));
ok($objJa->eq("\x{305C}", "\x{30BC}"));
ok($objJa->eq("\x{305D}", "\x{30BD}"));
ok($objJa->eq("\x{305E}", "\x{30BE}"));
ok($objJa->eq("\x{305F}", "\x{30BF}"));
ok($objJa->eq("\x{3060}", "\x{30C0}"));
ok($objJa->eq("\x{3061}", "\x{30C1}"));
ok($objJa->eq("\x{3062}", "\x{30C2}"));
ok($objJa->eq("\x{3063}", "\x{30C3}"));
ok($objJa->eq("\x{3064}", "\x{30C4}"));
ok($objJa->eq("\x{3065}", "\x{30C5}"));
ok($objJa->eq("\x{3066}", "\x{30C6}"));
ok($objJa->eq("\x{3067}", "\x{30C7}"));
ok($objJa->eq("\x{3068}", "\x{30C8}"));
ok($objJa->eq("\x{3069}", "\x{30C9}"));
ok($objJa->eq("\x{306A}", "\x{30CA}"));
ok($objJa->eq("\x{306B}", "\x{30CB}"));
ok($objJa->eq("\x{306C}", "\x{30CC}"));
ok($objJa->eq("\x{306D}", "\x{30CD}"));
ok($objJa->eq("\x{306E}", "\x{30CE}"));
ok($objJa->eq("\x{306F}", "\x{30CF}"));
ok($objJa->eq("\x{3070}", "\x{30D0}"));
ok($objJa->eq("\x{3071}", "\x{30D1}"));
ok($objJa->eq("\x{3072}", "\x{30D2}"));
ok($objJa->eq("\x{3073}", "\x{30D3}"));
ok($objJa->eq("\x{3074}", "\x{30D4}"));
ok($objJa->eq("\x{3075}", "\x{30D5}"));
ok($objJa->eq("\x{3076}", "\x{30D6}"));
ok($objJa->eq("\x{3077}", "\x{30D7}"));
ok($objJa->eq("\x{3078}", "\x{30D8}"));
ok($objJa->eq("\x{3079}", "\x{30D9}"));
ok($objJa->eq("\x{307A}", "\x{30DA}"));
ok($objJa->eq("\x{307B}", "\x{30DB}"));
ok($objJa->eq("\x{307C}", "\x{30DC}"));
ok($objJa->eq("\x{307D}", "\x{30DD}"));
ok($objJa->eq("\x{307E}", "\x{30DE}"));
ok($objJa->eq("\x{307F}", "\x{30DF}"));
ok($objJa->eq("\x{3080}", "\x{30E0}"));
ok($objJa->eq("\x{3081}", "\x{30E1}"));
ok($objJa->eq("\x{3082}", "\x{30E2}"));
ok($objJa->eq("\x{3083}", "\x{30E3}"));
ok($objJa->eq("\x{3084}", "\x{30E4}"));
ok($objJa->eq("\x{3085}", "\x{30E5}"));
ok($objJa->eq("\x{3086}", "\x{30E6}"));
ok($objJa->eq("\x{3087}", "\x{30E7}"));
ok($objJa->eq("\x{3088}", "\x{30E8}"));
ok($objJa->eq("\x{3089}", "\x{30E9}"));
ok($objJa->eq("\x{308A}", "\x{30EA}"));
ok($objJa->eq("\x{308B}", "\x{30EB}"));
ok($objJa->eq("\x{308C}", "\x{30EC}"));
ok($objJa->eq("\x{308D}", "\x{30ED}"));
ok($objJa->eq("\x{308E}", "\x{30EE}"));
ok($objJa->eq("\x{308F}", "\x{30EF}"));
ok($objJa->eq("\x{3090}", "\x{30F0}"));
ok($objJa->eq("\x{3091}", "\x{30F1}"));
ok($objJa->eq("\x{3092}", "\x{30F2}"));
ok($objJa->eq("\x{3093}", "\x{30F3}"));
ok($objJa->eq("\x{3094}", "\x{30F4}"));
ok($objJa->eq("\x{3095}", "\x{30F5}"));
ok($objJa->eq("\x{3096}", "\x{30F6}"));
ok($objJa->eq("\x{309D}", "\x{30FD}"));
ok($objJa->eq("\x{309E}", "\x{30FE}"));

# 120

$objJa->change(variable => 'Non-ignorable');

ok($objJa->eq("\x{3000}", "\ "));
ok($objJa->eq("\x{FF01}", "\!"));
ok($objJa->eq("\x{FF02}", "\""));
ok($objJa->eq("\x{FF03}", "\#"));
ok($objJa->eq("\x{FF04}", "\$"));
ok($objJa->eq("\x{FF05}", "\%"));
ok($objJa->eq("\x{FF06}", "\&"));
ok($objJa->eq("\x{FF07}", "\'"));
ok($objJa->eq("\x{FF08}", "\("));
ok($objJa->eq("\x{FF09}", "\)"));
ok($objJa->eq("\x{FF0A}", "\*"));
ok($objJa->eq("\x{FF0B}", "\+"));
ok($objJa->eq("\x{FF0C}", "\,"));
ok($objJa->eq("\x{FF0D}", "\-"));
ok($objJa->eq("\x{FF0E}", "\."));
ok($objJa->eq("\x{FF0F}", "\/"));
ok($objJa->eq("\x{FF10}", "0"));
ok($objJa->eq("\x{FF11}", "1"));
ok($objJa->eq("\x{FF12}", "2"));
ok($objJa->eq("\x{FF13}", "3"));
ok($objJa->eq("\x{FF14}", "4"));
ok($objJa->eq("\x{FF15}", "5"));
ok($objJa->eq("\x{FF16}", "6"));
ok($objJa->eq("\x{FF17}", "7"));
ok($objJa->eq("\x{FF18}", "8"));
ok($objJa->eq("\x{FF19}", "9"));
ok($objJa->eq("\x{FF1A}", "\:"));
ok($objJa->eq("\x{FF1B}", "\;"));
ok($objJa->eq("\x{FF1C}", "\<"));
ok($objJa->eq("\x{FF1D}", "\="));
ok($objJa->eq("\x{FF1E}", "\>"));
ok($objJa->eq("\x{FF1F}", "\?"));
ok($objJa->eq("\x{FF20}", "\@"));
ok($objJa->eq("\x{FF21}", "A"));
ok($objJa->eq("\x{FF22}", "B"));
ok($objJa->eq("\x{FF23}", "C"));
ok($objJa->eq("\x{FF24}", "D"));
ok($objJa->eq("\x{FF25}", "E"));
ok($objJa->eq("\x{FF26}", "F"));
ok($objJa->eq("\x{FF27}", "G"));
ok($objJa->eq("\x{FF28}", "H"));
ok($objJa->eq("\x{FF29}", "I"));
ok($objJa->eq("\x{FF2A}", "J"));
ok($objJa->eq("\x{FF2B}", "K"));
ok($objJa->eq("\x{FF2C}", "L"));
ok($objJa->eq("\x{FF2D}", "M"));
ok($objJa->eq("\x{FF2E}", "N"));
ok($objJa->eq("\x{FF2F}", "O"));
ok($objJa->eq("\x{FF30}", "P"));
ok($objJa->eq("\x{FF31}", "Q"));
ok($objJa->eq("\x{FF32}", "R"));
ok($objJa->eq("\x{FF33}", "S"));
ok($objJa->eq("\x{FF34}", "T"));
ok($objJa->eq("\x{FF35}", "U"));
ok($objJa->eq("\x{FF36}", "V"));
ok($objJa->eq("\x{FF37}", "W"));
ok($objJa->eq("\x{FF38}", "X"));
ok($objJa->eq("\x{FF39}", "Y"));
ok($objJa->eq("\x{FF3A}", "Z"));
ok($objJa->eq("\x{FF3B}", "\["));
ok($objJa->eq("\x{FF3C}", "\'"));
ok($objJa->eq("\x{FF3D}", "\]"));
ok($objJa->eq("\x{FF3E}", "\^"));
ok($objJa->eq("\x{FF3F}", "_"));
ok($objJa->eq("\x{FF40}", "\`"));
ok($objJa->eq("\x{FF41}", "a"));
ok($objJa->eq("\x{FF42}", "b"));
ok($objJa->eq("\x{FF43}", "c"));
ok($objJa->eq("\x{FF44}", "d"));
ok($objJa->eq("\x{FF45}", "e"));
ok($objJa->eq("\x{FF46}", "f"));
ok($objJa->eq("\x{FF47}", "g"));
ok($objJa->eq("\x{FF48}", "h"));
ok($objJa->eq("\x{FF49}", "i"));
ok($objJa->eq("\x{FF4A}", "j"));
ok($objJa->eq("\x{FF4B}", "k"));
ok($objJa->eq("\x{FF4C}", "l"));
ok($objJa->eq("\x{FF4D}", "m"));
ok($objJa->eq("\x{FF4E}", "n"));
ok($objJa->eq("\x{FF4F}", "o"));
ok($objJa->eq("\x{FF50}", "p"));
ok($objJa->eq("\x{FF51}", "q"));
ok($objJa->eq("\x{FF52}", "r"));
ok($objJa->eq("\x{FF53}", "s"));
ok($objJa->eq("\x{FF54}", "t"));
ok($objJa->eq("\x{FF55}", "u"));
ok($objJa->eq("\x{FF56}", "v"));
ok($objJa->eq("\x{FF57}", "w"));
ok($objJa->eq("\x{FF58}", "x"));
ok($objJa->eq("\x{FF59}", "y"));
ok($objJa->eq("\x{FF5A}", "z"));
ok($objJa->eq("\x{FF5B}", "\{"));
ok($objJa->eq("\x{FF5C}", "\|"));
ok($objJa->eq("\x{FF5D}", "\}"));
ok($objJa->eq("\x{FF5E}", "\~"));
ok($objJa->eq("\x{FF5F}", "\x{2985}"));
ok($objJa->eq("\x{FF60}", "\x{2986}"));
ok($objJa->eq("\x{FF61}", "\x{3002}"));
ok($objJa->eq("\x{FF62}", "\x{300C}"));
ok($objJa->eq("\x{FF63}", "\x{300D}"));
ok($objJa->eq("\x{FF64}", "\x{3001}"));
ok($objJa->eq("\x{FF65}", "\x{30FB}"));
ok($objJa->eq("\x{FF66}", "\x{30F2}"));
ok($objJa->eq("\x{FF67}", "\x{30A1}"));
ok($objJa->eq("\x{FF68}", "\x{30A3}"));
ok($objJa->eq("\x{FF69}", "\x{30A5}"));
ok($objJa->eq("\x{FF6A}", "\x{30A7}"));
ok($objJa->eq("\x{FF6B}", "\x{30A9}"));
ok($objJa->eq("\x{FF6C}", "\x{30E3}"));
ok($objJa->eq("\x{FF6D}", "\x{30E5}"));
ok($objJa->eq("\x{FF6E}", "\x{30E7}"));
ok($objJa->eq("\x{FF6F}", "\x{30C3}"));
ok($objJa->eq("\x{FF70}", "\x{30FC}"));
ok($objJa->eq("\x{FF71}", "\x{30A2}"));
ok($objJa->eq("\x{FF72}", "\x{30A4}"));
ok($objJa->eq("\x{FF73}", "\x{30A6}"));
ok($objJa->eq("\x{FF74}", "\x{30A8}"));
ok($objJa->eq("\x{FF75}", "\x{30AA}"));
ok($objJa->eq("\x{FF76}", "\x{30AB}"));
ok($objJa->eq("\x{FF77}", "\x{30AD}"));
ok($objJa->eq("\x{FF78}", "\x{30AF}"));
ok($objJa->eq("\x{FF79}", "\x{30B1}"));
ok($objJa->eq("\x{FF7A}", "\x{30B3}"));
ok($objJa->eq("\x{FF7B}", "\x{30B5}"));
ok($objJa->eq("\x{FF7C}", "\x{30B7}"));
ok($objJa->eq("\x{FF7D}", "\x{30B9}"));
ok($objJa->eq("\x{FF7E}", "\x{30BB}"));
ok($objJa->eq("\x{FF7F}", "\x{30BD}"));
ok($objJa->eq("\x{FF80}", "\x{30BF}"));
ok($objJa->eq("\x{FF81}", "\x{30C1}"));
ok($objJa->eq("\x{FF82}", "\x{30C4}"));
ok($objJa->eq("\x{FF83}", "\x{30C6}"));
ok($objJa->eq("\x{FF84}", "\x{30C8}"));
ok($objJa->eq("\x{FF85}", "\x{30CA}"));
ok($objJa->eq("\x{FF86}", "\x{30CB}"));
ok($objJa->eq("\x{FF87}", "\x{30CC}"));
ok($objJa->eq("\x{FF88}", "\x{30CD}"));
ok($objJa->eq("\x{FF89}", "\x{30CE}"));
ok($objJa->eq("\x{FF8A}", "\x{30CF}"));
ok($objJa->eq("\x{FF8B}", "\x{30D2}"));
ok($objJa->eq("\x{FF8C}", "\x{30D5}"));
ok($objJa->eq("\x{FF8D}", "\x{30D8}"));
ok($objJa->eq("\x{FF8E}", "\x{30DB}"));
ok($objJa->eq("\x{FF8F}", "\x{30DE}"));
ok($objJa->eq("\x{FF90}", "\x{30DF}"));
ok($objJa->eq("\x{FF91}", "\x{30E0}"));
ok($objJa->eq("\x{FF92}", "\x{30E1}"));
ok($objJa->eq("\x{FF93}", "\x{30E2}"));
ok($objJa->eq("\x{FF94}", "\x{30E4}"));
ok($objJa->eq("\x{FF95}", "\x{30E6}"));
ok($objJa->eq("\x{FF96}", "\x{30E8}"));
ok($objJa->eq("\x{FF97}", "\x{30E9}"));
ok($objJa->eq("\x{FF98}", "\x{30EA}"));
ok($objJa->eq("\x{FF99}", "\x{30EB}"));
ok($objJa->eq("\x{FF9A}", "\x{30EC}"));
ok($objJa->eq("\x{FF9B}", "\x{30ED}"));
ok($objJa->eq("\x{FF9C}", "\x{30EF}"));
ok($objJa->eq("\x{FF9D}", "\x{30F3}"));
ok($objJa->eq("\x{FF9E}", "\x{3099}"));
ok($objJa->eq("\x{FF9F}", "\x{309A}"));
ok($objJa->eq("\x{FFE0}", pack('U', 0xA2)));
ok($objJa->eq("\x{FFE1}", pack('U', 0xA3)));
ok($objJa->eq("\x{FFE2}", pack('U', 0xAC)));
ok($objJa->eq("\x{FFE3}", "\ "));
ok($objJa->eq("\x{FFE4}", pack('U', 0xA6)));
ok($objJa->eq("\x{FFE5}", pack('U', 0xA5)));
ok($objJa->eq("\x{FFE6}", "\x{20A9}"));
ok($objJa->eq("\x{FFE8}", "\x{2502}"));
ok($objJa->eq("\x{FFE9}", "\x{2190}"));
ok($objJa->eq("\x{FFEA}", "\x{2191}"));
ok($objJa->eq("\x{FFEB}", "\x{2192}"));
ok($objJa->eq("\x{FFEC}", "\x{2193}"));
ok($objJa->eq("\x{FFED}", "\x{25A0}"));
ok($objJa->eq("\x{FFEE}", "\x{25CB}"));

# 294

$objJa->change(level => 4);

ok($objJa->lt("\x{3041}", "\x{30A1}"));
ok($objJa->lt("\x{3042}", "\x{30A2}"));
ok($objJa->lt("\x{3043}", "\x{30A3}"));
ok($objJa->lt("\x{3044}", "\x{30A4}"));
ok($objJa->lt("\x{3045}", "\x{30A5}"));
ok($objJa->lt("\x{3046}", "\x{30A6}"));
ok($objJa->lt("\x{3047}", "\x{30A7}"));
ok($objJa->lt("\x{3048}", "\x{30A8}"));
ok($objJa->lt("\x{3049}", "\x{30A9}"));
ok($objJa->lt("\x{304A}", "\x{30AA}"));
ok($objJa->lt("\x{304B}", "\x{30AB}"));
ok($objJa->lt("\x{304C}", "\x{30AC}"));
ok($objJa->lt("\x{304D}", "\x{30AD}"));
ok($objJa->lt("\x{304E}", "\x{30AE}"));
ok($objJa->lt("\x{304F}", "\x{30AF}"));
ok($objJa->lt("\x{3050}", "\x{30B0}"));
ok($objJa->lt("\x{3051}", "\x{30B1}"));
ok($objJa->lt("\x{3052}", "\x{30B2}"));
ok($objJa->lt("\x{3053}", "\x{30B3}"));
ok($objJa->lt("\x{3054}", "\x{30B4}"));
ok($objJa->lt("\x{3055}", "\x{30B5}"));
ok($objJa->lt("\x{3056}", "\x{30B6}"));
ok($objJa->lt("\x{3057}", "\x{30B7}"));
ok($objJa->lt("\x{3058}", "\x{30B8}"));
ok($objJa->lt("\x{3059}", "\x{30B9}"));
ok($objJa->lt("\x{305A}", "\x{30BA}"));
ok($objJa->lt("\x{305B}", "\x{30BB}"));
ok($objJa->lt("\x{305C}", "\x{30BC}"));
ok($objJa->lt("\x{305D}", "\x{30BD}"));
ok($objJa->lt("\x{305E}", "\x{30BE}"));
ok($objJa->lt("\x{305F}", "\x{30BF}"));
ok($objJa->lt("\x{3060}", "\x{30C0}"));
ok($objJa->lt("\x{3061}", "\x{30C1}"));
ok($objJa->lt("\x{3062}", "\x{30C2}"));
ok($objJa->lt("\x{3063}", "\x{30C3}"));
ok($objJa->lt("\x{3064}", "\x{30C4}"));
ok($objJa->lt("\x{3065}", "\x{30C5}"));
ok($objJa->lt("\x{3066}", "\x{30C6}"));
ok($objJa->lt("\x{3067}", "\x{30C7}"));
ok($objJa->lt("\x{3068}", "\x{30C8}"));
ok($objJa->lt("\x{3069}", "\x{30C9}"));
ok($objJa->lt("\x{306A}", "\x{30CA}"));
ok($objJa->lt("\x{306B}", "\x{30CB}"));
ok($objJa->lt("\x{306C}", "\x{30CC}"));
ok($objJa->lt("\x{306D}", "\x{30CD}"));
ok($objJa->lt("\x{306E}", "\x{30CE}"));
ok($objJa->lt("\x{306F}", "\x{30CF}"));
ok($objJa->lt("\x{3070}", "\x{30D0}"));
ok($objJa->lt("\x{3071}", "\x{30D1}"));
ok($objJa->lt("\x{3072}", "\x{30D2}"));
ok($objJa->lt("\x{3073}", "\x{30D3}"));
ok($objJa->lt("\x{3074}", "\x{30D4}"));
ok($objJa->lt("\x{3075}", "\x{30D5}"));
ok($objJa->lt("\x{3076}", "\x{30D6}"));
ok($objJa->lt("\x{3077}", "\x{30D7}"));
ok($objJa->lt("\x{3078}", "\x{30D8}"));
ok($objJa->lt("\x{3079}", "\x{30D9}"));
ok($objJa->lt("\x{307A}", "\x{30DA}"));
ok($objJa->lt("\x{307B}", "\x{30DB}"));
ok($objJa->lt("\x{307C}", "\x{30DC}"));
ok($objJa->lt("\x{307D}", "\x{30DD}"));
ok($objJa->lt("\x{307E}", "\x{30DE}"));
ok($objJa->lt("\x{307F}", "\x{30DF}"));
ok($objJa->lt("\x{3080}", "\x{30E0}"));
ok($objJa->lt("\x{3081}", "\x{30E1}"));
ok($objJa->lt("\x{3082}", "\x{30E2}"));
ok($objJa->lt("\x{3083}", "\x{30E3}"));
ok($objJa->lt("\x{3084}", "\x{30E4}"));
ok($objJa->lt("\x{3085}", "\x{30E5}"));
ok($objJa->lt("\x{3086}", "\x{30E6}"));
ok($objJa->lt("\x{3087}", "\x{30E7}"));
ok($objJa->lt("\x{3088}", "\x{30E8}"));
ok($objJa->lt("\x{3089}", "\x{30E9}"));
ok($objJa->lt("\x{308A}", "\x{30EA}"));
ok($objJa->lt("\x{308B}", "\x{30EB}"));
ok($objJa->lt("\x{308C}", "\x{30EC}"));
ok($objJa->lt("\x{308D}", "\x{30ED}"));
ok($objJa->lt("\x{308E}", "\x{30EE}"));
ok($objJa->lt("\x{308F}", "\x{30EF}"));
ok($objJa->lt("\x{3090}", "\x{30F0}"));
ok($objJa->lt("\x{3091}", "\x{30F1}"));
ok($objJa->lt("\x{3092}", "\x{30F2}"));
ok($objJa->lt("\x{3093}", "\x{30F3}"));
ok($objJa->lt("\x{3094}", "\x{30F4}"));
ok($objJa->lt("\x{3095}", "\x{30F5}"));
ok($objJa->lt("\x{3096}", "\x{30F6}"));
ok($objJa->lt("\x{309D}", "\x{30FD}"));
ok($objJa->lt("\x{309E}", "\x{30FE}"));

# 382

ok($objJa->eq("\x{30AC}", "\x{30AB}\x{3099}"));
ok($objJa->eq("\x{30AE}", "\x{30AD}\x{3099}"));
ok($objJa->eq("\x{30B0}", "\x{30AF}\x{3099}"));
ok($objJa->eq("\x{30B2}", "\x{30B1}\x{3099}"));
ok($objJa->eq("\x{30B4}", "\x{30B3}\x{3099}"));
ok($objJa->eq("\x{30B6}", "\x{30B5}\x{3099}"));
ok($objJa->eq("\x{30B8}", "\x{30B7}\x{3099}"));
ok($objJa->eq("\x{30BA}", "\x{30B9}\x{3099}"));
ok($objJa->eq("\x{30BC}", "\x{30BB}\x{3099}"));
ok($objJa->eq("\x{30BE}", "\x{30BD}\x{3099}"));
ok($objJa->eq("\x{30C0}", "\x{30BF}\x{3099}"));
ok($objJa->eq("\x{30C2}", "\x{30C1}\x{3099}"));
ok($objJa->eq("\x{30C5}", "\x{30C4}\x{3099}"));
ok($objJa->eq("\x{30C7}", "\x{30C6}\x{3099}"));
ok($objJa->eq("\x{30C9}", "\x{30C8}\x{3099}"));
ok($objJa->eq("\x{30D0}", "\x{30CF}\x{3099}"));
ok($objJa->eq("\x{30D1}", "\x{30CF}\x{309A}"));
ok($objJa->eq("\x{30D3}", "\x{30D2}\x{3099}"));
ok($objJa->eq("\x{30D4}", "\x{30D2}\x{309A}"));
ok($objJa->eq("\x{30D6}", "\x{30D5}\x{3099}"));
ok($objJa->eq("\x{30D7}", "\x{30D5}\x{309A}"));
ok($objJa->eq("\x{30D9}", "\x{30D8}\x{3099}"));
ok($objJa->eq("\x{30DA}", "\x{30D8}\x{309A}"));
ok($objJa->eq("\x{30DC}", "\x{30DB}\x{3099}"));
ok($objJa->eq("\x{30DD}", "\x{30DB}\x{309A}"));
ok($objJa->eq("\x{30F4}", "\x{30A6}\x{3099}"));
ok($objJa->eq("\x{30F7}", "\x{30EF}\x{3099}"));
ok($objJa->eq("\x{30F8}", "\x{30F0}\x{3099}"));
ok($objJa->eq("\x{30F9}", "\x{30F1}\x{3099}"));
ok($objJa->eq("\x{30FA}", "\x{30F2}\x{3099}"));
ok($objJa->eq("\x{30FE}", "\x{30FD}\x{3099}"));

# 413

ok($objJa->eq("\x{304C}", "\x{304B}\x{3099}"));
ok($objJa->eq("\x{304E}", "\x{304D}\x{3099}"));
ok($objJa->eq("\x{3050}", "\x{304F}\x{3099}"));
ok($objJa->eq("\x{3052}", "\x{3051}\x{3099}"));
ok($objJa->eq("\x{3054}", "\x{3053}\x{3099}"));
ok($objJa->eq("\x{3056}", "\x{3055}\x{3099}"));
ok($objJa->eq("\x{3058}", "\x{3057}\x{3099}"));
ok($objJa->eq("\x{305A}", "\x{3059}\x{3099}"));
ok($objJa->eq("\x{305C}", "\x{305B}\x{3099}"));
ok($objJa->eq("\x{305E}", "\x{305D}\x{3099}"));
ok($objJa->eq("\x{3060}", "\x{305F}\x{3099}"));
ok($objJa->eq("\x{3062}", "\x{3061}\x{3099}"));
ok($objJa->eq("\x{3065}", "\x{3064}\x{3099}"));
ok($objJa->eq("\x{3067}", "\x{3066}\x{3099}"));
ok($objJa->eq("\x{3069}", "\x{3068}\x{3099}"));
ok($objJa->eq("\x{3070}", "\x{306F}\x{3099}"));
ok($objJa->eq("\x{3071}", "\x{306F}\x{309A}"));
ok($objJa->eq("\x{3073}", "\x{3072}\x{3099}"));
ok($objJa->eq("\x{3074}", "\x{3072}\x{309A}"));
ok($objJa->eq("\x{3076}", "\x{3075}\x{3099}"));
ok($objJa->eq("\x{3077}", "\x{3075}\x{309A}"));
ok($objJa->eq("\x{3079}", "\x{3078}\x{3099}"));
ok($objJa->eq("\x{307A}", "\x{3078}\x{309A}"));
ok($objJa->eq("\x{307C}", "\x{307B}\x{3099}"));
ok($objJa->eq("\x{307D}", "\x{307B}\x{309A}"));
ok($objJa->eq("\x{3094}", "\x{3046}\x{3099}"));
ok($objJa->eq("\x{309E}", "\x{309D}\x{3099}"));

# 440

$objJa->change(katakana_before_hiragana => 1);

ok($objJa->lt("\x{3041}", "\x{30A1}"));
ok($objJa->lt("\x{3042}", "\x{30A2}"));
ok($objJa->lt("\x{3043}", "\x{30A3}"));
ok($objJa->lt("\x{3044}", "\x{30A4}"));
ok($objJa->lt("\x{3045}", "\x{30A5}"));
ok($objJa->lt("\x{3046}", "\x{30A6}"));
ok($objJa->lt("\x{3047}", "\x{30A7}"));
ok($objJa->lt("\x{3048}", "\x{30A8}"));
ok($objJa->lt("\x{3049}", "\x{30A9}"));
ok($objJa->lt("\x{304A}", "\x{30AA}"));
ok($objJa->lt("\x{304B}", "\x{30AB}"));
ok($objJa->lt("\x{304C}", "\x{30AC}"));
ok($objJa->lt("\x{304D}", "\x{30AD}"));
ok($objJa->lt("\x{304E}", "\x{30AE}"));
ok($objJa->lt("\x{304F}", "\x{30AF}"));
ok($objJa->lt("\x{3050}", "\x{30B0}"));
ok($objJa->lt("\x{3051}", "\x{30B1}"));
ok($objJa->lt("\x{3052}", "\x{30B2}"));
ok($objJa->lt("\x{3053}", "\x{30B3}"));
ok($objJa->lt("\x{3054}", "\x{30B4}"));
ok($objJa->lt("\x{3055}", "\x{30B5}"));
ok($objJa->lt("\x{3056}", "\x{30B6}"));
ok($objJa->lt("\x{3057}", "\x{30B7}"));
ok($objJa->lt("\x{3058}", "\x{30B8}"));
ok($objJa->lt("\x{3059}", "\x{30B9}"));
ok($objJa->lt("\x{305A}", "\x{30BA}"));
ok($objJa->lt("\x{305B}", "\x{30BB}"));
ok($objJa->lt("\x{305C}", "\x{30BC}"));
ok($objJa->lt("\x{305D}", "\x{30BD}"));
ok($objJa->lt("\x{305E}", "\x{30BE}"));
ok($objJa->lt("\x{305F}", "\x{30BF}"));
ok($objJa->lt("\x{3060}", "\x{30C0}"));
ok($objJa->lt("\x{3061}", "\x{30C1}"));
ok($objJa->lt("\x{3062}", "\x{30C2}"));
ok($objJa->lt("\x{3063}", "\x{30C3}"));
ok($objJa->lt("\x{3064}", "\x{30C4}"));
ok($objJa->lt("\x{3065}", "\x{30C5}"));
ok($objJa->lt("\x{3066}", "\x{30C6}"));
ok($objJa->lt("\x{3067}", "\x{30C7}"));
ok($objJa->lt("\x{3068}", "\x{30C8}"));
ok($objJa->lt("\x{3069}", "\x{30C9}"));
ok($objJa->lt("\x{306A}", "\x{30CA}"));
ok($objJa->lt("\x{306B}", "\x{30CB}"));
ok($objJa->lt("\x{306C}", "\x{30CC}"));
ok($objJa->lt("\x{306D}", "\x{30CD}"));
ok($objJa->lt("\x{306E}", "\x{30CE}"));
ok($objJa->lt("\x{306F}", "\x{30CF}"));
ok($objJa->lt("\x{3070}", "\x{30D0}"));
ok($objJa->lt("\x{3071}", "\x{30D1}"));
ok($objJa->lt("\x{3072}", "\x{30D2}"));
ok($objJa->lt("\x{3073}", "\x{30D3}"));
ok($objJa->lt("\x{3074}", "\x{30D4}"));
ok($objJa->lt("\x{3075}", "\x{30D5}"));
ok($objJa->lt("\x{3076}", "\x{30D6}"));
ok($objJa->lt("\x{3077}", "\x{30D7}"));
ok($objJa->lt("\x{3078}", "\x{30D8}"));
ok($objJa->lt("\x{3079}", "\x{30D9}"));
ok($objJa->lt("\x{307A}", "\x{30DA}"));
ok($objJa->lt("\x{307B}", "\x{30DB}"));
ok($objJa->lt("\x{307C}", "\x{30DC}"));
ok($objJa->lt("\x{307D}", "\x{30DD}"));
ok($objJa->lt("\x{307E}", "\x{30DE}"));
ok($objJa->lt("\x{307F}", "\x{30DF}"));
ok($objJa->lt("\x{3080}", "\x{30E0}"));
ok($objJa->lt("\x{3081}", "\x{30E1}"));
ok($objJa->lt("\x{3082}", "\x{30E2}"));
ok($objJa->lt("\x{3083}", "\x{30E3}"));
ok($objJa->lt("\x{3084}", "\x{30E4}"));
ok($objJa->lt("\x{3085}", "\x{30E5}"));
ok($objJa->lt("\x{3086}", "\x{30E6}"));
ok($objJa->lt("\x{3087}", "\x{30E7}"));
ok($objJa->lt("\x{3088}", "\x{30E8}"));
ok($objJa->lt("\x{3089}", "\x{30E9}"));
ok($objJa->lt("\x{308A}", "\x{30EA}"));
ok($objJa->lt("\x{308B}", "\x{30EB}"));
ok($objJa->lt("\x{308C}", "\x{30EC}"));
ok($objJa->lt("\x{308D}", "\x{30ED}"));
ok($objJa->lt("\x{308E}", "\x{30EE}"));
ok($objJa->lt("\x{308F}", "\x{30EF}"));
ok($objJa->lt("\x{3090}", "\x{30F0}"));
ok($objJa->lt("\x{3091}", "\x{30F1}"));
ok($objJa->lt("\x{3092}", "\x{30F2}"));
ok($objJa->lt("\x{3093}", "\x{30F3}"));
ok($objJa->lt("\x{3094}", "\x{30F4}"));
ok($objJa->lt("\x{3095}", "\x{30F5}"));
ok($objJa->lt("\x{3096}", "\x{30F6}"));
ok($objJa->lt("\x{309D}", "\x{30FD}"));
ok($objJa->lt("\x{309E}", "\x{30FE}"));

# 528
