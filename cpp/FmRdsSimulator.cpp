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
/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "FmRdsSimulator.h"
#include <boost/thread.hpp>
#include <boost/filesystem/path.hpp>
#include <math.h>

#define DEFAULT_STREAM_ID "MyStreamID"
#define MAX_SAMPLE_RATE 2280000.0
#define MIN_SAMPLE_RATE (MAX_SAMPLE_RATE / 1000.0)

// From the FM Bandwidth
#define MIN_FREQ_RANGE 88000000
#define MAX_FREQ_RANGE 108000000

// Totally arbitrary gain limitations.
#define MIN_GAIN_RANGE -100
#define MAX_GAIN_RANGE 100

PREPARE_LOGGING(FmRdsSimulator_i)

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    construct();
}

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    construct();
}

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    construct();
}

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    construct();
}

FmRdsSimulator_i::~FmRdsSimulator_i()
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	if (digiSim) {
		delete(digiSim);
		digiSim = NULL;
	}

	if (cb) {
		delete(cb);
		cb = NULL;
	}
}

/**
 * Service function not used, instead the callback is used for data pushes.
 */
int FmRdsSimulator_i::serviceFunction()
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    return FINISH;
}

void FmRdsSimulator_i::construct()
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	digiSim = NULL;
	cb = NULL;

	/** Register callbacks **/
	addPropertyChangeListener("addAWGN", this, &FmRdsSimulator_i::addAWGNChanged);
	addPropertyChangeListener("noiseSigma", this, &FmRdsSimulator_i::noiseSigmaChanged);

	// 0.5 because of cast truncation.
	unsigned int maxSampleRateInt = (unsigned int) (MAX_SAMPLE_RATE + 0.5);
	int iterator = 2;
	unsigned int tmpSampleRate = maxSampleRateInt;
	availableSampleRates.push_back(tmpSampleRate);

	while (tmpSampleRate >= MIN_SAMPLE_RATE) {
		if (maxSampleRateInt % iterator == 0) {
			tmpSampleRate = maxSampleRateInt / iterator;

			if (tmpSampleRate >= MIN_SAMPLE_RATE) {
				availableSampleRates.push_back(tmpSampleRate);
			}
		}
		++iterator;
	}

	std::sort (availableSampleRates.begin(), availableSampleRates.end());

	// Initialize rfinfo packet
	rfinfo_pkt.if_center_freq =0;
	rfinfo_pkt.rf_flow_id = "";
}

void FmRdsSimulator_i::addAWGNChanged(const bool* old_value, const bool* new_value) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	if (digiSim) {
		digiSim->addNoise(*new_value);
	}
}

void FmRdsSimulator_i::noiseSigmaChanged(const float* old_value, const float* new_value) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	if (digiSim) {
		digiSim->setNoiseSigma(*new_value);
	}
}

void FmRdsSimulator_i::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	FmRdsSimulator_base::initialize();

	initDigitizer();

    /** As of the REDHAWK 1.8.3 release, device are not started automatically by the node. Therefore
     *  the device must start itself. */
    if(!started()){
        start();
    }
}

void FmRdsSimulator_i::initDigitizer() {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	setNumChannels(0);

	if (cb) {
		delete(cb);
		cb = NULL;
	}

	if (digiSim) {
		delete(digiSim);
		digiSim = NULL;
	}

	cb = new MyCallBackClass(dataFloat_out, &frontend_tuner_status,this->identifier() );


	digiSim = RfSimulators::RfSimulatorFactory::createFmRdsSimulator();
	digiSim->setCenterFrequencyRange(MIN_FREQ_RANGE, MAX_FREQ_RANGE);
	digiSim->setGainRange(MIN_GAIN_RANGE, MAX_GAIN_RANGE);
	digiSim->addNoise(addAWGN);
	digiSim->setNoiseSigma(noiseSigma);

	switch (this->log_level())
	{
	case 5:
		digiSim->init(PathToConfiguration, cb, RfSimulators::TRACE);
		break;
	case 4:
		digiSim->init(PathToConfiguration, cb, RfSimulators::DEBUG);
		break;
	case 3:
		digiSim->init(PathToConfiguration, cb, RfSimulators::INFO);
		break;
	case 2:
		digiSim->init(PathToConfiguration, cb, RfSimulators::WARN);
		break;
	case 1:
		digiSim->init(PathToConfiguration, cb, RfSimulators::ERROR);
		break;
	default:
		digiSim->init(PathToConfiguration, cb, RfSimulators::WARN);
		break;
	}


	setNumChannels(1);

    // Initialize status vector
    // this device only has a single tuner

    frontend_tuner_status[0].allocation_id_csv = "";
    frontend_tuner_status[0].tuner_type = "RX_DIGITIZER";
    frontend_tuner_status[0].center_frequency = digiSim->getCenterFrequency();
    frontend_tuner_status[0].sample_rate = digiSim->getSampleRate();
    // set bandwidth to the max sample rate.
    frontend_tuner_status[0].bandwidth = MAX_SAMPLE_RATE;
    frontend_tuner_status[0].rf_flow_id = "";
    frontend_tuner_status[0].gain = digiSim->getGain();
    frontend_tuner_status[0].group_id = "";
    frontend_tuner_status[0].enabled = false;
    frontend_tuner_status[0].stream_id.clear();

}

