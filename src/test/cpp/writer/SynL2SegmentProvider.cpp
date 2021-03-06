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

#include <algorithm>

#include "../../../main/cpp/util/Utils.h"

#include "SynL2SegmentProvider.h"

using std::min;

using boost::lexical_cast;

SynL2SegmentProvider::SynL2SegmentProvider() :
		AbstractModule("SYN_L2_SEGMENT_PROVIDER") {
}

SynL2SegmentProvider::~SynL2SegmentProvider() {
}

void SynL2SegmentProvider::start(Context& context) {
	size_t segmentLineCount = 400;
	const string segmentLineCountString = context.getJobOrder().getIpfConfiguration().getDynamicProcessingParameter("Segment_Line_Count");
	if (!segmentLineCountString.empty()) {
		segmentLineCount = lexical_cast<size_t>(segmentLineCountString);
	}
	context.getLogging().info("segment line count is " + lexical_cast<string>(segmentLineCount), getId());

	vector<SegmentDescriptor*> segmentDescriptors = context.getDictionary().getProductDescriptor(Constants::PRODUCT_SY2).getSegmentDescriptors();

	foreach(SegmentDescriptor* segDesc, segmentDescriptors) {
	    vector<VariableDescriptor*> variableDescriptors = segDesc->getVariableDescriptors();
	    foreach(VariableDescriptor* varDesc, variableDescriptors) {
	        const string& segmentName = segDesc->getName();
	        if (!context.hasSegment(segmentName)) {
	            valarray<size_t> dimensionSizes = Utils::getDimensionSizes(varDesc);
	            context.addSwathSegment(segmentName, min(segmentLineCount, dimensionSizes[1]), dimensionSizes[2], dimensionSizes[0], 0, dimensionSizes[1] - 1);
	        }
	        Segment& segment = context.getSegment(segmentName);
	        if (!segment.hasVariable(varDesc->getName())) {
	            segment.addVariable(*varDesc);
	        }
	    }
	}
}

void SynL2SegmentProvider::stop(Context& context) {
	reverse_foreach(const string id, context.getSegmentIds()) {
	    context.removeSegment(id);
	}
}

void SynL2SegmentProvider::process(Context& context) {
	vector<SegmentDescriptor*> segmentDescriptors = context.getDictionary().getProductDescriptor(Constants::PRODUCT_SY2).getSegmentDescriptors();
	foreach(SegmentDescriptor* segDesc, segmentDescriptors) {
	    Segment& segment = context.getSegment(segDesc->getName());
	    context.setLastComputedL(segment, *this, segment.getGrid().getMaxInMemoryL());
	}
}
