/*
 * main.c - Lisp Compiler/Interpreter Main Driver
 *
 * Usage:
 *   lisp                    - Start REPL
 *   lisp file.scm           - Execute file (interpreted)
 *   lisp -c file.scm        - Compile to MASM (outputs file.asm)
 *   lisp -c file.scm -o out - Compile to specified output file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lisp.h"
#include "lexer.h"
#include "parser.h"
#include "env.h"
#include "eval.h"
#include "primitives.h"
#include "codegen.h"
#include "debug.h"

#define VERSION "1.1.0"
#define MAX_LINE_LENGTH 4096

/* Print usage information */
static void print_usage(const char *program_name) {
    printf("Lisp Compiler/Interpreter v%s\n", VERSION);
    printf("\n");
    printf("Usage:\n");
    printf("  %s                      Start interactive REPL\n", program_name);
    printf("  %s <file.scm>           Execute file (interpreted)\n", program_name);
    printf("  %s -d <file.scm>        Debug file\n", program_name);
    printf("  %s -c <file.scm>        Compile to MASM assembly\n", program_name);
    printf("  %s -c <file.scm> -o out Compile to specified output file\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -c, --compile    Compile to MASM x64 assembly\n");
    printf("  -d, --debug      Run with debugger\n");
    printf("  --debug-json     Run debugger in JSON mode (for IDE)\n");
    printf("  -o, --output     Specify output file\n");
    printf("  -h, --help       Show this help message\n");
    printf("  -v, --version    Show version information\n");
    printf("\n");
    printf("REPL Commands:\n");
    printf("  ,quit            Exit the REPL\n");
    printf("  ,help            Show REPL help\n");
    printf("  ,env             Show current environment\n");
    printf("\n");
    printf("Debug Commands (when in debug mode):\n");
    printf("  run, r           Continue execution\n");
    printf("  step, s          Step into\n");
    printf("  next, n          Step over\n");
    printf("  finish, f        Step out\n");
    printf("  break <line>     Set breakpoint\n");
    printf("  backtrace, bt    Show call stack\n");
    printf("  print <expr>     Evaluate expression\n");
    printf("  help             Show debug help\n");
    printf("\n");
}

/* Read a file into a string */
static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    return buffer;
}

/* Execute a file */
static int execute_file(const char *path) {
    char *source = read_file(path);
    if (!source) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", path);
        return 1;
    }

    /* Initialize Lisp system */
    lisp_init();

    /* Create global environment */
    Environment *global = env_create_global();
    register_primitives(global);

    /* Register global environment as GC root */
    gc_add_env_root(global);

    /* Parse */
    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    LispObject *program = parse_program(&parser);

    if (parser_had_error(&parser)) {
        fprintf(stderr, "Parse error: %s\n", parser_error_message(&parser));
        free(source);
        lisp_shutdown();
        return 1;
    }

    /* Execute each expression */
    int exit_code = 0;
    while (is_cons(program)) {
        LispObject *result = eval(car(program), global);
        (void)result;  /* Ignore result for file execution */
        program = cdr(program);
    }

    free(source);
    gc_remove_env_root(global);
    env_free(global);
    lisp_shutdown();

    return exit_code;
}

/* Execute a file with debugger */
static int debug_file(const char *path, int json_mode) {
    char *source = read_file(path);
    if (!source) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", path);
        return 1;
    }

    /* Initialize Lisp system */
    lisp_init();

    /* Initialize debugger */
    debug_init();
    debug_enable();
    debug_set_json_mode(json_mode);
    debug_set_current_location(path, 1, 1);

    /* Create global environment */
    Environment *global = env_create_global();
    register_primitives(global);

    /* Register global environment as GC root */
    gc_add_env_root(global);

    /* Parse */
    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    LispObject *program = parse_program(&parser);

    if (parser_had_error(&parser)) {
        fprintf(stderr, "Parse error: %s\n", parser_error_message(&parser));
        free(source);
        debug_shutdown();
        lisp_shutdown();
        return 1;
    }

    if (!json_mode) {
        printf("Scheme Debugger v%s\n", VERSION);
        printf("Debugging: %s\n", path);
        printf("Type 'help' for debugger commands.\n\n");
    }

    /* Execute each expression */
    int exit_code = 0;
    int line_num = 1;

    while (is_cons(program)) {
        /* Update source location */
        debug_set_current_location(path, line_num, 1);

        LispObject *result = eval(car(program), global);
        (void)result;

        /* Check if debugger wants to stop */
        if (!debug_is_enabled()) {
            break;
        }

        program = cdr(program);
        line_num++;
    }

    if (!json_mode) {
        printf("\nProgram finished.\n");
    } else {
        debug_send_json_event("terminated", NULL);
    }

    free(source);
    gc_remove_env_root(global);
    env_free(global);
    debug_shutdown();
    lisp_shutdown();

    return exit_code;
}

