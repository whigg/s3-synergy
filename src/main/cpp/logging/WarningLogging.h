/* 
 * Copyright (C) 2011 by Brockmann Consult (info@brockmann-consult.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation. This program is distributed in the hope it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * File:   WarningLogging.h
 * Author: ralf
 *
 * Created on January 19, 2011, 3:40 PM
 */

#ifndef WARNINGLOGGING_H
#define	WARNINGLOGGING_H

#include "ErrorLogging.h"

class WarningLogging : public ErrorLogging {
public:
    WarningLogging(const string& logFileName);

    virtual ~WarningLogging();

    void warning(const string& message, const string& moduleName) {
        logToStdout(message, moduleName, PROCESSOR_VERSION, "[W]");
    };

protected:
    WarningLogging() {};

};

#endif	/* WARNINGLOGGING_H */
