# 4.14 Sample make Files

Source: `doc/original/ohhelp.pdf`, pages 554-556.

<!-- Page 554 -->

## 4.14 Sample make Files

As discussed in §3.14, OhHelp distribution just has sample make files for Fortran and C
simulators namely samplef.mk and samplec.mk shown in the following subsections.


#### 4.14.1 samplef.mk for Fortran

The sample make file for Fortran coded simulators, samplef.mk, at first declares that the
Fortran compiler FC is used for linking, and then declares the following sets of files.

- COMMONHDRS = {oh config.h, oh stats.h}
C header files commonly #included into library and simulator header/source files.

- OHHDRS = {ohhelp1.h, ohhelp2.h, ohhelp3.h, ohhelp4p.h, oh part.h}
C header files #included into library header/source files.

- OHBOJS = {ohhelp1.o, ohhelp2.o, ohhelp3.o, ohhelp3.o}
C object files compiled from library sources, ohhelp1.c, ohhelp2.c, ohhelp3.c and
ohhelp4p.c.

- FHDRS = {ohhelp f.h}
Fortran header file #included into Fortran simulator source files.

- FMODS = {oh type.o, oh mod1.o, oh mod2.o, oh mod3.o, oh mod4p.o}
Fortran object files compliled from Fortran module files used in Fortran module/
source files, oh type.F90, oh mod1.F90, oh mod2.F90, oh mod3.F90 and oh mod4p.F90.

- FOBJS = {simulator.o, sample.o}
Fortran object files compiled from Fortran simulator source files, simulator.F90 and
sample.F90.

- OBJS = FOBJS ∪FMODS ∪OHOBJS
All object files to be linked.


LINKER          = $(FC)

COMMONHDRS      = oh_config.h oh_stats.h
OHHDRS          = ohhelp1.h ohhelp2.h ohhelp3.h ohhelp4p.h oh_part.h
OHOBJS          = ohhelp1.o ohhelp2.o ohhelp3.o ohhelp4p.o

FHDRS           = ohhelp_f.h
FMODS           = oh_type.o oh_mod1.o oh_mod2.o oh_mod3.o oh_mod4p.o

FOBJS           = simulator.o sample.o

OBJS            = $(FOBJS) $(FMODS) $(OHOBJS)


Then the make file defines the dependency shown in the Table 2 in §3.14, and the
pseudo-dependency for cleaning the working directory by removing object files and .mod
files.

simulator:      $(OBJS)
$(LINKER) $(FFLAGS) $(LDFLAGS) $(OBJS) -o $@


<!-- Page 555 -->

$(FOBJS):%.o:   %.F90 $(FMODS) $(FHDRS) $(COMMONHDRS)
$(FC) $(FFLAGS) -c $< -o $@
simulator.o:    sample.o
$(FMODS):%.o:   %.F90 $(COMMONHDRS)
$(FC) $(FFLAGS) -c $< -o $@
oh_mod1.o:      oh_type.o
oh_mod2.o:      oh_mod1.o
oh_mod3.o:      oh_mod2.o
oh_mod4p.o:     oh_mod3.o
$(OHOBJS):%.o:  %.c $(COMMONHDRS) $(OHHDRS)
$(CC) $(CFLAGS) -c $< -o $@

clean:;
rm *.o *.mod


#### 4.14.2 samplec.mk for C

The sample make file for C coded simulators, samplec.mk, at first declares that the C
compiler CC is used for linking, and then declares the following sets of files.

- COMMONHDRS = {oh config.h, oh part.h, oh stats.h}
C header files commonly #included into library and simulator header/source files.

- OHHDRS = {ohhelp1.h, ohhelp2.h, ohhelp3.h, ohhelp4p.h}
C header files #included into library header/source files.

- OHBOJS = {ohhelp1.o, ohhelp2.o, ohhelp3.o, ohhelp4p.o}
C object files compiled from library sources, ohhelp1.c, ohhelp2.c, ohhelp3.c and
ohhelp4p.c.

- CHDRS = {ohhelp c.h}
C header file #included into C simulator source files.

- COBJS = {simulator.o, sample.o}
C object files compiled from C simulator source files, simulator.c and sample.c.

- OBJS = COBJS ∪OHOBJS
All object files to be linked.


LINKER          = $(CC)

COMMONHDRS      = oh_config.h oh_part.h oh_stats.h
OHHDRS          = ohhelp1.h ohhelp2.h ohhelp3.h ohhelp4p.h
OHOBJS          = ohhelp1.o ohhelp2.o ohhelp3.o ohhelp4p.o

CHDRS           = ohhelp_c.h
COBJS           = simulator.o sample.o

OBJS            = $(COBJS) $(OHOBJS)


Then the make file defines the dependency shown in the Table 3 in §3.14, and the
pseudo-dependency for cleaning the working directory by removing object files.


<!-- Page 556 -->

simulator:      $(OBJS)
$(LINKER) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

$(COBJS):%.o:   %.c $(COMMONHDRS) $(CHDRS)
$(CC) $(CFLAGS) -c $< -o $@
$(OHOBJS):%.o:  %.c $(COMMONHDRS) $(OHHDRS)
$(CC) $(CFLAGS) -c $< -o $@

clean:;
rm *.o
