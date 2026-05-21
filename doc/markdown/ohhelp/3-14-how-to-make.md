# 3.14 How to make

Source: `doc/original/ohhelp.pdf`, pages 128-129.

<!-- Page 128 -->

eb[x+y*we+z*wde].ey += (1/EPSILON)*((1/MU)*rot[1] + cd[x+y*wc+z*wdc].jy);
eb[x+y*we+z*wde].ez += (1/EPSILON)*((1/MU)*rot[2] + cd[x+y*wc+z*wdc].jz);
}
}


Function field_solve_b()
The seventh and last function field solve b() is given three arguments to specify the
primary or secondary subdomain and its field-array; eb for the electromagnetic field-array;
sdom for the size and the location of the subdomain; and fsize for the size of eb.
In the local variable declaration, we calculate the upper boundaries σx,y,z of the sub-
domain in its local coordinates referring to sdom and set them into xu and so on. We also
calculate the width and width times depth of eb to set them into w and wd.

void field_solve_b(struct ebfield *eb, int sdom[OH_DIMENSION][2],
int fsize[OH_DIMENSION][2]) {
int xu=sdom[0][1]-sdom[0][0], yu=sdom[1][1]-sdom[1][0],
zu=sdom[2][1]-sdom[2][0];
int w=fsize[0][1]-fsize[0][0], wd=w*(fsize[1][1]-fsize[1][0]);
int x, y, z;
double rot[OH_DIMENSION];


Then, in the loop for [0, σx−1] × [0, σy−1] × [0, σz−1], we update each magnetic field
vector following the Maxwell’s (or Faraday’s induction) law using ∇× E calculated by the
out-of-scope function rotate_e() and set into rot[3].

for (z=0; z<xu; z++)  for (y=0; y<xu; y++)  for (x=0; x<xu; x++) {
rotate_e(eb, x, y, z, fsize, rot);
eb[x+y*w+z*wd].bx += rot[0];
eb[x+y*w+z*wd].by += rot[1];
eb[x+y*w+z*wd].bz += rot[2];
}
}


## 3.14 How to make

Since the OhHelp library includes header files which may be (or is expected to be) cus-
tomized to your own simulator, it should be confusing if we provide a Makefile to build
a library archive which could be mistakingly assumed independent of your customization.
Therefore, the distribution of OhHelp merely has samples of Makefile namely samplef.mk
and samplec.mk to make your simulator in Fortran and C together with the librarary coded
in C.
The sample Makefile for Fortran samplef.mk represents the dependency shown in Ta-
ble 2, while its C counterpart samplec.mk corresponds to that shown in Table 3, providing
that you choose level-L library27.  In the sample files,  it is assumed that your simula-
tor has just two sources, sample.F90 and simulator.F90 or sample.c and simulator.c, and
simulator.{F90,c} provides main routines and out-of-scope subroutines/functions used in
sample.{F90,c}.  It is also assumed your source files need neither of your own header files
nor module files to be #include’d or use’d, although usually you should have some of them.

27The tables show dependencies accurately and strictly, but sample Makefile’s have redundant (but safe)
dependencies such as that ohhelp1.c depends on ohhelp3.h.


<!-- Page 129 -->

Table 2: File Dependency of Fortran Codes.

file          depends on
simulator      simulator.o  sample.o  oh modl.o  ohhelpl.o (l ∈[1, L])
simulator.o    simulator.F90  sample.o∗1  oh modL.o∗1  ohhelp f.h∗2  oh config.h∗3
oh stats.h∗4
sample.o     sample.F90  oh modL.o∗1  ohhelp f.h∗2  oh conifg.h∗3  oh stats.h∗4
oh mod4p.o  oh mod4p.F90  oh mod3.o∗1  oh config.h
oh mod4s.o   oh mod4s.F90  oh mod3.o∗1  oh config.h
oh mod3.o   oh mod3.F90  oh mod2.o∗1  oh config.h
oh mod2.o   oh mod2.F90  oh mod1.o∗1  oh config.h
oh mod1.o   oh mod1.F90  oh type.o∗1  oh config.h
oh type.o    oh type.F90
ohhelp4p.o    ohhelp4p.c  ohhelp4p.h  ohhelp3.h  ohhelp2.h  ohhelp1.h  oh config.h
oh stats.h  oh part.h
ohhelp4s.o    ohhelp4s.c  ohhelp4s.h  ohhelp3.h  ohhelp2.h  ohhelp1.h  oh config.h
oh stats.h  oh part.h
ohhelp3.o     ohhelp3.c  ohhelp3.h  ohhelp2.h  ohhelp1.h  oh config.h  oh stats.h
oh part.h
ohhelp2.o     ohhelp2.c  ohhelp2.h  ohhelp1.h  oh config.h  oh stats.h  oh part.h
ohhelp1.o     ohhelp1.c  ohhelp1.h  oh config.h  oh stats.h

∗1 Dependence to *.o files represents that a file providing a module must be compiled prior to files
which use it if it is modified.
∗2 If you use function aliasing.
∗3 If you refer to OH_DIMENSION.
∗4 If you use statistics functions.




Table 3: File Dependency of C Codes.

file         depends on
simulator     simulator.o  sample.o  ohhelpl.o (l ∈[1, L])
simulator.o   simulator.c  ohhelp c.h∗1  oh part.h∗2  oh config.h∗3  oh stats.h∗4
sample.o     sample.c  ohhelp c.h∗1  oh part.h∗2  oh config.h∗3  oh stats.h∗4
ohhelp4p.o   ohhelp4p.c  ohhelp4p.h  ohhelp3.h  ohhelp2.h  ohhelp1.h  oh config.h
oh stats.h  oh part.h
ohhelp4s.o   ohhelp4s.c  ohhelp4s.h  ohhelp3.h  ohhelp2.h  ohhelp1.h  oh config.h
oh stats.h  oh part.h
ohhelp3.o    ohhelp3.c  ohhelp3.h  ohhelp2.h  ohhelp1.h  oh config.h  oh stats.h
oh part.h
ohhelp2.o    ohhelp2.c  ohhelp2.h  ohhelp1.h  oh config.h  oh stats.h  oh part.h
ohhelp1.o    ohhelp1.c  ohhelp1.h  oh config.h  oh stats.h

∗1 If you use function aliasing.
∗2 If L ≥2.
∗3 If you refer to OH_DIMENSION.
∗4 If you use statistics functions.
