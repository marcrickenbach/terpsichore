# Terpsichore

Terpsichore is a signal animator Eurorack module built on the 
STM32F405RGT microcontroller. At the core of the module is a 
4/8 channel VCA circuit that takes audio or CV inputs and routes
them in interesting ways to the eight outputs on the front panel. 
Each of the four channels include an audio/cv input, a cv input
that controls the vca, and two outputs, x and y. Originally 
conceived of as a four-channel crossfader/panner, the sampled
CV value allows us to controll the animation of the signals in
a variety of useful and playful ways. 

Besides crossfading and panning, other (potential) functions 
include sequential swith, multi-channel morph, CV processing,
amplitude modulation, signal multiplication, sequencing, CV
delay line and much more. The original intent has grown into 
a robust utility module with a very basic interface. Internal
analog switches also allow the user to switch all outputs 
to audio/cv output to be tied directly to the 12-bit DAC so 
that Terpsichore can also be used as a 6 (possibly 8 in the
future) channel LFO, or even audio output (to be determined). 

A USB mini-B input will allow for easy firmware updates.


## Necessary Modules

Listed in order of importance:
1. Pot Module -- ADC pot readings
2. CV Sampling -- ADC inputs. 
3. LED Driver
   This will be simple as all LEDs are driven directly
   from the microcontroller. We'll just need a wrapper
   layer to map these pins. This only pertains to the
   mode buttons as the channel buttons are directly
   tied to the outputs. 
4. Button Driver
   Only two buttons directly into the mcu, but will need
   to handle double press and long press.
5. DAC Driver
   The 12-bit dac handles both the VCA voltages and can
   be used to output directly. 

More. 

## Devcontainer

Will want to containerize this at some point. 

## Debugging

[TODO] Debugging hasn't yet been tested within this devcontainer. Once that 
is tested and all bugs are figured out, update this section with any steps 
necessary to debug with J-Link.

Install the Cortex-Debug extension in the dev-container. We are debugging using
J-Link's GDB server. Launch the GDB server then start a debugging session in VS
Code.

Here is an example of what the launch.json should look like:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "J-Link Debug",
            "type": "cortex-debug",
            "request": "launch",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/build/zephyr.elf",
            "servertype": "external",
            "gdbTarget": "host.docker.internal:2331",
            "armToolchainPath": "/opt/gcc-arm-none-eabi/bin/",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "raw",
            "preLaunchCommands": [
                "set mem inaccessible-by-default off"
            ],
            "overrideLaunchCommands": [
                "monitor halt",
                "monitor reset",
                "load ${workspaceFolder}/build/zephyr.elf",
            ],
            // "svdFile": "${workspaceFolder}/.vscode/STM32F4x.svd",
        },
        {
            "name": "J-Link Attach",
            "type": "cortex-debug",
            "request": "attach",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/build/zephyr.elf",
            "servertype": "external",
            "gdbTarget": "host.docker.internal:2331",
            "armToolchainPath": "/opt/gcc-arm-none-eabi/bin/",
            // "svdFile": "${workspaceFolder}/.vscode/STM32H750x.svd",
            "showDevDebugOutput": "raw",
        },
    ]
}
```

## Resources

[Zephyr Getting Started](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
