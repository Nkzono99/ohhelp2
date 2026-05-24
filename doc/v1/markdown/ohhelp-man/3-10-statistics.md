# 3.10 Statistics

Source: `doc/v1/original/ohhelp-man.pdf`, pages 92-98.

<!-- Page 92 -->

remap_particle_to_subdomain().  If it is troublesome due to, for example, the necessity
of special care for injected particles in your particle pushing procedure, you may use oh2_
inject_particle() instead of level-4p’s oh4p_inject_particle() for the injection and
then call oh4p_map_particle_to_neighbor() or oh4p_map_particle_to_subdomain().
One caution for this second solution is that you have to set nid element of the particle to
−1 when you call oh2_inject_particle() so that the function excludes the particle from
the per-subdomain histogram.
As for the removal, you have to call oh4p_remove_mapped_particle() if and only if
the particle to be removed is mapped by a level-4p’s mapping function or injected by oh4p_
inject_particle() after the last call of oh4p_transbound(). For example, you might
move a particle and then call a mapping function for it before you find the particle should
be eliminated due to, e.g., ion-electron recombination. You can cope with this complication
by calling oh4p_remove_mapped_particle() for the electron recombined with the ion.
Note that if this recombination changes the species of the ion due to the discharge, you
also have to call oh4p_remove_mapped_particle() for the ion and then inject it by oh4p_
inject_particle() specifying the new species. Also note that the eliminating a particle
which a mapping function detected going out-of-bounds returning −1 to the caller does not
require to call oh4p_remove_mapped_particle(), though doing it is not harmful logically.
Finally,  the  discussion  of  level-4p  injection and  removal above  perfectly  holds
for  the   level-4s  extension  and   its  functions  oh4s_inject_particle(),  oh4s_
map_particle_to_neighbor(),  oh4s_map_particle_to_subdomain(),  oh4s_remove_
mapped_particle(), oh4s_remap_particle_to_neighbor(), oh4s_remap_particle_to_
subdomain() and oh4s_transbound().


#### 3.9.4 Identification of Injected Particles

The last issue on particle injection and removal  is the identification of particles.  In
the default definition of the Fortran structured type oh particle and C struct named
S particle, each particle has its identifier in pid element. Since this element is a 64-bit
integer, the space for the identification number is large enough for local numbering with-
out reclamation. For example, a node n may give a number kN + n to the k-th particle
created by the node. Since 264 should be much larger than N, the identification space is
hardly exhosted. For example, even if N = 220 and each node injects (and removes) 220
particles in each simulation step in addition to its initial accommodation of 230 particles, it
will take about 16.8 million time steps or, even if your simulator has an excellent per-node
performance of 10 million particles per second21, 1.68 billion seconds or 53 years.

## 3.10 Statistics

Level-1 library provides you with the functions to collect, process and report two types of
statistics data of timings and particle transfers. The timing statistics data is obtained by
measuring the execution time of intervals in your program including the library functions.
Since each interval is identified by a key being a non-negative unique integer, you have to
define the set of keys for the intervals which you want to measure together with strings
printed on the report, by modifying the C header file oh stats.h as discussed in §3.10.1.
Then, after calling oh1_init(), or one of its higher level counterparts oh2_init() and
oh3_init(), giving it fundamental parameters for statistics as discussed in §3.10.2, you
may call the following functions (subroutines) to collect, process and report statistics data
as explained in §3.10.3, 3.10.4, 3.10.5 and 3.10.6.

21The per-node perormance of our simulator reported in [1] is 2.55 million particle per second.


<!-- Page 93 -->

oh1_init_stats() initializes internal data structures for statistics and starts the execution
time measurement of the first interval.

oh1_stats_time() finishes the execution time measurement of the last interval, and starts
that of the next interval.

oh1_show_stats() gathers timing and particle transfer statistics data measured in a sim-
ulation step and, if specified, reports a subtotal for recent steps.

oh1_print_stats() reports the grand total of statistics data.


#### 3.10.1 Timing Statistics Keys and Header File oh stats.h

You can measure the execution time of an interval in your program by calling oh1_stats_
time() giving it a key to identify the interval. Since the key, a non-negative integer, should
be unique to the interval and should be associated to a character string printed on the
report together with the statistics of the measured timing, the library provides you with
a C header file named oh stats.h, which can be included from Fortran codes too, to assure
the uniqueness and the association with the string easily.
The file consists of two parts and the default definition given by the first part is as
follows.

