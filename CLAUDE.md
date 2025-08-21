# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HITools is a standalone CMSSW package for Heavy Ion analysis tools and machine learning inference methods for the CMS experiment at CERN. This package integrates with the CMSSW framework and follows CMS coding standards while maintaining standalone functionality.

## Package Structure

HITools follows the standard CMSSW package structure but is organized as a master package with subpackages:

```
HITools/                    # Master package
├── BuildFile.xml          # Main build configuration
├── interface/             # Master package headers (if any)
├── src/                   # Master package implementation (if any)
└── Inference/             # ML inference subpackage
    ├── interface/         # Header files (.h)
    ├── src/              # Implementation files (.cc)
    ├── plugins/          # CMSSW plugins with dedicated BuildFile.xml
    ├── python/           # Python configuration files
    └── test/             # Unit tests and example configs
```

## CMSSW Environment Setup

```bash
# Set up CMSSW environment (example for recent release)
cmsrel CMSSW_13_0_X
cd CMSSW_13_0_X/src
cmsenv

# Clone standalone package
git clone <repo> HITools

# Build the project
scram b -j8

# Run with CMSSW
cmsRun HITools/Inference/python/config_file.py
```

## Coding Standards (CMS Rules)

### File Structure
- Header files: `.h` suffix in `interface/` directory
- Source files: `.cc` suffix in `src/` directory  
- Plugins: Place in `plugins/` directory with dedicated `BuildFile.xml`
- Tests: Place in `test/` directory
- Configuration: Python files in `python/` directory

### Naming Conventions
- Class names: Upper camel case (e.g., `UParTEvaluator`)
- Namespaces: lowercase (e.g., `namespace hitools`)
- Method names: Start with lowercase (e.g., `runInference()`)
- Data members: lowercase with trailing underscore (e.g., `jetPt_`)
- Constants: Prefix with `k` (e.g., `kMaxIterations`)

### Code Style
- Use `.clang-format` and `.clang-tidy` for formatting
- Prefer `nullptr` over `NULL`
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Mark overridden methods with `override`
- Make methods and data `const` when possible
- Avoid global mutable data for thread safety

### Include Paths
For this standalone package, use:
```cpp
#include "HITools/Inference/interface/UParTEvaluator.h"
```

Namespace:
```cpp
namespace hitools {
  // Implementation
}
```

## ML Framework Integration

### Supported Frameworks
- **TensorFlow 2**: Available via `PhysicsTools/TensorFlow` (CPU support)
- **ONNX Runtime**: Available via `PhysicsTools/ONNXRuntime` (better performance)

### Usage Pattern
```cpp
#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"
#include "RecoBTag/ONNXRuntime/interface/tensor_fillers.h"
#include "RecoBTag/ONNXRuntime/interface/tensor_configs.h"

// Use ONNX Runtime for inference
std::unique_ptr<cms::Ort::ONNXRuntime> session_;
```

### Performance Notes
- ONNX Runtime shows ~3-5x speedup vs TensorFlow for some models
- Thread-safe inference supported
- Multi-backend support (CPU, CUDA, ROCm, Intel)

## Particle Transformer Models (ParT/UParT)

### Overview
The CMS experiment uses transformer-based neural networks for jet tagging and particle identification. Two main variants are implemented in CMSSW:

### ParticleTransformer (ParT)
- **Location**: `RecoBTag/ONNXRuntime/plugins/ParticleTransformerAK4ONNXJetTagsProducer.cc`
- **Model Path**: `RecoBTag/Combined/data/RobustParTAK4/PUPPI/V00/modelfile/model.onnx`
- **Input Features**: 6 tensors (charged particles, neutral particles, vertices + 4-vectors)
- **Namespace**: `parT` in `tensor_configs.h`

