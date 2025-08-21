#!/bin/bash

# Script to run UParT evaluation on MiniAOD files
# Usage: ./run_upart_evaluation.sh [input_file] [output_file] [max_events]

# Set default parameters
DEFAULT_INPUT="root://cms-xrd-global.cern.ch//store/mc/Run3Summer22MiniAODv4/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/MINIAODSIM/130X_mcRun3_2022_realistic_v5-v2/2520000/001b8c73-e8b6-463a-b8e1-0e8fb6a9bbda.root"
DEFAULT_OUTPUT="upart_evaluation_output.root"
DEFAULT_EVENTS=1000

# Parse command line arguments
INPUT_FILE=${1:-$DEFAULT_INPUT}
OUTPUT_FILE=${2:-$DEFAULT_OUTPUT}
MAX_EVENTS=${3:-$DEFAULT_EVENTS}

echo "========================================"
echo "UParT Standalone Evaluation"
echo "========================================"
echo "Input file: $INPUT_FILE"
echo "Output file: $OUTPUT_FILE"
echo "Max events: $MAX_EVENTS"
echo "========================================"

# Check if we're in a CMSSW environment
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW environment not set up!"
    echo "Please run: cmsenv"
    exit 1
fi

# Check if HITools package exists
if [ ! -d "$CMSSW_BASE/src/HITools" ]; then
    echo "ERROR: HITools package not found in $CMSSW_BASE/src/"
    echo "Please ensure the package is properly installed"
    exit 1
fi

# Check if the package is compiled
if [ ! -f "$CMSSW_BASE/lib/$SCRAM_ARCH/libHITools.so" ]; then
    echo "WARNING: HITools library not found. Attempting to compile..."
    cd $CMSSW_BASE/src/HITools
    scram b
    if [ $? -ne 0 ]; then
        echo "ERROR: Compilation failed!"
        exit 1
    fi
fi

# Create temporary config file
TEMP_CONFIG="temp_upart_config_$$.py"

# Generate configuration on the fly
cat > $TEMP_CONFIG << EOF
import FWCore.ParameterSet.Config as cms

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

# Source configuration
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('$INPUT_FILE')
)

# Max events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32($MAX_EVENTS)
)

# TFileService for output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('$OUTPUT_FILE')
)

# UParT Evaluator
process.upartEvaluator = cms.EDAnalyzer("UParTEvaluator",
    jets = cms.InputTag("slimmedJets"),
    pfCandidates = cms.InputTag("packedPFCandidates"),
    modelPath = cms.string("RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx"),
    jetPtMin = cms.double(20.0),
    jetEtaMax = cms.double(2.4)
)

# Analysis sequence
process.p = cms.Path(process.upartEvaluator)

# Schedule
process.schedule = cms.Schedule(process.p)
EOF

echo "Starting UParT evaluation..."
echo "Configuration file: $TEMP_CONFIG"

# Run cmsRun
cmsRun $TEMP_CONFIG

# Check exit status
EXIT_STATUS=$?
if [ $EXIT_STATUS -eq 0 ]; then
    echo "========================================"
    echo "UParT evaluation completed successfully!"
    echo "Output file: $OUTPUT_FILE"
    echo "========================================"
    
    # Show some basic info about the output file
    if command -v rootls &> /dev/null; then
        echo "Output file contents:"
        rootls $OUTPUT_FILE
    fi
else
    echo "========================================"
    echo "ERROR: UParT evaluation failed with exit code $EXIT_STATUS"
    echo "========================================"
fi

# Clean up temporary config
rm -f $TEMP_CONFIG

exit $EXIT_STATUS