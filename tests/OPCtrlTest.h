/***********************************************************************************************/
/* YOUNGLINGS ASSIGNMENT #3: tests with RTF
/***********************************************************************************************/

/* Author: Gabriele Nava 
   Date: gen 2017 */

/* Define the identifier */
#ifndef _OPCtrl_TEST_H_
#define _OPCtrl_TEST_H_

/* Include yarp test case and os */
#include <rtf/yarp/YarpTestCase.h>
#include <yarp/os/all.h>

/* setting namespaces */
using namespace yarp::os;

/* Create the class OPTest which will be used for testing the code */
class OPCtrlTest : public YarpTestCase 
{
protected:
    /* RpcServer: a port specialized as an RPC server */
    RpcClient rpcPort; 

public:
    /* Variables for the test */
    OPCtrlTest();
    virtual ~OPCtrlTest();
    virtual bool setup(yarp::os::Property& property);
    virtual void tearDown();
    virtual void run();
};
#endif //_OPCtrl_TEST_H
