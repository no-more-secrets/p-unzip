# This is only used at the start of the mono build file.

true  := T
false :=

not = $(if $1,$(false),$(true))

Makefile  := $(lastword $(MAKEFILE_LIST))
root      := $(dir $(Makefile))

-include $(HOME)/.nr-make-rc.mk

CWD := $(root)

makerules := $(dir $(realpath $(Makefile)))
# ----------------------------------------------------------------------------
#
# GNU Make Standard Library (GMSL)
#
# A library of functions to be used with GNU Make's $(call) that
# provides functionality not available in standard GNU Make.
#
# Copyright (c) 2005-2014 John Graham-Cumming
#
# This file is part of GMSL
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# Neither the name of the John Graham-Cumming nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# ----------------------------------------------------------------------------

# This is the GNU Make Standard Library version number as a list with
# three items: major, minor, revision

gmsl_version := 1 1 7

__gmsl_name := GNU Make Standard Library

# Used to output warnings and error from the library, it's possible to
# disable any warnings or errors by overriding these definitions
# manually or by setting GMSL_NO_WARNINGS or GMSL_NO_ERRORS

ifdef GMSL_NO_WARNINGS
__gmsl_warning :=
else
__gmsl_warning = $(if $1,$(warning $(__gmsl_name): $1))
endif

ifdef GMSL_NO_ERRORS
__gmsl_error :=
else
# __gmsl_error = $(if $1,$(error $(__gmsl_name): $1))
# Removed __gmsl_name -- David Sicilia
 __gmsl_error = $(if $1,$(error $1))
endif

# If GMSL_TRACE is enabled then calls to the library functions are
# traced to stdout using warning messages with their arguments

ifdef GMSL_TRACE
__gmsl_tr1 = $(warning $0('$1'))
__gmsl_tr2 = $(warning $0('$1','$2'))
__gmsl_tr3 = $(warning $0('$1','$2','$3'))
else
__gmsl_tr1 :=
__gmsl_tr2 :=
__gmsl_tr3 :=
endif

# See if spaces are valid in variable names (this was the case until
# GNU Make 3.82)
ifeq ($(MAKE_VERSION),3.82)
__gmsl_spaced_vars := $(false)
else
__gmsl_spaced_vars := $(true)
endif

# Figure out whether we have $(eval) or not (GNU Make 3.80 and above)
# if we do not then output a warning message, if we do then some
# functions will be enabled.

__gmsl_have_eval := $(false)
__gmsl_ignore := $(eval __gmsl_have_eval := $(true))

# If this is being run with Electric Cloud's emake then warn that
# their $(eval) support is incomplete in 1.x, 2.x, 3.x, 4.x and 5.0,
# 5.1, 5.2 and 5.3

