/*
 * Configuration.h
 *
 *  Created on: Nov 9, 2010
 *      Author: thomass
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

class Configuration {
private:
    string processorName;
    string version;
    string standardLogLevel;
    string errorLogLevel;
    bool test;
    bool breakpointEnable;
    string acquisitionStation;
    string processingStation;
    string sensingTimeStart;
    string sensingTimeStop;
    vector<string> configFileNames;

    string boolToString(bool input);
public:

    Configuration();
    ~Configuration();

    string getProcessorName();
    void setProcessorName(string processorName);
    
    void print();
    void setVersion(string version);
    string getVersion() const;
    void setConfigFileNames(vector<string> configFileNames);
    vector<string> getConfigFileNames() const;
    void setSensingTimeStop(string sensingTimeStop);
    string getSensingTimeStop() const;
    void setSensingTimeStart(string sensingTimeStart);
    string getSensingTimeStart() const;
    void setProcessingStation(string processingStation);
    string getProcessingStation() const;
    void setAcquisitionStation(string acquisitionStation);
    string getAcquisitionStation() const;
    void setBreakpointEnable(bool breakpointEnable);
    bool isBreakpointEnable() const;
    void setTest(bool test);
    bool isTest() const;
    void setErrorLogLevel(string errorLogLevel);
    string getErrorLogLevel() const;
    void setStandardLogLevel(string standardLogLevel);
    string getStandardLogLevel() const;
};

#endif /* CONFIGURATION_H_ */
