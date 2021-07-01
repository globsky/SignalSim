//----------------------------------------------------------------------
// XmlTrajectory.h:
//   Declaration of functions to put XML element tree to trajectory
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __XML_TRAJECTORY_H__
#define __XML_TRAJECTORY_H__

#include "XmlElement.h"
#include "Trajectory.h"

BOOL AssignStartTime(CXmlElement *Element, UTC_TIME &UtcTime);
BOOL SetTrajectory(CXmlElement *Element, LLA_POSITION &StartPos, LOCAL_SPEED &StartVel, CTrajectory &Trajectory);

#endif // __XML_TRAJECTORY_H__
