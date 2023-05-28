/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution,
 *             dissemination or disclosure not allowed.
 * Project:    GRC
 * File:       algorithm_manager.h
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#ifndef SQC_ALGORITHM_MANAGER_H_
#define SQC_ALGORITHM_MANAGER_H_

#include <input_handler.h>
#include <compute_manager.h>
#include <vector>
#include <config.h>
#include <writer.h>
#include <Types/Common.h>
#include <Types/Observations.h>
#include <Writer/File_Recorder.h>

namespace grc{
namespace sqc{

class AlgorithmManager {
    public:
        enum Mode {
            CN0_DROPS,
            CN0_DROPS_BETWEEN_EPOCH,
            CARR_PH_NOISE,
            POG_STATS,
            CN0_STATS,
            ALL
        };

        struct SQCResults {
            struct DropsBtwEpoch{
                GPS_Time ini_epoch, fin_epoch;
                double ini_cn0, fin_cn0;
            };
            std::map<GSS_ID,std::map<SAT_ID,std::map<SignalID,std::vector<DropsBtwEpoch>>>> cn0_drops_btw_epoch;
            std::map<GSS_ID,std::map<SAT_ID,std::map<SignalID,std::map<GPS_Time,double>>>> cn0_drops;
            std::map<GSS_ID,std::map<SAT_ID,std::map<SignalID,std::vector<double>>>> pog_stats;
            std::map<GSS_ID,std::map<SAT_ID,std::map<SignalID,std::vector<double>>>> cn0_stats;
            std::map<GPS_Time,std::map<SignalID,std::map<SAT_ID,std::string>>> carr_ph_noise;
        };
        

        AlgorithmManager(const std::string & out_dir,
                        unsigned int session_id,
                        const GPS_Time & start,
                        const GPS_Time & end,
                        const GPS_Time & crea_date,
                        const std::string & configFile,
                        const std::vector<std::string> & files_to_be_processed,
                        const Mode mode);

        bool Process();

    private:
        std::string output_dir_;
        unsigned int session_id_;
        GPS_Time start_;
        GPS_Time end_;
        GPS_Time creation_date_;
        Mode mode_;
        SQCResults results_;

        Config configuration_;
        InputHandler inputs_;
        ComputeManager computers_;
        std::map<Mode,std::unique_ptr<File_Recorder>> writers_;

        bool ParseInputs();
        bool ProcessOutputs();
        bool WriteResults();
        void InitializeWriters();


};

} // sqc
} // grc

#endif /* SQC_ALGORITHM_MANAGER_H_ */
