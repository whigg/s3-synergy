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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * File:   Context.h
 * Author: ralf
 *
 * Created on December 20, 2010, 12:34 PM
 */

#ifndef CONTEXT_H
#define	CONTEXT_H

#include <map>
#include <vector>

#include "Constants.h"
#include "Segment.h"
#include "Object.h"

using std::map;
using std::vector;

class Dictionary;
class JobOrder;
class Logging;
class Module;

/**
 * Represents the context of a processing.
 */
class Context {
public:
    /**
     * Constructs a new instance of this class.
     */
    Context();
    
    /**
     * Destructor.
     */
    ~Context();

    /**
     * Adds an object to the context.
     * @param object The object.
     * @return a reference to the object added.
     */
    Object& addObject(Object& object);

    /**
     * Adds a segement to the context.
     * @param id The segment ID.
     * @param sizeL The size of the segment's row index dimension.
     * @param sizeM The size of the segment's column index dimension.
     * @param sizeK The size of the segment's camara index dimension.
     * @return a reference to the segment added.
     */
    Segment& addSegment(const string& id, size_t sizeL, size_t sizeM = Constants::N_DET_CAM, size_t sizeK = Constants::N_CAM);

    /**
     * Returns the dictionary.
     * @return the dictionary.
     */
    Dictionary& getDictionary() const;

    /**
     * Returns the job order.
     * @return the job order.
     */
    JobOrder& getJobOrder() const;

    /**
     * Returns the logging.
     * @return the logging.
     */
    Logging& getLogging() const;

    /**
     * Returns the object associated with the supplied object ID.
     * @param id The object ID.
     * @return the object associated with {@code id}.
     */
    Object& getObject(const string& id) const;

    /**
     * Returns the segment associated with the supplied segment ID.
     * @param id The segment ID.
     * @return the segment associated with {@code id}.
     */
    Segment& getSegment(const string& id) const;

    /**
     * Returns the index of the minimum row in a segment, which will be
     * processed in this context.
     * @param segment The segment.
     * @return the index of the minimum row, which will be processed.
     */
    size_t getMinL(const Segment& segment) const;

    /**
     * Sets the index of the minimum row in a segment, which will be
     * processed in this context.
     * @param segment The segment.
     * @param l The index of the minimum row, which will be processed.
     */
    void setMinL(const Segment& segment, size_t l);

    /**
     * Returns the index of the maximum row in a segment, which will be
     * processed in this context.
     * @param segment The segment.
     * @return the index of the maximum row, which will be processed.
     */
    size_t getMaxL(const Segment& segment) const;

    /**
     * Sets the index of the maximum row in a segment, which will be
     * processed in this context.
     * @param segment The segment.
     * @param l The index of the maximum row, which will be processed.
     */
    void setMaxL(const Segment& segment, size_t l);

    /**
     * Returns the index of the maximum row in a segment, which has been
     * computed by a certain module.
     * @param segment The segment.
     * @param module The module.
     * @return the index of the maximum row in {@code segment}, which has
     *         been computed by {@code module}.
     */
    size_t getMaxLComputed(const Segment& segment, const Module& module) const;

    /**
     * Sets the index of the maximum row in a segment, which has been
     * computed by a certain module.
     * @param segment The segment.
     * @param module The module.
     * @param l The index of the maximum row in {@code segment}, which has
     *          been computed by {@code module}.
     */
    void setMaxLComputed(const Segment& segment, const Module& module, size_t l);

    /**
     * Returns the index of the minimum row in a segment, which is required for
     * processing any block of rows starting with a certain row index.
     * @param segment The segment.
     * @param l The row index.
     * @return the index of the minimum row in {@code segment}, which is
     *         required for processing a block of rows starting with row
     *         index {@code l}.
     */
    size_t getMinLRequired(const Segment& segment, size_t l) const;

    bool hasObject(const string& id) const;
    bool hasSegment(const string& id) const;

    bool hasMaxLineComputed(const Segment& segment, const Module& module) const;
    bool hasMinLineRequired(const Segment& segment) const;

private:
    void removeObject(Object& object);
    void removeSegment(Segment& segment);

    map<string, Object*> objectMap;
    vector<Object*> objectList;

    map<string, Segment*> segmentMap;
    vector<Segment*> segmentList;
};

#endif	/* CONTEXT_H */