#### Configuration
```cpp
// Input limits
constexpr unsigned n_cpf_accept = 25;  // charged particle flow
constexpr unsigned n_npf_accept = 25;  // neutral particle flow  
constexpr unsigned n_sv_accept = 5;    // secondary vertices

// Feature dimensions
kChargedCandidates: 16 features
kNeutralCandidates: 8 features  
kVertices: 14 features
k*4Vec variants: 4 features each (pt,eta,phi,e)
```

#### Output Classes
```cpp
{"probb", "probbb", "problepb", "probc", "probuds", "probg"}
```

### UnifiedParticleTransformer (UParT)
- **Location**: `RecoBTag/ONNXRuntime/plugins/UnifiedParticleTransformerAK4ONNXJetTagsProducer.cc`
- **Model Path**: `RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx`
- **Input Features**: 8 tensors (adds lost tracks to ParT inputs)
- **Namespace**: `UparT` in `tensor_configs.h`

#### Configuration
```cpp
// Input limits (extended from ParT)
constexpr unsigned n_cpf_accept = 29;  // charged particle flow
constexpr unsigned n_lt_accept = 5;    // lost tracks (new)
constexpr unsigned n_npf_accept = 25;  // neutral particle flow
constexpr unsigned n_sv_accept = 5;    // secondary vertices

// Feature dimensions
kChargedCandidates: 25 features (vs 16 in ParT)
kLostTracks: 18 features (new input type)
kNeutralCandidates: 8 features
kVertices: 14 features
k*4Vec variants: 4 features each
```

#### Output Classes (Extended)
```cpp
{"probb", "probbb", "problepb", "probc", "probs", "probu", "probd", "probg",
 "probele", "probmu", "probtaup1h0p", "probtaup1h1p", "probtaup1h2p", 
 "probtaup3h0p", "probtaup3h1p", "probtaum1h0p", "probtaum1h1p", 
 "probtaum1h2p", "probtaum3h0p", "probtaum3h1p", "ptcorr", "ptreshigh", 
 "ptreslow", "ptnu", "probemudata", "probemumc", "probdimudata", 
 "probdimumc", "probmutaudata", "probmutaumc"}
```

### Key Differences
1. **UParT vs ParT**: UParT adds lost track reconstruction, more output classes, and enhanced feature sets
2. **Dynamic Axes**: UParT V01+ supports dynamic input shapes for better efficiency  
3. **Model Evolution**: RobustParTAK4 (ParT) → UParTAK4 (UParT) represents architectural improvements

### Integration Pattern
```cpp
#include "RecoBTag/ONNXRuntime/interface/tensor_fillers.h"
#include "RecoBTag/ONNXRuntime/interface/tensor_configs.h"

// Use UparT namespace for extended functionality
using namespace UparT;  // or parT for basic implementation
```

## Development Workflow

1. Create feature branch from master
2. Implement following CMS coding standards
3. Add unit tests in appropriate `test/` directory
4. Use `scram b` to build and verify compilation
5. Test with `cmsRun` using appropriate config files
6. Submit pull request

## Build Commands

```bash
# Full build from package directory
scram b

# Parallel build (faster)
scram b -j8

# Clean build
scram b clean
scram b

# Build specific package (from CMSSW_BASE/src)
scram b HITools

# Debug build
scram b USER_CXXFLAGS="-g -O0"
```

## Standalone Package Guidelines

### Dependencies
- List all required packages in main `BuildFile.xml`
- Minimize external dependencies
- Use only standard CMSSW packages when possible

### Self-Contained Design
- All functionality should work without external CMS packages
- Include path should start with `HITools/`
- Use `hitools` namespace consistently

### Modularity
- Subpackages should be independent when possible
- Clear interfaces between components
- Separate concerns (inference, analysis, utilities)

## Testing

### Unit Tests
Create test configurations in `test/` directories:
- `test_minimal.py`: Basic functionality test
- `run_*.sh`: Shell scripts for automated testing
- Example configurations with small event samples

### Integration Tests
- Test with actual MiniAOD files
- Verify output format and content
- Performance benchmarking

## Important Instruction Reminders

Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.