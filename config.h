/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution,
 *             dissemination or disclosure not allowed.
 * Project:    GRC
 * File:       config.h
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#ifndef SQC_CONFIG_PARSER_H_
#define SQC_CONFIG_PARSER_H_

#include <Types/Common.h>
#include <Types/Observations.h>

namespace grc{
namespace sqc{

    enum class TypeServiceFrec: uint8_t{
        SF = 0,
        DF
    };

    struct Thresholds{
        double cn0_drops = 10;
        double cn0_between_epoch = 5;
        double carr_phase_noise = 1;
        double carr_phase_noise_stations = 3;
        double cs_stations = 2;
    };

    class Config{
        private:
            std::set<GSS_ID> stations_;
            std::map<Constellation_e, std::set<SAT_ID>> satellites_;
            std::map<Constellation_e,std::map<sqc::TypeServiceFrec, std::set<Service_e>>> channels_;
            std::map<Constellation_e, std::set<SignalID>> observables_;
            std::map<Constellation_e, std::map<Service_e, SignalID>> channels_to_observables_;
            unsigned int sampling_;
            bool use_health_;
            bool use_nanus_;
            bool only_prs_;
            Thresholds thresholds_;


        public:
            // General
            Config() {}
            bool Parse(const std::string & filemame, std::string &error_info);

            // Getters
            const std::set<GSS_ID>& GetStations()const{return stations_;}
            const std::map<Constellation_e,std::set<SAT_ID>>& GetSatellites()const{return satellites_;}
            const std::map<Constellation_e,std::map<sqc::TypeServiceFrec, std::set<Service_e>>>& GetChannels()const{return channels_;}
            const std::map<Constellation_e,std::set<SignalID>>& GetObservables()const{return observables_;}
            const std::map<Constellation_e,std::map<Service_e, SignalID>>& GetChannelsToObserbables() const{return channels_to_observables_;}
            unsigned int GetSampling()const{return sampling_;}
            bool GetHealthUsage()const{return use_health_;}
            bool GetUseNanus()const{return use_nanus_;}
            bool GetOnlyPrs()const{return only_prs_;}
            const Thresholds& GetThresholds()const{return thresholds_;}
    };

} // sqc
} // grc

#endif /* SQC_CONFIG_PARSER_H_ */
