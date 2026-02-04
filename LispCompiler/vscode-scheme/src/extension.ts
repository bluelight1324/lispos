import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { SchemeDebugAdapterFactory } from './debugAdapter';

let outputChannel: vscode.OutputChannel;

export function activate(context: vscode.ExtensionContext) {
    outputChannel = vscode.window.createOutputChannel('Scheme');

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand('scheme.run', runSchemeFile),
        vscode.commands.registerCommand('scheme.compile', compileSchemeFile),
        vscode.commands.registerCommand('scheme.buildAndRun', buildAndRunScheme),
        vscode.commands.registerCommand('scheme.debug', debugSchemeFile)
    );

    // Register debug adapter
    const factory = new SchemeDebugAdapterFactory();
    context.subscriptions.push(
        vscode.debug.registerDebugAdapterDescriptorFactory('scheme', factory)
    );

    // Register task provider
    context.subscriptions.push(
        vscode.tasks.registerTaskProvider('scheme', new SchemeTaskProvider())
    );

    outputChannel.appendLine('Scheme extension activated');
}

export function deactivate() {
    if (outputChannel) {
        outputChannel.dispose();
    }
}

function getCompilerPath(): string {
    const config = vscode.workspace.getConfiguration('scheme');
    let compilerPath = config.get<string>('compilerPath', '');

    if (!compilerPath) {
        // Try to find compiler relative to workspace
        const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
        if (workspaceFolder) {
            const possiblePaths = [
                path.join(workspaceFolder.uri.fsPath, 'LispCompiler', 'build', 'Release', 'lisp.exe'),
                path.join(workspaceFolder.uri.fsPath, 'LispCompiler', 'build', 'Debug', 'lisp.exe'),
                path.join(workspaceFolder.uri.fsPath, 'build', 'Release', 'lisp.exe'),
                path.join(workspaceFolder.uri.fsPath, 'build', 'Debug', 'lisp.exe'),
            ];

            for (const p of possiblePaths) {
                if (fs.existsSync(p)) {
                    compilerPath = p;
                    break;
                }
            }
        }
    }

    return compilerPath;
}

async function runSchemeFile() {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'scheme') {
        vscode.window.showErrorMessage('Please open a Scheme file first');
        return;
    }

    // Save the file first
    await editor.document.save();

    const filePath = editor.document.uri.fsPath;
    const compilerPath = getCompilerPath();

    if (!compilerPath) {
        vscode.window.showErrorMessage('Scheme compiler not found. Please set scheme.compilerPath in settings.');
        return;
    }

    outputChannel.clear();
    outputChannel.show();
    outputChannel.appendLine(`Running: ${filePath}`);
    outputChannel.appendLine('---');

    const terminal = vscode.window.createTerminal({
        name: 'Scheme',
        cwd: path.dirname(filePath)
    });
    terminal.show();
    terminal.sendText(`"${compilerPath}" "${filePath}"`);
}

async function compileSchemeFile() {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'scheme') {
        vscode.window.showErrorMessage('Please open a Scheme file first');
        return;
    }

    await editor.document.save();

    const filePath = editor.document.uri.fsPath;
    const compilerPath = getCompilerPath();

    if (!compilerPath) {
        vscode.window.showErrorMessage('Scheme compiler not found. Please set scheme.compilerPath in settings.');
        return;
    }

    const basename = path.basename(filePath, path.extname(filePath));
    const dirname = path.dirname(filePath);
    const asmPath = path.join(dirname, `${basename}.asm`);

    outputChannel.clear();
    outputChannel.show();
    outputChannel.appendLine(`Compiling: ${filePath}`);
    outputChannel.appendLine(`Output: ${asmPath}`);
    outputChannel.appendLine('---');

    const terminal = vscode.window.createTerminal({
        name: 'Scheme Compile',
        cwd: dirname
    });
    terminal.show();
    terminal.sendText(`"${compilerPath}" "${filePath}" -o "${asmPath}"`);
}

async function buildAndRunScheme() {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'scheme') {
        vscode.window.showErrorMessage('Please open a Scheme file first');
        return;
    }

    await editor.document.save();

    const filePath = editor.document.uri.fsPath;
    const compilerPath = getCompilerPath();

    if (!compilerPath) {
        vscode.window.showErrorMessage('Scheme compiler not found. Please set scheme.compilerPath in settings.');
        return;
    }

    const config = vscode.workspace.getConfiguration('scheme');
    const ml64Path = config.get<string>('ml64Path', 'ml64.exe');
    const linkPath = config.get<string>('linkPath', 'link.exe');

    const basename = path.basename(filePath, path.extname(filePath));
    const dirname = path.dirname(filePath);
    const asmPath = path.join(dirname, `${basename}.asm`);
    const objPath = path.join(dirname, `${basename}.obj`);
    const exePath = path.join(dirname, `${basename}.exe`);

    outputChannel.clear();
    outputChannel.show();
    outputChannel.appendLine(`Building: ${filePath}`);
    outputChannel.appendLine('---');

    const terminal = vscode.window.createTerminal({
        name: 'Scheme Build',
        cwd: dirname
    });
    terminal.show();

    // Chain the commands
    const commands = [
        `"${compilerPath}" "${filePath}" -o "${asmPath}"`,
        `"${ml64Path}" /c /Fo"${objPath}" "${asmPath}"`,
        `"${linkPath}" /SUBSYSTEM:CONSOLE /OUT:"${exePath}" "${objPath}"`,
        `"${exePath}"`
    ];

    terminal.sendText(commands.join(' && '));
}

async function debugSchemeFile() {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'scheme') {
        vscode.window.showErrorMessage('Please open a Scheme file first');
        return;
    }

    await editor.document.save();

    const filePath = editor.document.uri.fsPath;
    const compilerPath = getCompilerPath();

    if (!compilerPath) {
        vscode.window.showErrorMessage('Scheme compiler not found. Please set scheme.compilerPath in settings.');
        return;
    }

    // Start debugging with the debug adapter
    vscode.debug.startDebugging(undefined, {
        type: 'scheme',
        name: 'Debug Scheme',
        request: 'launch',
        program: filePath,
        compilerPath: compilerPath,
        stopOnEntry: false
    });
}

class SchemeTaskProvider implements vscode.TaskProvider {
    provideTasks(): vscode.ProviderResult<vscode.Task[]> {
        return [];
    }

    resolveTask(task: vscode.Task): vscode.ProviderResult<vscode.Task> {
        return task;
    }
}
