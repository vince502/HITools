#!/bin/bash

# Simple script to test HITools standalone package build
# This script verifies that the package structure is correct for CMSSW

echo "========================================"
echo "HITools Standalone Package Build Test"
echo "========================================"

# Check if we're in the right directory
if [ ! -f "BuildFile.xml" ]; then
    echo "ERROR: Run this script from the HITools package directory"
    echo "Expected to find BuildFile.xml in current directory"
    exit 1
fi

# Check required directories exist
REQUIRED_DIRS=("Inference/interface" "Inference/src" "Inference/plugins" "Inference/python" "Inference/test")

for dir in "${REQUIRED_DIRS[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "ERROR: Required directory '$dir' not found"
        exit 1
    fi
    echo "✓ Found directory: $dir"
done

# Check key files exist
REQUIRED_FILES=(
    "Inference/interface/UParTEvaluator.h"
    "Inference/src/UParTEvaluator.cc"
    "Inference/plugins/UParTStandaloneAnalyzer.cc"
    "Inference/plugins/BuildFile.xml"
    "Inference/python/upart_standalone_cfg.py"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "ERROR: Required file '$file' not found"
        exit 1
    fi
    echo "✓ Found file: $file"
done

# Check include guards and namespaces are correct
echo ""
echo "Checking include guards and namespaces..."

# Check header file
HEADER_FILE="Inference/interface/UParTEvaluator.h"
if grep -q "HITools_Inference_UParTEvaluator_h" "$HEADER_FILE"; then
    echo "✓ Include guard correct in $HEADER_FILE"
else
    echo "ERROR: Include guard incorrect in $HEADER_FILE"
    exit 1
fi

if grep -q "namespace hitools" "$HEADER_FILE"; then
    echo "✓ Namespace correct in $HEADER_FILE"
else
    echo "ERROR: Namespace incorrect in $HEADER_FILE"
    exit 1
fi

# Check source file
SOURCE_FILE="Inference/src/UParTEvaluator.cc"
if grep -q "#include \"HITools/Inference/interface/UParTEvaluator.h\"" "$SOURCE_FILE"; then
    echo "✓ Include path correct in $SOURCE_FILE"
else
    echo "ERROR: Include path incorrect in $SOURCE_FILE"
    exit 1
fi

if grep -q "using namespace hitools" "$SOURCE_FILE"; then
    echo "✓ Namespace usage correct in $SOURCE_FILE"
else
    echo "ERROR: Namespace usage incorrect in $SOURCE_FILE"
    exit 1
fi

# Check plugin file
PLUGIN_FILE="Inference/plugins/UParTStandaloneAnalyzer.cc"
if grep -q "#include \"HITools/Inference/interface/UParTEvaluator.h\"" "$PLUGIN_FILE"; then
    echo "✓ Include path correct in $PLUGIN_FILE"
else
    echo "ERROR: Include path incorrect in $PLUGIN_FILE"
    exit 1
fi

# Check BuildFile.xml dependencies
BUILD_FILE="BuildFile.xml"
REQUIRED_DEPS=("FWCore/Framework" "PhysicsTools/ONNXRuntime" "DataFormats/PatCandidates")

for dep in "${REQUIRED_DEPS[@]}"; do
    if grep -q "<use name=\"$dep\"/>" "$BUILD_FILE"; then
        echo "✓ Found dependency: $dep"
    else
        echo "ERROR: Missing dependency: $dep in $BUILD_FILE"
        exit 1
    fi
done

echo ""
echo "========================================"
echo "✅ All checks passed! Package structure is correct."
echo "========================================"
echo ""
echo "To use this package in CMSSW:"
echo "1. Copy HITools directory to \$CMSSW_BASE/src/"
echo "2. Run: scram b -j8"
echo "3. Run: cmsRun HITools/Inference/python/upart_standalone_cfg.py"
echo ""
echo "Package is ready for standalone CMSSW usage!"
echo "========================================"