#!/usr/bin/env python3

"""
Minimal test configuration for UParT evaluation
This can be used for quick testing with a small number of events
"""

import FWCore.ParameterSet.Config as cms

process = cms.Process("UParTTest")

# Load minimal necessary modules
process.load("FWCore.MessageService.MessageLogger_cfi")

# Message logger - verbose for debugging
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.MessageLogger.cerr.threshold = 'DEBUG'
process.MessageLogger.categories.append('UParTEvaluator')
process.MessageLogger.cerr.UParTEvaluator = cms.untracked.PSet(
    limit = cms.untracked.int32(-1)
)

# Test with a small MC file (adjust path as needed)
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        # Small test file - replace with actual available file
        'file:/eos/cms/store/mc/Run3Summer22MiniAODv4/QCD_PT-15to7000_TuneCP5_Flat_13p6TeV_pythia8/MINIAODSIM/130X_mcRun3_2022_realistic_v5-v2/2520000/test.root'
    )
)

# Process only 10 events for quick testing
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)

# Output service
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('test_upart_output.root')
)

# UParT Evaluator with debug-friendly settings
process.upartEvaluator = cms.EDAnalyzer("UParTEvaluator",
    jets = cms.InputTag("slimmedJets"),
    pfCandidates = cms.InputTag("packedPFCandidates"),
    modelPath = cms.string("RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx"),
    jetPtMin = cms.double(15.0),  # Lower threshold for testing
    jetEtaMax = cms.double(3.0)   # Wider eta range for testing
)

# Simple path
process.p = cms.Path(process.upartEvaluator)

print("="*50)
print("UParT Minimal Test Configuration")
print("="*50)
print("- Processing 10 events")
print("- Jet pT > 15 GeV")
print("- |eta| < 3.0")
print("- Output: test_upart_output.root")
print("="*50)