/*************************************************************
Functions supporting tuning allocation
*************************************************************/
void FmRdsSimulator_i::deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
    ************************************************************/

	if (not digiSim) {
		LOG_ERROR(FmRdsSimulator_i, "Tried to enable the device before it was initialized!");
		return;
	}

	digiSim->start();

	fts.center_frequency = digiSim->getCenterFrequency();
	fts.sample_rate = digiSim->getSampleRate();
	fts.bandwidth = MAX_SAMPLE_RATE;
	fts.stream_id = DEFAULT_STREAM_ID;
	fts.enabled = true;

    return;
}
void FmRdsSimulator_i::deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
    ************************************************************/
	if (digiSim) {
		digiSim->stop();
	}

	fts.enabled = false;
    return;
}
bool FmRdsSimulator_i::deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id){
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");

	bool sriChanged = false;

	/************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tuning succeeded, and false if it failed
    ************************************************************/
	LOG_INFO(FmRdsSimulator_i, "Received a request: ");
	LOG_INFO(FmRdsSimulator_i, "Allocation ID: " << request.allocation_id);
	LOG_INFO(FmRdsSimulator_i, "Bandwidth: " << request.bandwidth);
	LOG_INFO(FmRdsSimulator_i, "Bandwidth Tolerance: " << request.bandwidth_tolerance);
	LOG_INFO(FmRdsSimulator_i, "Center Frequency: " << request.center_frequency);
	LOG_INFO(FmRdsSimulator_i, "Device Control: " << request.device_control);
	LOG_INFO(FmRdsSimulator_i, "Group ID: " << request.group_id);
	LOG_INFO(FmRdsSimulator_i, "RF Flow ID: " << request.rf_flow_id);
	LOG_INFO(FmRdsSimulator_i, "Sample Rate: " << request.sample_rate);
	LOG_INFO(FmRdsSimulator_i, "Sample Rate Tolerance: " << request.sample_rate_tolerance);
	LOG_INFO(FmRdsSimulator_i, "Tuner Type: " << request.tuner_type);

	if (not digiSim) {
		LOG_WARN(FmRdsSimulator_i, "deviceSetTuning called when Simulator has not been created.  This is not expected");
		return false;
	}

	if (request.tuner_type != "RX_DIGITIZER") {
		LOG_WARN(FmRdsSimulator_i, "Tuner type does not equal RX_DIGITIZER.  Request denied.");
		return false;
	}

    // check request against RTL specs and analog input
    const bool complex = true; // RTL operates using complex data
    // Note: since samples are complex, assume BW == SR (rather than BW == SR/2 for Real samples)
    if (rfinfo_pkt.if_center_freq !=0)	{
		try {
			validateRequestVsRFInfo(request,rfinfo_pkt, complex);
		} catch(FRONTEND::BadParameterException& e){
			LOG_INFO(FmRdsSimulator_i," Failed to Validate against rfInfo_Pkt. deviceSetTuning|BadParameterException - " << e.msg);
			throw;
		}
	}
    double if_offset = 0.0;
    // calculate if_offset according to rx rfinfo packet
    if(frontend::floatingPointCompare(rfinfo_pkt.if_center_freq,0) > 0){
        if_offset = rfinfo_pkt.rf_center_freq-rfinfo_pkt.if_center_freq;
		LOG_DEBUG(FmRdsSimulator_i, "Set IF offset to : " << if_offset);

    }

	unsigned int sampleRateToSet = 0;

	// User cares about sample rate
	if (request.sample_rate != 0.0) {

		// For FEI tolerance, it is not a +/- it's give me this or better.
		float minAcceptableSampleRate = request.sample_rate;
		float maxAcceptableSampleRate = (1 + request.sample_rate_tolerance/100.0) * request.sample_rate;


		LOG_INFO(FmRdsSimulator_i, "Based on request, minimum acceptable sample rate: " << minAcceptableSampleRate);
		LOG_INFO(FmRdsSimulator_i, "Based on request, maximum acceptable sample rate: " << maxAcceptableSampleRate);

		// if the request isn't in the sample rate range return false
		if (minAcceptableSampleRate >= MAX_SAMPLE_RATE || maxAcceptableSampleRate <= MIN_SAMPLE_RATE) {
			LOG_WARN(FmRdsSimulator_i, "Requested sample rate is outside of range");
			return false;
		}

		std::vector<unsigned int>::iterator closestIterator;
		closestIterator = std::lower_bound(availableSampleRates.begin(), availableSampleRates.end(), (unsigned int) (minAcceptableSampleRate+0.5));

		if (closestIterator == availableSampleRates.end()) {
			LOG_ERROR(FmRdsSimulator_i, "Did not find sample rate in available list...this should not happen");
		}

		unsigned int closestSampleRate = *closestIterator;

		if (closestSampleRate > maxAcceptableSampleRate) {
			LOG_WARN(FmRdsSimulator_i, "Cannot deliver sample rate within the requested tolerance");
			return false;
		}

		sampleRateToSet = closestSampleRate;
	}

	// User cares about bandwidth...they shouldn't though.  It's not setable.
	if (request.bandwidth != 0.0) {
		float minAcceptableBandwidth = request.bandwidth;
		float maxAcceptableBandwidth = (1 + request.bandwidth_tolerance/100.0) * request.bandwidth;

		LOG_INFO(FmRdsSimulator_i, "Based on request, minimum acceptable bandwidth: " << minAcceptableBandwidth);
		LOG_INFO(FmRdsSimulator_i, "Based on request, maximum acceptable bandwidth: " << maxAcceptableBandwidth);


		// if MAX_SAMPLE_RATE < minAcceptable or MAX_SAMPLE_RATE > maxAcceptable we return an error.
		if (frontend::floatingPointCompare(MAX_SAMPLE_RATE, minAcceptableBandwidth) < 0
				|| frontend::floatingPointCompare(MAX_SAMPLE_RATE, maxAcceptableBandwidth) > 0) {
			LOG_WARN(FmRdsSimulator_i, "Bandwidth cannot be accommodated.  Set bandwidth to: " << MAX_SAMPLE_RATE);
			return false;
		}

	}

	try {
		digiSim->setCenterFrequency((float) request.center_frequency-if_offset);
		fts.center_frequency = digiSim->getCenterFrequency()+if_offset;
		sriChanged = true; // The COL and CHAN RF likely changed.
	} catch (OutOfRangeException &ex) {
		LOG_WARN(FmRdsSimulator_i, "Tried to tune to " << request.center_frequency-if_offset << " but request was rejected by simulator.");
		return false;
	}

	if (request.sample_rate > 0 && sampleRateToSet > 0) {
		LOG_INFO(FmRdsSimulator_i, "Setting the sample rate to: " << sampleRateToSet);
		digiSim->setSampleRate(sampleRateToSet);
		fts.sample_rate = digiSim->getSampleRate();
		sriChanged = true;
	}

	fts.bandwidth = MAX_SAMPLE_RATE;

	if (sriChanged) {
		cb->pushUpdatedSRI();
	}

    return true;
}
bool FmRdsSimulator_i::deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tune deletion succeeded, and false if it failed
    ************************************************************/


	if (digiSim) {
		digiSim->stop();
	}
	cb->pushEOS();

    fts.center_frequency = 0.0;
    fts.sample_rate = 0.0;
    fts.bandwidth = 0.0;
    fts.stream_id.clear();

    return true;
}

