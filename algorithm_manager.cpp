/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution,
 *             dissemination or disclosure not allowed.
 * Project:    GRC
 * File:       AlgorithmManager.cpp
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#include <algorithm_manager.h>
#include <Types/Observations.h>
#include <Misc/file_util_functions.h>
#include <Logger.h>

namespace grc{
namespace sqc{

AlgorithmManager::AlgorithmManager(const std::string & out_dir,
                                    unsigned int session_id,
                                    const GPS_Time & start,
                                    const GPS_Time & end,
                                    const GPS_Time & crea_date,
                                    const std::string & config_file,
                                    const std::vector<std::string> & files_to_be_processed,
                                    const Mode mode):output_dir_(out_dir),
                                                    inputs_(this->configuration_,start,end),
                                                    computers_(this->configuration_,this->inputs_,this->results_),
                                                    session_id_(session_id),
                                                    start_(start),
                                                    end_(end),
                                                    creation_date_(crea_date),
                                                    mode_(mode)
                                                    
    {
        std::string error_info;
        if (!this->configuration_.Parse(config_file,error_info)){
            throw std::runtime_error(error_info);
        }

        this->inputs_.SetInputFiles(files_to_be_processed);
        if (!this->inputs_.PrepareSession(error_info)){
            throw std::runtime_error(error_info);
        }
    }


bool AlgorithmManager::ParseInputs(){
    return this->inputs_.ParseSqmCn0() &&
           this->inputs_.ParseSqmCodePhaseError();
}

bool AlgorithmManager::ProcessOutputs(){
    if(this->mode_==Mode::CN0_DROPS|| this->mode_== Mode::POG_STATS || this->mode_ == Mode::CN0_STATS ||this->mode_== Mode::ALL){
        Logger::log_message(Logger::INFO,Logger::ALGORITHM,Logger::START_PROCESSING,"Processing CN0");
        if (!this->computers_.ProcessCn0Outputs()) {
            return false;
        }
    }
    
    if(this->mode_==Mode::CARR_PH_NOISE || this->mode_==Mode::ALL){
        Logger::log_message(Logger::INFO,Logger::ALGORITHM,Logger::START_PROCESSING,"Processing CARR-PH-NOISE");
        if (!this->computers_.ProcessCarrierPhaseNoiseOutputs()) {
            return false;
        }
    }
    return true;
}

bool AlgorithmManager::WriteResults(){
    this->InitializeWriters();
    if(this->mode_==Mode::CN0_DROPS||this->mode_==Mode::ALL){

        for(auto gss=results_.cn0_drops.begin();gss!=results_.cn0_drops.end();gss++){
            for(auto sat = gss->second.begin();sat!=gss->second.end();sat++){
                for(auto obs = sat->second.begin();obs!= sat->second.end();obs++){
                    for(auto epoch = obs->second.begin();epoch!= obs->second.end();epoch++){
                        if (!this->writers_.find(Mode::CN0_DROPS)->second.Write(gss->first, sat->first, obs->first, epoch->first, epoch->second)) {
                            Logger::log_message(Logger::ERROR,Logger::WRITER,Logger::WRITING_PROBLEM, "Error writing CN0_DROPS");
                            return false;
                        }
                    }
                }
            }
        }

        for(auto gss=results_.cn0_drops_btw_epoch.begin();gss!=results_.cn0_drops_btw_epoch.end();gss++){
            for(auto sat = gss->second.begin();sat!=gss->second.end();sat++){
                for(auto values_between_epoch = sat->second.begin();values_between_epoch!= sat->second.end();values_between_epoch++){
                    
                    if (!this->writers_.find(Mode::CN0_DROPS_BETWEEN_EPOCH)->second.Write(gss->first, sat->first, obs->first, 
                                                                                   values_between_epoch->second.ini_epoch, 
                                                                                   values_between_epoch->second.fin_epoch, 
                                                                                   values_between_epoch->second.ini_cn0,
                                                                                   values_between_epoch->second.fin_cn0)) {
                        Logger::log_message(Logger::ERROR,Logger::WRITER,Logger::WRITING_PROBLEM, "Error writing CN0_DROPS between epochs");
                        return false;
                    }

                }
            }
        }
    }
    if(this->mode_==Mode::POG_STATS||this->mode_==Mode::ALL){

        for(auto gss=results_.pog_stats.begin();gss!=results_.pog_stats.end();gss++){
            for(auto sat = gss->second.begin();sat!=gss->second.end();sat++){
                for(auto values = sat->second.begin();values!= sat->second.end();values){
                    double min,max,mean,sigman,rm,per;
                    min = *std::min_element(values->second.begin(),values->second.end());
                    compute_stats(values->second,mean,sigman,rm, max, per,false);

                    if (!this->writers_.find(Mode::POG_STATS)->second.Write(gss->first, sat->first, obs->first, 
                                                                     min, avg, rms, percentile, max)) {
                        Logger::log_message(Logger::ERROR,Logger::WRITER,Logger::WRITING_PROBLEM, "Error writing POG_STATS");
                        return false;
                    }

                }
            }
        }
    }
    if(this->mode_==Mode::CN0_STATS||this->mode_==Mode::ALL){

        for(auto gss=results_.cn0_stats.begin();gss!=results_.cn0_stats.end();gss++){
            for(auto sat = gss->second.begin();sat!=gss->second.end();sat++){
                for(auto values = sat->second.begin();values!= sat->second.end();values){
                    double min,max,mean,sigman,rm,per;
                    min = *std::min_element(values->second.begin(),values->second.end());
                    compute_stats(values->second,mean,sigman,rm, max, per,false);

                    if (!this->writers_.find(Mode::CN0_STATS)->Write(gss->first, sat->first, obs->first, 
                                                                     min, avg, rms, percentile, max)) {
                        Logger::log_message(Logger::ERROR,Logger::WRITER,Logger::WRITING_PROBLEM, "Error writing CN0_STATS");
                        return false;
                    }

                }
            }
        }
    }
    if(this->mode_==Mode::CARR_PH_NOISE||this->mode_==Mode::ALL){

        for(auto epoch=results_.carr_ph_noise.begin();epoch!=results_.carr_ph_noise.end();epoch++){
            for(auto obs = epoch->second.begin();obs!=epoch->second.end();obs++){
                for(auto sat = obs->second.begin();sat!= obs->second.end();sat){
                    if (!this->writers_.find(Mode::CARR_PH_NOISE)->Write(epoch->first, obs->first, sat->first, sat->second)) {
                        Logger::log_message(Logger::ERROR,Logger::WRITER,Logger::WRITING_PROBLEM, "Error writing CARR_PH_NOISE");
                        return false;
                    }
                }
            }
        } 
    }
    return true;
}

void AlgorithmManager::InitializeWriters(){
    if(this->mode_==Mode::CN0_DROPS||this->mode_==Mode::ALL){
        this->writers_[Mode::CN0_DROPS] = std::unique_ptr<File_Recorder>(new SqcWriter(this->output_dir_,
                                                                                   file_utils::set_GRC_filename(this->session_id_, file_utils::ORIGIN_SQC, file_utils::TYPE_SQC_CN0_DROPS, this->start_, this->creation_date_)));

        Logger::log_message(Logger::INFO,Logger::WRITER,Logger::FILE_CREATION,"Initializing writer: "+this->writers_[Mode::CN0_DROPS]->GetPathFile().string());

        this->writers_[Mode::CN0_DROPS_BETWEEN_EPOCH] = std::unique_ptr<File_Recorder>(new SqcBetweenEpochWriter(this->output_dir_,
                                                                                                                         file_utils::set_GRC_filename(this->session_id_,file_utils::ORIGIN_SQC,file_utils::TYPE_SQC_CN0_DROPS,this->start_,this->creation_date_,"EPOCH")));

        Logger::log_message(Logger::INFO,Logger::WRITER,Logger::FILE_CREATION,"Initializing writer: "+this->writers_[Mode::CN0_DROPS_BETWEEN_EPOCH]->GetPathFile().string());
        
    }
    if(this->mode_==Mode::POG_STATS||this->mode_==Mode::ALL){
        this->writers_[Mode::POG_STATS] = std::unique_ptr<File_Recorder>(new SqcStatsWriter(this->output_dir_,
                                                                                             file_utils::set_GRC_filename(this->session_id_,file_utils::ORIGIN_SQC,file_utils::TYPE_SQC_POG_STATS,this->start_,this->creation_date_)));

        Logger::log_message(Logger::INFO,Logger::WRITER,Logger::FILE_CREATION,"Initializing writer: "+this->writers_[Mode::POG_STATS]->GetPathFile().string());
    }
    if(this->mode_==Mode::CN0_STATS||this->mode_==Mode::ALL){
        this->writers_[Mode::CN0_STATS] = std::unique_ptr<File_Recorder>(new SqcStatsWriter(this->output_dir_,
                                                                                             file_utils::set_GRC_filename(this->session_id_,file_utils::ORIGIN_SQC,file_utils::TYPE_SQC_CN0_STS,this->start_,this->creation_date_)));

        Logger::log_message(Logger::INFO,Logger::WRITER,Logger::FILE_CREATION,"Initializing writer: "+this->writers_[Mode::CN0_STATS]->GetPathFile().string());
    }
    if(this->mode_==Mode::CARR_PH_NOISE||this->mode_==Mode::ALL){
        this->writers_[Mode::CARR_PH_NOISE] = std::unique_ptr<File_Recorder>(new SqcCarrierPhaseNoiseWriter(this->output_dir_,
                                                                                       file_utils::set_GRC_filename(this->session_id_,file_utils::ORIGIN_SQC,file_utils::TYPE_SQM_CODE_PHASE_ERROR,this->start_,this->creation_date_)));

        Logger::log_message(Logger::INFO,Logger::WRITER,Logger::FILE_CREATION,"Initializing writer: "+this->writers_[Mode::CARR_PH_NOISE]->GetPathFile().string());
    }
}

bool AlgorithmManager::Process(){
    std::string error_details = "";

    this->ParseInputs();
    this->ProcessOutputs();
    this->WriteResults();

    return true;
}

} // sqc
} // grc