ifdef ECLOUD_BUILD_ID
__gmsl_emake_major := $(word 1,$(subst ., ,$(EMAKE_VERSION)))
__gmsl_emake_minor := $(word 2,$(subst ., ,$(EMAKE_VERSION)))
ifneq ("$(findstring $(__gmsl_emake_major),1 2 3 4)$(findstring $(__gmsl_emake_major)$(__gmsl_emake_minor),50 51 52 53)","")
$(warning You are using a version of Electric Cloud's emake which has incomplete $$(eval) support)
__gmsl_have_eval := $(false)
endif
endif

# See if we have $(lastword) (GNU Make 3.81 and above)

__gmsl_have_lastword := $(lastword $(false) $(true))

# See if we have native or and and (GNU Make 3.81 and above)

__gmsl_have_or := $(if $(filter-out undefined,  \
    $(origin or)),$(call or,$(true),$(false)))
__gmsl_have_and := $(if $(filter-out undefined, \
    $(origin and)),$(call and,$(true),$(true)))

ifneq ($(__gmsl_have_eval),$(true))
$(call __gmsl_warning,Your make version $(MAKE_VERSION) does not support $$$$(eval): some functions disabled)
endif

__gmsl_dollar := $$
__gmsl_hash := \#

# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------
# Function:  gmsl_compatible
# Arguments: List containing the desired library version number (maj min rev)
# Returns:   $(true) if this version of the library is compatible
#            with the requested version number, otherwise $(false)
# ----------------------------------------------------------------------------
gmsl_compatible = $(strip                                                 \
    $(if $(call gt,$(word 1,$1),$(word 1,$(gmsl_version))),               \
        $(false),                                                         \
        $(if $(call lt,$(word 1,$1),$(word 1,$(gmsl_version))),           \
            $(true),                                                      \
            $(if $(call gt,$(word 2,$1),$(word 2,$(gmsl_version))),       \
                $(false),                                                 \
                $(if $(call lt,$(word 2,$1),$(word 2,$(gmsl_version))),   \
                    $(true),                                              \
                    $(call lte,$(word 3,$1),$(word 3,$(gmsl_version))))))))

# ###########################################################################
# LOGICAL OPERATORS
# ###########################################################################

# not is defined in gmsl

# ----------------------------------------------------------------------------
# Function:  and
# Arguments: Two boolean values
# Returns:   Returns $(true) if both of the booleans are true
# ----------------------------------------------------------------------------
ifneq ($(__gmsl_have_and),$(true))
and = $(__gmsl_tr2)$(if $1,$(if $2,$(true),$(false)),$(false))
endif

# ----------------------------------------------------------------------------
# Function:  or
# Arguments: Two boolean values
# Returns:   Returns $(true) if either of the booleans is true
# ----------------------------------------------------------------------------
ifneq ($(__gmsl_have_or),$(true))
or = $(__gmsl_tr2)$(if $1$2,$(true),$(false))
endif

# ----------------------------------------------------------------------------
# Function:  xor
# Arguments: Two boolean values
# Returns:   Returns $(true) if exactly one of the booleans is true
# ----------------------------------------------------------------------------
xor = $(__gmsl_tr2)$(if $1,$(if $2,$(false),$(true)),$(if $2,$(true),$(false)))

# ----------------------------------------------------------------------------
# Function:  nand
# Arguments: Two boolean values
# Returns:   Returns value of 'not and'
# ----------------------------------------------------------------------------
nand = $(__gmsl_tr2)$(if $1,$(if $2,$(false),$(true)),$(true))

# ----------------------------------------------------------------------------
# Function:  nor
# Arguments: Two boolean values
# Returns:   Returns value of 'not or'
# ----------------------------------------------------------------------------
nor = $(__gmsl_tr2)$(if $1$2,$(false),$(true))

# ----------------------------------------------------------------------------
# Function:  xnor
# Arguments: Two boolean values
# Returns:   Returns value of 'not xor'
# ----------------------------------------------------------------------------
xnor =$(__gmsl_tr2)$(if $1,$(if $2,$(true),$(false)),$(if $2,$(false),$(true)))

# ###########################################################################
# LIST MANIPULATION FUNCTIONS
# ###########################################################################

# ----------------------------------------------------------------------------
# Function:  first (same as LISP's car, or head)
# Arguments: 1: A list
# Returns:   Returns the first element of a list
# ----------------------------------------------------------------------------
first = $(__gmsl_tr1)$(firstword $1)

# ----------------------------------------------------------------------------
# Function:  last
# Arguments: 1: A list
# Returns:   Returns the last element of a list
# ----------------------------------------------------------------------------
ifeq ($(__gmsl_have_lastword),$(true))
last = $(__gmsl_tr1)$(lastword $1)
else
last = $(__gmsl_tr1)$(if $1,$(word $(words $1),$1))
endif

# ----------------------------------------------------------------------------
# Function:  rest (same as LISP's cdr, or tail)
# Arguments: 1: A list
# Returns:   Returns the list with the first element removed
# ----------------------------------------------------------------------------
rest = $(__gmsl_tr1)$(wordlist 2,$(words $1),$1)

# ----------------------------------------------------------------------------
# Function:  chop
# Arguments: 1: A list
# Returns:   Returns the list with the last element removed
# ----------------------------------------------------------------------------
chop = $(__gmsl_tr1)$(wordlist 2,$(words $1),x $1)

# ----------------------------------------------------------------------------
# Function:  map
# Arguments: 1: Name of function to $(call) for each element of list
#            2: List to iterate over calling the function in 1
# Returns:   The list after calling the function on each element
# ----------------------------------------------------------------------------
map = $(__gmsl_tr2)$(strip $(foreach a,$2,$(call $1,$a)))

# ----------------------------------------------------------------------------
# Function:  pairmap
# Arguments: 1: Name of function to $(call) for each pair of elements
#            2: List to iterate over calling the function in 1
#            3: Second list to iterate over calling the function in 1
# Returns:   The list after calling the function on each pair of elements
# ----------------------------------------------------------------------------
pairmap = $(strip $(__gmsl_tr3)\
          $(if $2$3,$(call $1,$(call first,$2),$(call first,$3))     \
                        $(call pairmap,$1,$(call rest,$2),$(call rest,$3))))

# ----------------------------------------------------------------------------
# Function:  leq
# Arguments: 1: A list to compare against...
#            2: ...this list
# Returns:   Returns $(true) if the two lists are identical
# ----------------------------------------------------------------------------
leq = $(__gmsl_tr2)$(strip $(if $(call seq,$(words $1),$(words $2)),     \
          $(call __gmsl_list_equal,$1,$2),$(false)))

__gmsl_list_equal = $(if $(strip $1),                                       \
                        $(if $(call seq,$(call first,$1),$(call first,$2)), \
                            $(call __gmsl_list_equal,                       \
                                $(call rest,$1),                            \
                                $(call rest,$2)),                           \
                            $(false)),                                      \
                     $(true))

# ----------------------------------------------------------------------------
# Function:  lne
# Arguments: 1: A list to compare against...
#            2: ...this list
# Returns:   Returns $(true) if the two lists are different
# ----------------------------------------------------------------------------
lne = $(__gmsl_tr2)$(call not,$(call leq,$1,$2))

# ----------------------------------------------------------------------------
# Function:  reverse
# Arguments: 1: A list to reverse
# Returns:   The list with its elements in reverse order
# ----------------------------------------------------------------------------
reverse =$(__gmsl_tr1)$(strip $(if $1,$(call reverse,$(call rest,$1)) \
                        $(call first,$1)))

# ----------------------------------------------------------------------------
# Function:  uniq
# Arguments: 1: A list from which to remove repeated elements
# Returns:   The list with duplicate elements removed without reordering
# ----------------------------------------------------------------------------
uniq = $(strip $(__gmsl_tr1) $(if $1,$(firstword $1) \
                               $(call uniq,$(filter-out $(firstword $1),$1))))

# ----------------------------------------------------------------------------
# Function:  length
# Arguments: 1: A list
# Returns:   The number of elements in the list
# ----------------------------------------------------------------------------
length = $(__gmsl_tr1)$(words $1)

# ###########################################################################
# STRING MANIPULATION FUNCTIONS
# ###########################################################################

# Helper function that translates any GNU Make 'true' value (i.e. a
# non-empty string) to our $(true)

__gmsl_make_bool = $(if $(strip $1),$(true),$(false))

# ----------------------------------------------------------------------------
# Function:  seq
# Arguments: 1: A string to compare against...
#            2: ...this string
# Returns:   Returns $(true) if the two strings are identical
# ----------------------------------------------------------------------------
seq = $(__gmsl_tr2)$(if $(subst x$1,,x$2)$(subst x$2,,x$1),$(false),$(true))

# ----------------------------------------------------------------------------
# Function:  sne
# Arguments: 1: A string to compare against...
#            2: ...this string
# Returns:   Returns $(true) if the two strings are not the same
# ----------------------------------------------------------------------------
sne = $(__gmsl_tr2)$(call not,$(call seq,$1,$2))

# ----------------------------------------------------------------------------
# Function:  split
# Arguments: 1: The character to split on
#            2: A string to split
# Returns:   Splits a string into a list separated by spaces at the split
#            character in the first argument
# ----------------------------------------------------------------------------
split = $(__gmsl_tr2)$(strip $(subst $1, ,$2))

# ----------------------------------------------------------------------------
# Function:  merge
# Arguments: 1: The character to put between fields
#            2: A list to merge into a string
# Returns:   Merges a list into a single string, list elements are separated
#            by the character in the first argument
# ----------------------------------------------------------------------------
merge = $(__gmsl_tr2)$(strip $(if $2,                                     \
            $(if $(call seq,1,$(words $2)),                               \
                $2,$(call first,$2)$1$(call merge,$1,$(call rest,$2)))))

ifdef __gmsl_have_eval
# ----------------------------------------------------------------------------
# Function:  tr
# Arguments: 1: The list of characters to translate from 
#            2: The list of characters to translate to
#            3: The text to translate
# Returns:   Returns the text after translating characters
# ----------------------------------------------------------------------------
tr = $(strip $(__gmsl_tr3)$(call assert_no_dollar,$0,$1$2$3)              \
     $(eval __gmsl_t := $3)                                               \
     $(foreach c,                                                         \
         $(join $(addsuffix :,$1),$2),                                    \
         $(eval __gmsl_t :=                                               \
             $(subst $(word 1,$(subst :, ,$c)),$(word 2,$(subst :, ,$c)), \
                 $(__gmsl_t))))$(__gmsl_t))

# Common character classes for use with the tr function.  Each of
# these is actually a variable declaration and must be wrapped with
# $() or ${} to be used.

[A-Z] := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z #
[a-z] := a b c d e f g h i j k l m n o p q r s t u v w x y z #
[0-9] := 0 1 2 3 4 5 6 7 8 9 #
[A-F] := A B C D E F #

# ----------------------------------------------------------------------------
# Function:  uc
# Arguments: 1: Text to upper case
# Returns:   Returns the text in upper case
# ----------------------------------------------------------------------------
uc = $(__gmsl_tr1)$(call assert_no_dollar,$0,$1)$(call tr,$([a-z]),$([A-Z]),$1)

# ----------------------------------------------------------------------------
# Function:  lc
# Arguments: 1: Text to lower case
# Returns:   Returns the text in lower case
# ----------------------------------------------------------------------------
lc = $(__gmsl_tr1)$(call assert_no_dollar,$0,$1)$(call tr,$([A-Z]),$([a-z]),$1)

# ----------------------------------------------------------------------------
# Function:  strlen
# Arguments: 1: A string
# Returns:   Returns the length of the string
# ----------------------------------------------------------------------------

# This results in __gmsl_tab containing a tab

__gmsl_tab :=	#

__gmsl_characters := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
__gmsl_characters += a b c d e f g h i j k l m n o p q r s t u v w x y z
__gmsl_characters += 0 1 2 3 4 5 6 7 8 9
__gmsl_characters += ` ~ ! @ \# $$ % ^ & * ( ) - _ = +
__gmsl_characters += { } [ ] \ : ; ' " < > , . / ? |

# This results in __gmsl_space containing just a space

__gmsl_space := 
__gmsl_space +=

strlen = $(__gmsl_tr1)$(call assert_no_dollar,$0,$1)$(strip $(eval __temp := $(subst $(__gmsl_space),x,$1))$(foreach a,$(__gmsl_characters),$(eval __temp := $$(subst $$a,x,$(__temp))))$(eval __temp := $(subst x,x ,$(__temp)))$(words $(__temp)))

# This results in __gmsl_newline containing just a newline

define __gmsl_newline


endef

# ----------------------------------------------------------------------------
# Function:  substr
# Arguments: 1: A string
#            2: Start position (first character is 1)
#            3: End position (inclusive)
# Returns:   A substring.  
# Note:      The string in $1 must not contain a §
# ----------------------------------------------------------------------------

substr = $(if $2,$(__gmsl_tr3)$(call assert_no_dollar,$0,$1$2$3)$(strip $(eval __temp := $$(subst $$(__gmsl_space),§ ,$$1))$(foreach a,$(__gmsl_characters),$(eval __temp := $$(subst $$a,$$a$$(__gmsl_space),$(__temp))))$(eval __temp := $(wordlist $2,$3,$(__temp))))$(subst §,$(__gmsl_space),$(subst $(__gmsl_space),,$(__temp))))

endif # __gmsl_have_eval

# ###########################################################################
# SET MANIPULATION FUNCTIONS
# ###########################################################################

# Sets are represented by sorted, deduplicated lists.  To create a set
# from a list use set_create, or start with the empty_set and
# set_insert individual elements

# This is the empty set
empty_set := 

# ----------------------------------------------------------------------------
# Function:  set_create
# Arguments: 1: A list of set elements
# Returns:   Returns the newly created set
# ----------------------------------------------------------------------------
set_create = $(__gmsl_tr1)$(sort $1)

# ----------------------------------------------------------------------------
# Function:  set_insert
# Arguments: 1: A single element to add to a set
#            2: A set
# Returns:   Returns the set with the element added
# ----------------------------------------------------------------------------
set_insert = $(__gmsl_tr2)$(sort $1 $2)

# ----------------------------------------------------------------------------
# Function:  set_remove
# Arguments: 1: A single element to remove from a set
#            2: A set
# Returns:   Returns the set with the element removed
# ----------------------------------------------------------------------------
set_remove = $(__gmsl_tr2)$(filter-out $1,$2)

# ----------------------------------------------------------------------------
# Function:  set_is_member, set_is_not_member
# Arguments: 1: A single element 
#            2: A set
# Returns:   (set_is_member) Returns $(true) if the element is in the set
#            (set_is_not_member) Returns $(false) if the element is in the set
# ----------------------------------------------------------------------------
set_is_member = $(__gmsl_tr2)$(if $(filter $1,$2),$(true),$(false))
set_is_not_member = $(__gmsl_tr2)$(if $(filter $1,$2),$(false),$(true))

# ----------------------------------------------------------------------------
# Function:  set_union
# Arguments: 1: A set
#            2: Another set
# Returns:   Returns the union of the two sets
# ----------------------------------------------------------------------------
set_union = $(__gmsl_tr2)$(sort $1 $2)

# ----------------------------------------------------------------------------
# Function:  set_intersection
# Arguments: 1: A set
#            2: Another set
# Returns:   Returns the intersection of the two sets
# ----------------------------------------------------------------------------
set_intersection = $(__gmsl_tr2)$(filter $1,$2)

# ----------------------------------------------------------------------------
# Function:  set_is_subset
# Arguments: 1: A set
#            2: Another set
# Returns:   Returns $(true) if the first set is a subset of the second
# ----------------------------------------------------------------------------
set_is_subset = $(__gmsl_tr2)$(call set_equal,$(call set_intersection,$1,$2),$1)

# ----------------------------------------------------------------------------
# Function:  set_equal
# Arguments: 1: A set
#            2: Another set
# Returns:   Returns $(true) if the two sets are identical
# ----------------------------------------------------------------------------
set_equal = $(__gmsl_tr2)$(call seq,$1,$2)

# ###########################################################################
# ARITHMETIC LIBRARY
# ###########################################################################

# Integers a represented by lists with the equivalent number of x's.
# For example the number 4 is x x x x. 

# ----------------------------------------------------------------------------
# Function:  int_decode
# Arguments: 1: A number of x's representation
# Returns:   Returns the integer for human consumption that is represented
#            by the string of x's
# ----------------------------------------------------------------------------
int_decode = $(__gmsl_tr1)$(words $1)

# ----------------------------------------------------------------------------
# Function:  int_encode
# Arguments: 1: A number in human-readable integer form
# Returns:   Returns the integer encoded as a string of x's
# ----------------------------------------------------------------------------
__int_encode = $(if $1,$(if $(call seq,$(words $(wordlist 1,$1,$2)),$1),$(wordlist 1,$1,$2),$(call __int_encode,$1,$(if $2,$2 $2,x))))
__strip_leading_zero = $(if $1,$(if $(call seq,$(patsubst 0%,%,$1),$1),$1,$(call __strip_leading_zero,$(patsubst 0%,%,$1))),0)
int_encode = $(__gmsl_tr1)$(call __int_encode,$(call __strip_leading_zero,$1))

# The arithmetic library functions come in two forms: one form of each
# function takes integers as arguments and the other form takes the
# encoded form (x's created by a call to int_encode).  For example,
# there are two plus functions:
#
# plus        Called with integer arguments and returns an integer
# int_plus    Called with encoded arguments and returns an encoded result
#
# plus will be slower than int_plus because its arguments and result
# have to be translated between the x's format and integers.  If doing
# a complex calculation use the int_* forms with a single encoding of
# inputs and single decoding of the output.  For simple calculations
# the direct forms can be used.

# Helper function used to wrap an int_* function into a function that
# takes a pair of integers, perhaps a function and returns an integer
# result
__gmsl_int_wrap = $(call int_decode,$(call $1,$(call int_encode,$2),$(call int_encode,$3)))
__gmsl_int_wrap1 = $(call int_decode,$(call $1,$(call int_encode,$2)))
__gmsl_int_wrap2 = $(call $1,$(call int_encode,$2),$(call int_encode,$3))

# ----------------------------------------------------------------------------
# Function:  int_plus
# Arguments: 1: A number in x's representation
#            2: Another number in x's represntation
# Returns:   Returns the sum of the two numbers in x's representation
# ----------------------------------------------------------------------------
int_plus = $(strip $(__gmsl_tr2)$1 $2)

# ----------------------------------------------------------------------------
# Function:  plus (wrapped version of int_plus)
# Arguments: 1: An integer
#            2: Another integer
# Returns:   Returns the sum of the two integers
# ----------------------------------------------------------------------------
plus = $(__gmsl_tr2)$(call __gmsl_int_wrap,int_plus,$1,$2)

# ----------------------------------------------------------------------------
# Function:  int_subtract
# Arguments: 1: A number in x's representation
#            2: Another number in x's represntation
# Returns:   Returns the difference of the two numbers in x's representation,
#            or outputs an error on a numeric underflow
# ----------------------------------------------------------------------------
int_subtract = $(strip $(__gmsl_tr2)$(if $(call int_gte,$1,$2), \
                $(filter-out xx,$(join $1,$2)),                 \
                $(call __gmsl_warning,Subtraction underflow)))

# ----------------------------------------------------------------------------
# Function:  subtract (wrapped version of int_subtract)
# Arguments: 1: An integer
#            2: Another integer
# Returns:   Returns the difference of the two integers,
#            or outputs an error on a numeric underflow
# ----------------------------------------------------------------------------
subtract = $(__gmsl_tr2)$(call __gmsl_int_wrap,int_subtract,$1,$2)

# ----------------------------------------------------------------------------
# Function:  int_multiply
# Arguments: 1: A number in x's representation
#            2: Another number in x's represntation
# Returns:   Returns the product of the two numbers in x's representation
# ----------------------------------------------------------------------------
int_multiply = $(strip $(__gmsl_tr2)$(foreach a,$1,$2))

# ----------------------------------------------------------------------------
# Function:  multiply (wrapped version of int_multiply)
# Arguments: 1: An integer
#            2: Another integer
# Returns:   Returns the product of the two integers
# ----------------------------------------------------------------------------
multiply = $(__gmsl_tr2)$(call __gmsl_int_wrap,int_multiply,$1,$2)

# ----------------------------------------------------------------------------
# Function:  int_divide
# Arguments: 1: A number in x's representation
#            2: Another number in x's represntation
# Returns:   Returns the result of integer division of argument 1 divided
#            by argument 2 in x's representation
# ----------------------------------------------------------------------------
int_divide = $(__gmsl_tr2)$(strip $(if $1,$(if $2,                         \
                 $(if $(call int_gte,$1,$2),                               \
                     x $(call int_divide,$(call int_subtract,$1,$2),$2),), \
                 $(call __gmsl_error,Division by zero))))

# ----------------------------------------------------------------------------
# Function:  divide (wrapped version of int_divide)
# Arguments: 1: An integer
#            2: Another integer
# Returns:   Returns the integer division of the first argument by the second
# ----------------------------------------------------------------------------
divide = $(__gmsl_tr2)$(call __gmsl_int_wrap,int_divide,$1,$2)

# ----------------------------------------------------------------------------
# Function:  int_max, int_min
# Arguments: 1: A number in x's representation
#            2: Another number in x's represntation
# Returns:   Returns the maximum or minimum of its arguments in x's
#            representation
# ----------------------------------------------------------------------------
int_max = $(__gmsl_tr2)$(subst xx,x,$(join $1,$2))
int_min = $(__gmsl_tr2)$(subst xx,x,$(filter xx,$(join $1,$2)))

# ----------------------------------------------------------------------------
# Function:  max, min
# Arguments: 1: An integer
#            2: Another integer
# Returns:   Returns the maximum or minimum of its integer arguments
# ----------------------------------------------------------------------------
max = $(__gmsl_tr2)$(call __gmsl_int_wrap,int_max,$1,$2)
min = $(__gmsl_tr2)$(call __gmsl_int_wrap,int_min,$1,$2)

# ----------------------------------------------------------------------------
# Function: int_gt, int_gte, int_lt, int_lte, int_eq, int_ne
# Arguments: Two x's representation numbers to be compared
# Returns:   $(true) or $(false)
#
# int_gt    First argument greater than second argument
# int_gte   First argument greater than or equal to second argument
# int_lt    First argument less than second argument 
# int_lte   First argument less than or equal to second argument
# int_eq    First argument is numerically equal to the second argument
# int_ne    First argument is not numerically equal to the second argument
# ----------------------------------------------------------------------------
int_gt = $(__gmsl_tr2)$(call __gmsl_make_bool,      \
                          $(filter-out $(words $2), \
                              $(words $(call int_max,$1,$2))))
int_gte = $(__gmsl_tr2)$(call __gmsl_make_bool,     \
                           $(call int_gt,$1,$2)$(call int_eq,$1,$2))
int_lt = $(__gmsl_tr2)$(call __gmsl_make_bool,      \
                          $(filter-out $(words $1), \
                              $(words $(call int_max,$1,$2))))
int_lte = $(__gmsl_tr2)$(call __gmsl_make_bool,     \
                           $(call int_lt,$1,$2)$(call int_eq,$1,$2))
int_eq = $(__gmsl_tr2)$(call __gmsl_make_bool,      \
                          $(filter $(words $1),$(words $2)))
int_ne = $(__gmsl_tr2)$(call __gmsl_make_bool,      \
                          $(filter-out $(words $1),$(words $2)))

# ----------------------------------------------------------------------------
# Function: gt, gte, lt, lte, eq, ne
# Arguments: Two integers to be compared
# Returns:   $(true) or $(false)
#
# gt    First argument greater than second argument
# gte   First argument greater than or equal to second argument
# lt    First argument less than second argument 
# lte   First argument less than or equal to second argument
# eq    First argument is numerically equal to the second argument
# ne    First argument is not numerically equal to the second argument
# ----------------------------------------------------------------------------
gt = $(__gmsl_tr2)$(call __gmsl_int_wrap2,int_gt,$1,$2)
gte = $(__gmsl_tr2)$(call __gmsl_int_wrap2,int_gte,$1,$2)
lt = $(__gmsl_tr2)$(call __gmsl_int_wrap2,int_lt,$1,$2)
lte = $(__gmsl_tr2)$(call __gmsl_int_wrap2,int_lte,$1,$2)
eq = $(__gmsl_tr2)$(call __gmsl_int_wrap2,int_eq,$1,$2)
ne = $(__gmsl_tr2)$(call __gmsl_int_wrap2,int_ne,$1,$2)

# increment adds 1 to its argument, decrement subtracts 1.  Note that
# decrement does not range check and hence will not underflow, but
# will incorrectly say that 0 - 1 = 0

# ----------------------------------------------------------------------------
# Function:  int_inc
# Arguments: 1: A number in x's representation
# Returns:   The number incremented by 1 in x's representation
# ----------------------------------------------------------------------------
int_inc = $(strip $(__gmsl_tr1)$1 x)

# ----------------------------------------------------------------------------
# Function:  inc
# Arguments: 1: An integer
# Returns:   The argument incremented by 1
# ----------------------------------------------------------------------------
inc = $(__gmsl_tr1)$(call __gmsl_int_wrap1,int_inc,$1)

# ----------------------------------------------------------------------------
# Function:  int_dec
# Arguments: 1: A number in x's representation
# Returns:   The number decremented by 1 in x's representation
# ----------------------------------------------------------------------------
int_dec = $(__gmsl_tr1)$(strip                        \
          $(if $(call sne,0,$(words $1)),             \
          $(wordlist 2,$(words $1),$1)))

# ----------------------------------------------------------------------------
# Function:  dec
# Arguments: 1: An integer
# Returns:   The argument decremented by 1
# ----------------------------------------------------------------------------
dec = $(__gmsl_tr1)$(call __gmsl_int_wrap1,int_dec,$1)

# double doubles its argument, and halve halves it

# ----------------------------------------------------------------------------
# Function:  int_double
# Arguments: 1: A number in x's representation
# Returns:   The number doubled (i.e. * 2) and returned in x's representation
# ----------------------------------------------------------------------------
int_double = $(strip $(__gmsl_tr1)$1 $1)

# ----------------------------------------------------------------------------
# Function:  double
# Arguments: 1: An integer
# Returns:   The integer times 2
# ----------------------------------------------------------------------------
double = $(__gmsl_tr1)$(call __gmsl_int_wrap1,int_double,$1)

# ----------------------------------------------------------------------------
# Function:  int_halve
# Arguments: 1: A number in x's representation
# Returns:   The number halved (i.e. / 2) and returned in x's representation
# ----------------------------------------------------------------------------
int_halve = $(__gmsl_tr1)$(strip $(subst xx,x,$(filter-out xy x y, \
                             $(join $1,$(foreach a,$1,y x)))))

# ----------------------------------------------------------------------------
# Function:  halve
# Arguments: 1: An integer
# Returns:   The integer divided by 2
# ----------------------------------------------------------------------------
halve = $(__gmsl_tr1)$(call __gmsl_int_wrap1,int_halve,$1)

# ----------------------------------------------------------------------------
# Function:  sequence
# Arguments: 1: An integer
#            2: An integer
# Returns:   The sequence [arg1, arg2] of integers if arg1 < arg2 or
#            [arg2, arg1] if arg2 > arg1. If arg1 == arg1 return [arg1]
# ----------------------------------------------------------------------------
sequence = $(__gmsl_tr2)$(strip $(if $(call lte,$1,$2),          \
         $(call __gmsl_sequence_up,$1,$2),                       \
         $(call __gmsl_sequence_dn,$2,$1)))

__gmsl_sequence_up = $(if $(call seq,$1,$2),$1,$1 $(call __gmsl_sequence_up,$(call inc,$1),$2))
__gmsl_sequence_dn = $(if $(call seq,$1,$2),$1,$2 $(call __gmsl_sequence_dn,$1,$(call dec,$2)))

# ----------------------------------------------------------------------------
# Function:  dec2hex, dec2bin, dec2oct
# Arguments: 1: An integer
# Returns:   The decimal argument converted to hexadecimal, binary or
#            octal
# ----------------------------------------------------------------------------

__gmsl_digit = $(subst 15,f,$(subst 14,e,$(subst 13,d,$(subst 12,c,$(subst 11,b,$(subst 10,a,$1))))))

dec2hex = $(call __gmsl_dec2base,$(call int_encode,$1),$(call int_encode,16))
dec2bin = $(call __gmsl_dec2base,$(call int_encode,$1),$(call int_encode,2))
dec2oct = $(call __gmsl_dec2base,$(call int_encode,$1),$(call int_encode,8))

__gmsl_base_divide = $(subst $2,X ,$1)
__gmsl_q = $(strip $(filter X,$1))
__gmsl_r = $(words $(filter x,$1))

__gmsl_dec2base = $(eval __gmsl_temp := $(call __gmsl_base_divide,$1,$2))$(call __gmsl_dec2base_,$(call __gmsl_q,$(__gmsl_temp)),$(call __gmsl_r,$(__gmsl_temp)),$2)
__gmsl_dec2base_ = $(if $1,$(call __gmsl_dec2base,$(subst X,x,$1),$3))$(call __gmsl_digit,$2)

ifdef __gmsl_have_eval
# ###########################################################################
# ASSOCIATIVE ARRAYS
# ###########################################################################

# Magic string that is very unlikely to appear in a key or value

__gmsl_aa_magic := faf192c8efbc25c27992c5bc5add390393d583c6

# ----------------------------------------------------------------------------
# Function:  set
# Arguments: 1: Name of associative array
#            2: The key value to associate
#            3: The value associated with the key
# Returns:   Nothing
# ----------------------------------------------------------------------------
set = $(__gmsl_tr3)$(call assert_no_space,$0,$1$2)$(call assert_no_dollar,$0,$1$2$3)$(eval __gmsl_aa_$1_$(__gmsl_aa_magic)_$2_gmsl_aa_$1 := $3)

# Only used internally by memoize function

__gmsl_set = $(call set,$1,$2,$3)$3

# ----------------------------------------------------------------------------
# Function:  get
# Arguments: 1: Name of associative array
#            2: The key to retrieve
# Returns:   The value stored in the array for that key
# ----------------------------------------------------------------------------
get = $(strip $(__gmsl_tr2)$(call assert_no_space,$0,$1$2)$(call assert_no_dollar,$0,$1$2)$(__gmsl_aa_$1_$(__gmsl_aa_magic)_$2_gmsl_aa_$1))

# ----------------------------------------------------------------------------
# Function:  keys
# Arguments: 1: Name of associative array
# Returns:   Returns a list of all defined keys in the array
# ----------------------------------------------------------------------------
keys = $(__gmsl_tr1)$(call assert_no_space,$0,$1)$(call assert_no_dollar,$0,$1)$(sort $(patsubst __gmsl_aa_$1_$(__gmsl_aa_magic)_%_gmsl_aa_$1,%, \
                  $(filter __gmsl_aa_$1_$(__gmsl_aa_magic)_%_gmsl_aa_$1,$(.VARIABLES))))

# ----------------------------------------------------------------------------
# Function:  defined
# Arguments: 1: Name of associative array
#            2: The key to test
# Returns:   Returns true if the key is defined (i.e. not empty)
# ----------------------------------------------------------------------------
defined = $(__gmsl_tr2)$(call assert_no_space,$0,$1$2)$(call assert_no_dollar,$0,$1$2)$(call sne,$(call get,$1,$2),)

endif # __gmsl_have_eval

ifdef __gmsl_have_eval
# ###########################################################################
# NAMED STACKS
# ###########################################################################

# ----------------------------------------------------------------------------
# Function:  push
# Arguments: 1: Name of stack
#            2: Value to push onto the top of the stack (must not contain
#               a space)
# Returns:   None
# ----------------------------------------------------------------------------
push = $(__gmsl_tr2)$(call assert_no_space,$0,$1$2)$(call assert_no_dollar,$0,$1$2)$(eval __gmsl_stack_$1 := $2 $(if $(filter-out undefined,\
    $(origin __gmsl_stack_$1)),$(__gmsl_stack_$1)))

# ----------------------------------------------------------------------------
# Function:  pop
# Arguments: 1: Name of stack
# Returns:   Top element from the stack after removing it
# ----------------------------------------------------------------------------
pop = $(__gmsl_tr1)$(call assert_no_space,$0,$1)$(call assert_no_dollar,$0,$1)$(strip $(if $(filter-out undefined,$(origin __gmsl_stack_$1)), \
    $(call first,$(__gmsl_stack_$1))                                       \
    $(eval __gmsl_stack_$1 := $(call rest,$(__gmsl_stack_$1)))))

# ----------------------------------------------------------------------------
# Function:  peek
# Arguments: 1: Name of stack
# Returns:   Top element from the stack without removing it
# ----------------------------------------------------------------------------
peek = $(__gmsl_tr1)$(call assert_no_space,$0,$1)$(call assert_no_dollar,$0,$1)$(call first,$(__gmsl_stack_$1))

# ----------------------------------------------------------------------------
# Function:  depth
# Arguments: 1: Name of stack
# Returns:   Number of items on the stack
# ----------------------------------------------------------------------------
depth = $(__gmsl_tr1)$(call assert_no_space,$0,$1)$(call assert_no_dollar,$0,$1)$(words $(__gmsl_stack_$1))

endif # __gmsl_have_eval

ifdef __gmsl_have_eval
# ###########################################################################
# STRING CACHE
# ###########################################################################

# ----------------------------------------------------------------------------
# Function:  memoize
# Arguments: 1. Name of the function to be called if the string
#               has not been previously seen
#            2. A string
# Returns:   Returns the result of a memo function (which the user must
#            define) on the passed in string and remembers the result.
#
# Example:   Set memo = $(shell echo "$1" | md5sum) to make a cache
#            of MD5 hashes of strings. $(call memoize,memo,foo bar baz)
# ----------------------------------------------------------------------------
__gmsl_memoize = $(subst $(__gmsl_space),§,$1)cc2af1bb7c4482f2ba75e338b963d3e7$(subst $(__gmsl_space),§,$2)
memoize = $(__gmsl_tr2)$(strip $(if $(call defined,__gmsl_m,$(__gmsl_memoize)),\
                  $(call get,__gmsl_m,$(__gmsl_memoize)),                      \
                  $(call __gmsl_set,__gmsl_m,$(__gmsl_memoize),$(call $1,$2))))

endif # __gmsl_have_eval

# ###########################################################################
# DEBUGGING FACILITIES
# ###########################################################################

# ----------------------------------------------------------------------------
# Target:    gmsl-print-%
# Arguments: The % should be replaced by the name of a variable that you
#            wish to print out.
# Action:    Echos the name of the variable that matches the % and its value.
#            For example, 'make gmsl-print-SHELL' will output the value of
#            the SHELL variable
# ----------------------------------------------------------------------------
gmsl-print-%: ; @echo $* = $($*)

# ----------------------------------------------------------------------------
# Function:  assert
# Arguments: 1: A boolean that must be true or the assertion will fail
#            2: The message to print with the assertion
# Returns:   None
# ----------------------------------------------------------------------------
assert = $(if $2,$(if $1,,$(call __gmsl_error,Assertion failure: $2)))

# ----------------------------------------------------------------------------
# Function:  assert_exists
# Arguments: 1: Name of file that must exist, if it is missing an assertion
#               will be generated
# Returns:   None
# ----------------------------------------------------------------------------
assert_exists = $(if $0,$(call assert,$(wildcard $1),file '$1' missing))

# ----------------------------------------------------------------------------
# Function:  assert_no_dollar
# Arguments: 1: Name of a function being executd
#            2: Arguments to check
# Returns:   None
# ----------------------------------------------------------------------------
assert_no_dollar = $(call __gmsl_tr2)$(call assert,$(call not,$(findstring $(__gmsl_dollar),$2)),$1 called with a dollar sign in argument)

# ----------------------------------------------------------------------------
# Function:  assert_no_space
# Arguments: 1: Name of a function being executd
#            2: Arguments to check
# Returns:   None
# ----------------------------------------------------------------------------
ifeq ($(__gmsl_spaced_vars),$(false))
assert_no_space = $(call assert,$(call not,$(findstring $(__gmsl_aa_magic),$(subst $(__gmsl_space),$(__gmsl_aa_magic),$2))),$1 called with a space in argument)
else
assert_no_space =
endif
# This file handles all config  info  that  can be deduced before
# any user config files are read in.
#
# Disable implicit rules
.SUFFIXES:

.DELETE_ON_ERROR:

.SECONDEXPANSION:

SHELL = /bin/bash

uname := $(shell uname)
valid_os = Darwin Linux CYGWIN*

ifeq (Darwin,$(uname))
    OS := OSX
    CFLAGS += -DOS_OSX
    SO_EXT := dylib
    bin_platform = osx
    soname_ld_option_prefix = -Wl,-install_name,@loader_path/
    ld_no_undefined =
    bison_no_deprecated =
    lib_prefix = lib
else
ifeq (Linux,$(uname))
    OS := Linux
    CFLAGS += -DOS_LINUX
    SO_EXT := so
    bin_platform = linux64
    soname_ld_option_prefix = -Wl,-soname,
    ld_no_undefined = -Wl,--no-undefined
    bison_no_deprecated = -Wno-deprecated
    lib_prefix = lib
else
ifneq (,$(filter CYGWIN%,$(uname)))
    OS := Windows
    CFLAGS += -DOS_WIN
    SO_EXT := dll
    bin_platform = win64
    LDFLAGS += -static
    ifdef CC
        PATH := $(PATH):$(dir $(CC))
    endif
    soname_ld_option_prefix = -Wl,-soname,
    ld_no_undefined = -Wl,--no-undefined
    #bison_no_deprecated = -Wno-deprecated
    lib_prefix =
else
    $(error supported OS unames must be one of: $(valid_os))
endif
endif
endif

PRECOMP_NAME := precomp.hpp

# Use this anytime we need to distinguish two  files  or  folders
# from the build output based on OPT status.
opt-suffix :=
ifneq ($(origin OPT),undefined)
    opt-suffix := .opt
endif

# This is the name that will  be  used for all the binary folders
# both in the source tree and at the top level.
lib_name := .lib-$(bin_platform)$(opt-suffix)
bin_name := bin-$(bin_platform)$(opt-suffix)

# There is one top-level bin folder per project. At least this is
# the default name unless it is overridden by the
# project-specific files.
bin_folder := $(root)$(bin_name)
# General utilities. Make sure that gmsl is included before  this
# file

# ===============================================================
# Tuples
#
# Take two arguments and put them in the below  format  which  is
# supposed to represent a tuple  given  that make has only string
# types.
makeTuple = __$1@__$2
# Extract the elements from a tuple; note that this assumes  that
# they  have  no  spaces  in  them (should probably eventually be
# fixed).
fst = $(patsubst __%,%,$(firstword $(call split,@,$1)))
snd = $(patsubst __%,%,$(lastword  $(call split,@,$1)))
# Test if the two elements in a tuple are equal.  Note  that  the
# string  passed  to this function should never be empty, even if
# both elements are because of the above representation.
_tupleElemsEq = $(call seq,$(call fst,$1),$(call snd,$1))
# Should call this one with the check
tupleElemsEq = $(call assert,$1,Empty tuple!)$(call _tupleElemsEq,$1)

# ===============================================================
# Zipping
#
# zip  two  lists using the given function. This is just an alias
# for the gmsl function pairmap. Note that this function will not
# perform any truncation if the two lists are of different
# lengths.
zipWith = $(call pairmap,$1,$2,$3)
# zip two lists into a list of tuples
zip = $(call zipWith,makeTuple,$1,$2)
# unzip a zipped list into a  list containing only the first ele-
# ments or only the second elements, respectively.
unzipFst = $(call map,fst,$1)
unzipSnd = $(call map,snd,$1)

# ===============================================================
# Some general list functions
#
# Use recursion to drop elements of  a list that satisfy a predi-
# cate  only  until an element is found that does not satisfy it.
dropWhile =                             \
    $(if $(strip $2),                   \
        $(if                            \
            $(call $1,$(firstword $2)), \
            $(call dropWhile,           \
                $1,                     \
                $(call rest,$2)         \
            ),                          \
            $2                          \
        )                               \
    ,)

# This will take two lists  and  remove  common elements from the
# beginnings of the lists, returning a list of tuples.
stripCommonPrefix = $(call dropWhile,tupleElemsEq,$(call zip,$1,$2))

# ===============================================================
# Path functions
#
# relPath: This function takes  two  paths  as  arguments.  These
# paths can be either absolute or relative or any mix of the two.
# Relative is always with respect to the system's current working
# directory.
#
# The first can be interpreted either as file or a folder but the
# second will always be interpreted  as  a  folder  (although  it
# doesn't have to actually exist; see below).
#
# The  function  returns a relative path from the second (folder)
# to the first (file or folder).
#
# In general, this function works whether the paths exist or not.
# However,  that since we are calling abspath, this function will
# make use of real information about your file system if possible
# and so may return different results depending if the paths  you
# specify actually exist (in which case the  results  it  returns
# will tend to be more "optimal").
#
# Note that spaces in this function are important.
relPath =                                          \
    $(call merge,,                                 \
        $(patsubst %,../,                          \
            $(call unzipSnd,                       \
                $(call stripCommonPrefix,          \
                    $(call split,/,$(abspath $1)), \
                    $(call split,/,$(abspath $2))  \
                )                                  \
            )                                      \
        )                                          \
    )$(call merge,/,$(strip                        \
        $(call unzipFst,                           \
            $(call stripCommonPrefix,              \
                $(call split,/,$(abspath $1)),     \
                $(call split,/,$(abspath $2))      \
            )                                      \
        )))

# Definitions:
#   PWD: the system's current directory as seen by make
#   CWD: the folder that make is currently processing
# This function will return a relative path from  make's  PWD  to
# the given path.
relPWD = $(call relPath,$1,$(PWD))
# Get the value of CWD relative to make's PWD. 
relCWD = $(patsubst %//,%/,$(call trailingSlash,$(_relCWD)))
_relCWD = $(call relPWD,$(patsubst %/,%,$(CWD)))
# If the first is non-empty then return it with an extra slash at
# the end, else return empty.
trailingSlash = $(if $(strip $1),$(strip $1)/,)
# This will remove redundant slashes because of the way that make
# uses spaces to separate list elements. Note that it will remove
# slashes from the end as well (except for a single  slash  which
# is leaves alone).
normalizeSlashes = $(if $(call seq,/,$1),/,$(call merge,/,$(call split,/,$1)))
# If it's just a dot then  make  it  blank. In some cases we want
# the current directory to be represented by a dot but  in  other
# cases an empty string.
noDot = $(if $(call seq,$1,.),,$1)
# And the reverse:
yesDot = $(if $(call seq,$1,),.,$1)
# These are normalized relative paths of  the  CWD  of  a  target
# (which is stored as a target-specific variable) and folder con-
# taining the current target.
target_path = $(call noDot,$(call normalizeSlashes,$(dir $@)))
# Get a path relative to the project root
rel_root = $(call relPath,$1,$(root))
# Directory from which make was  invoked, relative to the project
# root. Will be empty if make is invoked from the root.
pwd_rel_root := $(call rel_root,$(PWD))
# This is a function of  zero  arguments  that, when called, will
# return the CWD (which changes as the source tree is  traversed)
# relative to the project root.
cwd_rel_root = $(call rel_root,$(call yesDot,$(relCWD)))

# ===============================================================
# String functions
#
# if strings equal
ifseq = $(if $(call seq,$1,$2),$3,$4)
# Keep if matches predicate
keep_if = $(foreach i,$2,$(if $(call $1,$(i)),$(i),))
# Does string in second arg end with string in first arg
ends_with = $(filter %$1,$2)
# Does the first parameter have any  of  the exts in second para-
# meter
has_exts = $(strip $(foreach _i,$2,$(call ends_with,$(_i),$1)))
# These functions are for filtering  lists  based  on  extensions
is_cpp_src   = $(call has_exts,$1,.c .cpp)
is_link_file = $(call has_exts,$1,.o .a .so .dylib .dll)
# For convenience
keep_cpp_srcs   = $(call keep_if,is_cpp_src,$1)
keep_link_files = $(call keep_if,is_link_file,$1)

# ===============================================================
# Miscellaneous stuff
#
# Single quotes so that bash doesn't  try to expand any left-over
# dollar signs
print-%:
	@echo '$*=$(value $*) ($($*))'
.PHONY: print-%
# This is a target that is  always  run  but  does  nothing.  Any
# target that depends on it will always be rerun.
.PHONY: always

set_default_goal = $(eval .DEFAULT_GOAL := $$(DEFAULT_GOAL_$1))

# Define "not specified" as either of the the below two cases.
not_specified = $(filter $(origin $1),undefined default)
# Function that will assign a variable but only if the variable's
# value  is "not specified" as defined above. This function is ba-
# sically  used because make's ?= operator will not do the assign-
# ment when a variable's origin is "default".
set_if_not_specified = $(eval      \
    $(if $(call not_specified,$1), \
        $1 := $2,)                 \
)
# This  make file controls echoing/logging output during the make
# run.

c_red     := \033[31m
c_green   := \033[32m
c_yellow  := \033[33m
c_blue    := \033[34m
c_magenta := \033[35m
c_cyan    := \033[36m
c_norm    := \033[00m

# There needs to be a blank line after this.
space := \

ifneq (undefined, $(origin V))
    at :=
    print_rule :=
else
    at := @
    print_rule = @echo -e '$1' &&
endif

print_compile = $(call print_rule,$(c_green)compiling$(c_norm) $<)
print_link    = $(call print_rule,  $(c_cyan)linking$(c_norm) $@)
#print_run     = $(call print_rule,  $(c_blue)running$(c_norm) $<)
print_remove  = $(call print_rule, $(c_red)removing$(c_norm) $*)
print_copy    = $(call print_rule,  $(c_magenta)copying$(c_norm) $< to $(bin_folder))
print_copy_   = $(call print_rule,  $(c_magenta)copying$(c_norm) $$< to $(bin_folder))
print_mkdir   = $(call print_rule, $(c_magenta)creating$(c_norm) $@)
print_flex    = $(call print_rule,     $(c_yellow)flex$(c_norm) $<)
print_bison   = $(call print_rule,    $(c_yellow)bison$(c_norm) $<)

# A shell command to echo a  line  potentially  with  color.  The
# space  is so that the function itself returns something so that
# make won't complain about a 'missing separator' when calling it
# inside  an $(info) command (though the shell does the printing).
output = $(space)$(shell echo -e '$1')

# This one does not go in a rule
output_using = $(call output,    $(c_magenta)using$(c_norm) $1)
# ===============================================================
# Error handling functions

assert_equal = $(call assert,$(call seq,$1,$2),$3)

undefined_error = $(error Error: make variable $1 must be defined)
assert_defined = $(if $(call seq,$(origin $1),undefined),$(call undefined_error,$1),)
# Note  that  the argument to this function must be the name of a
# variable (because it is defererenced twice). This way both  the
# name and contents of the  variable  are known to this function.
assert_nonempty = $(call assert,$(strip $($(1))),variable $1 is empty but must not be so!)

# This will assert that there are  no  duplicates  in  the  list.
assert_no_dup = $(call assert,$(call leq,$1,$(call uniq,$1)),$2)
# This make file will detect if the auto-dependency files need to
# be  processed  to  have to paths adjusted to be relative to the
# current working directory. This is  needed because the .d files
# (written by gcc) will always contain paths relative to the PWD.
# Since the user could then run make from another PWD (which will
# cause the paths in the targets to change) the existing .d files
# would become invalid. If we detect that we've changed PWD since
# the last time running make then  we  will invoke a shell script
# (which then invokes  a  perl  script)  to  do the reprocessing.
$(call assert,$(root),root not defined!)

reloc_sh      := $(makerules)/reloc.sh
location_file := $(root).location
top_wd        := $(abspath $(root))
# This is the system's current working directory  from  which  we
# invoked make.
new_wd        := $(abspath $(PWD))
# This is the function that will record the new PWD in the marker
# file.
update_location = $(shell echo $(new_wd) > $(location_file))

# Don't do anything here if the target is "clean" because in that
# case all the .d files and location file will be deleted anyway.
ifneq ($(MAKECMDGOALS),clean)
    # If the marker file  doesn't  exist  then  just update it so
    # that the `cat' command that follows won't get mad.
    ifeq ($(wildcard $(location_file)),)
        $(call update_location)
    endif
    # This is where the user previously ran make from.
    last_wd := $(shell cat $(location_file))
    # If we've changed locations then  we  should  reprocess  the
    # .d's.
    ifneq ($(last_wd),$(new_wd))
        $(shell $(reloc_sh) $(top_wd) $(last_wd) $(new_wd) 1>&2)
        # Update with our new location
        $(call update_location)
    endif
endif
# ===============================================================
# Locations
# ===============================================================
# FIXME: need separate location file for each build configuration
#
# This contains a list of all locations encountered in the entire
# build tree, even ones which may not be built in this invocation
# of  make,  depending on the target requested and/or the current
# system directory.
all_locations :=
# This contains a list of only those locations which  are  at  or
# below the current system working directory.
sub_locations :=

# Registers  a  location. Also, if the path corresponding to this
# location  is  at  or below the current system working directory
# then  this  will  also  be recorded and later used to determine
# what to build when the user runs  make  in  a  sub  folder.  If
# pwd_rel_root is empty then this means that we are at the  root,
# so all locations are subfolder. Note that the LOCATION variable
# must be set because it is used by other makefiles.
define _set_location
    LOCATION    := $1
    LOCATION_$1 := $(relCWD)
    sub_locations += $(if $(pwd_rel_root),                      \
        $(if $(filter $(pwd_rel_root)/%,$(cwd_rel_root)/),$1,), \
        $1                                                      \
    )
    all_locations += $1
endef

set_location = $(eval $(call _set_location,$1))

location_to_binary = $($1_BINARY)
# ===============================================================
# NOTE:  the  attempt  in this file is to always use eager evalua-
# tion within the "define" blocks (including inside  rules)  when
# possible, and only use deferred  evaluation (i.e., the $$) when
# it is actually necessary. Therefore, be very careful if
# changing a $$ to $, since the values of some variables are  not
# known at the time the define blocks are evaluated.

# ===============================================================
# Create lib folder
# ===============================================================
define _create_lib
    $(relCWD)$(lib_name):
	    $$(print_mkdir) mkdir $$@
endef

create_lib = $(eval $(call _create_lib))

# ===============================================================
# Compiling sources into object files
# ===============================================================
define _compile_srcs

    NEW_L_SRCS     := $(wildcard $(relCWD)*.l)
    NEW_Y_SRCS     := $(wildcard $(relCWD)*.y)

    NEW_L_SRCS_CPP := $$(NEW_L_SRCS:.l=.l.cpp)
    NEW_Y_SRCS_CPP := $$(NEW_Y_SRCS:.y=.y.cpp)
    NEW_Y_SRCS_HPP := $$(NEW_Y_SRCS:.y=.y.hpp)

    NEW_L_SRCS_OBJ := $$(call map,into_lib,$$(NEW_L_SRCS:.l=.l.o))

    NEW_C_SRCS   := $(wildcard $(relCWD)*.c)
    NEW_CPP_SRCS := $(wildcard $(relCWD)*.cpp) $$(NEW_L_SRCS_CPP) $$(NEW_Y_SRCS_CPP)

    # If there is a header file in the folder  with  the  special
    # name then assume it will be a precompiled header which will
    # be compiled to an object  file  with the extension .gch (on
    # top of existing extension).
    NEW_PRECOMP  := $(wildcard $(relCWD)$(PRECOMP_NAME))
    INC_PRECOMP  := $$(if $$(NEW_PRECOMP),-Winvalid-pch -include $$(call into_lib,$$(NEW_PRECOMP)),)
    PRECOMP_GCH  := $$(if $$(NEW_PRECOMP),$$(call into_lib,$$(NEW_PRECOMP).gch),)

    # It  is possible that the cpp sources may contain duplicates
    # if a flex/bison generated cpp file is already in the direc-
    # tory.
    NEW_CPP_SRCS := $$(call uniq,$$(NEW_CPP_SRCS))

    NEW_OBJS_C   := $$(call map,into_lib,$$(NEW_C_SRCS:.c=.o))
    NEW_OBJS_CPP := $$(call map,into_lib,$$(NEW_CPP_SRCS:.cpp=.o))

    # For deps we don't need to distinguish between c/cpp
    NEW_DEPS     := $$(call map,into_lib,$$(NEW_C_SRCS:.c=.d))   \
                    $$(call map,into_lib,$$(NEW_CPP_SRCS:.cpp=.d))
    ifneq ($$(NEW_PRECOMP),)
        NEW_DEPS := $$(NEW_DEPS) $$(call map,into_lib,$$(NEW_PRECOMP).d)
    endif

    C_SRCS      := $(C_SRCS)  $$(NEW_C_SRCS) $$(NEW_CPP_SRCS)
    YL_SRCS     := $(YL_SRCS) $$(NEW_L_SRCS_CPP) $$(NEW_Y_SRCS_CPP) $$(NEW_Y_SRCS_HPP)
    OBJS        := $(OBJS)    $$(NEW_OBJS_C) $$(NEW_OBJS_CPP)
    DEPS        := $(DEPS)    $$(NEW_DEPS)
    GCHS        := $(GCHS)    $$(PRECOMP_GCH)

    -include $$(NEW_DEPS)

    # Rule  for  running  flex on a .l file. The `sed` step is to
    # make a change that suppresses  a  warning. This is supposed
    # to  be corrected in more recent versions of flex, so should
    # probably be removed eventually.
    $$(NEW_L_SRCS_CPP): $(project_files)
    $$(NEW_L_SRCS_CPP): $(relCWD)%.l.cpp: $(relCWD)%.l
	    $$(print_flex) flex --posix -s -o $$@ -c $$<
    ifeq ($(OS),Linux)
	    $(at)sed -i.tmp 's/yy_size_t yy_buf_size/int yy_buf_size/' $$@
	    $(at)rm $$@.tmp
    endif
    # We  cannot  compile the flex-generated cpp until bison runs
    # and generates the hpp file. It seems easiest  to  to  state
    # this dependency to  make  using  the  cpp file. Technically
    # this  is  not exact, but seems fine since the bison cpp and
    # hpp files should go out of  date  or  become up to date to-
    # gether in normal usage.
    $$(NEW_L_SRCS_OBJ): $(relCWD)$(lib_name)/%.l.o: $(relCWD)%.y.cpp

    $$(NEW_Y_SRCS_CPP): $(project_files)
    $$(NEW_Y_SRCS_CPP): $(relCWD)%.y.cpp: $(relCWD)%.y
	    $$(print_bison) bison $(bison_no_deprecated) -d -o $$@ $$<

    # Here we put a static pattern rule  otherwise  when  we  run
    # make out of the folder containing this make file the relCWD
    # will  be  empty  and  so the pattern would match any object
    # file and cause this rule  to  be  used  to compile files in
    # other folders which is not correct.
    #
    # Note  that in the rule below we are adding the project file
    # as an explicit dependency so as to cause all  files  to  be
    # rebuilt if it changes (because usually changes to this file
    # would  change  a  compiler  flag  or add a dependency which
    # would not otherwise trigger rebuilding. We assume that this
    # file  ends in a .mk extension and then filter it out in the
    # rule.
    $$(NEW_OBJS_C): $(project_files)
    $$(NEW_OBJS_C): $(relCWD)$(lib_name)/%.o: $(relCWD)%.c | $(relCWD)$(lib_name)
	    $$(print_compile) $$(CC) $(TP_INCLUDES_$(LOCATION)) $(TP_INCLUDES_EXTRA) $(call include_flags,$(LOCATION)) $$($1) $(CXXFLAGS_TO_USE) -c $$< -o $$@

    # Note that we only support PCH for C++, and so we  have  the
    # dependency  on the .gch file and also a -include flag given
    # to the compiler so that all sources in this folder will in-
    # clude the pch header first.
    $$(NEW_OBJS_CPP): $(project_files) $$(PRECOMP_GCH)
    $$(NEW_OBJS_CPP): $(relCWD)$(lib_name)/%.o: $(relCWD)%.cpp | $(relCWD)$(lib_name)
	    $$(print_compile) $$(CXX) $(TP_INCLUDES_$(LOCATION)) $(TP_INCLUDES_EXTRA) $(call include_flags,$(LOCATION)) $$(INC_PRECOMP) $$($1) $(CXXFLAGS_TO_USE) -c $$< -o $$@

    # If we're doing PCH then create a target that builds it. Im-
    # portant:  this compile rule should be kept identical to the
    # one above used to compile cpp files with the exception that
    # we use the PCH compiler flags variable and don't explicitly
    # add in the flags used for including PCHs.
    ifneq ($$(NEW_PRECOMP),)
    $$(PRECOMP_GCH): $(project_files)
    $$(PRECOMP_GCH): $$(NEW_PRECOMP) | $(relCWD)$(lib_name)
	    $$(print_compile) $$(CXX) $(TP_INCLUDES_$(LOCATION)) $(TP_INCLUDES_EXTRA) $(call include_flags,$(LOCATION)) $$($1) $(CXXFLAGS_TO_USE) -c $$< -o $$@
    endif

endef

compile_srcs_exe = $(eval $(call _compile_srcs,))
compile_srcs_so  = $(eval $(call _compile_srcs,CFLAGS_LIB))

# ===============================================================
# Linking binary
# ===============================================================
# In this function we use a hack to determine if we're linking an
# SO verses exe, and  that  is  to  check  for  the presence of a
# second parameter.
define _link

    OUT_NAME := $1

    $(LOCATION)_BINARY       := $$(call into_lib,$(relCWD)$$(OUT_NAME))
    DEFAULT_GOAL_$(LOCATION) := $$($(LOCATION)_BINARY)

    NEW_L_SRCS     := $(wildcard $(relCWD)*.l)
    NEW_Y_SRCS     := $(wildcard $(relCWD)*.y)
    NEW_L_SRCS_CPP := $$(NEW_L_SRCS:.l=.l.cpp)
    NEW_Y_SRCS_CPP := $$(NEW_Y_SRCS:.y=.y.cpp)

    NEW_C_SRCS  := $(wildcard $(relCWD)*.c $(relCWD)*.cpp) $$(NEW_L_SRCS_CPP) $$(NEW_Y_SRCS_CPP)
    NEW_OBJS    := $$(NEW_C_SRCS:.cpp=.o)
    NEW_OBJS    := $$(NEW_OBJS:.c=.o)
    NEW_OBJS    := $$(call map,into_lib,$$(NEW_OBJS))

    BINARIES    := $(BINARIES)    $$($(LOCATION)_BINARY)
    EXECUTABLES := $(EXECUTABLES) $$(if $2,,$$($(LOCATION)_BINARY))

    SONAME := $(soname_ld_option_prefix)$$(OUT_NAME)
    # Clear this string if we're not building an SO
    SONAME_$(LOCATION) := $$(if $2,$$(SONAME),)

    OUT_PATH := $$(call into_lib,$(relCWD)$$(OUT_NAME))

    # Note that in the rule below we are adding the project
    # file as an explicit dependency so as to cause all files
    # to be rebuilt if it changes (because usually changes to
    # this file would change a compiler flag or add a dependency
    # which would not otherwise trigger rebuilding.  We assume
    # that this file ends in a .mk extension and then filter
    # it out in the rule.
    $$(OUT_PATH): $(project_files)
    $$(OUT_PATH): $$(NEW_OBJS) $(call link_binaries,$(LOCATION)) | $(relCWD)$(lib_name)
	    $$(print_link) $$(LD) $$(LDFLAGS) $$($2) $$(SONAME_$(LOCATION)) $(ld_no_undefined) -Wl,-rpath,'$$$$ORIGIN' $$(call keep_link_files,$$^) $(TP_LINK_EXTRA) $(TP_LINK_$(LOCATION)) -o $$@

endef

link_exe = $(eval $(call _link,$1,))
link_so  = $(eval $(call _link,$(lib_prefix)$1.$(SO_EXT),LDFLAGS_LIB))

# ===============================================================
# Highlevel functions
# ===============================================================
define _make_so
    $$(call set_location,$1)
    $$(call create_lib)
    $$(call compile_srcs_so)
    $$(call link_so,$2)
endef

make_so = $(eval $(call _make_so,$1,$2))

define _make_exe
    $$(call set_location,$1)
    $$(call create_lib)
    $$(call compile_srcs_exe)
    $$(call link_exe,$2)
endef

make_exe = $(eval $(call _make_exe,$1,$2))
# ===============================================================
# General dependencies processing
first_level_deps = $($(1).deps)
expand_deps_1    = $(call map,first_level_deps,$1)
# This will recursively expand a single location into a full list
# of all dependencies, including  itself  (there may be redunden-
# cies).
expand_deps_full = $1 $(call map,expand_deps_full,$(call expand_deps_1,$1))
# Now just remove redundencies.
all_deps         = $(call uniq,$(call expand_deps_full,$1))
all_deps_noself  = $(filter-out $1,$(call uniq,$(call expand_deps_full,$1)))

# ===============================================================
# Include dependencies
# ===============================================================
# Take a location, look up the associated folder,  and  make  the
# compiler option.
include_flag  = -I$(if $(LOCATION_$1),$(LOCATION_$1),.)
# Might want to try memoizing the results of this call since cur-
# rently it has to be recomputed for  every  file  in  a  library
# (gmsl has a memoizer).
include_flags = $(call map,include_flag,$(call all_deps,$1))

# ===============================================================
# Linking dependencies
# ===============================================================
# List only direct dependencies (not including self) and retrieve
# binary
link_binaries = $(foreach i,$(call all_deps_noself,$1),$($(i)_BINARY))
# ===============================================================
# This is the `enter' function used  to traverse folders. It sets
# the variable CWD with the name of the current folder  (relative
# to  the  system's current working directory) upon entering each
# new folder. This function is also used to enter into the
# makerules folder itself.
define enterimpl
    CWD_SP := $$(CWD_SP)_x
    $$(CWD_SP) := $$(CWD)
    CWD := $$(CWD)$1/
    #$$(info Entering $$(CWD))
    include $$(CWD)/Makefile
    #$$(info Leaving $$(CWD))
    CWD := $$($$(CWD_SP))
    CWD_SP := $$(patsubst %_x,%,$$(CWD_SP))
endef

enter = $(eval $(call enterimpl,$1))
enter_all = $(call map,enter,$1)

# ===============================================================
# Load  all  the  modules in nr-make that can be loaded before we
# load any project-specific config files  and traverse the source
# tree. Note that these need to be in a certain order

# ===============================================================
# Standard top-level targets
build: $$(BINARIES)
# If  run  as  a target this will build and copy all binaries. We
# use  second expansion here because we don't know what the BINA-
# RIES are at this point since the src tree hasn't been traversed
# yet.
copy-bin: $$(call map,to_bin_folder,$$(BINARIES))
# Does everything.
all: copy-bin

.DEFAULT_GOAL = all

.PHONY: all build copy-bin

clean_targets = $(OBJS) $(BINARIES) $(DEPS) $(YL_SRCS) $(GCHS) \
                $(call map,to_bin_folder,$(BINARIES))

# Use secondary expansion for the dependencies  here  because  we
# won't yet know the contents of  clean_targets  at  this  point.
# Also, use wildcard so that we  only  run  remove  commands  for
# those which exist.
clean: $$(addsuffix .clean,$$(wildcard $$(clean_targets)))
	$(at)-rm -f $(location_file)

# Do one target per removed file for  ease  of  printing  output.
%.clean:
	$(print_remove) rm -f $*

.PHONY: clean

# Given A/B/C.cpp this will return A/B/X/C.cpp, where  X  is  the
# lib folder name, specific to the platform.
into_lib = $(dir $1)$(lib_name)/$(notdir $1)
# ===============================================================
# Default project settings
# ===============================================================
CFLAGS         += -MMD -MP -m64 -Wall -Wextra -pedantic
CXXFLAGS       += $(CFLAGS)

CFLAGS_DEBUG   += $(CXXFLAGS) -g -ggdb
CFLAGS_RELEASE += $(CXXFLAGS) -Ofast

# Here  "not  specified" is defined as the `origin` of a variable
# being either "undefined" or  "default".  The  ?= operator would
# catch  the  former,  but  not  the latter. We need to catch the
# latter to prevent make from setting the variable to  a  default
# value  if it is undefined (in which case ?= won't then override
# it). In other words, the  only  values  we  want the below vari-
# ables to take are either  1)  a  value specified in the environ-
# ment or command line,  2)  a  value  that  we specify below. We
# don't want the default values that make gives them.
$(call set_if_not_specified,CC,gcc)
$(call set_if_not_specified,CXX,g++)
$(call set_if_not_specified,LD,g++)

ifneq ($(OS),Windows)
    CFLAGS_LIB += -fPIC
endif

ifneq ($(origin OPT),undefined)
    CXXFLAGS_TO_USE = $(CFLAGS_RELEASE)
else
    CXXFLAGS_TO_USE = $(CFLAGS_DEBUG)
endif

LDFLAGS     ?=
LDFLAGS_LIB := -shared

INSTALL_PREFIX := $(HOME)/tmp
# ===============================================================
# This will load most of the machinery in the make system
# ===============================================================

# ===============================================================
# This will load the default settings for  a  project  which  the
# user can override in their own make files if they want.
# ===============================================================

# ===============================================================
# Now invoke any user-defined project files -- this means any .mk
# files in the top-level folder of the project. If there are none
# then just assume a basic minimal project.
# ===============================================================
OLD_CWD := $(CWD)
CWD     := $(root)
project_files := $(wildcard $(root)*.mk $(root).*.mk)
-include $(project_files)
# If  this  project  contains multiple projects within subfolders
# then the above included project files  will  have  already  tra-
# versed the entire source tree  and  added  in at least one loca-
# tion. However, if either there are no project files or if those
# project files did not  descend  into  subolders  (i.e., if this
# project contains only one top-level folder with  source  files)
# then the list of locations  will  be  empty and no targets will
# have been created. In that case, for convenience, let's add  in
# a default target  for  convenience  that  simply  builds an exe-
# cutable  called  a.out  consisting of any source files found in
# the top-level folder, and we'll  also  copy  the binary file to
# this folder as well, but with a debug/opt suffix to distinguish
# those two.
ifeq ($(all_locations),)
    # Basic  minimal project with environment defaults. Creates a
    # single executable out of all the sources in  the  top-level
    # folder and that's it.
    $(call make_exe,MAIN,a.out$(opt-suffix))
    # Also in this case just dump the binary into the top  folder
    # instead of creating a binary folder for it.
    no_top_bin_folder = 1
    # Specify  that this is the location with the main executable
    # binary.
    main_is := MAIN
endif
CWD     := $(OLD_CWD)

# ===============================================================
# These are things that can only  be  done  after  the  project's
# source tree has been scanned.
# ===============================================================
# This file contains config info that can only be  run  after  we
# have loaded any project-specific make files and/or traverse the
# source tree. It is the companion to pre-config.mk.

ifneq ($(origin STATIC_LIBSTDCXX),undefined)
    LDFLAGS += -static-libstdc++
endif

ifneq ($(origin STATIC_LIBGCC),undefined)
    LDFLAGS += -static-libgcc
endif
# This  file  will  do  anything  can  can only be done after the
# source tree has been traversed.

# Load the remainder of  the  nr-make  modules  which can only be
# loaded after the source tree has been traversed.

# ===============================================================
# Make sure all variables are defined that need to be  (but  note
# that they could be empty).

must_be_defined = CFLAGS CXXFLAGS LDFLAGS
$(call map,assert_defined,$(must_be_defined))

# Make sure all variables are non-empty that need to be
must_be_nonempty = CC CXX LD bin_folder            \
                   bin_name lib_name sub_locations
$(call map,assert_nonempty,$(must_be_nonempty))

# Here we get a list of all the file names of all binaries (which
# does not include object files) and assert that there are no du-
# plicates. There should not be  duplicates because we might want
# to copy them all into the same top-level folder.
bin_dup = duplicate file name in list of binaries!
$(call assert_no_dup,$(call map,notdir,$(BINARIES)),$(bin_dup))

# This file will dump some info on what tools are being used or
# the contents of certain variables if the user requested.

# ===============================================================
# Things having to do with bin/lib folders

ifneq (undefined,$(origin no_top_bin_folder))
    bin_folder = .
endif

$(bin_folder):
	$(print_mkdir) mkdir $(bin_folder)

to_bin_folder = $(bin_folder)/$(notdir $1)

define __bin_copy_rule
    $(call to_bin_folder,$1): $1 | $(bin_folder)
	    $(print_copy_) cp -f $1 $$@
endef
# This function will create a rule to  copy  one  binary  to  the
# top-level binary folder. We won't actually call  this  function
# until after we have traversed the source tree.
bin_copy_rule = $(eval $(call __bin_copy_rule,$1))

# Create  a  bin_copy rule for each binary. These rules will copy
# binary  outputs  (which  do  not include object files) into the
# top-level binary folder.
$(call map,bin_copy_rule,$(BINARIES))

# When the user runs make  in  a subfolder without specifying any
# targets  then  this  default  target  will cause all targets to
# build whose locations are  subfolders  of  the  current  system
# folder. We take all of the binaries under the PWD and make them
# targets. This will have the effect of building not  only  them,
# but also their dependencies even if they are not under the PWD.
subfolders: $(call map,location_to_binary,$(sub_locations))
.PHONY: subfolders

# If  we  run make at the root of the project then just build the
# `all`  target which includes copying; otherwise, just build the
# binaries in the subfolders.
ifeq ($(pwd_rel_root),)
    .DEFAULT_GOAL := all
else
    .DEFAULT_GOAL := subfolders
endif

# If  the  project has defined which location holds the main exe-
# cutable binary then we  will  create  a  target called `run` so
# that  the  user can easily run the program from anywhere in the
# source tree by typing  `make  run`.  It  will first ensure that
# everything is fully build, then  will change into the top-level
# binary folder to run the binary in case we are  on  a  platform
# where it would otherwise not be able to find the  other  shared
# libraries that are also in the bin folder.
ifdef main_is
    # This is the file name of the  executable.  Assume  that  it
    # will be built as part of the `all` target and that it  will
    # be copied into the bin folder, wherever that is.
    bin_name := $(notdir $(call location_to_binary,$(main_is)))
    run: all
	    $(at)cd $(bin_folder) && ./$(bin_name)
    .PHONY: run
endif
# This file will display some info about the various settings
# if the user sets the SHOW_CONFIG variable.
ifneq ($(origin SHOW_CONFIG),undefined)

gxx-version := $(shell $(CXX) -dumpfullversion)
$(info $(call output_using,g++ $(gxx-version)))

endif
