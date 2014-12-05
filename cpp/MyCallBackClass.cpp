/*
 * MyCallBackClass.cpp
 *
 *  Created on: Dec 1, 2014
 */

#include "MyCallBackClass.h"
#include <iostream>
#include <vector>
#include <complex>

MyCallBackClass::MyCallBackClass(bulkio::OutFloatPort * outputPort, std::vector<frontend_tuner_status_struct_struct> *fts) {
	this->outputPort = outputPort;
    this->fts = fts;

    pushedSRI = false;
}

MyCallBackClass::~MyCallBackClass() {
}

void MyCallBackClass::dataDelivery(std::valarray< std::complex<float> > samples) {

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

	if (not pushedSRI) {
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

		outputPort->pushSRI(sri);
		pushedSRI = true;
	}
	outputPort->pushPacket(data, my_tmp_time, false, (*fts)[0].stream_id);
}