/* REPL (Read-Eval-Print Loop) */
static void repl(void) {
    char line[MAX_LINE_LENGTH];
    char input_buffer[MAX_LINE_LENGTH * 10];  /* For multi-line input */
    int paren_depth = 0;

    printf("Lisp REPL v%s\n", VERSION);
    printf("Type ',help' for help, ',quit' to exit.\n\n");

    /* Initialize Lisp system */
    lisp_init();

    /* Create global environment */
    Environment *global = env_create_global();
    register_primitives(global);

    /* Register global environment as GC root */
    gc_add_env_root(global);

    input_buffer[0] = '\0';

    while (1) {
        /* Print prompt */
        if (paren_depth == 0) {
            printf("> ");
        } else {
            printf("  ");
            for (int i = 0; i < paren_depth; i++) printf("  ");
        }
        fflush(stdout);

        /* Read line */
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        /* Check for REPL commands */
        if (paren_depth == 0 && line[0] == ',') {
            if (strcmp(line, ",quit") == 0 || strcmp(line, ",q") == 0) {
                break;
            }
            if (strcmp(line, ",help") == 0 || strcmp(line, ",h") == 0) {
                printf("REPL Commands:\n");
                printf("  ,quit or ,q    Exit the REPL\n");
                printf("  ,help or ,h    Show this help\n");
                printf("  ,env           Show current environment\n");
                printf("  ,clear         Clear the screen\n");
                printf("\n");
                printf("Special Forms:\n");
                printf("  (define x v)   Define variable x with value v\n");
                printf("  (define (f x) ...) Define function f\n");
                printf("  (lambda (x) ...) Create anonymous function\n");
                printf("  (if c t e)     Conditional\n");
                printf("  (quote x) or 'x Return x unevaluated\n");
                printf("  (let ((x v) ...) ...) Local bindings\n");
                printf("  (begin ...)    Sequence of expressions\n");
                printf("\n");
                continue;
            }
            if (strcmp(line, ",env") == 0) {
                env_print(global);
                continue;
            }
            if (strcmp(line, ",clear") == 0) {
                printf("\033[2J\033[H");  /* ANSI clear screen */
                continue;
            }
            printf("Unknown command: %s\n", line);
            continue;
        }

        /* Accumulate input */
        strcat(input_buffer, line);
        strcat(input_buffer, " ");

        /* Count parentheses */
        for (size_t i = 0; i < len; i++) {
            if (line[i] == '(') paren_depth++;
            else if (line[i] == ')') paren_depth--;
        }

        /* Skip if unbalanced (need more input) */
        if (paren_depth > 0) {
            continue;
        }

        /* Reset if negative (syntax error) */
        if (paren_depth < 0) {
            printf("Error: Unbalanced parentheses\n");
            input_buffer[0] = '\0';
            paren_depth = 0;
            continue;
        }

        /* Skip empty input */
        if (strlen(input_buffer) == 0 || input_buffer[0] == '\n') {
            continue;
        }

        /* Parse and evaluate */
        Lexer lexer;
        lexer_init(&lexer, input_buffer);

        Parser parser;
        parser_init(&parser, &lexer);

        LispObject *expr = parse_expression(&parser);

        if (parser_had_error(&parser)) {
            printf("Parse error: %s\n", parser_error_message(&parser));
        } else if (expr) {
            LispObject *result = eval(expr, global);
            lisp_print(result);
            printf("\n");
        }

        /* Reset for next input */
        input_buffer[0] = '\0';
        paren_depth = 0;
    }

    printf("Goodbye!\n");

    gc_remove_env_root(global);
    env_free(global);
    lisp_shutdown();
}

/* Main entry point */
int main(int argc, char *argv[]) {
    int compile_mode = 0;
    int debug_mode = 0;
    int debug_json_mode = 0;
    const char *input_file = NULL;
    const char *output_file = NULL;

    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("Lisp Compiler/Interpreter v%s\n", VERSION);
            return 0;
        }
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compile") == 0) {
            compile_mode = 1;
            continue;
        }
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
            continue;
        }
        if (strcmp(argv[i], "--debug-json") == 0) {
            debug_mode = 1;
            debug_json_mode = 1;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
            continue;
        }
        if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            return 1;
        }

        /* Input file */
        if (!input_file) {
            input_file = argv[i];
        } else {
            fprintf(stderr, "Error: Multiple input files specified\n");
            return 1;
        }
    }

    /* Determine action */
    if (compile_mode) {
        if (!input_file) {
            fprintf(stderr, "Error: No input file specified for compilation\n");
            return 1;
        }

        /* Generate output filename if not specified */
        if (!output_file) {
            static char default_output[256];
            const char *dot = strrchr(input_file, '.');
            if (dot) {
                size_t base_len = dot - input_file;
                strncpy(default_output, input_file, base_len);
                strcpy(default_output + base_len, ".asm");
            } else {
                snprintf(default_output, sizeof(default_output), "%s.asm", input_file);
            }
            output_file = default_output;
        }

        printf("Compiling %s -> %s\n", input_file, output_file);
        int result = compile_file(input_file, output_file);

        if (result == 0) {
            printf("Compilation successful.\n");
            printf("\nTo assemble and link (Windows):\n");
            printf("  ml64 /c %s\n", output_file);
            printf("  link /subsystem:console /entry:main %.*s.obj lisp_rt.lib\n",
                   (int)(strrchr(output_file, '.') - output_file), output_file);
        }

        return result;
    }

    if (debug_mode) {
        if (!input_file) {
            fprintf(stderr, "Error: No input file specified for debugging\n");
            return 1;
        }
        return debug_file(input_file, debug_json_mode);
    }

    if (input_file) {
        /* Execute file */
        return execute_file(input_file);
    }

    /* Start REPL */
    repl();
    return 0;
}
