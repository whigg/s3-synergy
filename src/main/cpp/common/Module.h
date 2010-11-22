/* 
 * File:   Module.h
 * Author: thomass
 *
 * Created on November 17, 2010, 3:35 PM
 */

#ifndef MODULE_H
#define	MODULE_H

#include "ProcessorContext.h"
#include "Segment.h"

class Module {
public:
    virtual void start() = 0;
    virtual void stop() = 0;

    // virtual string getTargetSegmentId() = 0;
    virtual Segment* processSegment(ProcessorContext& context) = 0;
};

#endif	/* MODULE_H */

