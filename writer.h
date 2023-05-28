/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution, 
 *             dissemination or disclosure not allowed. 
 * Project:    GRC 
 * File:       writer.h
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#ifndef SQC_WRITER_H_
#define SQC_WRITER_H_

#include <Writer/File_Recorder.h>
#include <Types/GPSTime.h>
#include <Types/Space_Vector.h>
#include <Types/Common.h>
#include <Types/Observations.h>
#include <Misc/file_util_functions.h>

namespace grc{
namespace sqc{

    void RemoveEmptyFile(const boost::filesystem::path& absolute_path, bool content_written);

    class SqcWriter: public File_Recorder{
        public:
            SqcWriter(SqcWriter&& other)=default;
            SqcWriter& operator=(SqcWriter&& other) = default;
            SqcWriter(const boost::filesystem::path& output_directory,
                      const std::string& preamble,
                      file_utils::Filename_Type type = file_utils::Filename_Type::only_direc,
                      bool set_tree_directory=false);
            virtual ~SqcWriter();

            void Write(const std::string& gss_id,
                       const SAT_ID& sat_id, 
                       const std::string& observation, 
                       const GPS_Time& epoch, 
                       const std::string& value);
        
        private:
            bool delete_empty_file_;
            bool content_written_;
    };

    class SqcBetweenEpochWriter: public File_Recorder{
        public:
            SqcBetweenEpochWriter(SqcBetweenEpochWriter&& other)=default;
            SqcBetweenEpochWriter& operator=(SqcBetweenEpochWriter&& other) = default;
            SqcBetweenEpochWriter(const boost::filesystem::path& output_directory,
                      const std::string& preamble,
                      file_utils::Filename_Type type = file_utils::Filename_Type::only_direc,
                      bool set_tree_directory=false);
            virtual ~SqcBetweenEpochWriter();

            void Write(const std::string& gss_id, 
                       const SAT_ID& sat_id, 
                       const std::string& observation, 
                       const GPS_Time& epoch_ini,
                       const GPS_Time& epoch_fin,
                       double ini_value,
                       double fin_value);
        
        private:
            bool delete_empty_file_;
            bool content_written_;
    };


class SqcStatsWriter: public File_Recorder{
        public:
            SqcStatsWriter(SqcStatsWriter&& other)=default;
            SqcStatsWriter& operator=(SqcStatsWriter&& other) = default;
            SqcStatsWriter(const boost::filesystem::path& output_directory,
                      const std::string& preamble,
                      file_utils::Filename_Type type = file_utils::Filename_Type::only_direc,
                      bool set_tree_directory=false);
            virtual ~SqcStatsWriter();
            
            void Write(const std::string& gss_id, 
                       const SAT_ID& sat_id, 
                       const std::string& observation, 
                       double min,
                       double avg,
                       double rms,
                       double percentile,
                       double max);
        
        private:
            bool delete_empty_file_;
            bool content_written_;
    };

    class SqcCarrierPhaseNoiseWriter: public File_Recorder{
        public:
            SqcCarrierPhaseNoiseWriter(SqcCarrierPhaseNoiseWriter&& other)=default;
            SqcCarrierPhaseNoiseWriter& operator=(SqcCarrierPhaseNoiseWriter&& other) = default;
            SqcCarrierPhaseNoiseWriter(const boost::filesystem::path& output_directory,
                                       const std::string& preamble,
                                       file_utils::Filename_Type type = file_utils::Filename_Type::only_direc,
                                       bool set_tree_directory=false);
            virtual ~SqcCarrierPhaseNoiseWriter();

            void Write(const GPS_Time& epoch,
                       const std::string& observation,
                       const SAT_ID& sat_id,
                       const std::string& value);
        
        private:
            bool delete_empty_file_;
            bool content_written_;
    };


} // sqc
} // grc

#endif // SQC_WRITER_H_