/*************************************************************
Functions servicing the tuner control port
*************************************************************/
std::string FmRdsSimulator_i::getTunerType(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].tuner_type;
}

bool FmRdsSimulator_i::getTunerDeviceControl(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if (getControlAllocationId(idx) == allocation_id)
        return true;
    return false;
}

std::string FmRdsSimulator_i::getTunerGroupId(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].group_id;
}

std::string FmRdsSimulator_i::getTunerRfFlowId(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].rf_flow_id;
}

void FmRdsSimulator_i::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (freq<0) throw FRONTEND::BadParameterException();

    try {
		digiSim->setCenterFrequency((float) freq);
		this->frontend_tuner_status[idx].center_frequency = freq;
		// The COL_RF and CHAN_RF has changed
		cb->pushUpdatedSRI();
    } catch (OutOfRangeException &ex) {
    	throw FRONTEND::BadParameterException();
    }

}

double FmRdsSimulator_i::getTunerCenterFrequency(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");

    return frontend_tuner_status[idx].center_frequency;
}

void FmRdsSimulator_i::setTunerBandwidth(const std::string& allocation_id, double bw) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (bw<0) throw FRONTEND::BadParameterException();

    if (bw != MAX_SAMPLE_RATE) {
    	LOG_WARN(FmRdsSimulator_i, "User tried to set bandwidth to: " << bw);
    	LOG_WARN(FmRdsSimulator_i, "Only acceptable bw is " << MAX_SAMPLE_RATE);
    	throw FRONTEND::BadParameterException();
    }

    this->frontend_tuner_status[idx].bandwidth = MAX_SAMPLE_RATE;
}