#define STATS_TRANSBOUND  0
#define STATS_TRY_STABLE  (STATS_TRANSBOUND + 1)
#define STATS_REBALANCE   (STATS_TRY_STABLE + 1)
#define STATS_REB_COMM    (STATS_REBALANCE + 1)
#define STATS_TB_MOVE     (STATS_REB_COMM + 1)
#ifdef  OH_LIB_LEVEL_4PS
#define STATS_TB_SORT     (STATS_TB_MOVE + 1)
#define STATS_TB_COMM     (STATS_TB_SORT + 1)
#else
#define STATS_TB_COMM     (STATS_TB_MOVE + 1)
#endif
#define STATS_TIMINGS     (STATS_TB_COMM + 1)


The code above #define’s the following six (or seven  if you activate level-4p/4s exten-
sion) keys to measure the execution times in oh1_transbound() and/or its higher level
counterparts and a special key STATS TIMINGS to have the number of keys.

STATS_TRANSBOUND is for the interval to examine if the execution mode in the next step is
primary.

STATS_TRY_STABLE is for the interval to examine if the helpand-helper configuration can
be kept in the next step.

STATS_REBALANCE is for the interval to (re)build a new helpand-helper configuration.

STATS_REB_COMM is for the interval to create family communicators for the newly built
helpand-helper configuration.

STATS_TB_MOVE is for the interval in oh2_transbound() or oh3_transbound() to move
particles in pbuf.

STATS_TB_SORT is for the interval in oh4p_transbound() for sorting particles by their
position.


<!-- Page 94 -->

STATS_TB_COMM is for the interval in oh2_transbound() or oh3_transbound() to transfer
particles among nodes.

On adding your own keys, it is recommended to follow the convention shown in the file.
That is, defining a key by;

#define ⟨new key⟩(⟨last key⟩+1)

will assure the uniqueness and continuity of keys. For example, to add three keys namely
STATS_PARTICLE_PUSHING, STATS_CURRENT_SCATTERING and STATS_FIELD_SOLVING for
the intervals of particle pushing, current scattering and field solving in your main loop,
replacing the first line for STATS_TRANSBOUND with the followings is safe and correct.

#define STATS_PARTICLE_PUSHING   0
#define STATS_CURRENT_SCATTERING (STATS_PARTICLE_PUSHING + 1)
#define STATS_FIELD_SOLVING      (STATS_CURRENT_SCATTERING + 1)
#define STATS_TRANSBOUND         (STATS_FILED_SOLVING + 1)


Note that you must not remove any definitions given in the original oh stats.h, or you cannot
compile the library correctly.
The second part of the file defines the character strings for keys as follows.

#ifdef OH_DEFINE_STATS
static char *StatsTimeStrings[2*STATS_TIMINGS] = {
"transbound",         "",
"try_stable",         "",
"rebalance",          "",
"reb comm create",    "",
"part move[pri]",     "part move[sec]",
#ifdef  OH_LIB_LEVEL_4PS
"part sort[pri]",     "part sort[sec]",
#endif
"part comm[pri]",     "part comm[sec]",
};
#endif
In the code above, #ifdef/#endif construct is to protect your code from erroneous com-
pilation especially if your code is in Fortran. That is, OH_DEFINE_STATS is defined only in
the library source files and thus the compiler for your own codes will skip the part which
cannot be parsed as a Fortran code.
The important part in the code is the sequence of the string pairs, one pair for each
line. The pairs correspond to keys in the same order and each pair gives a short explanation
of the pair of intervals, one for primary particles/subdomains and the other for secondary
ones, identified by the corresponding key. That is, if your interval is executed twice as a
primary execution and a secondary execution, the first and second strings are used as the
titles of two executions separately. Otherwise, or if you measure two executions as a whole,
defining the first string and letting the second be empty string are necessary and sufficient.
For example, adding the following three lines just before the line having "transbound"
is what you need to do for the three keys exemplified above, providing you want to measure
primary and secondary executions of each interval separately.

"particle pushing[pri]",   "particle pushing[sec]",
"current scattering[pri]", "current scattering[sec]",
"field solving[pri]",      "field solving[sec]",


<!-- Page 95 -->

Remember that a title can be arbitrarily long but that of 30 characters or longer will cause
an ungly line in the report.

