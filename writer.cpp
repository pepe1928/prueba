
/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution, 
 *             dissemination or disclosure not allowed. 
 * Project:    GRC 
 * File:       writer.cpp
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#include <writer.h>

namespace grc{
namespace sqc{
    
SqcWriter::SqcWriter(const boost::filesystem::path& output_directory,
                     const std::string& preamble,
                     file_utils::Filename_Type type,
                     bool set_tree_directory): File_Recorder(output_directory, preamble, "res", "SqcWriter", type, set_tree_directory),content_written_(false)
{   
    open(GPS_Time::now(), true);
    stream_out_ << "STATION  SATELLITE  SIGNAL  EPOCH                CN0        \n";
    stream_out_ << "-------  ---------  ------  -------------------  -----------\n";
}

SqcBetweenEpochWriter::SqcBetweenEpochWriter(const boost::filesystem::path& output_directory,
                                             const std::string& preamble,
                                             file_utils::Filename_Type type,
                                             bool set_tree_directory): File_Recorder(output_directory, preamble, "res", "SqcBetweenEpochWriter", type, set_tree_directory),content_written_(false)
{   
    open(GPS_Time::now(), true);
    stream_out_ << "STATION  SATELLITE  SIGNAL  INI_EPOCH            FIN_EPOCH            INI_CN0      FIN_CN0    \n";
    stream_out_ << "-------  ---------  ------  -------------------  -------------------  -----------  -----------\n";
}

SqcStatsWriter::SqcStatsWriter(const boost::filesystem::path& output_directory,
                               const std::string& preamble,
                               file_utils::Filename_Type type,
                               bool set_tree_directory): File_Recorder(output_directory, preamble, "res", "SqcStatsWriter", type, set_tree_directory),content_written_(false)
{   
    open(GPS_Time::now(), true);
    stream_out_ << "STATION  SATELLITE  SIGNAL  MIN          AVG          RMS          PER95        MAX          \n";
    stream_out_ << "-------  ---------  ------  -----------  -----------  -----------  -----------  -----------  \n";
}

SqcCarrierPhaseNoiseWriter::SqcCarrierPhaseNoiseWriter(const boost::filesystem::path& output_directory,
                                                       const std::string& preamble,
                                                       file_utils::Filename_Type type,
                                                       bool set_tree_directory): File_Recorder(output_directory, preamble, "res", "SqcWriter", type, set_tree_directory),content_written_(false)
{   
    open(GPS_Time::now(), true);
    stream_out_ << "EPOCH                SIGNAL  SATELLITE  TARGET\n";
    stream_out_ << "-------------------  ------  ---------  ------\n";
}

void SqcWriter::Write(const std::string& gss_id,
                       const SAT_ID& sat_id, 
                       const std::string& observation, 
                       const GPS_Time& epoch, 
                       const std::string& value){

    stream_out_ << std::fixed << std::left << std::setw(7) << gss_id <<"  "
                << std::setw(9)  << std::left << sat_id << "  "
                << std::setw(6)  << std::left << observation << "  "
                << std::setw(19) << std::left << epoch.calendar_text_str() << "  "
                << std::setw(11) << std::left << value << "\n";
                    
    this->content_written_ = true;
}

void SqcBetweenEpochWriter::Write(const std::string& gss_id, 
                                  const SAT_ID& sat_id, 
                                  const std::string& observation, 
                                  const GPS_Time& epoch_ini,
                                  const GPS_Time& epoch_fin,
                                  double ini_value,
                                  double fin_value){

    stream_out_ << std::fixed << std::left << std::setw(7) << gss_id <<"  "
                << std::setw(9)  << std::left << sat_id << "  "
                << std::setw(6)  << std::left << observation << "  "
                << std::setw(19) << std::left << epoch_ini.calendar_text_str() << "  "
                << std::setw(19) << std::left << epoch_fin.calendar_text_str() << "  "
                << std::setw(11) << std::left << ini_value << "  "
                << std::setw(11) << std::left << fin_value << "\n";
                    
    this->content_written_ = true;
}

void SqcStatsWriter::Write(const std::string& gss_id, 
                           const SAT_ID& sat_id, 
                           const std::string& observation, 
                           double min,
                           double avg,
                           double rms,
                           double percentile,
                           double max){
    stream_out_ << std::fixed << std::left << std::setw(7) << gss_id <<"  "
                << std::setw(9)  << std::left << sat_id << "  "
                << std::setw(6)  << std::left << observation << "  "
                << std::setw(11) << std::left << min << "  "
                << std::setw(11) << std::left << avg << "  "
                << std::setw(11) << std::left << rms << "  "
                << std::setw(11) << std::left << percentile << "  "
                << std::setw(11) << std::left << max << "\n";
                    
    this->content_written_ = true;
}

void SqcCarrierPhaseNoiseWriter::Write(const GPS_Time& epoch,
                                       const std::string& observation,
                                       const SAT_ID& sat_id,
                                       const std::string& value){

    stream_out_ << std::fixed << std::left << std::setw(19) << epoch.calendar_text_str() <<"  "
                << std::setw(6)  << std::left << observation << "  "
                << std::setw(9)  << std::left << sat_id << "  "
                << std::setw(6)  << std::left << value << "\n";
}

void RemoveEmptyFile(const boost::filesystem::path& absolute_path, bool content_written){
    if (!content_written){
        if (boost::filesystem::is_regular_file(absolute_path)){
            boost::system::error_code ec;
            boost::filesystem::remove(absolute_path, ec);
        }
    }
}

SqcWriter::~SqcWriter() {
    this->close();
    RemoveEmptyFile(this->GetPathFile(), this->content_written_);
}

SqcBetweenEpochWriter::~SqcBetweenEpochWriter() {
    this->close();
    RemoveEmptyFile(this->GetPathFile(), this->content_written_);
}

SqcStatsWriter::~SqcStatsWriter() {
    this->close();
    RemoveEmptyFile(this->GetPathFile(), this->content_written_);
}

SqcCarrierPhaseNoiseWriter::~SqcCarrierPhaseNoiseWriter() {
    this->close();
    RemoveEmptyFile(this->GetPathFile(), this->content_written_);
}

} // sqc
} // grc