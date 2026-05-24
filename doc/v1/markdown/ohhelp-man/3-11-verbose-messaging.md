# 3.11 Verbose Messaging

Source: `doc/v1/original/ohhelp-man.pdf`, pages 99-99.

<!-- Page 99 -->

Fortran Interface

subroutine oh1_print_stats(nstep)
implicit none
integer,intent(in) :: nstep
end subroutine


C Interface

void oh1_print_stats(int nstep);


nstep is the total simulation step count to calculate the average numbers in Column-4.

## 3.11 Verbose Messaging

Although the application of the OhHelp library to your PIC simulator is fairly simple and
straightforward, it should be hard to compose a bug-free program instantly. Therefore, you
will want to investigate what is going on in your program including the functions in the
library when you encounter a problem.
Verbose messaging provided by the library is a fundamental mean for the investigation.
You can activate or inactivate the verbose messaging in library functions by giving one of
the followings to the argument verbose of oh1_init() or its higher level counterparts.

- verbose = 0 inactivates verbose messaging and thus makes library functions execute
silently.

- verbose = 1 activates verbose messaging to have fundamental reports from library
functions.

- verbose = 2 activates more verbose messaging than the case of 1 to capture some
details of the events happening in library functions.

- verbose = 3 is similar to 2 but you will have messages from all nodes with their
identifier (MPI rank).

If activated, messages are printed to standard output with a common header “*Starting”
optionally followed by a node identifier surrounded by brackets.
In addition, you may have your own verbose messaging to be controlled by verbose of
oh1_init() by calling the following function oh1 verbose().


Fortran Interface

subroutine oh1_verbose(message)
implicit none
character(*),intent(in) :: message
end subroutine


C Interface

void oh1_verbose(char *message);


message is a character string to be printed following the header. Since it should be null-
ternminated, you have to remember that a Fortran string constant, say ’hello’ does
not have the terminator and thus you have to explicitly give a null code by ’hello\0’.
