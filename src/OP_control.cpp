/***********************************************************************************************/
/* YOUNGLINGS ASSIGNMENT #2: operational control with iCub
/***********************************************************************************************/

/* Author: Gabriele Nava 
   Date: dec 2016 */

/* Libraries: include the string output command, and all the yarp and iCub-related libraries */
#include <string>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

/* Namespace definition */
using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;

/***********************************************************************************************/
/* Super-class definition: CtrlModule
/***********************************************************************************************/

/* RfModule: call the resource finder (finds config files and extrernal resources) */
class CtrlModule: public RFModule
{
protected:
    /* PolyDriver: a container for a device driver */
    PolyDriver drvArm, drvGaze, drvPosArm, drvPosTorso;
    /* ICartesianControl: interface for a cartesian controller */
    ICartesianControl *iarm;
    /* IGazeControl: interface for a gaze controller */
    IGazeControl *igaze;
    /* Joint interfaces */
    IEncoders *iEncArm, *iEncTorso;
    IPositionControl *iPosArm, *iPosTorso;
    /* Switch control modes */
    IControlMode2 *ctrlModeArm, *ctrlModeTorso;
    /* BufferedPort: a way to send/receive messages in background from a port */
    BufferedPort<ImageOf<PixelRgb> > imgLPortIn,imgRPortIn;
    /* Port: a way to send/receive messages and info from a device */
    Port imgLPortOut,imgRPortOut;
    /* RpcServer: a port specialized as an RPC server */
    RpcServer rpcPort;    
    /* Mutex: the thread that is the owner of this can lock/unlock a resource */  
    Mutex mutex;
    /* Vector: a vector of elements */
    Vector cogL,cogR;
    /* bool: boolean variables */
    bool okL,okR;
    /* Other variables for the home position */
    int n_jarm,n_jtorso;
    Vector encArm0,encTorso0;

    /* FUNCTION: get the CoG from an image */
    bool getCOG(ImageOf<PixelRgb> &img, Vector &cog)
    {
        int xMean=0;
        int yMean=0;
        int ct=0;

        /* Loop cycle to find the center of the ball in the image from the eye */
        for (int x=0; x<img.width(); x++)
        {
            for (int y=0; y<img.height(); y++)
            {
                PixelRgb &pixel=img.pixel(x,y);
                if ((pixel.b>5.0*pixel.r) && (pixel.b>5.0*pixel.g))
                {
                    xMean+=x;
                    yMean+=y;
                    ct++;
                }
            }
        }
        /* If it found something blue, then rewrite cog and return true; otherwise return false */
        if (ct>0)
        {
            cog.resize(2);
            cog[0]=xMean/ct;
            cog[1]=yMean/ct;
            return true;
        }
        else
            return false;
    }

    /* TODO: a function to find the 3D ball position */
    Vector retrieveTarget3D(const Vector &cogL, const Vector &cogR)
    {
        /* I will use the function "triangulate3DPoint" that is inside the class "IGazeControl" */
        Vector x(3);
        igaze->triangulate3DPoint(cogL,cogR,x);
        return x;
    }

    /* TODO: a function to let the robot fixate the ball*/
    void fixate(const Vector &x)
    {
        /* Set tracking mode on */
        igaze->setTrackingMode(1);	
        /* Look at the ball CoG */
        igaze->lookAtFixationPoint(x); 
        /* Wait until it finishes the job */
        const int period=0.1;
        const int Tlimit=10;             
        igaze->waitMotionDone(period,Tlimit);  
    }

    /* TODO: a function for computing the hand orientation w.r.t. the root link */
    Vector computeHandOrientation()
    {
        /* Define a proper rotation matrix: a rotation of 90 [deg] around x-axis and 
           and a rotaton of 180 [deg] around z-axis (in the root frame) */  
        Matrix R(3,3);
        R(0,0)=-1.0;  R(0,1)= 0.0;  R(0,2)= 0.0; 
        R(1,0)= 0.0;  R(1,1)= 0.0;  R(1,2)=-1.0;   
        R(2,0)= 0.0;  R(2,1)=-1.0;  R(2,2)= 0.0;   
        /* This is the hand orientation in quaternions */
        Vector o0=dcm2axis(R);
        return o0;
    }

