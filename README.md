# Multiprocessing Projects

## Install

* Open MPI

Remember to give the scripts running auth:
`chmod +x *.sh`

## Run

Simply call [./run_mpi.sh](./run_mpi.sh) or [./run_mp.sh](./run_mp.sh)

### VsCode running task

You can create a vscode task to automate the running process:

```json
{
    "tasks": [
        {
            "type": "shell",
            "label": "RUN MPI (fast)",
            "command": "${workspaceFolder}/run_mpi.sh",
            "args": [
                "${fileBasenameNoExtension}",
                "2", /* Change this to increase the number of threads */
                /*"-n", */     /* You can optionally set the data size */
                /*"1048576" */ /* to processed, to fasten the execution */
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
                "reveal": "silent",
                "focus": true,
                "panel": "shared",
                "showReuseMessage": false,
                "clear": true
            },
            "problemMatcher": [
                "$gcc"
            ],
            "isBackground": false,
        },
    ],
    "version": "2.0.0"
}
```

## Debug (VsCode) - Linux

### Setup

You need to install `gdb` debugger.

You can optionally run the docker container for this project.

Add the content to the `.vscode/` setup files:

`tasks.json`:
```json
{
    "tasks": [
        {
            "type": "shell",
            "label": "RUN MPI DEBUG",
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
                "reveal": "silent",
                "focus": true,
                "panel": "shared",
                "showReuseMessage": false,
                "clear": true
            },
            "problemMatcher": [
                "$gcc"
            ],
            "isBackground": false,
        },
    ],
    "version": "2.0.0"
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
    "version": "0.2.0",
    "configurations": [
        {
            "name": "gdb - MPI debug",
            "type": "cppdbg",
            "request": "launch",
            //"processId": "${command:pickProcess}",
            "useExtendedRemote": true,
            "program": "${workspaceRoot}/.bin/program.out",
            "miDebuggerServerAddress": "localhost:${input:getPort}",
            "cwd": "${workspaceRoot}",
            "linux": {
                "MIMode": "gdb"
            },
            "postRemoteConnectCommands": [
                {
                    "text": "source ${workspaceRoot}/.vscode/mpi.gdb",
                    "description": "Run script",
                    "ignoreFailures": true
                },
            ],
        }
    ],
    "inputs": [
        {
            "id": "getPort",
            "type": "pickString",
            "options": [
                "1234",
                "1235"
            ],
            "description": "Select the gdb server port to attach to"
        }
    ]
}


```

Add the gdb script `mpi.gdb` to your `.vscode` folder:

```gdb
# This is a comment in the GDB script file

# Set a breakpoint
break gdb_breakpoint
commands
silent
set attached = 1
continue
end
```

### Running

* First you need to build the program:
    > 1. Vscode command pallet `> Tasks: Run task`
    > 2. Select `> RUN MPI DEBUG`
* Then you will attach to the server:
    > 1. Open the file you gonna build in Vscode File explorer
    > 2. Vscode command pallet `> Debug: Start Debugging` / `F5`
    > 3. Choose the PID of the thread you wanna debug to
    > 4. Set a breakpoint where you want to start from
    > 5. Vscode command pallet `> Workspaces: Duplicate As Workspace in New Window`
    > 6. Repeat the process [1-3] (you can also set a breakpoint to this thread -
    > remember to do so before running debugger, otherwise the execution will flow).
    > 7. That's it! Happy bug smash!
