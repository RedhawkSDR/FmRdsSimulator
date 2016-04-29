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
 * MyCallBackClass.cpp
 *
 *  Created on: Dec 1, 2014
 */

#include "MyCallBackClass.h"
#include <iostream>
#include <vector>
#include <complex>

MyCallBackClass::MyCallBackClass(bulkio::OutFloatPort * outputPort, std::vector<frontend_tuner_status_struct_struct> *fts,char* identifier) {
	this->outputPort = outputPort;
    this->fts = fts;
    this->identifier = identifier;
    pushSRI = true;
}

MyCallBackClass::~MyCallBackClass() {
}

void MyCallBackClass::pushEOS() {
	struct timeval tmp_time;
	struct timezone tmp_tz;
	gettimeofday(&tmp_time, &tmp_tz);
	BULKIO::PrecisionUTCTime my_tmp_time = bulkio::time::utils::create(tmp_time.tv_sec,	tmp_time.tv_usec / 1e6);
	std::vector<float> data;
	data.resize(0);
	outputPort->pushPacket(data, my_tmp_time,true, (*fts)[0].stream_id);
}

void MyCallBackClass::pushUpdatedSRI() {
	pushSRI = true;
}

void MyCallBackClass::dataDelivery(std::valarray< std::complex<float> > &samples) {

	struct timeval tmp_time;
	struct timezone tmp_tz;
	gettimeofday(&tmp_time, &tmp_tz);
	BULKIO::PrecisionUTCTime my_tmp_time = bulkio::time::utils::create(tmp_time.tv_sec,	tmp_time.tv_usec / 1e6);
	std::vector<float> data;

	data.resize(2*samples.size());
	for (unsigned int i = 0; i < samples.size(); i++) {
			data[2*i] = samples[i].real();
			data[2*i+1] = samples[i].imag();
	}

	if (pushSRI) {
		pushSRI = false;

        BULKIO::StreamSRI sri;
        sri.hversion = 1;
        sri.xstart = 0.0;

        if ( (*fts)[0].sample_rate <= 0.0 )
            sri.xdelta =  1.0;
        else
            sri.xdelta = 1/(*fts)[0].sample_rate;
        sri.xunits = BULKIO::UNITS_TIME;
        sri.subsize = 0;
        sri.ystart = 0.0;
        sri.ydelta = 0.0;
        sri.yunits = BULKIO::UNITS_NONE;
        sri.mode = 1;
        sri.blocking = false;
        sri.streamID = (*fts)[0].stream_id.c_str();

		sri.keywords.length(5);
		sri.keywords[0].id = "CHAN_RF";
		sri.keywords[0].value <<= (*fts)[0].center_frequency;

		sri.keywords[1].id = "COL_RF";
		sri.keywords[1].value <<= (*fts)[0].center_frequency;

		sri.keywords[2].id = "FRONTEND::BANDWIDTH";
		sri.keywords[2].value <<= (*fts)[0].bandwidth;

		sri.keywords[3].id = "FRONTEND::RF_FLOW_ID";
		sri.keywords[3].value <<= (*fts)[0].rf_flow_id;

		sri.keywords[4].id = "FRONTEND::DEVICE_ID";
		sri.keywords[4].value <<= identifier;


		outputPort->pushSRI(sri);
	}
	outputPort->pushPacket(data, my_tmp_time, false, (*fts)[0].stream_id);
}