double FmRdsSimulator_i::getTunerBandwidth(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].bandwidth;
}

void FmRdsSimulator_i::setTunerAgcEnable(const std::string& allocation_id, bool enable)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");
}

bool FmRdsSimulator_i::getTunerAgcEnable(const std::string& allocation_id)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");
}

void FmRdsSimulator_i::setTunerGain(const std::string& allocation_id, float gain)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	long idx = getTunerMapping(allocation_id);
	if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
	if(allocation_id != getControlAllocationId(idx))
		throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());

	if (digiSim) {
		try {
			digiSim->setGain(gain);
		} catch (OutOfRangeException &ex) {
			throw FRONTEND::BadParameterException();
		}
		this->frontend_tuner_status[0].gain = gain;
	}
}

float FmRdsSimulator_i::getTunerGain(const std::string& allocation_id)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
	long idx = getTunerMapping(allocation_id);
	    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");

	if (digiSim) {
		return digiSim->getGain();
	}

	return -1;
}

void FmRdsSimulator_i::setTunerReferenceSource(const std::string& allocation_id, long source)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");
}

long FmRdsSimulator_i::getTunerReferenceSource(const std::string& allocation_id)
{
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");
}

void FmRdsSimulator_i::setTunerEnable(const std::string& allocation_id, bool enable) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());

    if (enable) {
    	digiSim->start();
    } else {
    	digiSim->stop();
    }

    this->frontend_tuner_status[idx].enabled = enable;
}

bool FmRdsSimulator_i::getTunerEnable(const std::string& allocation_id) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].enabled;
}

void FmRdsSimulator_i::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (sr<0) throw FRONTEND::BadParameterException();

    try {
		digiSim->setSampleRate((float) sr);
    } catch (InvalidValue &ex) {
    	throw FRONTEND::BadParameterException();
    	return;
    }

    this->frontend_tuner_status[idx].sample_rate = digiSim->getSampleRate();
    cb->pushUpdatedSRI();
}

double FmRdsSimulator_i::getTunerOutputSampleRate(const std::string& allocation_id){
	LOG_TRACE(FmRdsSimulator_i, "Entering Method");
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].sample_rate;
}

/*************************************************************
Functions servicing the RFInfo port(s)
- port_name is the port over which the call was received
*************************************************************/
std::string FmRdsSimulator_i::get_rf_flow_id(const std::string& port_name)
{
	LOG_TRACE(FmRdsSimulator_i,__PRETTY_FUNCTION__ << " port_name=" << port_name);

    if( port_name == "RFInfo_in"){
        return rfinfo_pkt.rf_flow_id;
    } else {
        LOG_WARN(FmRdsSimulator_i, "get_rf_flow_id|Unknown port name: " << port_name);
        return std::string("none");
    }
}

void FmRdsSimulator_i::set_rf_flow_id(const std::string& port_name, const std::string& id)
{
	LOG_TRACE(FmRdsSimulator_i,__PRETTY_FUNCTION__ << " port_name=" << port_name << " id=" << id);

    if( port_name == "RFInfo_in"){
        rfinfo_pkt.rf_flow_id = id;
        frontend_tuner_status[0].rf_flow_id = id;
        cb->pushUpdatedSRI();
    } else {
        LOG_WARN(FmRdsSimulator_i, "set_rf_flow_id|Unknown port name: " << port_name);
    }
}

