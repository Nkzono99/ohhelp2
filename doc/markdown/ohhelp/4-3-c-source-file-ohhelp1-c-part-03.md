# 4.3 C Source File ohhelp1.c - Part 3

Source: `doc/original/ohhelp.pdf`, pages 221-225.

<!-- Page 221 -->

scp[STATS_PART_PG_PRI_AVE] = Round(scp[STATS_PART_PG_PRI_AVE], nn);
}
for (k=0; k<STATS_PART_MOVE_SEC_MIN; k++) {
dint n = scp[k];
if (n<spps[k].min) spps[k].min = n;
if (n>spps[k].max) spps[k].max = n;
spps[k].total += n;
}
}
if      (trans==1) sp[transkey].min++;
else if (trans==2) sp[transkey].max++;
else if (trans==3) sp[transkey].total++;
}


#### 4.3.42 Macro Stats_Reduce_Part_{Min, Max, Sum}()

Stats_Reduce_Part_Min()  The macros Stats Reduce Part x (x ∈{Min, Max, Sum}), used only in stats_reduce_
Stats_Reduce_Part_Max()  part(), peforms pairwise operations for reductions to obtain minimum, maximum and
Stats_Reduce_Part_Sum()  sum over the in/out array io[k] and input array in[k] to update io[k], where k = KEY being
the macro argument.


#define Stats_Reduce_Part_Min(KEY) { if (io[KEY]<in[KEY]) io[KEY] = in[KEY]; }
#define Stats_Reduce_Part_Max(KEY) { if (io[KEY]>in[KEY]) io[KEY] = in[KEY]; }
#define Stats_Reduce_Part_Sum(KEY) { io[KEY] += in[KEY]; }



#### 4.3.43 stats_reduce_part()

stats_reduce_part()  The  function stats reduce part(),  called from MPI_Reduce()  in update_stats()
through Op_StatsPart,  performs pairwise reduction  of the  local  statistics  of  parti-
cle transfer stored in Stats.curr.part[] and given through  its argument inarg and
ioarg to have the minimum of the elemnts at STATS PART x z MIN, the maximum of
those at STATS PART x z MAX, and the sum of those at STATS PART y z AVE, where x ∈
{MOVE, GET, PUT}, y ∈{MOVE, PG} and z ∈{PRI, SEC}. The pairwise reduction is performed
by the macros Stats Reduce Part {Min, Max, Sum}().


static void
stats_reduce_part(void* inarg, void* ioarg, int* len, MPI_Datatype* type) {
dint *in=(dint*)inarg, *io=(dint*)ioarg;
int ps, statsbase=0;

for (ps=0; ps<2; ps++,statsbase+=STATS_PART_MOVE_SEC_MIN) {
Stats_Reduce_Part_Min(statsbase+STATS_PART_MOVE_PRI_MIN);
Stats_Reduce_Part_Max(statsbase+STATS_PART_MOVE_PRI_MAX);
Stats_Reduce_Part_Sum(statsbase+STATS_PART_MOVE_PRI_AVE);
Stats_Reduce_Part_Min(statsbase+STATS_PART_GET_PRI_MIN);
Stats_Reduce_Part_Max(statsbase+STATS_PART_GET_PRI_MAX);
Stats_Reduce_Part_Min(statsbase+STATS_PART_PUT_PRI_MIN);
Stats_Reduce_Part_Max(statsbase+STATS_PART_PUT_PRI_MAX);
Stats_Reduce_Part_Sum(statsbase+STATS_PART_PG_PRI_AVE);
}
}


<!-- Page 222 -->

#### 4.3.44 print_stats()

print_stats()  The function print stats(), called from oh1_show_stats() and oh1_print_stats[_](),
at  first reduce timing  statistics  in the argument stotal which  is Stats.total or
Stats.subtotal for the nstep iteration steps to have the global minimum, maximum and
total by MPI_Reduce() of T_StatsTime data, which calls stats_reduce_time() through
Op_StatsTime.
Then,  if the local node rank is 0, it prints the statistics in stotal, with the current
step number is given by the argument step  if stotal = Stats.subtotal. The statis-
tics data stored in the element array time[] or part[] is judged valid if the leaf elements
min and max satisfies min ≤max. To print the meaning of each statistics data, we refer
to StatsTimeStrings[k] for time[k] and StatsPartStrings[k] for part[k]. The average
number of the leaf element part[].total is calculated by Round().


static void
print_stats(struct S_statstotal *stotal, int step, int nstep) {
int i;
struct S_statstime *st = stotal->time;
struct S_statspart *sp = stotal->part;

if (myRank!=0) {
MPI_Reduce(st, NULL, STATS_TIMINGS<<1, T_StatsTime, Op_StatsTime, 0, MCW);
return;
}
MPI_Reduce(MPI_IN_PLACE, st, STATS_TIMINGS<<1, T_StatsTime, Op_StatsTime,
0, MCW);
printf("\n");
if (stotal==&Stats.subtotal)
printf("# Subtotal Statistics for %d..%d\n",  step-nstep+1, step);
else
printf("# Total Statistics\n");
printf("## Execution Times (sec)\n");
for (i=0; i<STATS_TIMINGS<<1; i++) {
if (st[i].ev==0)
printf("  %-29s = -------- / -------- / -------- / ------------\n",
StatsTimeStrings[i]);
else
printf("  %-29s = %8.3f / %8.3f / %8.3f / %12.3f\n",
StatsTimeStrings[i],
st[i].min, st[i].max, st[i].total/st[i].ev, st[i].total);
}
printf("## Particle Movements\n");
for (i=0; i<STATS_PARTS; i++) {
if (i<STATS_PART_PRIMARY && sp[i].min>sp[i].max)
printf("  %-29s = -------- / -------- / -------- / ------------\n",
StatsPartStrings[i]);
else if (i<STATS_PART_PRIMARY)
printf("  %-29s = %8lld / %8lld / %8lld / %12lld\n",
StatsPartStrings[i],
sp[i].min, sp[i].max, Round(sp[i].total,nstep), sp[i].total);
else
printf("  %-29s = %8lld / %8lld / %8lld / %12lld\n",
StatsPartStrings[i],


<!-- Page 223 -->

sp[i].min, sp[i].max, sp[i].total,
sp[i].min+sp[i].max+sp[i].total);
}
}


