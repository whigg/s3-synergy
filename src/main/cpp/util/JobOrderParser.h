/*
 * Copyright (C) 2012  Brockmann Consult GmbH (info@brockmann-consult.de)
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
 */

#ifndef JOBORDERPARSER_H
#define	JOBORDERPARSER_H

#include "../core/JobOrder.h"

#include "XmlParser.h"

using std::string;

class JobOrderParser {
public:
	JobOrderParser();
	virtual ~JobOrderParser();

	shared_ptr<JobOrder> parse(const string& path);
    const shared_ptr<Logging> createLogging(const string& logFileName) const;

private:
	IpfConfiguration parseIpfConfiguration(const string& path);
	vector<IpfProcessor> parseIpfProcessors(const string& path) const;
	IpfProcessor parseIpfProcessor(const string& path, int index) const;
	vector<BreakpointFile> parseBreakpointFiles(const string& path, const string& baseQuery) const;
	BreakpointFile parseBreakpointFile(const string& path, const string& baseQuery) const;
	vector<Input> parseInputs(const string& path, const string& baseQuery) const;
	Input parseInput(const string& path, const string& baseQuery) const;
	vector<Output> parseOutputs(const string& path, const string& baseQuery) const;
	Output parseOutput(const string& path, const string& baseQuery) const;
    void parseConfigurationFile(string& configFileName, IpfConfiguration& configuration) const;

    string standardLogLevel;
	XmlParser parser;
};

#endif	/* JOBORDERPARSER_H */

