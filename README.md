# Operational Control with iCub

iCub has a new interesting pastime: make a ball roll on a table. This repo contains a module that can help iCub with its new hobby, by resorting to the [Cartesian Control](http://wiki.icub.org/iCub/main/dox/html/icub_cartesian_interface.html) and the [Gaze Control](http://wiki.icub.org/iCub/main/dox/html/icub_gaze_interface.html).

## Dependencies 

[Yarp](https://github.com/robotology/yarp) and [iCub-main](https://github.com/robotology/icub-main) are required to make this module work properly. Also, if you want to test the softwrare infrastructure, you need to install [RTF](https://github.com/robotology/robot-testing);

## Installation

Tested with Ubuntu 14.04 LTS and 16.04 LTS. This is the first release (V1.0).

- First, open a terminal and compute the following commands:

`git clone https://github.com/vvv-school/test-operational-control-with-icub-gabrielenava`
 
`cd test-operational-control-with-icub-gabrielenava`
 
`mkdir build`
 
`cd build`
 
`ccmake ../`
 
`make`
 
`make install`
 
- To make the blue ball show up within the simulator, you have to turn on the flag **RENDER::objects** in the [**`iCub_parts_activation.ini`**](https://github.com/robotology/icub-main/blob/master/app/simConfig/conf/iCub_parts_activation.ini#L28) file.

## Run the code

- Open `yarpserver` and `yarpmanager`. Currently, it is necessary to open the yarpmanager from the `$SOURCE_DIR/build` directory;
- Launch the `OPcontrol_system` and `OPcontrol_app` applications;
- In another terminal, open the rpc port by typing `yarp rpc /service`;
- Type `help` to retrieve the list of available commands;
- Enjoy!

OPTIONAL: if the CMake option `INSTALL_YARPMANAGER_APPS` is set to ON, the yarpmanager applications are installed on the pc. This may be useful if the program has to be executed several times.

## Tests

It is now also possible to test some funtionalities of the software. In particular, we want to verify if the ball is hit. To do so, the user can run a RTF test from the `$SOURCE_DIR/build` directory by typing the command:

`testrunner --verbose --suit OPCtrlSuit.xml` 



