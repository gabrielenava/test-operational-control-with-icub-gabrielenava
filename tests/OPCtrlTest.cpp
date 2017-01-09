/***********************************************************************************************/
/* YOUNGLINGS ASSIGNMENT #3: tests with RTF
/***********************************************************************************************/

/* Author: Gabriele Nava 
   Date: gen 2017 */

/* Include libraries */
#include <rtf/dll/Plugin.h>
#include "OPCtrlTest.h"

/* Setting namespaces */
using namespace std;
using namespace RTF;
using namespace yarp::os;

/* Prepare the plugin */
PREPARE_PLUGIN(OPCtrlTest)
OPCtrlTest::OPCtrlTest() : YarpTestCase("OPCtrlTest") { }
OPCtrlTest::~OPCtrlTest() { }

/* Setup the test */
bool OPCtrlTest::setup(yarp::os::Property &property) {
    // initialization goes here ...

    /* Updating the test name */
    if(property.check("name"))
        setName(property.find("name").asString());

    string example = property.check("example", Value("default value")).asString();
    RTF_TEST_REPORT(Asserter::format("Use '%s' for the example param!",example.c_str()));
    return true;
}
void OPCtrlTest::tearDown() {
    // finalization goes here ...
}

/* Run the code */
void OPCtrlTest::run() {

    Network yarp;
    rpcPort.open("/OPCtrlTest");
    int ct = 0;
    while (true) {
        if (rpcPort.getOutputCount()==0) {
           printf("Trying to connect to %s\n", "/service");
           yarp.connect("/OPCtrlTest","/service");
        } 
        else {
           Bottle cmd;
           cmd.addString("look_down");
           cmd.addInt(ct);
           ct++;
           printf("Sending message... %s\n", cmd.toString().c_str());
           Bottle response;
           rpcPort.write(cmd,response);
           printf("Got response: %s\n", response.toString().c_str());
        }
        Time::delay(1);
    }
}



