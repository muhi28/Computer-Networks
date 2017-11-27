/*
 * Txc18.cc
 *
 *  Created on: 27.11.2017
 *      Author: muhamed
 */


#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "msgData/tictoc18_m.h"

using namespace omnetpp;

class Txc18 : public cSimpleModule
{

private:
    simsignal_t arrivalSignal;

protected:
    virtual TicTocMsg18 *generateMessage();
    virtual void forwardMessage(TicTocMsg18 *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};
Define_Module(Txc18);

void Txc18::initialize()
{
    arrivalSignal = registerSignal("arrival");
    // Module 0 sends the first message
    if (getIndex() == 0) {
        // Boot the process scheduling the initial message as a self-message.
        TicTocMsg18 *msg = generateMessage();
        scheduleAt(0.0, msg);
    }
}
void Txc18::handleMessage(cMessage *msg)
{
    TicTocMsg18 *ttmsg = check_and_cast<TicTocMsg18 *>(msg);
    if (ttmsg->getDestination() == getIndex()) {
        // Message arrived
        int hopcount = ttmsg->getHopCount();
        // send a signal
        emit(arrivalSignal, hopcount);
        if (hasGUI()) {
            char label[50];
            // Write last hop count to string
            sprintf(label, "last hopCount = %d", hopcount);
            // Get pointer to figure
            cCanvas *canvas = getParentModule()->getCanvas();
            cTextFigure *textFigure = check_and_cast<cTextFigure*>(canvas->getFigure("lasthopcount"));
            // Update figure text
            textFigure->setText(label);
        }
        EV << "Message " << ttmsg << " arrived after " << hopcount << " hops.\n";
        bubble("ARRIVED, starting new one!");
        delete ttmsg;
        // Generate another one.
        EV << "Generating another message: ";
        TicTocMsg18 *newmsg = generateMessage();
        EV << newmsg << endl;
        forwardMessage(newmsg);
    }
    else {
        // We need to forward the message.
        forwardMessage(ttmsg);
    }
}
TicTocMsg18 *Txc18::generateMessage()
{
    // Produce source and destination addresses.
    int src = getIndex();
    int n = getVectorSize();
    int dest = intuniform(0, n-2);
    if (dest >= src)
        dest++;
    char msgname[20];
    sprintf(msgname, "tic-%d-to-%d", src, dest);
    // Create message object and set source and destination field.
    TicTocMsg18 *msg = new TicTocMsg18(msgname);
    msg->setSource(src);
    msg->setDestination(dest);
    return msg;
}
void Txc18::forwardMessage(TicTocMsg18 *msg)
{
    // Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);
    // Same routing as before: random gate.
    int n = gateSize("gate");
    int k = intuniform(0, n-1);
    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    send(msg, "gate$o", k);
}


