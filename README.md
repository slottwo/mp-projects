# Multiprocessing Projects

## Install

* Open MPI

Remember to give the scripts running auth:
`chmod +x *.sh`

## Debug (VsCode) - Linux

### Setup

You need to install `gdb` debugger.

Add the content to the `.vscode/` setup files:

`tasks.json`:
```json
{
    "type": "shell",
    "label": "RUN MPI",
    "command": "${workspaceFolder}/run_mpi.sh",
    "args": [
        "${fileBasenameNoExtension}",
        "2",
        "-D"
    ],
    "options": {
        "cwd": "${workspaceFolder}"
    },
    "icon": {
        "color": "terminal.ansiMagenta",
        "id": "rocket"
    },
    "promptOnClose": false,
    "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": false,
        "clear": true
    },
    "problemMatcher": [
        "$gcc"
    ]
}
```

`c_cpp_properties.json`:
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/lib/openmpi" /* Use your openmpi local path */
            ],
            "defines": [],
            "compilerPath": "mpicc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "gcc-x64"
        }
    ],
    "version": 4
}
```

`launch.json`:
```json
{
    "name": "gdb - MPI debug",
    "type": "cppdbg",
    "request": "attach",
    "processId": "${command:pickProcess}",
    "program": "${workspaceRoot}/.bin/program.out",
    "MIMode": "gdb",
    "setupCommands": [
        {
            "description": "Enable pretty-printing for gdb",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
        }
    ],
    "miDebuggerPath": "/usr/bin/gdb"
}
```

### Running

* First you need to build the program:
    > 1. Vscode command pallet `> Tasks: Run task`
    > 2. Select `> RUN MPI`
* Then you will attach to the server:
    > 1. Open the file you gonna build in Vscode File explorer
    > 2. Vscode command pallet `> Debug: Start Debugging` / `F5`
    > 3. Select the program.out instance to attach to
    > 4. Set the variable `attached` to 1 to start debug section
    > 5. Vscode command pallet `> Workspace: Duplicate As Workspace in New Window`
    > 6. Select the next program.out instance to attach
    > 7. Set the variable `attached` to 1 to start debug section in the second thread
