#include "HITools/Inference/interface/UParTEvaluator.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <algorithm>
#include <cmath>

using namespace hitools;

UParTEvaluator::UParTEvaluator(const edm::ParameterSet& iConfig)
    : jetToken_(consumes<std::vector<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jets"))),
      pfCandToken_(consumes<std::vector<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("pfCandidates"))),
      modelPath_(iConfig.getParameter<std::string>("modelPath")),
      jetPtMin_(iConfig.getParameter<double>("jetPtMin")),
      jetEtaMax_(iConfig.getParameter<double>("jetEtaMax")) {
  
  usesResource("TFileService");
  
  // Initialize ONNX Runtime
  onnxSession_ = std::make_unique<cms::Ort::ONNXRuntime>(modelPath_);
  
  // Define input/output names based on UParT configuration
  inputNames_ = {"input_1", "input_2", "input_3", "input_4", "input_5", "input_6", "input_7", "input_8"};
  outputNames_ = {"softmax"};
  
  // Define class names from UParT configuration
  classNames_ = {"probb", "probbb", "problepb", "probc", "probs", "probu", "probd", "probg",
                 "probele", "probmu", "probtaup1h0p", "probtaup1h1p", "probtaup1h2p", 
                 "probtaup3h0p", "probtaup3h1p", "probtaum1h0p", "probtaum1h1p", 
                 "probtaum1h2p", "probtaum3h0p", "probtaum3h1p", "ptcorr", "ptreshigh", 
                 "ptreslow", "ptnu", "probemudata", "probemumc", "probdimudata", 
                 "probdimumc", "probmutaudata", "probmutaumc"};
  
  upart_probs_.resize(classNames_.size());
  
  edm::LogInfo("UParTEvaluator") << "Initialized with model: " << modelPath_;
  edm::LogInfo("UParTEvaluator") << "Number of output classes: " << classNames_.size();
}

void UParTEvaluator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("jets", edm::InputTag("slimmedJets"));
  desc.add<edm::InputTag>("pfCandidates", edm::InputTag("packedPFCandidates"));
  desc.add<std::string>("modelPath", "RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx");
  desc.add<double>("jetPtMin", 20.0);
  desc.add<double>("jetEtaMax", 2.4);
  descriptions.add("upartEvaluator", desc);
}

void UParTEvaluator::beginJob() {
  // Initialize histograms
  jetPtHist_ = fs_->make<TH1F>("jetPt", "Jet pT;pT [GeV];Jets", 100, 0, 500);
  jetEtaHist_ = fs_->make<TH1F>("jetEta", "Jet #eta;#eta;Jets", 50, -2.5, 2.5);
  ptVsProb_ = fs_->make<TH2F>("ptVsProb", "Jet pT vs b-tag probability;pT [GeV];P(b)", 100, 0, 500, 100, 0, 1);
  
  // Create probability histograms for each class
  for (const auto& className : classNames_) {
    probHists_[className] = fs_->make<TH1F>(
        ("prob_" + className).c_str(),
        (className + " probability;" + className + ";Jets").c_str(),
        100, 0, 1);
  }
  
  // Initialize output tree
  outputTree_ = fs_->make<TTree>("upartTree", "UParT Evaluation Results");
  outputTree_->Branch("jet_pt", &jet_pt_);
  outputTree_->Branch("jet_eta", &jet_eta_);
  outputTree_->Branch("jet_phi", &jet_phi_);
  outputTree_->Branch("jet_mass", &jet_mass_);
  outputTree_->Branch("upart_probs", &upart_probs_);
  
  edm::LogInfo("UParTEvaluator") << "Histograms and tree initialized";
}

void UParTEvaluator::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Get jets from event
  edm::Handle<std::vector<pat::Jet>> jets;
  iEvent.getByToken(jetToken_, jets);
  
  if (!jets.isValid()) {
    edm::LogWarning("UParTEvaluator") << "Invalid jet collection";
    return;
  }
  
  edm::LogInfo("UParTEvaluator") << "Processing " << jets->size() << " jets in event " << iEvent.id().event();
  
  // Process each jet
  for (const auto& jet : *jets) {
    // Apply basic selection
    if (jet.pt() < jetPtMin_ || std::abs(jet.eta()) > jetEtaMax_) {
      continue;
    }
    
    processJet(jet, iEvent);
  }
}

void UParTEvaluator::processJet(const pat::Jet& jet, const edm::Event& iEvent) {
  // Fill basic jet variables
  jet_pt_ = jet.pt();
  jet_eta_ = jet.eta();
  jet_phi_ = jet.phi();
  jet_mass_ = jet.mass();
  
  // Fill basic histograms
  jetPtHist_->Fill(jet_pt_);
  jetEtaHist_->Fill(jet_eta_);
  
  try {
    // Run inference directly with jet
    auto predictions = runInference(jet, iEvent);
    
    if (predictions.size() == classNames_.size()) {
      upart_probs_ = predictions;
      
      // Fill probability histograms
      for (size_t i = 0; i < classNames_.size(); ++i) {
        probHists_[classNames_[i]]->Fill(predictions[i]);
      }
      
      // Fill b-tag specific histogram (assuming first class is b)
      if (!predictions.empty()) {
        ptVsProb_->Fill(jet_pt_, predictions[0]);
      }
      
      // Fill tree
      outputTree_->Fill();
      
      edm::LogInfo("UParTEvaluator") << "Processed jet: pT=" << jet_pt_ 
                                     << ", eta=" << jet_eta_ 
                                     << ", prob_b=" << (predictions.empty() ? -1 : predictions[0]);
    } else {
      edm::LogWarning("UParTEvaluator") << "Unexpected prediction size: " << predictions.size();
    }
    
  } catch (const std::exception& e) {
    edm::LogError("UParTEvaluator") << "Error processing jet: " << e.what();
  }
}

