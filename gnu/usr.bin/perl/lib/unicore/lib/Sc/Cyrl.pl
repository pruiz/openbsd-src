# !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
# This file is machine-generated by lib/unicore/mktables from the Unicode
# database, Version 5.2.0.  Any changes made here will be lost!

# !!!!!!!   INTERNAL PERL USE ONLY   !!!!!!!
# This file is for internal use by the Perl program only.  The format and even
# the name or existence of this file are subject to change without notice.
# Don't use it directly.

# This file returns the 404 code points in Unicode Version 5.2.0 that match
# any of the following regular expression constructs:
# 
#         \p{Script=Cyrillic}
#         \p{Sc=Cyrl}
#         \p{Is_Script=Cyrillic}
#         \p{Is_Sc=Cyrl}
# 
#         \p{Cyrillic}
#         \p{Is_Cyrillic}
#         \p{Cyrl}
#         \p{Is_Cyrl}
# 
#     Note that contrary to what you might expect, the above is NOT the same
#     as \p{Block=Cyrillic}
# 
# perluniprops.pod should be consulted for the syntax rules for any of these,
# including if adding or subtracting white space, underscore, and hyphen
# characters matters or doesn't matter, and other permissible syntactic
# variants.  Upper/lower case distinctions never matter.
# 
# A colon can be substituted for the equals sign, and anything to the left of
# the equals (or colon) can be combined with anything to the right.  Thus,
# for example,
#         \p{Is_Sc: Cyrillic}
# is also valid.
# 
# The format of the lines of this file is: START\tSTOP\twhere START is the
# starting code point of the range, in hex; STOP is the ending point, or if
# omitted, the range has just one code point.  Numbers in comments in
# [brackets] indicate how many code points are in the range.

return <<'END';
0400	0484	 # [133]
0487	0525	 # [159]
1D2B		
1D78		
2DE0	2DFF	 # [32]
A640	A65F	 # [32]
A662	A673	 # [18]
A67C	A697	 # [28]
END
