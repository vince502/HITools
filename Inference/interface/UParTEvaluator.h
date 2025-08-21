#ifndef HITools_Inference_UParTEvaluator_h
#define HITools_Inference_UParTEvaluator_h

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/BTauReco/interface/UnifiedParticleTransformerAK4TagInfo.h"
#include "DataFormats/BTauReco/interface/UnifiedParticleTransformerAK4Features.h"

#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"
#include "RecoBTag/ONNXRuntime/interface/tensor_fillers.h"
#include "RecoBTag/ONNXRuntime/interface/tensor_configs.h"

#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

#include <vector>
#include <memory>

namespace hitools {

class UParTEvaluator : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit UParTEvaluator(const edm::ParameterSet&);
  ~UParTEvaluator() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // UParT inference methods
  void processJet(const pat::Jet& jet, const edm::Event& iEvent);
  btagbtvdeep::UnifiedParticleTransformerAK4Features extractFeatures(const pat::Jet& jet, const edm::Event& iEvent);
  std::vector<float> runInference(const btagbtvdeep::UnifiedParticleTransformerAK4Features& features);
  void fillInputTensors(const btagbtvdeep::UnifiedParticleTransformerAK4Features& features);

  // Configuration
  const edm::EDGetTokenT<std::vector<pat::Jet>> jetToken_;
  const edm::EDGetTokenT<std::vector<pat::PackedCandidate>> pfCandToken_;
  const std::string modelPath_;
  const double jetPtMin_;
  const double jetEtaMax_;
  
  // ONNX Runtime
  std::unique_ptr<cms::Ort::ONNXRuntime> onnxSession_;
  std::vector<std::string> inputNames_;
  std::vector<std::string> outputNames_;
  
  // UParT tensor data
  std::vector<unsigned> inputSizes_;
  std::vector<std::vector<int64_t>> inputShapes_;
  cms::Ort::FloatArrays tensorData_;
  
  // Output histograms and trees
  edm::Service<TFileService> fs_;
  TTree* outputTree_;
  
  // Tree variables
  float jet_pt_, jet_eta_, jet_phi_, jet_mass_;
  std::vector<float> upart_probs_;
  std::vector<std::string> classNames_;
  
  // Histograms
  std::map<std::string, TH1F*> probHists_;
  TH2F* ptVsProb_;
  TH1F* jetPtHist_;
  TH1F* jetEtaHist_;
};

} // namespace hitools

#endif