std::vector<float> UParTEvaluator::runInference(const pat::Jet& jet, const edm::Event& iEvent) {
  try {
    // Fill input tensors directly from jet
    fillInputTensors(jet, iEvent);
    
    // Run ONNX inference
    auto outputs = onnxSession_->run(inputNames_, tensorData_, inputShapes_, outputNames_, 1);
    
    if (outputs.empty() || outputs[0].empty()) {
      edm::LogWarning("UParTEvaluator") << "Empty inference output";
      return std::vector<float>(classNames_.size(), -1.0);
    }
    
    return outputs[0];
    
  } catch (const std::exception& e) {
    edm::LogError("UParTEvaluator") << "Inference error: " << e.what();
    return std::vector<float>(classNames_.size(), -1.0);
  }
}


void UParTEvaluator::fillInputTensors(const pat::Jet& jet, const edm::Event& iEvent) {
  // Get PF candidates
  edm::Handle<std::vector<pat::PackedCandidate>> pfCands;
  iEvent.getByToken(pfCandToken_, pfCands);
  
  if (!pfCands.isValid()) {
    edm::LogWarning("UParTEvaluator") << "Invalid PF candidate collection";
    return;
  }
  
  // Extract constituents within jet cone
  const double deltaR_max = 0.4;
  
  std::vector<const pat::PackedCandidate*> chargedCands;
  std::vector<const pat::PackedCandidate*> neutralCands;
  std::vector<const pat::PackedCandidate*> lostTracks;
  
  for (const auto& cand : *pfCands) {
    double deltaR = reco::deltaR(jet.eta(), jet.phi(), cand.eta(), cand.phi());
    if (deltaR > deltaR_max) continue;
    
    if (cand.charge() != 0 && cand.hasTrackDetails()) {
      if (cand.lostInnerHits() > 0) {
        lostTracks.push_back(&cand);
      } else {
        chargedCands.push_back(&cand);
      }
    } else if (cand.charge() == 0) {
      neutralCands.push_back(&cand);
    }
  }
  
  // Sort by pT (descending)
  auto ptSort = [](const pat::PackedCandidate* a, const pat::PackedCandidate* b) {
    return a->pt() > b->pt();
  };
  
  std::sort(chargedCands.begin(), chargedCands.end(), ptSort);
  std::sort(neutralCands.begin(), neutralCands.end(), ptSort);
  std::sort(lostTracks.begin(), lostTracks.end(), ptSort);
  
  // Define UParT constants (hardcoded for CMSSW 13.2.10 compatibility)
  const unsigned max_cpf = 29;  // n_cpf_accept
  const unsigned max_lt = 5;    // n_lt_accept  
  const unsigned max_npf = 25;  // n_npf_accept
  const unsigned max_sv = 5;    // n_sv_accept
  
  // Feature dimensions
  const unsigned cpf_features = 25;  // ChargedCandidates features
  const unsigned lt_features = 18;   // LostTracks features
  const unsigned npf_features = 8;   // NeutralCandidates features
  const unsigned sv_features = 14;   // Vertices features
  const unsigned vec4_features = 4;  // 4-vector features (pt,eta,phi,e)
  
  const unsigned n_cpf = std::clamp((unsigned)chargedCands.size(), 1u, max_cpf);
  const unsigned n_lt = std::clamp((unsigned)lostTracks.size(), 1u, max_lt);
  const unsigned n_npf = std::clamp((unsigned)neutralCands.size(), 1u, max_npf);
  const unsigned n_sv = 1u; // No SV collection for now
  
  // Set input shapes for dynamic batching
  inputShapes_.clear();
  inputShapes_.push_back({1, (int64_t)n_cpf, (int64_t)cpf_features});
  inputShapes_.push_back({1, (int64_t)n_lt, (int64_t)lt_features});
  inputShapes_.push_back({1, (int64_t)n_npf, (int64_t)npf_features});
  inputShapes_.push_back({1, (int64_t)n_sv, (int64_t)sv_features});
  inputShapes_.push_back({1, (int64_t)n_cpf, (int64_t)vec4_features});
  inputShapes_.push_back({1, (int64_t)n_lt, (int64_t)vec4_features});
  inputShapes_.push_back({1, (int64_t)n_npf, (int64_t)vec4_features});
  inputShapes_.push_back({1, (int64_t)n_sv, (int64_t)vec4_features});
  
  // Calculate input sizes
  inputSizes_.clear();
  inputSizes_.push_back(n_cpf * cpf_features);
  inputSizes_.push_back(n_lt * lt_features);
  inputSizes_.push_back(n_npf * npf_features);
  inputSizes_.push_back(n_sv * sv_features);
  inputSizes_.push_back(n_cpf * vec4_features);
  inputSizes_.push_back(n_lt * vec4_features);
  inputSizes_.push_back(n_npf * vec4_features);
  inputSizes_.push_back(n_sv * vec4_features);
  
  // Initialize tensor data
  tensorData_.clear();
  for (const auto& size : inputSizes_) {
    tensorData_.emplace_back(size, 0.0f);
  }
  
  // Fill tensors with simplified features (for compatibility with CMSSW 13.2.10)
  // This is a placeholder implementation - in practice you'd need proper tensor fillers
  
  edm::LogInfo("UParTEvaluator") << "Filled tensors for inference: " 
                                 << "cpf=" << n_cpf << ", lt=" << n_lt 
                                 << ", npf=" << n_npf << ", sv=" << n_sv;
}

void UParTEvaluator::endJob() {
  edm::LogInfo("UParTEvaluator") << "Analysis completed";
}

DEFINE_FWK_MODULE(UParTEvaluator);