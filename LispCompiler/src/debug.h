/*
 * debug.h - Scheme Debugger Interface
 *
 * Provides debugging capabilities including breakpoints, stepping,
 * call stack inspection, and variable inspection.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "lisp.h"
#include "env.h"

/* Maximum limits */
#define MAX_BREAKPOINTS 256
#define MAX_CALL_STACK 1024
#define MAX_CONDITION_LENGTH 256
#define MAX_WATCH_EXPRESSIONS 64
#define MAX_WATCH_NAME_LENGTH 128

/* Debug execution modes */
typedef enum {
    DEBUG_MODE_NONE,        /* Normal execution (no debugging) */
    DEBUG_MODE_STEP_INTO,   /* Stop at next expression */
    DEBUG_MODE_STEP_OVER,   /* Stop at next expression same level */
    DEBUG_MODE_STEP_OUT,    /* Stop when returning from current function */
    DEBUG_MODE_CONTINUE,    /* Run until breakpoint */
    DEBUG_MODE_PAUSED       /* Execution paused, waiting for command */
} DebugMode;

/* Breakpoint stop reason */
typedef enum {
    STOP_REASON_NONE,
    STOP_REASON_BREAKPOINT,
    STOP_REASON_STEP,
    STOP_REASON_STEP_OVER,
    STOP_REASON_STEP_OUT,
    STOP_REASON_EXCEPTION,
    STOP_REASON_ENTRY,
    STOP_REASON_DATA_CHANGE      /* Watch expression value changed */
} StopReason;

/* Exception break mode */
typedef enum {
    EXCEPTION_BREAK_NONE,        /* Don't break on exceptions */
    EXCEPTION_BREAK_UNCAUGHT,    /* Break only on uncaught exceptions */
    EXCEPTION_BREAK_ALL          /* Break on all exceptions */
} ExceptionBreakMode;

/* Watch expression */
typedef struct WatchExpression {
    int id;                                  /* Unique watch ID */
    char expression[MAX_WATCH_NAME_LENGTH];  /* Expression to watch */
    LispObject *last_value;                  /* Last evaluated value */
    int break_on_change;                     /* 1 = data breakpoint mode */
    int enabled;                             /* Is watch active */
} WatchExpression;

/* Source location */
typedef struct SourceLocation {
    const char *filename;
    int line;
    int column;
} SourceLocation;

/* Breakpoint definition */
typedef struct Breakpoint {
    int id;                             /* Unique breakpoint ID */
    const char *filename;               /* Source file */
    int line;                           /* Line number */
    int enabled;                        /* Is breakpoint active */
    int hit_count;                      /* Number of times hit */
    char condition[MAX_CONDITION_LENGTH]; /* Conditional expression (optional) */
} Breakpoint;

/* Call stack frame */
typedef struct CallFrame {
    const char *function_name;          /* Name of function (or "<lambda>") */
    LispObject *arguments;              /* Arguments passed */
    Environment *env;                   /* Local environment */
    SourceLocation location;            /* Where the call was made */
    int frame_id;                       /* Frame identifier */
} CallFrame;

/* Debugger callback function type */
typedef void (*DebugCallback)(void *context, StopReason reason,
                              LispObject *expr, Environment *env);

/* Debug state structure */
typedef struct DebugState {
    /* Execution control */
    DebugMode mode;
    int step_depth;                     /* For step-over/step-out */
    int is_running;                     /* Is program running */

    /* Breakpoints */
    Breakpoint breakpoints[MAX_BREAKPOINTS];
    int num_breakpoints;
    int next_breakpoint_id;

    /* Watch expressions */
    WatchExpression watches[MAX_WATCH_EXPRESSIONS];
    int num_watches;
    int next_watch_id;

    /* Exception breakpoints */
    ExceptionBreakMode exception_break_mode;
    const char *last_exception_message;

    /* Call stack */
    CallFrame call_stack[MAX_CALL_STACK];
    int stack_depth;

    /* Current execution state */
    LispObject *current_expr;           /* Expression being evaluated */
    Environment *current_env;           /* Current environment */
    SourceLocation current_location;    /* Current source location */
    StopReason stop_reason;

    /* Source file tracking */
    const char *current_file;

    /* Callbacks */
    DebugCallback on_stop;              /* Called when execution stops */
    void *callback_context;             /* Context for callbacks */

    /* JSON mode flag */
    int json_mode;
} DebugState;

/* Global debug state */
extern DebugState *g_debug_state;

/* Initialize/shutdown debugger */
void debug_init(void);
void debug_shutdown(void);

/* Enable/disable debugging */
void debug_enable(void);
void debug_disable(void);
int debug_is_enabled(void);

/* Set JSON mode for IDE integration */
void debug_set_json_mode(int enabled);

/* Breakpoint management */
int debug_add_breakpoint(const char *filename, int line);
int debug_add_conditional_breakpoint(const char *filename, int line, const char *condition);
int debug_remove_breakpoint(int id);
int debug_enable_breakpoint(int id);
int debug_disable_breakpoint(int id);
Breakpoint *debug_get_breakpoint(int id);
void debug_list_breakpoints(void);
void debug_clear_all_breakpoints(void);

/* Watch expression management */
int debug_add_watch(const char *expression);
int debug_add_data_breakpoint(const char *expression);  /* Watch with break on change */
int debug_remove_watch(int id);
int debug_enable_watch(int id);
int debug_disable_watch(int id);
WatchExpression *debug_get_watch(int id);
void debug_list_watches(void);
void debug_update_watches(void);  /* Called after each step to check for changes */
void debug_clear_all_watches(void);

/* Exception breakpoint management */
void debug_set_exception_break(ExceptionBreakMode mode);
ExceptionBreakMode debug_get_exception_break(void);
void debug_on_exception(const char *message, LispObject *expr);  /* Called by error handler */

/* Execution control */
void debug_run(void);
void debug_step_into(void);
void debug_step_over(void);
void debug_step_out(void);
void debug_pause(void);
void debug_stop(void);

/* Called by evaluator - check if should break */
int debug_check_break(LispObject *expr, Environment *env);

/* Call stack management */
void debug_push_frame(const char *func_name, LispObject *args,
                      Environment *env, SourceLocation *loc);
void debug_pop_frame(void);
int debug_get_stack_depth(void);
CallFrame *debug_get_frame(int index);
void debug_print_backtrace(void);

/* Source location */
void debug_set_current_location(const char *file, int line, int column);
SourceLocation debug_get_current_location(void);

/* Variable inspection */
LispObject *debug_get_locals(int frame_index);
LispObject *debug_eval_in_frame(const char *expr_str, int frame_index);
LispObject *debug_lookup_variable(const char *name, int frame_index);

/* Callback registration */
void debug_set_callback(DebugCallback callback, void *context);

/* Debug REPL */
void debug_repl(void);

/* JSON protocol commands (for IDE integration) */
void debug_handle_json_command(const char *json_cmd);
void debug_send_json_event(const char *event_type, const char *body);

/* Utility */
void debug_print_location(SourceLocation *loc);
void debug_print_expression(LispObject *expr);

#endif /* DEBUG_H */
