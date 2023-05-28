
/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution, 
 *             dissemination or disclosure not allowed. 
 * Project:    GRC 
 * File:       input_handler.cpp
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#include <input_handler.h>
#include <Misc/util_functions2.h>
#include <Algos/Geometry.h>
#include <Algos/math_functions.h>
#include <Logger.h>

namespace grc{
namespace sqc{


    void InputHandler::SetInputFiles(const std::vector<std::string>& input_files){
        for (const std::string& file: input_files){
            this->input_files_.push_back(file);
        }
    }
    

    bool InputHandler::PrepareSession(std::string& error_info){
        error_info = "";
        if (this->input_files_.empty()) {
            Logger::log_message(Logger::ERROR,Logger::ALGORITHM,Logger::DATA_AVAILABILITY,"No input files provided");
            return true;
        }

        for(std::string& file: this->input_files_){
            file_utils::File_Type type;
            if (!get_GRC_file_type(file,type)) {
                Logger::log_message(Logger::WARNING,Logger::ALGORITHM,Logger::INVALID_FILENAME,"Unknown filename "+file);
                continue;
            }
            // Classify files
            switch (type) {
                case file_utils::TYPE_SQM_CN0 : cn0_files_.push_back(file_utils::SQM_CN0(file)); break;
                case file_utils::TYPE_SQM_CODE_PHASE_ERROR : code_phase_error_files_.push_back(file_utils::SQM_CODE_PHASE_ERROR(file)); break;
                default: Logger::log_message(Logger::WARNING,Logger::ALGORITHM,Logger::INVALID_FILENAME,"File not processed by SQC "+file);
            }
        }
    }


    bool InputHandler::ParseSqmCn0(){
        std::string error_info="";
        for(grc::file_utils::SQM_CN0 file: this->cn0_files_){
            if(!SQM_CN0_Parser::parse_file(file,this->sqm_cn0_,error_info)){
                Logger::log_message(Logger::ERROR,Logger::READER,Logger::PARSING_PROBLEM,error_info);
                return false;
            }
        }
        return true;
    }


    bool InputHandler::ParseSqmCodePhaseError(){
        std::string error_info="";
        for(grc::file_utils::SQM_CODE_PHASE_ERROR file: this->code_phase_error_files_){
            if(!SQM_COD_PHAS_ERR::parse_file(file,this->sqm_code_phase_error_,error_info)){
                Logger::log_message(Logger::ERROR,Logger::READER,Logger::PARSING_PROBLEM,error_info);
                return false;
            }
        }
        return true;
    }

} // sqc
} // grc