    /* TODO: a function for approaching the ball */
    bool approachTargetWithHand(const Vector &x, const Vector &o)
    {
        /* I will use the function goToPose */
        Vector xApproach=x; 
        xApproach[1]+=0.075;   
        iarm->goToPose(xApproach,o,1.5);
        /* Verify if it is possible to reach the ball */
        const int period=0.5;
        const int Tlimit=30; 
        if(iarm->waitMotionDone(period,Tlimit))
        {
            return true;
        }
        else
            return false;
    }

    /* TODO: a function for rolling the ball */
    bool makeItRoll(const Vector &x, const Vector &o)
    {
        /* Further limitate the head movements while rolling */ 
        igaze->bindNeckYaw(-0.5,0.5);
        igaze->bindNeckRoll(-0.5,0.5);
        /* Very similar to the previous one; consider merging */
        Vector xApproach(3); 
        xApproach=x; 
        xApproach[1]=x[1]-0.1;
        iarm->goToPose(xApproach,o,0.45);
        /* Verify if it is possible to roll the ball */
        const int period=0.1;
        const int Tlimit=10; 
        if(iarm->waitMotionDone(period,Tlimit)){
            return true;
        }
        else
            return false;
    }

    /* TODO: a function for looking down */
    void look_down()
    {
        /* I will use the function lookAtAbsAngles */
        Vector angle(3);
        angle[0]=+0.0;                   // azimuth-component   [deg]
        angle[1]=-35.0;                  // elevation-component [deg]
        angle[2]=+0.0;                   // vergence-component  [deg]
        /* Check the angle */
        yInfo("looking at angle = (%s)",angle.toString(3,3).c_str());
        igaze->lookAtAbsAngles(angle);
        /* Wait until it finishes the job */
        const int period=0.1;
        const int Tlimit=10;             
        igaze->waitMotionDone(period,Tlimit); 
    }

    /* FUNCTION: roll the ball */
    bool roll(const Vector &cogL, const Vector &cogR)
    {
        /* Print the left and right eye CoG position */
        yInfo("detected cogs = (%s) (%s)",
              cogL.toString(0,0).c_str(),cogR.toString(0,0).c_str());

        /* Find the 3D point from left and right CoG */ 
        Vector x=retrieveTarget3D(cogL,cogR);
        yInfo("retrieved 3D point = (%s)",x.toString(3,3).c_str());

        /* Stare at that point */
        fixate(x);
        yInfo("fixating at (%s)",x.toString(3,3).c_str());

        /* Find the hand orientation */
        Vector o=computeHandOrientation();
        yInfo("computed orientation = (%s)",o.toString(3,3).c_str());

        /* Approach the target... */
        if (approachTargetWithHand(x,o))
        {
             yInfo("approached");
        }
        else
             return false;
    
        /* ...and make it roll!! */
        if (makeItRoll(x,o))
        {
             yInfo("roll!");
        }
        else
             return false;

        return true;
    }

