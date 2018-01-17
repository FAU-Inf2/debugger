#include "helper.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>

static void run_debugee(char **command_line);
static void run_debugger(pid_t debugee_pid, void *breakpoint_address);

/*
 * Entry point. 
 */
int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "[!] USAGE: debugger <breakpoint address> <debugee command> <debugee argument>*\n");
    return 1;
  }

  // fork process in debugger and debugee
  // the debugee will be a child of the debugger process
  const pid_t child_pid = fork();

  if (child_pid == 0) {
    // debugee process
    char **command_line = (char **)(&(argv[2]));
    run_debugee(command_line);
  } else if (child_pid > 0) {
    // debugger process
    void *breakpoint_address = (void *)(strtol(argv[1], NULL, 16));
    run_debugger(child_pid, breakpoint_address);
  } else {
    perror("fork");
  }

  return 0;
}

/*
 * Runs the debugee.
 */
static void run_debugee(char **command_line) {
  DOUT("[i] run debugee (%s)\n", command_line[0]);

  // indicate that the debugee process should be traced by its parent
  if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
    perror("ptrace");
    return;
  }

  // replace the process with the debugee
  execvp(command_line[0], command_line);
}

/*
 * Prints some registers values of the debugee.
 */
static void print_debug_info(pid_t debugee_pid) {
  // get register values
  struct user_regs_struct registers;
  if (ptrace(PTRACE_GETREGS, debugee_pid, 0, &registers) < 0) {
    perror("ptrace");
    return;
  }

  // print values of (some registers)
  printf("@[%#018x]:\n  %rax: %d\n  %rbx: %d\n  %rcx: %d\n  %rdx: %d\n  %rdi: %d\n  %rsi: %d\n",
      registers.rip, registers.rax, registers.rbx, registers.rcx, registers.rdx,
      registers.rdi, registers.rsi);
}

/*
 * Sets a breakpoint at the given address in the debugee.
 */
static long set_breakpoint(pid_t debugee_pid, void *breakpoint_address) {
  // get the original instructions at the breakpoint address
  long original_insn = ptrace(PTRACE_PEEKTEXT, debugee_pid, breakpoint_address, 0);

  // replace first instruction with `int 3` (debug trap)
  long patched_insn = ((original_insn & 0xFFFFFFFFFFFFFF00) | 0xCC);
  ptrace(PTRACE_POKETEXT, debugee_pid, breakpoint_address, patched_insn);

  return original_insn;
}

/*
 * Resumes the execution of the debugee after a breakpoint was hit.
 */
static void resume_from_breakpoint(pid_t debugee_pid, void *breakpoint_address, long original_insn) {
  // restore original instruction at breakpoint address
  ptrace(PTRACE_POKETEXT, debugee_pid, breakpoint_address, original_insn);

  // decrement instruction pointer by 1, such that the original instruction (the one that has been
  // replaced with the debug trap) will be executed next
  struct user_regs_struct registers;
  ptrace(PTRACE_GETREGS, debugee_pid, 0, &registers);
  registers.rip -= 1;
  ptrace(PTRACE_SETREGS, debugee_pid, 0, &registers);

  // execute a single instruction (the one that has been replaced)
  if (ptrace(PTRACE_SINGLESTEP, debugee_pid, 0, 0) < 0) {
    perror("ptrace");
    return;
  }

  int debugee_status;
  wait(&debugee_status);

  // set the breakpoint again (since it may be contained in a loop or a recursive function and thus
  // could be reached again)
  set_breakpoint(debugee_pid, breakpoint_address);

  // resume execution of the debugee until it receives the next signal
  if (ptrace(PTRACE_CONT, debugee_pid, 0, 0) < 0) {
    perror("ptrace");
    return;
  }
}

/*
 * Runs the debugger.
 */
static void run_debugger(pid_t debugee_pid, void *breakpoint_address) {
  DOUT("[i] run debugger (debugee pid: %d, breakpoint address: %#018x)\n",
    debugee_pid, breakpoint_address);

  int debugee_status;

  // wait for the initial SIGTRAP (the one that is signaled due to the execvp() call)
  wait(&debugee_status);

  if (WIFEXITED(debugee_status)) {
    DOUT("[i] debugee exited\n");
    return;
  }

  // set the breakpoint at the given breakpoint address and remember the original instructions
  long original_insn = set_breakpoint(debugee_pid, breakpoint_address);

  // resume execution of the debugee until it receives the next signal
  if (ptrace(PTRACE_CONT, debugee_pid, 0, 0) < 0) {
    perror("ptrace");
    return;
  }

  // wait for the next SIGTRAP
  wait(&debugee_status);

  // check if debugee reached a breakpoint (or if it terminated instead) 
  while (WIFSTOPPED(debugee_status)) {
    DOUT("[i] reached breakpoint\n");

    // print some 'debug info'
    print_debug_info(debugee_pid);

    // resume execution of the debugee
    resume_from_breakpoint(debugee_pid, breakpoint_address, original_insn);

    // wait for the next SIGTRAP
    wait(&debugee_status);
  }

  DOUT("[i] debugee terminated\n");
}