frontend::RFInfoPkt FmRdsSimulator_i::get_rfinfo_pkt(const std::string& port_name)
{
    LOG_TRACE(FmRdsSimulator_i,__PRETTY_FUNCTION__ << " port_name=" << port_name);

    frontend::RFInfoPkt pkt;
    if( port_name != "RFInfo_in"){
        LOG_WARN(FmRdsSimulator_i, "get_rfinfo_pkt|Unknown port name: " << port_name);
        return pkt;
    }
    pkt.rf_flow_id = rfinfo_pkt.rf_flow_id;
    pkt.rf_center_freq = rfinfo_pkt.rf_center_freq;
    pkt.rf_bandwidth = rfinfo_pkt.rf_bandwidth;
    pkt.if_center_freq = rfinfo_pkt.if_center_freq;
    pkt.spectrum_inverted = rfinfo_pkt.spectrum_inverted;
    pkt.sensor.collector = rfinfo_pkt.sensor.collector;
    pkt.sensor.mission = rfinfo_pkt.sensor.mission;
    pkt.sensor.rx = rfinfo_pkt.sensor.rx;
    pkt.sensor.antenna.description = rfinfo_pkt.sensor.antenna.description;
    pkt.sensor.antenna.name = rfinfo_pkt.sensor.antenna.name;
    pkt.sensor.antenna.size = rfinfo_pkt.sensor.antenna.size;
    pkt.sensor.antenna.type = rfinfo_pkt.sensor.antenna.type;
    pkt.sensor.feed.name = rfinfo_pkt.sensor.feed.name;
    pkt.sensor.feed.polarization = rfinfo_pkt.sensor.feed.polarization;
    pkt.sensor.feed.freq_range.max_val = rfinfo_pkt.sensor.feed.freq_range.max_val;
    pkt.sensor.feed.freq_range.min_val = rfinfo_pkt.sensor.feed.freq_range.min_val;
    pkt.sensor.feed.freq_range.values.resize(rfinfo_pkt.sensor.feed.freq_range.values.size());
    for (unsigned int i=0; i<rfinfo_pkt.sensor.feed.freq_range.values.size(); i++) {
        pkt.sensor.feed.freq_range.values[i] = rfinfo_pkt.sensor.feed.freq_range.values[i];
    }
    return pkt;
}

void FmRdsSimulator_i::set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt)
{
    LOG_DEBUG(FmRdsSimulator_i, "set_rfinfo_pkt|port_name=" << port_name << " pkt.rf_flow_id=" << pkt.rf_flow_id);
    LOG_DEBUG(FmRdsSimulator_i, "set_rfinfo_pkt|rf_center_freq=" << pkt.rf_center_freq );
    LOG_DEBUG(FmRdsSimulator_i, "set_rfinfo_pkt|rf_bandwidth=" << pkt.rf_bandwidth );
    LOG_DEBUG(FmRdsSimulator_i, "set_rfinfo_pkt|if_center_freq=" << pkt.if_center_freq );

    if( port_name == "RFInfo_in"){
        rfinfo_pkt.rf_flow_id = pkt.rf_flow_id;
        rfinfo_pkt.rf_center_freq = pkt.rf_center_freq;
        rfinfo_pkt.rf_bandwidth = pkt.rf_bandwidth;
        rfinfo_pkt.if_center_freq = pkt.if_center_freq;
        rfinfo_pkt.spectrum_inverted = pkt.spectrum_inverted;
        rfinfo_pkt.sensor.collector = pkt.sensor.collector;
        rfinfo_pkt.sensor.mission = pkt.sensor.mission;
        rfinfo_pkt.sensor.rx = pkt.sensor.rx;
        rfinfo_pkt.sensor.antenna.description = pkt.sensor.antenna.description;
        rfinfo_pkt.sensor.antenna.name = pkt.sensor.antenna.name;
        rfinfo_pkt.sensor.antenna.size = pkt.sensor.antenna.size;
        rfinfo_pkt.sensor.antenna.type = pkt.sensor.antenna.type;
        rfinfo_pkt.sensor.feed.name = pkt.sensor.feed.name;
        rfinfo_pkt.sensor.feed.polarization = pkt.sensor.feed.polarization;
        rfinfo_pkt.sensor.feed.freq_range.max_val = pkt.sensor.feed.freq_range.max_val;
        rfinfo_pkt.sensor.feed.freq_range.min_val = pkt.sensor.feed.freq_range.min_val;
        rfinfo_pkt.sensor.feed.freq_range.values.resize(pkt.sensor.feed.freq_range.values.size());
        for (unsigned int i=0; i<pkt.sensor.feed.freq_range.values.size(); i++) {
            rfinfo_pkt.sensor.feed.freq_range.values[i] = pkt.sensor.feed.freq_range.values[i];
        }
        frontend_tuner_status[0].rf_flow_id = pkt.rf_flow_id;
        cb->pushUpdatedSRI();
    } else {
        LOG_WARN(FmRdsSimulator_i, "set_rfinfo_pkt|Unknown port name: " + port_name);
    }

}