#### 3.10.2 Arguments of oh1_init() for Statistics

As shown in §3.4.1, the function (subroutine) oh1_init() and its higher level counterparts
have the following two arguments to control statistics operations.

stats activates or inactivates statistics operations as follows.

- If stats = 0, statistics operations are inactivated and thus the functions dis-
cussed in the following sections do nothing.
- if stats = 122, statistics operations are activated but only the grand total is
reported by oh1_print_stats().
- if stats = 2, statistics operations are activated and oh1_show_stats() will re-
port subtotal when it is given the simulation step count divisible by the argument
repiter of oh1_init().

Note that oh1_transbound() and its higher level counterparts also have an argument
stats to control the statistics collection in the function temporarily overriding what
stats of oh1_init() specifies. That is, statistics collection in oh1_transbound() is
inactivated if its stats is 0 regardless stats of oh1_init(), while non-zero means
that statistics collection follows what stats of oh1_init() specifies. This feature is
useful to exclude statistics data in, for example, initialization process.

repiter defines the frequency of subtotal reporting by oh1_show_stats(). That is,  if
stats = 2, it defines the gap of periodical reporting by oh1_show_stats().

#### 3.10.3 oh1_init_stats()

The function (subroutine) oh1 init stats() initializes internal data structures for statis-
tics, and starts first interval of timing measurement, if stats of oh1_init() is not zero.
The other statistics function must be called after oh1 init stats() is called.


Fortran Interface

subroutine oh1_init_stats(key, ps)
implicit none
integer,intent(in) :: key
integer,intent(in) :: ps
end subroutine


C Interface

void oh1_init_stats(int key, int ps);


key is the key of the first interval whose execution time is measured.  If you do not want
to include the first interval in the timing statistics, give this argment the special key
STATS_TIMINGS.

ps indicates whether the first interval is for primary execution (0) or secondary execu-
tion(1).

22Or some other value excluding 0 and 2.


<!-- Page 96 -->

#### 3.10.4 oh1_stats_time()

The function (subroutine) oh1 stats time() finishes the last interval of timinig measure-
ment and starts next one, if stats of oh1_init() is not zero.


Fortran Interface

subroutine oh1_stats_time(key, ps)
implicit none
integer,intent(in) :: key
integer,intent(in) :: ps
end subroutine


C Interface

void oh1_stats_time(int key, int ps);


key is the key of the interval to start for execution time measurement. If you want only to
finish the last interval, give this argment the special key STATS_TIMINGS.

ps indicates whether the next interval is for primary execution (0) or secondary execution
(1).


#### 3.10.5 oh1_show_stats()

The function (subroutine) oh1 show stats() performs the following statistics operations if
stats of oh1_init() non-zero.

- Finish the last interval of timing measurement.

- Gather statistics data measured since the last call of this function or the call of oh1_
init_stats().

- Update grand total statistics and, if stats of oh1_init() is 2, subtoal statistics.

- Print subtotal statistics as oh1_print_stats() does,  if stats of oh1_init() is 2
and step argument of this function is divisible by repiter of oh1_init().

- Start a new interval whose execution time is excluded from timing statistics.

It is expected to call this function every simulation step so that it collect statistics data
for each step.


Fortran Interface

subroutine oh1_show_stats(step, currmode)
implicit none
integer,intent(in) :: step
integer,intent(in) :: currmode
end subroutine


<!-- Page 97 -->

C Interface

void oh1_show_stats(int step, int currmode);


step is the simulation step count to control periodical statistics reporting.  If stats of
oh1_init() is 2 and step is divisible by repiter of oh1_init(), subtotal statistics
is reported.

currmode indicates whether the current execution mode is primary (0) or secondary (1).
This value should be corresponding to the return value of the last call of oh1_
transbound() or its higher level counterparts.

#### 3.10.6 oh1_print_stats()

The function (subroutine) oh1 print stats() report the grand total (so far) of statistics
through standard output in the following format. The first part of the report is for execution
time of each interval as follows.

