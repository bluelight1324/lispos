import * as vscode from 'vscode';
import * as path from 'path';
import { ChildProcess, spawn } from 'child_process';
import {
    LoggingDebugSession,
    InitializedEvent,
    StoppedEvent,
    TerminatedEvent,
    BreakpointEvent,
    OutputEvent,
    Thread,
    StackFrame,
    Scope,
    Source,
    Breakpoint
} from '@vscode/debugadapter';
import { DebugProtocol } from '@vscode/debugprotocol';

interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
    program: string;
    compilerPath?: string;
    stopOnEntry?: boolean;
}

interface SchemeBreakpoint {
    id: number;
    line: number;
    verified: boolean;
    condition?: string;
}

interface SchemeWatch {
    id: number;
    expression: string;
    value?: string;
}

export class SchemeDebugSession extends LoggingDebugSession {
    private static THREAD_ID = 1;

    private _process: ChildProcess | null = null;
    private _breakpoints: Map<string, SchemeBreakpoint[]> = new Map();
    private _breakpointId = 0;
    private _watches: SchemeWatch[] = [];
    private _watchId = 0;
    private _program: string = '';
    private _compilerPath: string = '';
    private _currentLine: number = 1;
    private _currentFile: string = '';
    private _variables: Map<string, any> = new Map();
    private _callStack: Array<{ name: string; file: string; line: number }> = [];
    private _responseBuffer: string = '';
    private _exceptionBreakMode: 'never' | 'always' | 'uncaught' = 'never';

    public constructor() {
        super('scheme-debug.txt');

        this.setDebuggerLinesStartAt1(true);
        this.setDebuggerColumnsStartAt1(true);
    }

    protected initializeRequest(response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments): void {
        response.body = response.body || {};

        response.body.supportsConfigurationDoneRequest = true;
        response.body.supportsEvaluateForHovers = true;
        response.body.supportsStepBack = false;
        response.body.supportsSetVariable = false;
        response.body.supportsRestartFrame = false;
        response.body.supportsGotoTargetsRequest = false;
        response.body.supportsStepInTargetsRequest = false;
        response.body.supportsCompletionsRequest = false;
        response.body.supportsBreakpointLocationsRequest = false;
        response.body.supportsConditionalBreakpoints = true;
        response.body.supportsHitConditionalBreakpoints = false;
        response.body.supportsExceptionInfoRequest = true;
        response.body.supportsExceptionOptions = true;
        response.body.exceptionBreakpointFilters = [
            {
                filter: 'all',
                label: 'All Exceptions',
                description: 'Break on all exceptions'
            },
            {
                filter: 'uncaught',
                label: 'Uncaught Exceptions',
                description: 'Break only on uncaught exceptions',
                default: true
            }
        ];

        this.sendResponse(response);
        this.sendEvent(new InitializedEvent());
    }

    protected async launchRequest(response: DebugProtocol.LaunchResponse, args: LaunchRequestArguments): Promise<void> {
        this._program = args.program;
        this._compilerPath = args.compilerPath || '';
        this._currentFile = args.program;

        if (!this._compilerPath) {
            this.sendErrorResponse(response, 1, 'Compiler path not specified');
            return;
        }

        // Start the debugger process with JSON mode
        this._process = spawn(this._compilerPath, ['--debug-json', this._program], {
            cwd: path.dirname(this._program)
        });

        this._process.stdout?.on('data', (data: Buffer) => {
            this.handleOutput(data.toString());
        });

        this._process.stderr?.on('data', (data: Buffer) => {
            this.sendEvent(new OutputEvent(data.toString(), 'stderr'));
        });

        this._process.on('exit', (code: number) => {
            this.sendEvent(new TerminatedEvent());
        });

        this._process.on('error', (err: Error) => {
            this.sendEvent(new OutputEvent(`Error: ${err.message}\n`, 'stderr'));
            this.sendEvent(new TerminatedEvent());
        });

        // If stopOnEntry, don't send continue command
        if (args.stopOnEntry) {
            this.sendCommand({ command: 'step' });
        } else {
            this.sendCommand({ command: 'continue' });
        }

        this.sendResponse(response);
    }

