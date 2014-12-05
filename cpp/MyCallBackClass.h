/*
 * MyCallBackClass.h
 *
 *  Created on: Dec 1, 2014
 */

#ifndef MYCALLBACKCLASS_H_
#define MYCALLBACKCLASS_H_

#include <RfSimulators/CallbackInterface.h>
#include <bulkio/bulkio.h>
#include "struct_props.h"

class MyCallBackClass: public RfSimulators::CallbackInterface {
public:
	MyCallBackClass(bulkio::OutFloatPort * outputPort, std::vector<frontend_tuner_status_struct_struct> *fts);
	virtual ~MyCallBackClass();
	void dataDelivery(std::valarray< std::complex<float> > samples);

private:
	bulkio::OutFloatPort * outputPort;
	int pushedSRI;
	std::vector<frontend_tuner_status_struct_struct> *fts;

};

#endif /* MYCALLBACKCLASS_H_ */
