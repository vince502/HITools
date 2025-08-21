# HITools

A CMSSW standalone package for Heavy Ion analysis tools and machine learning inference methods for the CMS experiment. This package is designed to be self-contained and work independently of other CMS packages.

## Package Structure

```
HITools/
├── BuildFile.xml           # Main package build configuration
├── README.md               # This file
├── CLAUDE.md               # Development documentation
└── Inference/              # ML inference subpackage
    ├── interface/          # Header files (.h)
    ├── src/               # Implementation files (.cc)
    ├── plugins/           # CMSSW plugins (producers, analyzers)
    ├── python/            # Python configuration files
    ├── test/              # Test scripts and configurations
    └── README.md          # Subpackage documentation
```

## Installation

### Quick Setup

1. Set up your CMSSW environment:
```bash
cmsrel CMSSW_13_0_X  # or appropriate version
cd CMSSW_13_0_X/src
cmsenv
```

2. Clone this standalone package:
```bash
git clone <repository-url> HITools
```

3. Compile the package:
```bash
scram b -j8
```

### Verification

Check that the package compiled successfully:
```bash
ls $CMSSW_BASE/lib/$SCRAM_ARCH/libHITools.so
```

## Subpackages

### Inference

The Inference subpackage provides machine learning model evaluation capabilities:

- **UParT Evaluation**: Standalone Unified Particle Transformer evaluation on MiniAOD files
- **Feature Extraction**: Convert MiniAOD objects to ML model input features
- **ONNX Integration**: Use CMSSW's ONNXRuntime for model inference
- **Analysis Output**: ROOT histograms and trees for analysis

See [Inference/README.md](Inference/README.md) for detailed usage instructions.

## Usage

### Quick Start (UParT Evaluation)

```bash
# Navigate to the test directory
cd HITools/Inference/test

# Run UParT evaluation with default settings
./run_upart_evaluation.sh

# Or specify custom parameters
./run_upart_evaluation.sh input.root output.root 1000
```

### Python Configuration

```bash
# Use the provided Python configuration
cmsRun HITools/Inference/python/upart_standalone_cfg.py

# With custom parameters
cmsRun HITools/Inference/python/upart_standalone_cfg.py inputFiles=file.root maxEvents=1000
```

## Key Features

### Standalone Operation
- **Self-contained**: All dependencies specified in BuildFile.xml
- **No external packages**: Works with standard CMSSW installation
- **Modular design**: Subpackages can be used independently

### CMS Standard Compliance
- **CMSSW integration**: Full compatibility with CMSSW framework
- **CMS coding standards**: Follows official CMS development guidelines
- **Thread safety**: Designed for multi-threaded CMSSW operation

### Performance Optimized
- **ONNX Runtime**: Uses high-performance inference engine
- **Memory efficient**: Optimized for large-scale processing
- **Configurable**: Flexible parameters for different use cases

## Development

### Adding New Subpackages

1. Create new subdirectory under HITools/
2. Follow CMSSW package structure (interface/, src/, plugins/, python/)
3. Update main BuildFile.xml if needed
4. Add appropriate documentation

### Contributing

1. Follow CMS coding standards (see CLAUDE.md)
2. Add tests for new functionality
3. Update documentation
4. Ensure backward compatibility

## Dependencies

### Required CMSSW Packages
- FWCore/Framework
- FWCore/ParameterSet  
- FWCore/Utilities
- DataFormats/PatCandidates
- DataFormats/JetReco
- DataFormats/BTauReco
- PhysicsTools/ONNXRuntime
- RecoBTag/ONNXRuntime
- CommonTools/UtilAlgos

### External Dependencies
- ROOT (included with CMSSW)
- ONNX Runtime (available in CMSSW)

## Troubleshooting

### Compilation Issues

1. **Missing dependencies**: Ensure all required packages are available in your CMSSW release
2. **Include path errors**: Verify that HITools is in `$CMSSW_BASE/src/HITools`
3. **Library not found**: Run `scram b clean && scram b` to force recompilation

### Runtime Issues

1. **Model file not found**: Ensure UParT model files are available in CMSSW data area
2. **Input file access**: Verify MiniAOD file accessibility
3. **Memory issues**: Reduce batch size or number of events for large files

### Debug Mode

Enable verbose logging:
```python
process.MessageLogger.cerr.threshold = 'DEBUG'
```

## Version Compatibility

- **CMSSW 13_0_X**: Full support
- **CMSSW 12_4_X**: Compatible (may need model path adjustments)  
- **CMSSW 13_2_X**: Recommended for UParT evaluation

## References

- [CMS Software Guide](https://cms-sw.github.io/)
- [CMSSW Framework](https://github.com/cms-sw/cmssw)
- [Particle Transformer Paper](https://arxiv.org/abs/2202.03772)

## License

This software is distributed under the CMS Software License. See LICENSE file for details.