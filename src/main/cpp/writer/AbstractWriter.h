/*
 * File:   SynL2Writer.h
 * Author: thomass
 *
 * Created on November 17, 2010, 5:19 PM
 */

#ifndef ABSTRACTWRITER_H
#define	ABSTRACTWRITER_H

#include <map>
#include <valarray>

#include "../modules/BasicModule.h"

using std::map;
using std::valarray;

class AbstractWriter: public BasicModule {
public:
	AbstractWriter(const string& name);
	virtual ~AbstractWriter();
	void process(Context& context);
	void start(Context& context);
	void stop(Context& context);

protected:
    path targetDirPath;

    map<string, int> ncFileIdMap;
    map<string, valarray<int> > ncDimIdMap;
    map<string, int> ncVarIdMap;

	virtual const string& getProductDescriptorIdentifier() const = 0;
	virtual const string& getSafeManifestName() const = 0;
	virtual const vector<SegmentDescriptor*> getSegmentDescriptors(const Context& context) const = 0;
	virtual void writeCommonVariables(const Context& context) = 0;
	virtual void defineCommonDimensions(int fileId, bool isSubsampled) = 0;
	virtual void defineCommonVariables(int fileId, bool isSubsampled) = 0;

	const ProductDescriptor& getProductDescriptor(const Context& context) const;
private:
	friend class AbstractWriterTest;
	void defineNcVar(const ProductDescriptor& productDescriptor, const SegmentDescriptor& segmentDescriptor, const VariableDescriptor& variable,
	        const Grid& grid, bool isSubsampled);
	void defineDimensions(const int fileId, const string& name, const vector<Dimension*>& dimensions, const Grid& grid, valarray<int>& dimIds);
	void putGlobalAttributes(int fileId, const VariableDescriptor& variableDescriptor, const vector<Attribute*>& attributes) const;
	void createSafeProduct(const Context& context);
	void copyTemplateFiles() const;
	string readManifest() const;
	void setStartTime(const Context& context, string& manifest) const;
	void setChecksums(string& manifest) const;
	void writeManifest(string& manifest) const;
	void removeManifestTemplate() const;
	void replaceString(const string& toReplace, const string& replacement, string& input) const;
	string getMd5Sum(const string& file) const;
	bool isSubsampled(const string& segmentName);
};

#endif	/* ABSTRACTWRITER_H */
