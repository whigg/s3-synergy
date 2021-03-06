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

#include <sstream>
#include <stdexcept>

#include "Accessors.h"
#include "SwathSegment.h"

using std::invalid_argument;
using std::logic_error;
using std::ostringstream;

SwathSegment::SwathSegment(const string& s, long l, long m, long k, long minL, long maxL) :
		id(s), grid(k, l, m, minL, maxL), accessorMap() {
}

SwathSegment::~SwathSegment() {
}

Accessor& SwathSegment::addVariable(const VariableDescriptor& d) throw (logic_error) {
	return addVariable(d, d.getName());
}

Accessor& SwathSegment::addVariable(const VariableDescriptor& d, const string& targetName) throw (logic_error) {
	using boost::numeric_cast;

	switch (d.getType()) {
	case Constants::TYPE_BYTE:
		return addVariableByte(targetName, numeric_cast<int8_t>(d.getFillValue<int16_t>()), d.getScaleFactor(), d.getAddOffset());
	case Constants::TYPE_UBYTE:
		return addVariableUByte(targetName, numeric_cast<uint8_t>(d.getFillValue<uint16_t>()), d.getScaleFactor(), d.getAddOffset());
	case Constants::TYPE_SHORT:
		return addVariableShort(targetName, d.getFillValue<int16_t>(), d.getScaleFactor(), d.getAddOffset());
	case Constants::TYPE_USHORT:
		return addVariableUShort(targetName, d.getFillValue<uint16_t>(), d.getScaleFactor(), d.getAddOffset());
	case Constants::TYPE_INT:
		return addVariableInt(targetName, d.getFillValue<int32_t>(), d.getScaleFactor(), d.getAddOffset());
	case Constants::TYPE_UINT:
		return addVariableUInt(targetName, d.getFillValue<uint32_t>(), d.getScaleFactor(), d.getAddOffset());
	case Constants::TYPE_LONG:
		return addVariableLong(targetName, d.getFillValue<int64_t>());
	case Constants::TYPE_ULONG:
		return addVariableULong(targetName, d.getFillValue<uint64_t>());
	case Constants::TYPE_FLOAT:
		return addVariableFloat(targetName, d.getFillValue<float>());
	case Constants::TYPE_DOUBLE:
		return addVariableDouble(targetName, d.getFillValue<double>());
	default:
		BOOST_THROW_EXCEPTION( std::invalid_argument("Cannot add variable '" + targetName + "' to segment '" + getId() + "'. Unsupported data type."));
	}
}

Accessor& SwathSegment::addVariable(const string& name, int type, double scaleFactor, double addOffset) throw (logic_error) {
	switch (type) {
	case Constants::TYPE_BYTE:
		return addVariableByte(name, numeric_limits<int8_t>::min(), scaleFactor, addOffset);
	case Constants::TYPE_UBYTE:
		return addVariableUByte(name, 0, scaleFactor, addOffset);
	case Constants::TYPE_SHORT:
		return addVariableShort(name, numeric_limits<int16_t>::min(), scaleFactor, addOffset);
	case Constants::TYPE_USHORT:
		return addVariableUShort(name, 0, scaleFactor, addOffset);
	case Constants::TYPE_INT:
		return addVariableInt(name, numeric_limits<int32_t>::min(), scaleFactor, addOffset);
	case Constants::TYPE_UINT:
		return addVariableUInt(name, 0, scaleFactor, addOffset);
	case Constants::TYPE_LONG:
		return addVariableLong(name);
	case Constants::TYPE_ULONG:
		return addVariableULong(name);
	case Constants::TYPE_FLOAT:
		return addVariableFloat(name);
	case Constants::TYPE_DOUBLE:
		return addVariableDouble(name);
	default:
		BOOST_THROW_EXCEPTION( std::invalid_argument("Cannot add variable '" + name + "' to segment '" + getId() + "'. Unsupported data type."));
	}
}