## Execution Times (sec)
particle pushing[pri]        =    0.024 /    2.297 /    1.015 / 1824925.604
particle pushing[sec]        =    0.077 /    2.440 /    1.564 / 2135827.627
current scattering[pri]      =    0.011 /    1.223 /    0.422 /  736536.722
current scattering[sec]      =    0.032 /    1.332 /    0.836 / 1296109.407
field solving[pri]           =    0.003 /    0.089 /    0.011 /   27344.603
field solving[sec]           =    0.003 /    0.053 /    0.012 /   19633.007
transbound                   =    0.004 /    0.837 /    0.222 /  364201.720
= -------- / -------- / -------- / -----------
try_stable                   =    0.001 /    0.025 /    0.002 /    2366.882
= -------- / -------- / -------- / -----------
rebalance                    =    0.001 /    0.002 /    0.001 /      21.432
= -------- / -------- / -------- / -----------
reb comm create              =    0.023 /    2.569 /    0.740 /    4358.333
= -------- / -------- / -------- / -----------
part move[pri]               =    0.021 /    1.668 /    0.528 /  283491.149
part move[sec]               =    0.014 /    1.772 /    0.541 /  606886.809
part comm[pri]               =    0.001 /    1.077 /    0.025 /   16184.129
part comm[sec]               =    0.002 /    1.677 /    0.023 /   21244.773


Each column of the table above shows the followings of each interval.

- Column-1: titie of the interval.

- Column-2: minimum execution time of the interval.

- Column-3: maximum execution time of the interval.

- Column-4: average execution time of the interval.

- Column-5: sum of execution times of the interval.

Note that the minimum, maximum, average and sum are over all occasions of each interval
in all nodes and all simulation time steps.
Then the second part reports the statistics of particle transfer as follows.


<!-- Page 98 -->

## Particle Movements
p2p transfer[pri,min]        =      235 /  4272367 /     7368 /    47153707
p2p transfer[pri,max]        =     1891 /  8054375 /    14909 /    95416324
p2p transfer[pri,ave]        =      441 /  6194818 /    12796 /    81894514
get[pri,min]                 =        0 /      589 /        3 /       19210
get[pri,max]                 =     6511 /  8054765 /    22971 /   147011962
put[pri,min]                 =        0 /      984 /        5 /       29490
put[pri,max]                 =     6209 /  8054375 /    16387 /   104877429
put&get[pri,ave]             =       13 /    31464 /       90 /      574318
p2p transfer[sec,min]        =        1 /      656 /        2 /       10488
p2p transfer[sec,max]        =     2198 /  6034178 /    22907 /   146602986
p2p transfer[sec,ave]        =       31 /  1393581 /     2748 /    17587875
get[sec,min]                 =        0 /      289 /        2 /       10021
get[sec,max]                 =     3577 /  8387296 /    51544 /   329883298
put[sec,min]                 =        0 /     1476 /        4 /       24108
put[sec,max]                 =     3809 /  9732486 /    47848 /   306224944
put&get[sec,ave]             =      118 /  1812744 /     3473 /    22225683
transition to pri            =     1594 /        0 /        1 /        1595
transition to sec            =        1 /     4782 /       22 /        4805


The rows above except for the last two are for the following particle transfers which are
scheduled in one execution of oh1_transbound() or are actually performed in one execution
of oh2_transbound() or oh3_transbound().

p2p transfer[] shows the number of transferred particles between a pair of nodes. The
minimum, maximum and average are calculated over all pairs such that at least one
particle is transferred between each node pair.

get[] shows the number of particles a node received. The minimum and maximum are
calculated over all nodes including those received nothing.

put[] shows the number of particles a node sent. The minimum and maximum are calcu-
lated over all nodes including those sent nothing.

put&get[] shows the average number of particles a node received (or sent). The average
are calculated over all nodes including those received nothing.

Note that the categorization of primary (pri) and secondary (sec) particles is based on
the viewpoint of receivers.  Also note that the columns from Column-2 to Column-5 of
these rows are for the minimum, maximum, average and sum which are calculated over all
simulation time steps.
On the other hand, the last two raws show number of transitions to primary and sec-
ondary modes. In these rows, Column-2 and Column-3 are for the number of transitions
from primary and secondary modes respectively. Column-4 of the transition to primary is
the number of primary to primary transition at which non-neighboring particle transfers
are taken, while that of to secondary means the number of secondary to secondary with
rebuilding of helpand-helper configuration.  Finally Column-5 of both rows is the total
number of transition to primary or secondary mode.
The function oh1_show_stats() also reports the statistics if stats and repiter of oh1_
init() and step argument of the function satisfy the reporting condition, but the numbers
shown in columns of the minimum and others are calculated over the recent repiter steps.
