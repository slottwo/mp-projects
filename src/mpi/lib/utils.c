#ifdef DEBUG
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t gdb_attached = 0;

void gdb_signal_handler(int signum) {
    gdb_attached = 1;
}

void setup_debug() {
    // Set up the signal handler for SIGUSR1
    signal(SIGUSR1, gdb_signal_handler);
}

bool is_debugger_attached() {
    // Check if the custom signal flag is set
    return gdb_attached;

    // Continue with your program logic...
    //const char* env_debugger = getenv("GDB");
    //const char* env_debugger2 = getenv("GDB_ATTACHED");
//
    //if (env_debugger != NULL || env_debugger2 != NULL) {
    //    fprintf(stderr, "\33[33m""Debugger environment variable detected: %s""\33[m\n", env_debugger);
    //    return true;
    //}
//
    //return false;
}
#endif
