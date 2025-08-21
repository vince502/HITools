# HIMetInference

A CMSSW submodule for High Intensity Muon analysis and inference methods for the CMS experiment, with specialized support for Unified Particle Transformer (UParT) evaluation.

## Overview

This package provides standalone evaluation capabilities for UParT models on MiniAOD files, enabling jet tagging analysis outside of the full CMSSW b-tagging workflow.

## Features

- **Standalone UParT Evaluation**: Run UParT inference directly on MiniAOD files
- **Comprehensive Output**: ROOT histograms and TTree with jet properties and UParT predictions
- **Configurable Analysis**: Flexible configuration for different jet selections and model paths
- **CMSSW Integration**: Full compatibility with CMSSW framework and standards

## Installation

1. Set up your CMSSW environment:
```bash
cmsrel CMSSW_13_0_X
cd CMSSW_13_0_X/src
cmsenv
```

2. Clone this package:
```bash
git clone <repository-url> HIMetInference
```

3. Compile the package:
```bash
scram b -j8
```

## Usage

### Quick Start

Use the provided shell script for quick evaluation:

```bash
cd HIMetInference/test
chmod +x run_upart_evaluation.sh
./run_upart_evaluation.sh [input_file] [output_file] [max_events]
```

### Python Configuration

Use the provided Python configuration:

```bash
cmsRun python/upart_standalone_cfg.py inputFiles=file.root maxEvents=1000
```

### Minimal Test

For debugging or quick tests:

```bash
cmsRun test/test_upart_minimal.py
```

## Configuration Options

### UParT Evaluator Parameters

- `jets`: Input jet collection (default: `"slimmedJets"`)
- `pfCandidates`: PF candidate collection (default: `"packedPFCandidates"`)
- `modelPath`: Path to UParT ONNX model (default: UParT V01 model)
- `jetPtMin`: Minimum jet pT in GeV (default: `20.0`)
- `jetEtaMax`: Maximum jet |eta| (default: `2.4`)

### Command Line Options

The main configuration (`upart_standalone_cfg.py`) supports:

- `inputFiles`: List of input MiniAOD files
- `outputFile`: Output ROOT file name
- `maxEvents`: Maximum events to process
- `modelPath`: Custom UParT model path

## Output Structure

### ROOT Tree Variables

- `jet_pt`, `jet_eta`, `jet_phi`, `jet_mass`: Basic jet kinematics
- `upart_probs`: Vector of UParT probability outputs for all classes

### UParT Output Classes

The UParT model provides 30 probability outputs:
- **Flavor tagging**: `probb`, `probbb`, `problepb`, `probc`, `probs`, `probu`, `probd`, `probg`
- **Lepton identification**: `probele`, `probmu`
- **Tau identification**: `probtaup1h0p`, `probtaup1h1p`, `probtaup1h2p`, `probtaup3h0p`, `probtaup3h1p`, `probtaum1h0p`, `probtaum1h1p`, `probtaum1h2p`, `probtaum3h0p`, `probtaum3h1p`
- **Jet corrections**: `ptcorr`, `ptreshigh`, `ptreslow`, `ptnu`
- **Probe studies**: `probemudata`, `probemumc`, `probdimudata`, `probdimumc`, `probmutaudata`, `probmutaumc`

### Histograms

- `jetPt`, `jetEta`: Basic jet distributions
- `prob_*`: Probability distributions for each UParT class
- `ptVsProb`: 2D correlation between jet pT and b-tag probability

## Architecture

### Key Components

1. **UParTEvaluator** (`interface/UParTEvaluator.h`, `src/UParTEvaluator.cc`): Main analyzer class
2. **Feature Extraction**: Converts MiniAOD objects to UParT input features
3. **ONNX Integration**: Uses CMSSW's ONNXRuntime for model inference
4. **Output Management**: ROOT-based histogram and tree output

### UParT Model Integration

The evaluator uses the same tensor configurations as the official CMSSW UParT implementation:

- **Input Features**: 8 tensor inputs (charged particles, lost tracks, neutral particles, vertices + 4-vectors)
- **Dynamic Batching**: Supports variable-length input sequences
- **Feature Limits**: Respects UParT acceptance limits (29 charged, 5 lost tracks, 25 neutral, 5 vertices)

## Model Requirements

### Default Model Path
```
RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx
```

### Custom Models

You can specify custom UParT models using the `modelPath` parameter. Ensure the model:
- Uses ONNX format compatible with CMSSW's ONNXRuntime
- Has the expected input/output tensor structure
- Matches the UParT V01+ architecture for dynamic axes support

## Troubleshooting

### Common Issues

1. **Missing Model File**: Ensure UParT model files are available in your CMSSW installation
2. **Compilation Errors**: Check that all dependencies (ONNXRuntime, BTau packages) are available
3. **Input File Access**: Verify MiniAOD file accessibility (especially for remote files)

### Debug Mode

Enable verbose logging by setting:
```python
process.MessageLogger.cerr.threshold = 'DEBUG'
```

### Performance Notes

- Processing time scales with number of jets and constituents
- Memory usage depends on batch size and model size
- For large-scale processing, consider splitting into smaller jobs

## Development

### Adding New Features

1. Follow CMS coding standards (see `CLAUDE.md`)
2. Add appropriate tests in `test/` directory
3. Update documentation and configuration examples

### Integration with Other Packages

This package is designed to work alongside other CMSSW packages:
- Can be combined with other analyzers in the same job
- Compatible with standard CMSSW filters and selectors
- Outputs can be used by downstream analysis tools

## References

- [Particle Transformer Paper](https://arxiv.org/abs/2202.03772)
- [CMSSW UParT Implementation](https://github.com/cms-sw/cmssw/tree/master/RecoBTag/ONNXRuntime)
- [CMS B-tagging Documentation](https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation)

## License

This software is distributed under the CMS Software License. See LICENSE file for details.