    private handleOutput(data: string): void {
        this._responseBuffer += data;

        // Try to parse complete JSON messages
        const lines = this._responseBuffer.split('\n');
        this._responseBuffer = lines.pop() || '';

        for (const line of lines) {
            if (line.trim()) {
                try {
                    const msg = JSON.parse(line);
                    this.handleDebugMessage(msg);
                } catch (e) {
                    // Regular output, not JSON
                    this.sendEvent(new OutputEvent(line + '\n', 'stdout'));
                }
            }
        }
    }

    private handleDebugMessage(msg: any): void {
        switch (msg.event) {
            case 'paused':
                this._currentLine = msg.line || 1;
                this._currentFile = msg.file || this._program;
                this.sendEvent(new StoppedEvent(msg.reason || 'step', SchemeDebugSession.THREAD_ID));
                break;

            case 'breakpoint':
                this._currentLine = msg.line || 1;
                this._currentFile = msg.file || this._program;
                this.sendEvent(new StoppedEvent('breakpoint', SchemeDebugSession.THREAD_ID));
                break;

            case 'stopped':
                this._currentLine = msg.line || 1;
                this._currentFile = msg.file || this._program;
                // Handle different stop reasons
                let reason = msg.reason || 'step';
                if (reason === 'exception') {
                    this.sendEvent(new StoppedEvent('exception', SchemeDebugSession.THREAD_ID, msg.message));
                } else if (reason === 'data breakpoint') {
                    this.sendEvent(new StoppedEvent('data breakpoint', SchemeDebugSession.THREAD_ID));
                } else {
                    this.sendEvent(new StoppedEvent(reason, SchemeDebugSession.THREAD_ID));
                }
                break;

            case 'terminated':
                this.sendEvent(new TerminatedEvent());
                break;

            case 'output':
                this.sendEvent(new OutputEvent(msg.text || '', msg.category || 'stdout'));
                break;

            case 'stack':
                this._callStack = msg.frames || [];
                break;

            case 'variables':
                if (msg.variables) {
                    this._variables.clear();
                    for (const v of msg.variables) {
                        this._variables.set(v.name, v.value);
                    }
                }
                break;

            case 'watch':
                // Handle watch expression updates
                if (msg.watches) {
                    this._watches = msg.watches;
                }
                break;
        }
    }

    private sendCommand(cmd: any): void {
        if (this._process?.stdin) {
            this._process.stdin.write(JSON.stringify(cmd) + '\n');
        }
    }

    protected setBreakPointsRequest(response: DebugProtocol.SetBreakpointsResponse, args: DebugProtocol.SetBreakpointsArguments): void {
        const clientPath = args.source.path || '';
        const clientBreakpoints = args.breakpoints || [];

        // Clear existing breakpoints for this file
        const oldBreakpoints = this._breakpoints.get(clientPath) || [];
        for (const bp of oldBreakpoints) {
            this.sendCommand({ command: 'delete', breakpoint: bp.id });
        }

        // Set new breakpoints
        const breakpoints: Breakpoint[] = [];
        const newBreakpoints: SchemeBreakpoint[] = [];

        for (const sbp of clientBreakpoints) {
            const id = ++this._breakpointId;
            const line = sbp.line;
            const condition = sbp.condition;

            // Send breakpoint command with optional condition
            if (condition) {
                this.sendCommand({ command: 'break', file: clientPath, line: line, condition: condition });
            } else {
                this.sendCommand({ command: 'break', file: clientPath, line: line });
            }

            const bp: SchemeBreakpoint = { id, line, verified: true, condition };
            newBreakpoints.push(bp);

            const vsBp = new Breakpoint(true, line);
            breakpoints.push(vsBp);
        }

        this._breakpoints.set(clientPath, newBreakpoints);

        response.body = { breakpoints };
        this.sendResponse(response);
    }

