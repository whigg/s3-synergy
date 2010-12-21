/* 
 * Copyright (C) 2010 by Brockmann Consult (info@brockmann-consult.de)
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * File:   Logger.h
 * Author: thomass
 *
 * Created on November 24, 2010, 4:08 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <fstream>
#include <string>
#include <vector>

#include "../core/Logging.h"

using std::vector;
using std::ofstream;

class Logger : public virtual Logging {
public:
    ~Logger();
    static Logger* get();
    void debug(const string& message, const string& moduleName,
            const string& processorVersion = Constants::PROCESSOR_VERSION);
    void info(const string& message, const string& moduleName,
            const string& processorVersion = Constants::PROCESSOR_VERSION);
    void progress(const string& message, const string& moduleName,
            const string& processorVersion = Constants::PROCESSOR_VERSION);
    void warning(const string& message, const string& moduleName,
            const string& processorVersion = Constants::PROCESSOR_VERSION);
    void error(const string& message, const string& moduleName,
            const string& processorVersion = Constants::PROCESSOR_VERSION);
    void setOutLogLevel(const string& outLogLevel);
    void setErrLogLevel(const string& errLogLevel);
    vector<string*> getMessageBuffer() const;
    void init(const string& orderId);
    void close();
protected:
    Logger();
private:
    string createMessageHeader(const string& moduleName, const string& moduleVersion);
    void logToError(const string& message, const string& moduleName, const string& moduleVersion);
    void logToStdout(const string& message, const string& moduleName, const string& moduleVersion, const string& logType);
    string getTimeString();
    static Logger *instance;
    string outLogLevel;
    string errLogLevel;
    string orderId;
    Logger(const Logger&);
    ofstream logFile;
};

#endif	/* LOGGER_H */