Accessor& SwathSegment::addVariableAlias(const string& alias, const Segment& segment, const string& name) throw (logic_error) {
	if (segment.hasVariable(name)) {
		if (segment.getGrid().getSizeK() == getGrid().getSizeK() && segment.getGrid().getSizeL() == getGrid().getSizeL() && segment.getGrid().getSizeM() == getGrid().getSizeM()) {
			unique(alias);
			shared_ptr<Accessor> accessor = segment.getSharedAccessor(name);
			accessorMap[alias] = accessor;
			return *accessor;
		}
	}
	BOOST_THROW_EXCEPTION( logic_error("Alias for variable '" + name + "' from segment '" + segment.getId() + "' cannot be added to segment '" + segment.getId() + "'."));
}

Accessor& SwathSegment::addVariableByte(const string& name, int8_t fillValue, double scaleFactor, double addOffset) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new ByteAccessor(grid.getSize(), fillValue, scaleFactor, addOffset));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableDouble(const string& name, double fillValue) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new DoubleAccessor(grid.getSize(), fillValue));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableFloat(const string& name, float fillValue) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new FloatAccessor(grid.getSize(), fillValue));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableInt(const string& name, int32_t fillValue, double scaleFactor, double addOffset) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new IntAccessor(grid.getSize(), fillValue, scaleFactor, addOffset));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableLong(const string& name, int64_t fillValue) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new LongAccessor(grid.getSize(), fillValue));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableShort(const string& name, int16_t fillValue, double scaleFactor, double addOffset) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new ShortAccessor(grid.getSize(), fillValue, scaleFactor, addOffset));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableUByte(const string& name, uint8_t fillValue, double scaleFactor, double addOffset) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new UByteAccessor(grid.getSize(), fillValue, scaleFactor, addOffset));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableUInt(const string& name, uint32_t fillValue, double scaleFactor, double addOffset) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new UIntAccessor(grid.getSize(), fillValue, scaleFactor, addOffset));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableULong(const string& name, uint64_t fillValue) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new ULongAccessor(grid.getSize(), fillValue));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::addVariableUShort(const string& name, uint16_t fillValue, double scaleFactor, double addOffset) throw (logic_error) {
	unique(name);
	try {
		shared_ptr<Accessor> accessor = shared_ptr<Accessor>(new UShortAccessor(grid.getSize(), fillValue, scaleFactor, addOffset));
		accessorMap[name] = accessor;
		accessorList.push_back(accessor);
		return *accessor;
	} catch (std::exception& e) {
		BOOST_THROW_EXCEPTION(e);
	}
}

Accessor& SwathSegment::getAccessor(const string& name) const throw (logic_error) {
	if (!hasVariable(name)) {
		BOOST_THROW_EXCEPTION( logic_error("No accessor for variable '" + name + "'."));
	}
	return *accessorMap.at(name);
}

void SwathSegment::moveForward(long l) throw (logic_error) {
	if (l < grid.getMinInMemoryL()) {
		BOOST_THROW_EXCEPTION( logic_error("Class: " + className + ": l < grid.getStartL()."));
	}
	if (l > grid.getMinInMemoryL() + grid.getSizeL()) {
		BOOST_THROW_EXCEPTION( logic_error("Class: " + className + ": l > grid.getStartL() + grid.getSizeL()."));
	}
	if (l + grid.getSizeL() - 1 > grid.getMaxL()) {
		l = grid.getMaxL() - grid.getSizeL() + 1;
	}
	for (size_t i = 0; i < accessorList.size(); i++) {
		accessorList[i]->shift(l - grid.getMinInMemoryL(), grid.getStrideK(), grid.getStrideL());
	}
	grid.moveForward(l);
}

string SwathSegment::toString() const {
	ostringstream oss;
	oss << className << "[";
	oss << "id = " << getId() << ", ";
	oss << "firstL = " << getGrid().getMinInMemoryL() << ", ";
	oss << "sizeL = " << getGrid().getSizeL() << "]";

	return oss.str();
}

void SwathSegment::unique(const string& name) const throw (logic_error) {
	if (hasVariable(name)) {
		BOOST_THROW_EXCEPTION( logic_error("Variable '" + name + "' has already been added to segment '" + id + "'."));
	}
}

const string SwathSegment::className = "SwathSegment";
