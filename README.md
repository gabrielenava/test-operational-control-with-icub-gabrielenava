# Operational Control with iCub

Christmas is approaching, and iCub has a new interesting pastime: make a Christmas ball roll on a table. This repo contains a module that can help iCub with its new hobby, by resorting to the [Cartesian Control](http://wiki.icub.org/iCub/main/dox/html/icub_cartesian_interface.html) and the [Gaze Control](http://wiki.icub.org/iCub/main/dox/html/icub_gaze_interface.html).

## Dependencies 

[Yarp](https://github.com/robotology/yarp) and [iCub-main](https://github.com/robotology/icub-main) are required to make this module work properly.

## Installation

Tested Only with Ubuntu 14.04 LTS. This is the first release (V1.0).

- First, do the following:

`git clone https://github.com/vvv-school/test-operational-control-with-icub-gabrielenava`
 
 `cd test-operational-control-with-icub-gabrielenava`
 
 `mkdir build`
 
 `cd build`
 
 `ccmake ../`
 
 `make`
 
 `make install`
 
- To make the blue ball show up within the simulator, you have to turn on the
flag **RENDER::objects** in the [**`iCub_parts_activation.ini`**](https://github.com/robotology/icub-main/blob/master/app/simConfig/conf/iCub_parts_activation.ini#L28) file.

## Test

- Open `yarpserver` and `yarpmanager`;
- Launch the `OPcontrol_system` and `OPcontrol_app` applications;
- In another terminal, open the rpc port by doing `yarp rpc /service`;
- Type `help` to retrieve the list of available commands;
- Enjoy!