    protected setExceptionBreakPointsRequest(response: DebugProtocol.SetExceptionBreakpointsResponse, args: DebugProtocol.SetExceptionBreakpointsArguments): void {
        const filters = args.filters || [];

        if (filters.includes('all')) {
            this._exceptionBreakMode = 'always';
            this.sendCommand({ command: 'catch', mode: 'all' });
        } else if (filters.includes('uncaught')) {
            this._exceptionBreakMode = 'uncaught';
            this.sendCommand({ command: 'catch', mode: 'uncaught' });
        } else {
            this._exceptionBreakMode = 'never';
            this.sendCommand({ command: 'catch', mode: 'none' });
        }

        this.sendResponse(response);
    }

    protected threadsRequest(response: DebugProtocol.ThreadsResponse): void {
        response.body = {
            threads: [
                new Thread(SchemeDebugSession.THREAD_ID, 'Main Thread')
            ]
        };
        this.sendResponse(response);
    }

    protected stackTraceRequest(response: DebugProtocol.StackTraceResponse, args: DebugProtocol.StackTraceArguments): void {
        // Request stack from debugger
        this.sendCommand({ command: 'backtrace' });

        // Build stack frames from cached data
        const frames: StackFrame[] = [];

        // Current location
        frames.push(new StackFrame(
            0,
            this._callStack[0]?.name || '<top>',
            new Source(path.basename(this._currentFile), this._currentFile),
            this._currentLine
        ));

        // Add call stack frames
        for (let i = 0; i < this._callStack.length; i++) {
            const frame = this._callStack[i];
            frames.push(new StackFrame(
                i + 1,
                frame.name,
                new Source(path.basename(frame.file), frame.file),
                frame.line
            ));
        }

        response.body = {
            stackFrames: frames,
            totalFrames: frames.length
        };
        this.sendResponse(response);
    }

    protected scopesRequest(response: DebugProtocol.ScopesResponse, args: DebugProtocol.ScopesArguments): void {
        response.body = {
            scopes: [
                new Scope('Local', 1, false),
                new Scope('Global', 2, true)
            ]
        };
        this.sendResponse(response);
    }

    protected variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments): void {
        // Request variables from debugger
        this.sendCommand({ command: 'locals' });

        const variables: DebugProtocol.Variable[] = [];

        for (const [name, value] of this._variables) {
            variables.push({
                name: name,
                value: String(value),
                variablesReference: 0
            });
        }

        response.body = { variables };
        this.sendResponse(response);
    }

    protected evaluateRequest(response: DebugProtocol.EvaluateResponse, args: DebugProtocol.EvaluateArguments): void {
        this.sendCommand({ command: 'print', expression: args.expression });

        // For now, return a placeholder - the actual result comes async
        response.body = {
            result: `<evaluating ${args.expression}>`,
            variablesReference: 0
        };
        this.sendResponse(response);
    }

    protected continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments): void {
        this.sendCommand({ command: 'continue' });
        this.sendResponse(response);
    }

    protected nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments): void {
        this.sendCommand({ command: 'next' });
        this.sendResponse(response);
    }

    protected stepInRequest(response: DebugProtocol.StepInResponse, args: DebugProtocol.StepInArguments): void {
        this.sendCommand({ command: 'step' });
        this.sendResponse(response);
    }

    protected stepOutRequest(response: DebugProtocol.StepOutResponse, args: DebugProtocol.StepOutArguments): void {
        this.sendCommand({ command: 'finish' });
        this.sendResponse(response);
    }

    protected disconnectRequest(response: DebugProtocol.DisconnectResponse, args: DebugProtocol.DisconnectArguments): void {
        if (this._process) {
            this.sendCommand({ command: 'quit' });
            this._process.kill();
            this._process = null;
        }
        this.sendResponse(response);
    }

    protected configurationDoneRequest(response: DebugProtocol.ConfigurationDoneResponse, args: DebugProtocol.ConfigurationDoneArguments): void {
        this.sendResponse(response);
    }
}

export class SchemeDebugAdapterFactory implements vscode.DebugAdapterDescriptorFactory {
    createDebugAdapterDescriptor(session: vscode.DebugSession): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
        return new vscode.DebugAdapterInlineImplementation(new SchemeDebugSession());
    }
}