    /* TODO: define a function for returning to the home position */ 
    void home()
    {       
        /* Set the reference speed */
        int iarm,itorso;
        Vector tmpArm(n_jarm), tmpTorso(n_jtorso); 
        for (iarm = 0; iarm < n_jarm; iarm++) {
            tmpArm[iarm] = 40.0;
            ctrlModeArm->setControlMode(iarm,VOCAB_CM_POSITION);
        }
        for (itorso = 0; itorso < n_jtorso; itorso++) {
            tmpTorso[itorso] = 40.0;
            ctrlModeTorso->setControlMode(itorso,VOCAB_CM_POSITION);
        }
        iPosArm->setRefSpeeds(tmpArm.data());
        iPosTorso->setRefSpeeds(tmpTorso.data());
        /* Bring the joints back to home position */
        iPosTorso->positionMove(encTorso0.data());
        iPosArm->positionMove(encArm0.data()); 
        Vector encArm(n_jarm);
        iEncArm->getEncoders(encArm.data());
        yInfo("retrieved enc data = (%s)",encArm.toString(5,5).c_str());
        yInfo("retrieved enc0 data = (%s)",encArm0.toString(5,5).c_str());

        /* Wait until it finishes the job */ 
        Vector xStare(3),angle(4);
        const int period=0.1;
        const int Tlimit=30;  

        /* Bring the neck back in home postion */
        igaze->lookAtAbsAngles(angle);
        igaze->waitMotionDone(period,Tlimit); 

        /* Look in front of the robot */
        igaze->lookAtFixationPoint(xStare);
        igaze->waitMotionDone(period,Tlimit); 
    }

public:
    /* FUNCTION: configure the resource finder module */
    bool configure(ResourceFinder &rf)
    {
        /* Opening the Cartesian Interface */
        Property option;
        option.put("device","cartesiancontrollerclient");
        option.put("remote","/icubSim/cartesianController/right_arm");
        option.put("local","/client/right_arm");
        drvArm.open(option);

        if (drvArm.isValid()) 
        {
            drvArm.view(iarm);
        }
        else
            return false;

        /* Opening the Gaze Interface */ 
        Property optionGaze;
        optionGaze.put("device","gazecontrollerclient");
        optionGaze.put("remote","/iKinGazeCtrl");
        optionGaze.put("local","/client/gaze");
        drvGaze.open(optionGaze);

        if (drvGaze.isValid())
        {
            drvGaze.view(igaze);
        }
        else
            return false;

        /* To limitate the head movements while fixating, reduce the range of neck yaw and roll */ 
        igaze->bindNeckYaw(-5,5);
        igaze->bindNeckRoll(-5,5);

        /* Opening the Joint Interface for the arm */
        Property optionsPosArm;
        optionsPosArm.put("device", "remote_controlboard");
        optionsPosArm.put("local", "/client/pos/right_arm");                 
        optionsPosArm.put("remote", "/icubSim/right_arm");
        drvPosArm.open(optionsPosArm);  

        if (drvPosArm.isValid())
        {
            drvPosArm.view(iPosArm);
            drvPosArm.view(iEncArm);
            drvPosArm.view(ctrlModeArm);
        }
        else
            return false;

        /* Opening the Joint Interface for the arm */
        Property optionsPosTorso;
        optionsPosTorso.put("device", "remote_controlboard");
        optionsPosTorso.put("local", "/client/pos/torso");                 
        optionsPosTorso.put("remote", "/icubSim/torso");
        drvPosTorso.open(optionsPosTorso);  

        if (drvPosTorso.isValid())
        {
            drvPosTorso.view(iPosTorso);
            drvPosTorso.view(iEncTorso);
            drvPosTorso.view(ctrlModeTorso);
        }
        else
            return false;  

        /* Evaluate the joint configuration: controlled axis */ 
        iPosArm->getAxes(&n_jarm);
        iPosTorso->getAxes(&n_jtorso);
        encArm0.resize(n_jarm);
        encTorso0.resize(n_jtorso);
        /* Get the initial position */
        iEncArm->getEncoders(encArm0.data());
        iEncTorso->getEncoders(encTorso0.data());
        /* Avoid a joint limit in the arm */
        encArm0[5]=encArm0[5]-1;
    
        /* Activate the torso DOF for the ikin solver */
        Vector CurDOF(10),NewDOF(10);
        /* Get the current DOF vector */
        iarm->getDOF(CurDOF);
        /* Activate the torso DOF */
        NewDOF=CurDOF;
        NewDOF[0]=1;
        NewDOF[1]=1;
        NewDOF[2]=1;
        /* Set the new DOF vector */
        iarm->setDOF(NewDOF,CurDOF);
    
        /* Open eyes and rpc ports */
        imgLPortIn.open("/imgL:i");
        imgRPortIn.open("/imgR:i");

        imgLPortOut.open("/imgL:o");
        imgRPortOut.open("/imgR:o");

        rpcPort.open("/service");
        attach(rpcPort);

        return true;
    }

