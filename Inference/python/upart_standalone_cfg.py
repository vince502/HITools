import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.ParameterSet.VarParsing as VarParsing

# Parse command line options
options = VarParsing.VarParsing('analysis')
options.register('inputFiles', 
                 'root://cms-xrd-global.cern.ch//store/mc/Run3Summer22MiniAODv4/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/MINIAODSIM/130X_mcRun3_2022_realistic_v5-v2/2520000/001b8c73-e8b6-463a-b8e1-0e8fb6a9bbda.root',
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.string,
                 "Input MiniAOD files")
options.register('outputFile',
                 'upart_evaluation_output.root',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Output ROOT file")
options.register('maxEvents',
                 1000,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Maximum number of events to process")
options.register('modelPath',
                 'RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Path to UParT ONNX model")
options.parseArguments()

# Process configuration
process = cms.Process("UParTEvaluation")

# Load necessary modules
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# Global tag for Run 3
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '130X_mcRun3_2022_realistic_v5', '')

# Message logger configuration
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.MessageLogger.cerr.threshold = 'INFO'
process.MessageLogger.categories.append('UParTEvaluator')
process.MessageLogger.cerr.UParTEvaluator = cms.untracked.PSet(
    limit = cms.untracked.int32(-1)
)

# Source configuration
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

# Max events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

# TFileService for output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

# UParT Evaluator
process.upartEvaluator = cms.EDAnalyzer("UParTEvaluator",
    jets = cms.InputTag("slimmedJets"),
    pfCandidates = cms.InputTag("packedPFCandidates"),
    modelPath = cms.string(options.modelPath),
    jetPtMin = cms.double(20.0),
    jetEtaMax = cms.double(2.4)
)

# Analysis sequence
process.p = cms.Path(process.upartEvaluator)

# Schedule
process.schedule = cms.Schedule(process.p)

# Print configuration summary
print("\n" + "="*60)
print("UParT Standalone Evaluation Configuration")
print("="*60)
print(f"Input files: {len(options.inputFiles)} files")
if len(options.inputFiles) <= 3:
    for f in options.inputFiles:
        print(f"  {f}")
else:
    for f in options.inputFiles[:3]:
        print(f"  {f}")
    print(f"  ... and {len(options.inputFiles)-3} more")
print(f"Output file: {options.outputFile}")
print(f"Max events: {options.maxEvents}")
print(f"Model path: {options.modelPath}")
print(f"Jet pT min: 20.0 GeV")
print(f"Jet |eta| max: 2.4")
print("="*60 + "\n")