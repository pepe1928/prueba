/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution, 
 *             dissemination or disclosure not allowed. 
 * Project:    GRC 
 * File:       input_handler.h
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#ifndef SQC_INPUT_HANDLER_H_
#define SQC_INPUT_HANDLER_H_

#include <Types/Common.h>
#include <HealthStatusManager.h>
#include <Parser/SQM_CNO_Parser.h>
#include <Parser/SQM_COD_PHAS_ERR.h>
#include <config.h>
#include <ObservationManager.h>
#include <NavigationMessageManager.h>
#include <ReferenceDataManager.h>

namespace grc{
namespace sqc{
    
    class InputHandler{
        private:
            const Config& configuration_;
            Health_Status_Manager hs_mgr_;
            ReferenceDataManager ref_mgr_;
            std::vector<std::string> input_files_;

            std::vector<file_utils::SQM_CN0> cn0_files_;
            std::vector<file_utils::SQM_CODE_PHASE_ERROR> code_phase_error_files_;

            SQM_CN0_Parser::SQM_CN0_Data sqm_cn0_;
            SQM_COD_PHAS_ERR::SQM_COD_Data sqm_code_phase_error_;

        public:
            // General
            InputHandler(const Config& configuration, const GPS_Time& start, const GPS_Time& end):configuration_(configuration),
                                                                                                  ref_mgr_{start,end}{};
            InputHandler(const Config& configuration, 
                         const std::vector<std::string>& input_files,
                         const GPS_Time& start,
                         const GPS_Time& end):configuration_(configuration),
                                              input_files_(input_files),
                                              ref_mgr_{start,end}{};

            bool PrepareSession(std::string& error_info);

            //Parsers
            bool ParseSqmCn0();
            bool ParseSqmCodePhaseError();

            // Getters
            const SQM_CN0_Parser::SQM_CN0_Data& GetCN0Data()const{return sqm_cn0_;}
            const SQM_COD_PHAS_ERR::SQM_COD_Data& GetCodePhaseErrorData()const{return sqm_code_phase_error_;}
            
            // Setters
            void SetInputFiles(const std::vector<std::string>& input_files);

    };

} // namespace sqc
} // namespace grc

#endif // SQC_INPUT_HANDLER_H_