#### 4.3.45 stats_reduce_time()

stats_reduce_time()  The function stats reduce time(), called from MPI_Reduce() in print_stats() through
Op_StatsTime,  performs  pairwise  reduction  of the  local timing  statistics  stored  in
Stats.total[] or Stats.subtotal[] and given through the arguments inarg and ioarg.
The reduction is performed on the leaf elements to obtain the minimum of min, the maxi-
mum of max and the total of total, if the element ev > 0.


static void
stats_reduce_time(void* inarg, void* ioarg, int* len, MPI_Datatype* type) {
struct S_statstime *in=(struct S_statstime*)inarg;
struct S_statstime *io=(struct S_statstime*)ioarg;
int n=*len, i;

for (i=0; i<n; i++) {
if (in[i].ev>0) {
io[i].ev += in[i].ev;
if (in[i].min<io[i].min) io[i].min = in[i].min;
if (in[i].max>io[i].max) io[i].max = in[i].max;
io[i].total += in[i].total;
}
}
}


#### 4.3.46 oh1_print_stats()

oh1_print_stats_()  The API functions oh1 print stats () for Fortran and oh1 print stats() for C sim-
oh1_print_stats()  ply call print_stats() giving it Stats.total and the argment nstep to print statistics
in Stats.total over the whole simulation steps nstep,  if statsMode ̸= 0. The second
argument step of print stats() can be anything and thus is set to 0.


void
oh1_print_stats_(int *nstep) {
if (statsMode)  print_stats(&Stats.total, 0, *nstep);
}
void
oh1_print_stats(int nstep) {
if (statsMode)  print_stats(&Stats.total, 0, nstep);
}


#### 4.3.47 oh1_verbose()

oh1_verbose_()  The API functions oh1 verbose () for Fortran and oh1 verbose() for C simply invoke
oh1_verbose()  macro Verbose() giving it vprint() with the argument message as its second argument
for verbose messaging. The first argument of Verbose() is set to 1 to indicate fundamental
messaging.


<!-- Page 224 -->

void
oh1_verbose_(char *message) {
Verbose(1, vprint(message));
}
void
oh1_verbose(char *message) {
Verbose(1, vprint(message));
}


#### 4.3.48 Macros Vprint() and Vprint_Norank()

Vprint()  The macro Vprint(), used in vprint() and dprint(), constructs a message header in-
Vprint_Norank()  cluding the local node’s rank myRank into a buffer calling sprintf() with the argmument
RANKFORMAT, and then concatenates it with the argument FORMAT and an end-of-line char-
acter by strcat() to pass it to vprintf() as the format argument. The remaining variable
number arguments of the caller function, which is handled in this macro with va_start()
and va_end(), are also passed to vprintf(). The concatenation of the format argument of
vprintf() aims at minimizing the possibility of interleaving of the messages from multiple
nodes. The macro also calls fflush() to flush the message.
The relative macro VPrint Norank(), used solely in vprint(), works almost in same
manner as Vprint() but myRank is not passed to sprintf() because its RANKFORMAT does
not have the specifier for it.


#define Vprint(FORMAT, RANKFORMAT) {\
char buf[1024];\
va_list v;\
sprintf(buf, RANKFORMAT, myRank);\
strcat(buf, FORMAT);\
strcat(buf, "\n");\
va_start(v, FORMAT);\
vprintf(buf, v);\
fflush(stdout);\
va_end(v);\
}
#define Vprint_Norank(FORMAT, RANKFORMAT) {\
char buf[1024];\
va_list v;\
sprintf(buf, RANKFORMAT);\
strcat(buf, FORMAT);\
strcat(buf, "\n");\
va_start(v, FORMAT);\
vprintf(buf, v);\
fflush(stdout);\
va_end(v);\
}


#### 4.3.49 vprint()

vprint()  The function vprint(), given to the macro Verbose() as its argument and called in it,
prints verbose message specified by the variable number arguments following format by


<!-- Page 225 -->

the macro Vprint() or Vprint_Norank(). If verboseMode ≥3 to specify the most verbose
execution with message printing from all nodes, the rank of the local node is also printed
by Vprint(), while Vprint_Norank() is used otherwise.


void
vprint(char* format, ...) {

if (verboseMode>=3) { Vprint(format, "*Starting[%d] "); }
else                { Vprint_Norank(format, "*Starting "); }
}


#### 4.3.50 dprint()

dprint()  The function dprint(), defined only for debugging and thus not used in the production
version of the library, prints a debug message specified by its variable number arguments
following format by the macro Vprint(). The message always contains the rank of the
local node.


void
dprint(char* format, ...) {

if (nOfNodes>=1000)     { Vprint(format, "#Debug[%04d] "); }
else if (nOfNodes>=100) { Vprint(format, "#Debug[%03d] "); }
else if (nOfNodes>=10)  { Vprint(format, "#Debug[%02d] "); }
else                    { Vprint(format, "#Debug[%d] "); }
}
