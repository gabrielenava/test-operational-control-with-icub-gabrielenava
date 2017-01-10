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

/* Updating the test name */
    if(property.check("name"))
        setName(property.find("name").asString());

    string example = property.check("example", Value("default value")).asString();
    RTF_TEST_REPORT(Asserter::format("Use '%s' for the example param!",example.c_str()));
    return true;
}

/* Close fixtures */
void OPCtrlTest::tearDown() {
}

/***********************************************************************************************/
/* Main code: test if the ball is hit or not
/***********************************************************************************************/
void OPCtrlTest::run() {

/* Open two rpc ports; connect them to the /service port and the /icubSim/world port */
    Network yarp;
    portCmd.open("/OPCtrlCmd");
    portBall.open("/OPCtrlBall");

    printf("Trying to connect to %s\n", "/icubSim/world");
    bool okBall=yarp.connect("/OPCtrlBall","/icubSim/world");
    RTF_ASSERT_ERROR_IF(okBall,"Connection with /icubSim/world port failed");

    printf("Trying to connect to %s\n", "/service");
    bool okCmd=yarp.connect("/OPCtrlCmd","/service");
    RTF_ASSERT_ERROR_IF(okCmd,"Connection with /service port failed");

/* Execute the test */
    Bottle cmd,response,positionBefore,positionAfter;

/* Execute the command "look down" */
    Time::delay(3);
    cmd.addString("look_down");
    printf("Sending message... %s\n", cmd.toString().c_str());
    bool okLook=portCmd.write(cmd,response);
    RTF_ASSERT_ERROR_IF(okLook,"Look down command failed");
    printf("Got response: %s\n", response.toString().c_str());     

/* Read the ball position before rolling */
    cmd.clear();
    cmd.addString("world");
    cmd.addString("get");
    cmd.addString("ball");
    bool okBallBef=portBall.write(cmd,positionBefore);
    RTF_ASSERT_ERROR_IF(okBallBef,"Failed to retrieve ball position");
    printf("Got initial ball position: %s\n", positionBefore.toString().c_str());

/* Execute the command "roll" */
    Time::delay(3);
    cmd.clear();
    cmd.addString("roll");
    printf("Sending message... %s\n", cmd.toString().c_str());
    bool okRoll=portCmd.write(cmd,response);
    RTF_ASSERT_ERROR_IF(okRoll,"Roll command failed");
    printf("Got response: %s\n", response.toString().c_str());
    
/* Read the ball position after rolling */
    Time::delay(0.5);
    cmd.clear();
    cmd.addString("world");
    cmd.addString("get");
    cmd.addString("ball");
    bool okBallAft=portBall.write(cmd,positionAfter);
    RTF_ASSERT_ERROR_IF(okBallAft,"Not able to locate the ball after roll command");
    printf("Got ball position after rolling: %s\n", positionAfter.toString().c_str());

/* Retrieve info from the simulator about the ball position: if it changes, the ball is hit */
    RTF_TEST_REPORT("Checking ball position..");
    Time::delay(0.5);
    RTF_TEST_CHECK(positionBefore!=positionAfter,"Ball position test Done");

/* Execute the command "home" */
    Time::delay(3);
    cmd.clear();
    cmd.addString("home");
    printf("Sending message... %s\n", cmd.toString().c_str());
    bool okHome=portCmd.write(cmd,response);
    RTF_ASSERT_ERROR_IF(okHome,"Home command failed");
    printf("Got response: %s\n", response.toString().c_str());

/* If the test fails, print this message */
    RTF_TEST_FAIL_IF(positionBefore!=positionAfter, "The Ball did not move!");      
}


