# 3.3 Configuration: Dimension of Simulated Space and Library Level

Source: `doc/original/ohhelp.pdf`, pages 25-25.

<!-- Page 25 -->

3.3  Configuration:  Dimension of Simulated Space and Library
Level

The OhHelp library can be applied to PIC simulations of one-dimensional, two-dimensional
or three-dimensional space domain.  For the sake of efficiency, however, the number
of dimensions D  is hard-coded in the library source code using a C constant macro
named OH DIMENSION whose default value is three.  Therefore,  if your simulator is one-
or two-dimensional, you have to explicitly define the macro through the compiler option
-DOH DIMENSION=1 or -DOH DIMENSION=2, or have to edit the header file oh config.h in
which the default definition of OH DIMENSION is given as follows.

#ifndef OH_DIMENSION
#define OH_DIMENSION    3
#endif


Remember that oh config.h is included by ohhelp f.h and ohhelp c.h for function aliasing
and thus modifying oh config.h is easier to have consistent definition if you use aliases. Also
remember that oh config.h has the following lines which you may modify (remove comment)
to #define a macro named OH_LIB_LEVEL_4P or OH_LIB_LEVEL_4S for the activation of
level-4p or level-4s extension, which we will discuss in §3.7 and §3.8 respectively.  Note
that the lines following the commented-out definitions #defines another macro OH_LIB_
LEVEL_4PS if you #define either OH_LIB_LEVEL_4P or OH_LIB_LEVEL_4S by removing the
comment surrounding the definition.

/* If you want to activate level-4p functions, remove this comment surrounding
the line below.
#define OH_LIB_LEVEL_4P
*/
/* If you want to activate level-4s functions, remove this comment surrounding
the line below.
#define OH_LIB_LEVEL_4S
*/
#ifdef  OH_LIB_LEVEL_4P
#define OH_LIB_LEVEL_4PS
#endif
#ifdef  OH_LIB_LEVEL_4S
#define OH_LIB_LEVEL_4PS
#endif
The final contents of oh config.h is the definition of OH LIB LEVEL to control the level-
dependent function/subroutine name aliases which we will discuss in §3.12. You may edit
the following line to define it so that it has 1, 2 or 3 representing the layer you choose
unless you use the level-4p/4s extension. Otherwise, i.e., with level-4p/4s extension, OH_
LIB_LEVEL is set to 4 and you have options to #define the following two macros which we
will discuss in §3.7.

- OH BIG SPACE for your simulation with a significantly large space domain.

- OH NO CHECK for your well-debugged simulation code which does not need the argu-
ment consistency check in some API functions.

.

#ifdef  OH_LIB_LEVEL_4PS
#define OH_LIB_LEVEL 4
/* If you want to use level-4p/4s functions with a large simulation space,
