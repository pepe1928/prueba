
/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution, 
 *             dissemination or disclosure not allowed. 
 * Project:    GRC 
 * File:       compute_manager.cpp
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/

#include <compute_manager.h>
#include <Algos/hatch_filter.h>
#include <Algos/math_functions.h>
#include <math.h>
#include <Types/Observations.h>
#include <Types/Common.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <Algos/Linear_Algebra.h>
#include <Algos/Geometry.h>
#include <Logger.h>
#include <cmath>

namespace grc{
namespace sqc{

    void ComputeManager::ProcessCarrierPhaseNoiseOutputs(){
        const std::set<GSS_ID>& stations = this->configuration_.GetStations();
        const Thresholds& thresholds = this->configuration_.GetThresholds();
        SQM_COD_PHAS_ERR::SQM_COD_Data& code_phase_error_data = this->inputs_.GetCodePhaseErrorData();

        std::map<GPS_Time, std::map<SignalID, std::map<SAT_ID, std::vector<GSS_ID>>>> gss_over_threshold;
        for (const GSS_ID& gss_id: stations){
            for (const std::pair<const Constellation_e,std::set<SAT_ID>>& per_constel: this->configuration_.GetSatellites()){
                Constellation_e constel = per_constel.first;
                for (const SAT_ID& sat_id: per_constel.second){
                    for (const SignalID& obs: this->configuration_.GetObservables().at(constel)){
                        for (GPS_Time epoch=this->start_; epoch<=this->end_; epoch+=this->configuration_.GetSampling()){
                            auto it_gss = code_phase_error_data.find(gss_id);
                            if(it_gss==code_phase_error_data.end()){
                                continue;
                            }
                            auto it_obs = it_gss->second.find(obs);
                            if(it_obs==it_gss->second.end()){
                                continue;
                            }
                            auto it_sat = it_obs->second.find(sat_id);
                            if(it_sat==it_obs->second.end()){
                                continue;
                            }
                            auto per_epoch = it_sat->second.find(epoch);
                            if(per_epoch!=it_sat->second.end()){
                                SQM_COD_PHAS_ERR::Info info = per_epoch->second;
                                if(info.code_carrier.second >= thresholds.carr_phase_noise){
                                    gss_over_threshold[epoch][obs][sat_id].push_back(gss_id);
                                }
                            }
                        }

                    }
                }
            }
        }

        for(auto epoch=gss_over_threshold.begin();epoch!=gss_over_threshold.end();epoch++){
            for(auto obs = epoch->second.begin();obs!=epoch->second.end();obs++){
                for(auto sat = obs->second.begin();sat!= obs->second.end();sat++){
                    if (static_cast<int>(sat->second.size()) >= thresholds.carr_phase_noise_stations){
                        this->results_.carr_ph_noise[epoch->first][obs->first][sat->first] = "YES";
                    }
                }
            }
        }

    }


    void ComputeManager::ProcessCn0Outputs(){
        std::string error_info = "";
        const std::set<GSS_ID>& stations = this->configuration_.GetStations();
        const Thresholds& thresholds = this->configuration_.GetThresholds();
        SQM_CN0_Parser::SQM_CN0_Data& cn0_data = this->inputs_.GetCN0Data();

        for (const GSS_ID& gss_id: stations){
            for (const std::pair<const Constellation_e,std::set<SAT_ID>>& per_constel: this->configuration_.GetSatellites()){
                Constellation_e constel = per_constel.first;
                for (const SAT_ID& sat_id: per_constel.second){
                    for (const SignalID& obs: this->configuration_.GetObservables().at(constel)){
                        GPS_Time prev_epoch;
                        for (GPS_Time epoch=this->start_; epoch<=this->end_; epoch+=this->configuration_.GetSampling()){
                            auto it_gss = cn0_data.find(gss_id);
                            if(it_gss==cn0_data.end()){
                                continue;
                            }
                            auto it_obs = it_gss->second.find(obs);
                            if(it_obs==it_gss->second.end()){
                                continue;
                            }
                            auto it_sat = it_obs->second.find(sat_id);
                            if(it_sat==it_obs->second.end()){
                                continue;
                            }
                            auto fin_epoch = it_sat->second.find(epoch);
                            auto ini_epoch = it_sat->second.find(prev_epoch);

                            if(fin_epoch!=it_sat->second.end() && ini_epoch!=it_sat->second.end()){
                                SQM_CN0_Parser::Info ini_info = ini_epoch->second;
                                SQM_CN0_Parser::Info fin_info = fin_epoch->second;

                                if(fin_info.cn0.second - ini_info.cn0.second >= thresholds.cn0_between_epoch){
                                    AlgorithmManager::SQCResults::DropsBtwEpoch values;
                                    values.ini_epoch = prev_epoch;
                                    values.fin_epoch = epoch;
                                    values.ini_cn0 = ini_info.cn0.second;
                                    values.fin_cn0 = fin_info.cn0.second;
                                    this->results_.cn0_drops_btw_epoch[gss_id][sat_id][obs].push_back(values);
                                }
                            }
                            if(fin_epoch!=it_sat->second.end()){
                                SQM_CN0_Parser::Info info = fin_epoch->second;
                                if(info.cn0.first && info.cn0.second<thresholds.cn0_drops){
                                    this->results_.cn0_drops[gss_id][sat_id][obs][epoch] = info.cn0.second;
                                }
                                if(info.pog.first){
                                    this->results_.cn0_stats[gss_id][sat_id][obs].push_back(info.cn0.second);
                                    this->results_.pog_stats[gss_id][sat_id][obs].push_back(info.pog.second);
                                }
                            }
                            prev_epoch = epoch;
                        }
                    }
                }
            }                   
        }
    }

} // sqc
} // grc