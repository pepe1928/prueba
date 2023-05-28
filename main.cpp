/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution,
 *             dissemination or disclosure not allowed.
 * Project:    GRC
 * File:       main.cpp
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <algorithm_manager.h>
#include <iostream>
#include <string>
#include "Misc/file_util_functions.h"
#include "Logger.h"

using namespace boost::filesystem;
using namespace grc;

bool get_mode(const std::string & str_mode, AlgorithmManager::Mode & mode) {
    // This function set the mode of the SQC
    std::string temp = boost::to_upper_copy(str_mode);
    if (temp=="CN0_DROPS") {
        mode = AlgorithmManager::Mode::CN0_DROPS;
    }
    else if (temp=="CARR_PH_NOISE") {
        mode = AlgorithmManager::Mode::CARR_PHASE_NOISE;
    }
    else if (temp=="POG-STATS") {
        mode = AlgorithmManager::Mode::POG-STATS;
    }
    else if (temp=="CN0_STATS") {
        mode = AlgorithmManager::Mode::CN0_STATS;
    }
    else if (temp=="ALL") {
        mode = AlgorithmManager::Mode::ALL;
    }
    else {
        return false;
    }
    return true;
}

// Main function
int main(int argc, char **argv) {
    namespace po = boost::program_options;

    // Help message creation
    const std::string help_msg  = std::string("Usage: ") + std::string(argv[0]) + std::string("[options] [CN0-DROPS/CARR-PH-NOISE/POG-STATS/CN0-STATS/CS/ALL] session_id start_date(YYYY/MM/DD-HH:MM:SS) end_date(YYYY/MM/DD-HH:MM:SS) config_file files\n");

    boost::system::error_code ec;

    // Paths definition
    boost::filesystem::path output_path=boost::filesystem::current_path()/"outputs";

    // Options
    po::options_description gral_desc("General options");
    gral_desc.add_options()
                ("help", "Produce help message")
                ("version","Output the version number")
                ("config_template","Produce configuration file template")
                ;

    po::options_description writer_desc("Writer options");
    writer_desc.add_options()
                ("output_directory", po::value<std::string>()->default_value(output_path.string()), "Output directory")
                ("product_id", po::value<std::string>()->default_value(""), "Additional identifier added to the filename additional information fields")
                ;
    
    po::options_description input_desc("Input options");
    input_desc.add_options()
                ("session_file", po::value<std::string>()->default_value(""), "Session file")
                ;

    po::options_description log_desc("Log options");
    log_desc.add_options()
                ("log_level", po::value<Logger::Category>()->default_value(Logger::INFO), "Log level (DEBUG,INFO,WARNING,ERROR)")
                ("hide_log_messages","Do not show log messages on screen")
                ;

    po::options_description hidden_desc("Hidden options");
    hidden_desc.add_options()
                ("mode", po::value<std::string>()->required())
                ("session_id", po::value<int>()->required())
                ("start_date", po::value<std::string>()->required())
                ("end_date", po::value<std::string>()->required())
                ("config_file", po::value<vstring>()->required())
                ("input_files", po::value<vector <std::string> >())
                ;

    po::positional_options_description p;
    p.add("mode",1).add("session_id",1).add("start_date",1).add("end_date",1).add("config_file",1).add("input_files",-1);

    po::options_description visible_desc("Allowed options");
    visible_desc.add(gral_desc).add(writer_desc).add(input_desc).add(log_desc);

    po::options_description all_desc("All");
    all_desc.add(visible_desc).add(hidden_desc);

    if (argc==1) {
        cout << help_msg;
        cout << visible_desc;
        return EXIT_SUCCESS;
    }

    po::variables_map parsed_values;
    try {
        po::store(po::command_line_parser(argc, argv).options(all_desc).run(),parsed_values);
        // Print version
        if (parsed_values.count("version")) {
            std::cout << RELEASE_INFO<<std::endl;
            return EXIT_SUCCESS;
        }

        //Print help message
        if (parsed_values.count("help")) {
            std::cout << help_msg;
            std::cout << visible_desc;
            return EXIT_SUCCESS;
        }
        // Create a configuration template
        if (parsed_values.count("config_template")) {
            std::cout<<"Sorry but you are going to have to do it yourself ;)\n";
            return EXIT_SUCCESS;
        }
        po::notify(parsed_values);
    }
    catch (exception &e) {
        cout<<"Error parsing command line: "<<e.what()<<"\n\n";
        cout << help_msg;
        cout << visible_desc;
        return (EXIT_FAILURE);
    }


    // Set output directory
    std::string out_dir = parsed_values["output_directory"].as<std::string>();
    grc::file_utils::create_directory_recursively(boost::filesystem::path(out_dir));

    if (parsed_values.count("hide_log_messages")) {
        grc::Logger::display_mgs_console(false);
    }
    grc::Logger::set_category(parsed_values["log_level"].as<grc::Logger::Category>());

    grc::GPS_Time start_interval, end_interval;
    std::string start_int_str(parsed_values["start_date"].as<std::string>());
    std::string end_int_str(parsed_values["end_date"].as<std::string>());

    // Get start date
    if (!grc::file_utils::get_date_from_command_line(start_int_str,start_interval)) {
        std::cerr<<"Invalid start date "<< start_int_str<<". Format is YYYY/MM/DD-HH:MM:SS \n";
        std::cerr<< help_msg;
        return EXIT_FAILURE;
    }

    // Get end date
    if (!grc::file_utils::get_date_from_command_line(end_int_str,end_interval)) {
        std::cerr<<"Invalid end date "<< end_int_str<<". Format is YYYY/MM/DD-HH:MM:SS \n";
        std::cerr<< help_msg;
        return EXIT_FAILURE;
    }

    // Check date coherence
    if (end_interval < start_interval) {
        std::cerr<<"Invalid time interval "<<start_int_str<<" "<<end_int_str <<"\n";
        std::cerr<< help_msg;
        return EXIT_FAILURE;
    }

    // Check session ID
    unsigned int session_id = parsed_values["session_id"].as<unsigned int>();

    // Create the log file in the out path
    grc::GPS_Time crea_date = grc::GPS_Time::now();
    std::string log_filename = grc::file_utils::set_GRC_filename(session_id,grc::file_utils::ORIGIN_FEARED_EVENTS,grc::file_utils::TYPE_LOG,start_interval,crea_date,"","log");
    std::string error_info;
    if (!grc::Logger::init_single_file(out_dir,log_filename,error_info)) {
        std::cerr<<error_info<<std::endl;
        return EXIT_FAILURE;
    }

    // Files to be parsed
    std::string config_file  = parsed_values["config_file"].as<std::string>();
    std::vector<std::string> files_to_be_processed;

    if (parsed_values.count("input_files")) {
        files_to_be_processed = parsed_values["input_files"].as<std::vector<std::string> >();
    }

    if (!grc::file_utils::parse_session_file(parsed_values["session_file"].as<std::string>(),files_to_be_processed,error_info)) {
        std::cerr<<error_info<<std::endl;
        return EXIT_FAILURE;
    }

    AlgorithmManager::Mode mode;  // Mode = CN0-DROPS/CARR-PH-NOISE/POG-STATS/CN0-STATS/CS/ALL
    if (!get_mode(parsed_values["mode"].as<std::string>(),mode)) {
        cerr<<"Invalid mode "<<parsed_values["mode"].as<string>() <<". Modes are [CN0-DROPS/CARR-PH-NOISE/POG-STATS/CN0-STATS/CS/ALL] "<<"\n";
        cerr<< help_msg;
        return (EXIT_FAILURE);
    }

    grc::Logger::log_message(grc::Logger::INFO,grc::Logger::MAIN,grc::Logger::START_PROCESSING,std::string(RELEASE_INFO)+" started");
    grc::Logger::log_message(grc::Logger::INFO,grc::Logger::MAIN,grc::Logger::ALGO_PROCESSING,"Processing from "+start_interval.calendar_text_str()+" to "+end_interval.calendar_text_str());

    bool exit_failure = true;

    auto start = std::chrono::system_clock::now();
    try {
        grc::feared_events::AlgorithmManager manager(out_dir, //Output directory
                                                    session_id, //Session number
                                                    start_interval, //Start date
                                                    end_interval, //End date
                                                    crea_date,
                                                    config_file, //Configuration file
                                                    files_to_be_processed, //Files
                                                    mode //Mode 
                                                    );

        exit_failure = !manager.Process();
    }
    catch (const boost::filesystem::filesystem_error &e) {
        grc::Logger::log_message(grc::Logger::ERROR,grc::Logger::MAIN,grc::Logger::FILE_SYSTEM,std::string("File system error. ")+e.what());
    }
    catch (const std::exception &e) {
        grc::Logger::log_message(grc::Logger::ERROR,grc::Logger::MAIN,grc::Logger::STD_EXCEPTION,std::string("Uncaught standard exception ")+e.what());
    }
    catch (...) {
        grc::Logger::log_message(grc::Logger::ERROR,grc::Logger::MAIN,grc::Logger::UNKNOWN_EXCEPTION,"Unknown exception raised");
    }

    grc::Logger::log_message(grc::Logger::INFO,grc::Logger::MAIN,grc::Logger::END_PROCESSING,"Program finished");
    return exit_failure?EXIT_FAILURE:EXIT_SUCCESS;
}

