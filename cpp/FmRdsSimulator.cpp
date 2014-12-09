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

PREPARE_LOGGING(FmRdsSimulator_i)

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl)
{
    construct();

}

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
    construct();
}

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
    construct();
}

FmRdsSimulator_i::FmRdsSimulator_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    FmRdsSimulator_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
    construct();
}

FmRdsSimulator_i::~FmRdsSimulator_i()
{
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
    return FINISH;
}

void FmRdsSimulator_i::construct()
{
	xmlPath = "/tmp/testDir";
	digiSim = NULL;
	cb = NULL;
}


void FmRdsSimulator_i::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
	FmRdsSimulator_base::initialize();

	initDigitizer();

    /** As of the REDHAWK 1.8.3 release, device are not started automatically by the node. Therefore
     *  the device must start itself. */
    if(!started()){
        start();
    }
}

void FmRdsSimulator_i::initDigitizer() {
	setNumChannels(0);

	if (cb) {
		delete(cb);
		cb = NULL;
	}

	if (digiSim) {
		delete(digiSim);
		digiSim = NULL;
	}

	cb = new MyCallBackClass(dataFloat_out, &frontend_tuner_status);

	digiSim = RfSimulators::RfSimulatorFactory::createFmRdsSimulator();

	switch (this->log_level())
	{
	case 5:
		digiSim->init(xmlPath, cb, RfSimulators::TRACE);
		break;
	case 4:
		digiSim->init(xmlPath, cb, RfSimulators::DEBUG);
		break;
	case 3:
		digiSim->init(xmlPath, cb, RfSimulators::INFO);
		break;
	case 2:
		digiSim->init(xmlPath, cb, RfSimulators::WARN);
		break;
	case 1:
		digiSim->init(xmlPath, cb, RfSimulators::ERROR);
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

	/************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tuning succeeded, and false if it failed
    ************************************************************/

	if (not digiSim) {
		return false;
	}

	if (request.tuner_type != "RX_DIGITIZER") {
		return false;
	}

	// User cares about sample rate
	if (request.sample_rate != 0.0) {

		float minAcceptableSampleRate = (1 - request.sample_rate_tolerance) * request.sample_rate;
		float maxAcceptableSampleRate = (1 + request.sample_rate_tolerance) * request.sample_rate;

		// if the request isn't in the sample rate range return false
		if (minAcceptableSampleRate >= MAX_SAMPLE_RATE || maxAcceptableSampleRate <= MIN_SAMPLE_RATE) {
			return false;
		}


		unsigned int closestInteger = round(MAX_SAMPLE_RATE / request.sample_rate);
		unsigned int closestSampleRate = round(MAX_SAMPLE_RATE / closestInteger);

		if (closestSampleRate > maxAcceptableSampleRate || closestSampleRate < minAcceptableSampleRate) {
			return false;
		}
	}

	// User cares about bandwidth...they shouldn't though.  It's not settable.
	if (request.bandwidth != 0.0) {
		float minAcceptableBandwidth = (1 - request.bandwidth_tolerance) * request.bandwidth;
		float maxAcceptableBandwidth = (1 + request.bandwidth_tolerance) * request.bandwidth;

		// if the request isn't near the only bandwidth supported
		if (minAcceptableBandwidth >= MAX_SAMPLE_RATE || maxAcceptableBandwidth <= MIN_SAMPLE_RATE) {
			return false;
		}
	}

	try {
		digiSim->setCenterFrequency((float) request.center_frequency);
		fts.center_frequency = digiSim->getCenterFrequency();
	} catch (OutOfRangeException &ex) {
		LOG_WARN(FmRdsSimulator_i, "Tried to tune to " << request.center_frequency << " but request was rejected by simulator.");
		return false;
	}

	if (request.sample_rate > 0) {
		digiSim->setSampleRate((unsigned int) request.sample_rate);
		fts.sample_rate = digiSim->getSampleRate();
	}

	fts.bandwidth = MAX_SAMPLE_RATE;

    return true;
}
bool FmRdsSimulator_i::deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id) {
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tune deletion succeeded, and false if it failed
    ************************************************************/

	// TODO: Catch exceptions and stuff I guess?
	// TODO: Should I push EOS?
	if (digiSim) {
		digiSim->stop();
	}

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
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].tuner_type;
}

bool FmRdsSimulator_i::getTunerDeviceControl(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if (getControlAllocationId(idx) == allocation_id)
        return true;
    return false;
}

std::string FmRdsSimulator_i::getTunerGroupId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].group_id;
}

std::string FmRdsSimulator_i::getTunerRfFlowId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].rf_flow_id;
}

void FmRdsSimulator_i::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (freq<0) throw FRONTEND::BadParameterException();

    try {
		digiSim->setCenterFrequency((float) freq);
		this->frontend_tuner_status[idx].center_frequency = freq;
    } catch (OutOfRangeException &ex) {
    	throw FRONTEND::BadParameterException();
    }

}

double FmRdsSimulator_i::getTunerCenterFrequency(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].center_frequency;
}

void FmRdsSimulator_i::setTunerBandwidth(const std::string& allocation_id, double bw) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (bw<0) throw FRONTEND::BadParameterException();

    if (bw != MAX_SAMPLE_RATE) {
    	LOG_WARN(FmRdsSimulator_i, "User tried to set bandwidth to: " << bw);
    	LOG_WARN(FmRdsSimulator_i, "Only acceptable bw is " << MAX_SAMPLE_RATE);
    }

    this->frontend_tuner_status[idx].bandwidth = MAX_SAMPLE_RATE;
}

double FmRdsSimulator_i::getTunerBandwidth(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].bandwidth;
}

void FmRdsSimulator_i::setTunerAgcEnable(const std::string& allocation_id, bool enable)
{
    throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");
}

bool FmRdsSimulator_i::getTunerAgcEnable(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");
}

void FmRdsSimulator_i::setTunerGain(const std::string& allocation_id, float gain)
{
	if (digiSim) {
		digiSim->setGain(gain);
		this->frontend_tuner_status[0].gain = gain;
	}
}

float FmRdsSimulator_i::getTunerGain(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerGain not supported");
}

void FmRdsSimulator_i::setTunerReferenceSource(const std::string& allocation_id, long source)
{
    throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");
}

long FmRdsSimulator_i::getTunerReferenceSource(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");
}

void FmRdsSimulator_i::setTunerEnable(const std::string& allocation_id, bool enable) {
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
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].enabled;
}

void FmRdsSimulator_i::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
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

    this->frontend_tuner_status[idx].sample_rate = sr;
}

double FmRdsSimulator_i::getTunerOutputSampleRate(const std::string& allocation_id){
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
    return std::string("none");
}

void FmRdsSimulator_i::set_rf_flow_id(const std::string& port_name, const std::string& id)
{
}

frontend::RFInfoPkt FmRdsSimulator_i::get_rfinfo_pkt(const std::string& port_name)
{
    frontend::RFInfoPkt pkt;
    return pkt;
}

void FmRdsSimulator_i::set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt)
{
}

