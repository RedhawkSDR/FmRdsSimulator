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
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>

#include <frontend/fe_tuner_struct_props.h>

struct frontend_tuner_status_struct_struct : public frontend::default_frontend_tuner_status_struct_struct {
    frontend_tuner_status_struct_struct () : frontend::default_frontend_tuner_status_struct_struct()
    {
    };

    static std::string getId() {
        return std::string("FRONTEND::tuner_status_struct");
    };

    double gain;
    std::string stream_id;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FRONTEND::tuner_status::allocation_id_csv", props[idx].id)) {
            if (!(props[idx].value >>= s.allocation_id_csv)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::bandwidth", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::center_frequency", props[idx].id)) {
            if (!(props[idx].value >>= s.center_frequency)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::enabled", props[idx].id)) {
            if (!(props[idx].value >>= s.enabled)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::gain", props[idx].id)) {
            if (!(props[idx].value >>= s.gain)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::group_id", props[idx].id)) {
            if (!(props[idx].value >>= s.group_id)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::rf_flow_id", props[idx].id)) {
            if (!(props[idx].value >>= s.rf_flow_id)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::sample_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.sample_rate)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::tuner_type", props[idx].id)) {
            if (!(props[idx].value >>= s.tuner_type)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::stream_id", props[idx].id)) {
            if (!(props[idx].value >>= s.stream_id)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const frontend_tuner_status_struct_struct& s) {
    CF::Properties props;
    props.length(10);
    props[0].id = CORBA::string_dup("FRONTEND::tuner_status::allocation_id_csv");
    props[0].value <<= s.allocation_id_csv;
    props[1].id = CORBA::string_dup("FRONTEND::tuner_status::bandwidth");
    props[1].value <<= s.bandwidth;
    props[2].id = CORBA::string_dup("FRONTEND::tuner_status::center_frequency");
    props[2].value <<= s.center_frequency;
    props[3].id = CORBA::string_dup("FRONTEND::tuner_status::enabled");
    props[3].value <<= s.enabled;
    props[4].id = CORBA::string_dup("FRONTEND::tuner_status::gain");
    props[4].value <<= s.gain;
    props[5].id = CORBA::string_dup("FRONTEND::tuner_status::group_id");
    props[5].value <<= s.group_id;
    props[6].id = CORBA::string_dup("FRONTEND::tuner_status::rf_flow_id");
    props[6].value <<= s.rf_flow_id;
    props[7].id = CORBA::string_dup("FRONTEND::tuner_status::sample_rate");
    props[7].value <<= s.sample_rate;
    props[8].id = CORBA::string_dup("FRONTEND::tuner_status::tuner_type");
    props[8].value <<= s.tuner_type;
    props[9].id = CORBA::string_dup("FRONTEND::tuner_status::stream_id");
    props[9].value <<= s.stream_id;
    a <<= props;
};

inline bool operator== (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    if (s1.allocation_id_csv!=s2.allocation_id_csv)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.enabled!=s2.enabled)
        return false;
    if (s1.gain!=s2.gain)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.stream_id!=s2.stream_id)
        return false;
    return true;
};

inline bool operator!= (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
