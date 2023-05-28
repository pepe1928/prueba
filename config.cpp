/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution,
 *             dissemination or disclosure not allowed.
 * Project:    GRC
 * File:       config.cpp
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#include "config.h"
#include "yaml-cpp/yaml.h"
#include "yaml/utils.h"


namespace grc{
namespace sqc{

    bool Config::Parse(const std::string & filename, std::string &error_info){
        error_info = "[CONFIG] ";
        YAML::Node config = YAML::LoadFile(filename);
        try{
            YAML::Node general = config["general"];
            std::vector<std::string> stations, satellites, channels, observables;
            std::vector<std::vector<std::string>> channels_obs;


            yaml::ParseMandatoryValue<std::vector<std::string>>(general,"stations",stations);
            for (const std::string& station: stations){
                this->stations_.insert(GSS_ID(station));
            }
            yaml::ParseMandatoryValue<std::vector<std::string>>(general,"satellites",satellites);
            for (const std::string& sat_id: satellites){
                Constellation_e constel = get_constellation_from_sat(SAT_ID(sat_id));
                this->satellites_[constel].insert(SAT_ID(sat_id));
            }
            yaml::ParseMandatoryValue<std::vector<std::vector<std::string>>>(general,"channels_to_observables",channels_obs);
            for (const std::vector<std::string>& chann_obs_pair: channels_obs){
                if (chann_obs_pair.size()!=2){
                    error_info += "Given a correspondence with more than 2 elements: ["+chann_obs_pair[0];
                    for (size_t i=1;i<chann_obs_pair.size();++i){
                        error_info += "," + chann_obs_pair[i];
                    }
                    error_info += "]";
                    return false;
                }
                Constellation_e constel = get_constel_from_service(chann_obs_pair[0]);
                this->channels_to_observables_[constel].insert(std::make_pair(getServiceFromString(chann_obs_pair[0]),get_Signal(chann_obs_pair[1])));
            }
            yaml::ParseMandatoryValue<std::vector<std::string>>(general,"channels",channels);
            for (const std::string& service: channels){
                Constellation_e constel = get_constel_from_service(service);
                std::vector<Service_e> services;
                get_base_services(getServiceFromString(service), services);
                sqc::TypeServiceFrec frec_type = services.size() == 1? sqc::TypeServiceFrec::SF : sqc::TypeServiceFrec::DF;
                this->channels_[constel][frec_type].insert(getServiceFromString(service));
                if (sqc::TypeServiceFrec::DF==frec_type){
                    auto it1 = this->channels_to_observables_[constel].find(services[0]);
                    if (it1 == this->channels_to_observables_[constel].end()){
                        error_info += "Missing channels_to_observables for service "+get_string_from_Service(services[0]);
                        return false;
                    }
                    auto it2 = this->channels_to_observables_[constel].find(services[1]);
                    if (it2 == this->channels_to_observables_[constel].end()){
                        error_info += "Missing channels_to_observables for service "+get_string_from_Service(services[1]);
                        return false;
                    }
                }
            }
            yaml::ParseMandatoryValue<std::vector<std::string>>(general,"observables",observables);
            for (const std::string& service: observables){
                Constellation_e constel = get_constellation_from_signal(get_Signal(service));
                this->observables_[constel].insert(get_Signal(service));
            }

            bool missing_obs_constels = false;
            std::string tmp_obs_fail = "";
            bool missing_channel_constels = false;
            std::string tmp_channel_fail = "";
            for (const std::pair<const Constellation_e, std::set<SAT_ID>>& per_constel : this->satellites_){
                Constellation_e constel = per_constel.first;
                if (this->observables_.end()==this->observables_.find(constel)){
                    missing_obs_constels = true;
                    tmp_obs_fail += get_string_from_constel(constel) + ",";
                }
                if (this->channels_.end()==this->channels_.find(constel)){
                    missing_channel_constels = true;
                    tmp_channel_fail += get_string_from_constel(constel) + ",";
                }
            }
            if (missing_obs_constels){
                std::stringstream ss;
                ss << "Missing "<<tmp_obs_fail.substr(0,tmp_obs_fail.length()-1)<<" observables in observables parameter";
                error_info += ss.str();
                return false;
            }
            if (missing_channel_constels){
                std::stringstream ss;
                ss << "Missing "<<tmp_channel_fail.substr(0,tmp_channel_fail.length()-1)<<" channels in channels parameter";
                error_info += ss.str();
                return false;
            }

            yaml::ParseMandatoryValue<bool>(general,"use_health",this->use_health_);
            yaml::ParseMandatoryValue<bool>(general,"use_nanus",this->use_nanus_);
            yaml::ParseMandatoryValue<bool>(general,"only_prs",this->only_prs_);

            YAML::Node thresholds = config["thresholds"];
            yaml::ParseMandatoryValue<double>(thresholds,"cn0_drops",this->thresholds_.cn0_drops);
            yaml::ParseMandatoryValue<double>(thresholds,"cn0_between_epoch",this->thresholds_.cn0_between_epoch);
            yaml::ParseMandatoryValue<double>(thresholds,"carr_phase_noise",this->thresholds_.carr_phase_noise);
            yaml::ParseMandatoryValue<double>(thresholds,"carr_phase_noise_stations",this->thresholds_.carr_phase_noise_stations);
            yaml::ParseMandatoryValue<double>(thresholds,"cs_stations",this->thresholds_.cs_stations);

        }catch(const YAML::Exception& e){
            error_info += e.what();
            return false;
        }
        return true;
    }

} // sqc
} // grc
