/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK FmRdsSimulator.
 *
 * REDHAWK FmRdsSimulator is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK FmRdsSimulator is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
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
	MyCallBackClass(bulkio::OutFloatPort * outputPort, std::vector<frontend_tuner_status_struct_struct> *fts,char* identifier);
	virtual ~MyCallBackClass();
	void pushEOS();
	void dataDelivery(std::valarray< std::complex<float> > &samples);
	void pushUpdatedSRI();

private:
	char* identifier;
	bulkio::OutFloatPort * outputPort;
	bool pushSRI;
	std::vector<frontend_tuner_status_struct_struct> *fts;

};

#endif /* MYCALLBACKCLASS_H_ */
