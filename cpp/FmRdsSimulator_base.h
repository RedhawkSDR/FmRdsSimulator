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
#ifndef FMRDSSIMULATOR_BASE_IMPL_BASE_H
#define FMRDSSIMULATOR_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <frontend/frontend.h>
#include <ossie/ThreadedComponent.h>

#include <frontend/frontend.h>
#include <bulkio/bulkio.h>
#include "struct_props.h"

#define BOOL_VALUE_HERE 0

class FmRdsSimulator_base : public frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>, public virtual frontend::digital_tuner_delegation, public virtual frontend::rfinfo_delegation, protected ThreadedComponent
{
    public:
        FmRdsSimulator_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        FmRdsSimulator_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        FmRdsSimulator_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        FmRdsSimulator_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~FmRdsSimulator_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();
        void removeAllocationIdRouting(const size_t tuner_id);

        virtual CF::Properties* getTunerStatus(const std::string& allocation_id);
        virtual void assignListener(const std::string& listen_alloc_id, const std::string& allocation_id);
        virtual void removeListener(const std::string& listen_alloc_id);
        void frontendTunerStatusChanged(const std::vector<frontend_tuner_status_struct_struct>* oldValue, const std::vector<frontend_tuner_status_struct_struct>* newValue);

    protected:
        // Member variables exposed as properties
        /// Property: PathToConfiguration
        std::string PathToConfiguration;
        /// Property: noiseSigma
        float noiseSigma;
        /// Property: addAWGN
        bool addAWGN;

        // Ports
        /// Port: RFInfo_in
        frontend::InRFInfoPort *RFInfo_in;
        /// Port: DigitalTuner_in
        frontend::InDigitalTunerPort *DigitalTuner_in;
        /// Port: dataFloat_out
        bulkio::OutFloatPort *dataFloat_out;

        std::map<std::string, std::string> listeners;

        virtual void setNumChannels(size_t num);
        virtual void setNumChannels(size_t num, std::string tuner_type);

    private:
        void construct();
};
#endif // FMRDSSIMULATOR_BASE_IMPL_BASE_H
