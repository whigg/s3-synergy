/*
 * XmlParser.cpp
 *
 *  Created on: Nov 8, 2010
 *      Author: thomass
 */

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

#include "../core/Boost.h"

#include "XmlParser.h"

XERCES_CPP_NAMESPACE_USE

XmlParser::XmlParser() {
}

XmlParser::~XmlParser() {
}

const string XmlParser::evaluateToString(const string& path,
		const string& expression) const throw (invalid_argument) {
	return evaluateToString(path, expression.c_str());
}

const string XmlParser::evaluateToString(const string& path,
		const char* expression) const throw (invalid_argument) {
	DOMImplementation* xqillaImplementation =
			DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

	AutoRelease<DOMLSParser> parser(
			xqillaImplementation->createLSParser(
					DOMImplementationLS::MODE_SYNCHRONOUS, 0));
	parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, true);
	parser->getDomConfig()->setParameter(XMLUni::fgXercesSchema, true);
	parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);

	DOMDocument *document = parser->parseURI(path.c_str());
	if (document == 0) {
		BOOST_THROW_EXCEPTION(
				invalid_argument("Path '" + path + "' does not refer to a valid XML document."));
	}
	AutoRelease<DOMXPathNSResolver> resolver(
			document->createNSResolver(document->getDocumentElement()));
	AutoRelease<DOMXPathExpression> parsedExpression(
			document->createExpression(X(expression), resolver));
	AutoRelease<DOMXPathResult> iteratorResult(
			parsedExpression->evaluate(document->getDocumentElement(),
					DOMXPathResult::ORDERED_NODE_ITERATOR_TYPE, 0));

	string result = "";
	while (iteratorResult->iterateNext()) {
		if (iteratorResult->isNode()) {
			char* buffer = XMLString::transcode(
					iteratorResult->getStringValue());
			result = buffer;
			XMLString::release(&buffer);
			break;
		}
	}

	return result;
}

const vector<string> XmlParser::evaluateToStringList(const string& path,
		const string& expression) const throw (invalid_argument) {
	return evaluateToStringList(path, expression.c_str());
}

const vector<string> XmlParser::evaluateToStringList(const string& path,
		const char* expression) const throw (invalid_argument) {
	DOMImplementation* xqillaImplementation =
			DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

	AutoRelease<DOMLSParser> parser(
			xqillaImplementation->createLSParser(
					DOMImplementationLS::MODE_SYNCHRONOUS, 0));
	parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, true);
	parser->getDomConfig()->setParameter(XMLUni::fgXercesSchema, true);
	parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);

	DOMDocument *document = parser->parseURI(path.c_str());
	if (document == 0) {
		BOOST_THROW_EXCEPTION(
				invalid_argument("Path '" + path + "' does not refer to a valid XML document."));
	}

	AutoRelease<DOMXPathNSResolver> resolver(
			document->createNSResolver(document->getDocumentElement()));
	AutoRelease<DOMXPathExpression> parsedExpression(
			document->createExpression(X(expression), resolver));
	AutoRelease<DOMXPathResult> iteratorResult(
			parsedExpression->evaluate(document->getDocumentElement(),
					DOMXPathResult::ORDERED_NODE_ITERATOR_TYPE, 0));

	vector<string> resultList;
	while (iteratorResult->iterateNext()) {
		if (iteratorResult->isNode()) {
			char *buffer = XMLString::transcode(
					iteratorResult->getStringValue());
			const string result = buffer;
			resultList.push_back(result);
			XMLString::release(&buffer);
		}
	}

	return resultList;
}
