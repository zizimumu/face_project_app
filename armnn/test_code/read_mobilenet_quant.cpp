#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/time.h>

#include "armnn/BackendId.hpp"
#include "armnn/IRuntime.hpp"
#include "armnnTfLiteParser/ITfLiteParser.hpp"
#include "TensorIOUtils.hpp"
         
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/variant.hpp"

double get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

#define unit_t (unsigned char)


// using namespace armnn::test;



int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "minimal <tflite model>\n");
    return 1;
  }
  const char* tflite_file = argv[1];

    struct timeval start_time, stop_time;
    
    const std::string inputName = "input";
    const std::string outputName = "output";

    using TContainer = boost::variant<std::vector<unsigned char>>;
    
    const unsigned int inputTensorWidth = 224;
    const unsigned int inputTensorHeight = 224;
    const unsigned int inputTensorBatchSize = 1;
    unsigned int outputNumElements = 1001;
    const armnn::DataLayout inputTensorDataLayout = armnn::DataLayout::NHWC;




  
      // Load and preprocess input image
    const std::vector<TContainer> inputDataContainers = { std::vector<unsigned char>(inputTensorWidth*inputTensorHeight*3,110)}; //inputDataContainers(inputTensorWidth*inputTensorHeight*3);
 

    // Import the TensorFlow model. 
    // Note: use CreateNetworkFromBinaryFile for .tflite files.
    armnnTfLiteParser::ITfLiteParserPtr parser = armnnTfLiteParser::ITfLiteParser::Create();
    armnn::INetworkPtr network = parser->CreateNetworkFromBinaryFile(tflite_file);

    // Find the binding points for the input and output nodes 
        
     using BindingPointInfo = armnnTfLiteParser::BindingPointInfo;    
    const std::vector<BindingPointInfo> inputBindings  = { parser->GetNetworkInputBindingInfo(0, inputName) };
    const std::vector<BindingPointInfo> outputBindings = { parser->GetNetworkOutputBindingInfo(0, outputName) };        
 
    // Output tensor size is equal to the number of model output labels
    //const unsigned int outputNumElements = modelOutputLabels.size();
    std::vector<TContainer> outputDataContainers = { std::vector<unsigned char>(outputNumElements)};


    // Optimize the network for a specific runtime compute 
    // device, e.g. CpuAcc, GpuAcc
    armnn::IRuntime::CreationOptions options;
    armnn::IRuntimePtr runtime = armnn::IRuntime::Create(options);
    armnn::IOptimizedNetworkPtr optNet = armnn::Optimize(*network,
       {armnn::Compute::CpuAcc, armnn::Compute::CpuRef},  
       runtime->GetDeviceSpec());

       // Load the optimized network onto the runtime device
    armnn::NetworkId networkIdentifier;
    runtime->LoadNetwork(networkIdentifier, std::move(optNet));

  
    
    gettimeofday(&start_time, nullptr);
    // Predict
    
    for(int i=0;i<10;i++)
        armnn::Status ret = runtime->EnqueueWorkload(networkIdentifier,
              armnnUtils::MakeInputTensors(inputBindings, inputDataContainers),
              armnnUtils::MakeOutputTensors(outputBindings, outputDataContainers));

    gettimeofday(&stop_time, nullptr);  
          
          
    std::vector<unsigned char> output = boost::get<std::vector<unsigned char>>(outputDataContainers[0]);

    //size_t labelInd = std::distance(output.begin(), std::max_element(output.begin(), output.end()));
    //std::cout << "Prediction: ";
    //std::cout << modelOutputLabels[labelInd] << std::endl;          
      
      
      
    std::cout << "invoked \n";
    std::cout << "average time: "
            << (get_us(stop_time) - get_us(start_time)) / (10 * 1000)
            << " ms \n";   


  return 0;
}