    /* FUNCTION: interrupt module */
    bool interruptModule()
    {
        imgLPortIn.interrupt();
        imgRPortIn.interrupt();
        return true;
    }

    /* FUNCTION: close module */
    bool close()
    {
        drvArm.close();
        drvGaze.close();
        drvPosArm.close();
        drvPosTorso.close();
        imgLPortIn.close();
        imgRPortIn.close();
        imgLPortOut.close();
        imgRPortOut.close();
        rpcPort.close();
        return true;
    }

    /* FUNCTION: respond module; this define the response to all commands */
    bool respond(const Bottle &command, Bottle &reply)
    {
        string cmd=command.get(0).asString();    
        /* Available commands: help */
        if (cmd=="help")
        {
            reply.addVocab(Vocab::encode("many"));
            reply.addString("Available commands:");
            reply.addString("- look_down");
            reply.addString("- roll");
            reply.addString("- home");
            reply.addString("- quit");
        }
        /* Look down */
        else if (cmd=="look_down")
        {
            look_down();
            reply.addString("Yep! I'm looking down now!");
        }
        /* Roll the ball */
        else if (cmd=="roll")
        {
           /* TODO: if the ball is detected, then proceed with the roll */
           mutex.lock();
           bool okLeft=okL;
           bool okRight=okR;
           Vector cogLeft=cogL;
           Vector cogRight=cogR; 
           mutex.unlock();
           
           if ((okLeft)||(okRight))
           {       
               if (roll(cogLeft,cogRight))
               {
                   reply.addString("Yeah! I've made it roll like a charm!");
               }
               else
                   reply.addString("The ball is too far for me!");
           }
           else
               reply.addString("I don't see any object!");
        }
        /* Home position */
        else if (cmd=="home")
        {
            home();
            reply.addString("I've got the hard work done! Going home.");
        }
        else
            return RFModule::respond(command,reply);

        return true;
    }

    /* FUNCTION: Sync upon incoming images */
    double getPeriod()
    {
        return 0.0;     
    }

    /* FUNCTION: update the module */
    bool updateModule()
    {
        /* get fresh images */
        ImageOf<PixelRgb> *imgL=imgLPortIn.read();
        ImageOf<PixelRgb> *imgR=imgRPortIn.read();

        /* interrupt sequence detected */
        if ((imgL==NULL) || (imgR==NULL)) 
            return false;

        /* compute the center-of-mass of pixels of our color */
        mutex.lock();
        okL=getCOG(*imgL,cogL);
        okR=getCOG(*imgR,cogR);
        mutex.unlock();

        PixelRgb color;
        color.r=255; color.g=0; color.b=0;

        if (okL)
            draw::addCircle(*imgL,color,(int)cogL[0],(int)cogL[1],5);

        if (okR)
            draw::addCircle(*imgR,color,(int)cogR[0],(int)cogR[1],5);

        imgLPortOut.write(*imgL);
        imgRPortOut.write(*imgR);

        return true;
    }
};

/***********************************************************************************************/
/* Main code: verify yarp network, and call the control module and the resource finder
/***********************************************************************************************/
int main()
{
    Network yarp;
    if (!yarp.checkNetwork())
        return 1;

    Time::delay(3);
    CtrlModule mod;
    ResourceFinder rf;
    return mod.runModule(rf);
}

