/*
 * Ave.h
 *
 *  Created on: Sep 22, 2011
 *      Author: thomasstorm
 */

#ifndef AVE_H_
#define AVE_H_

#include <stdexcept>
#include <vector>

#include "../modules/BasicModule.h"
#include "../core/Boost.h"

using std::logic_error;

/**
 * AVE - the SYN L2 averaging module.
 *
 * @author Thomas Storm
 */
class Ave: public BasicModule {
public:
	Ave();

	virtual ~Ave();

	void start(Context& context);
	void stop(Context& context);
	void process(Context& context);

private:
	friend class AveTest;

	static const int8_t AVERAGING_FACTOR;

    const Grid* averagedGrid;
    const Segment* collocatedSegment;
    const Accessor* synFlags;

    Segment* averagedSegment;
    vector<string> variables;
    Accessor* averagedSynFlags;
    long minCollocatedL;

    void averageVariables(Context& context, long firstL, long lastL);
    void averageFlags(Context& context, long firstL, long lastL);
    void averageLatLon(Context& context, long firstL, long lastL);
    bool isFillValue(const string& variableName, const long index) const;
    double getValue(const string& variableName, const long index) const;
    uint16_t getFlagFillValue(Context& context);
    void addFlagsVariable(Context& context);
    void setupVariables(Context& context);
    bool matches(const string& varName);
};

#endif /* AVE_H_ */
