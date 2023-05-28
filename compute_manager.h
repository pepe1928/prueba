/********************************************************************
 * Copyright : EU Proprietary information. Unauthorized distribution, 
 *             dissemination or disclosure not allowed. 
 * Project:    GRC 
 * File:       compute_manager.h
 * Language:   C/C++
 * Author:     GMV
 *******************************************************************/
#ifndef SQC_COMPUTE_MANAGER_H_
#define SQC_COMPUTE_MANAGER_H_

#include <algorithm_manager.h>
#include <Types/Common.h>
#include <config.h>
#include <input_handler.h>
#include <queue>
#include <ObservationManager.h>


namespace grc{
namespace sqc{

    struct AlgorithmManager::SQCResults; 
    
    class ComputeManager{
        private:
            const Config& configuration_;
            const InputHandler& inputs_;
            GPS_Time start_;
            GPS_Time end_;
            AlgorithmManager::SQCResults& results_;

        public:
            // General
            ComputeManager(const Config& configuration, const InputHandler& inputs, AlgorithmManager::SQCResults& results):configuration_(configuration),inputs_(inputs),results_(results){};

            void ProcessCn0Outputs();
            void ProcessCarrierPhaseNoiseOutputs();
    };

} // sqc
} // grc

#endif // SQC_COMPUTE_MANAGER_H_