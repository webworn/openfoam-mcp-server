# OpenFOAM MCP Server - Branch Strategy & Development Workflow

## 🌳 **Branch Strategy Overview**

This document defines the comprehensive branching strategy, development workflow, and implementation guidelines for the OpenFOAM MCP Server project. Our strategy balances CFD development requirements with modern software engineering practices.

---

## 🎯 **Branch Structure**

### **Primary Branches**

#### `main` - Production Branch
- **Purpose**: Production-ready code only
- **Protection**: Fully protected, requires PR + reviews
- **Deployment**: Automatically deploys to production
- **Quality Gate**: All CI/CD checks must pass

#### `Development` - Integration Branch  
- **Purpose**: Latest development features integration
- **Protection**: Protected, requires PR + basic checks
- **Deployment**: Deploys to staging environment
- **Quality Gate**: Basic CI checks + CFD validation

### **Supporting Branches**

#### Feature Branches (`feature/*`)
```
feature/pipe-flow-validation
feature/heat-transfer-optimization
feature/mcp-protocol-v2
feature/aerospace-tools
```

#### Release Branches (`release/*`)
```
release/v1.0.0
release/v1.1.0-beta
```

#### Hotfix Branches (`hotfix/*`)
```
hotfix/critical-memory-leak
hotfix/cfd-validation-fix
```

#### Physics Branches (`physics/*`)
```
physics/compressible-flow
physics/multiphase-validation
physics/turbulence-models
```

---

## 🔄 **Development Workflow**

### **1. Feature Development Process**

#### **Step 1: Create Feature Branch**
```bash
# Start from latest Development
git checkout Development
git pull origin Development

# Create feature branch
git checkout -b feature/descriptive-name

# Examples:
git checkout -b feature/external-flow-validation
git checkout -b feature/heat-exchanger-analysis
git checkout -b feature/mcp-error-handling
```

#### **Step 2: Development and Testing**
```bash
# Make changes and commit frequently
git add .
git commit -m "feat: implement basic external flow setup"

# Run local validation
./scripts/run-local-validation.sh

# Run CFD-specific tests
./scripts/validate-physics.sh feature/external-flow-validation
```

#### **Step 3: Create Pull Request**
```bash
# Push feature branch
git push origin feature/external-flow-validation

# Create PR via GitHub CLI or web interface
gh pr create --title "Add external flow validation framework" \
  --body-file .github/pull_request_template.md \
  --base Development \
  --label "physics,validation,feature"
```

### **2. Physics Validation Workflow**

#### **CFD-Specific Development**
```bash
# Create physics branch for major physics work
git checkout -b physics/conjugate-heat-transfer

# Implement physics
mkdir -p src/openfoam/conjugate_heat_transfer
mkdir -p tests/validation/heat_transfer

# Validate against analytical solutions
python3 tests/validation/run_analytical_validation.py \
  --physics heat_transfer \
  --test_cases all \
  --tolerance 0.05

# Validate against experimental data
python3 tests/validation/run_experimental_validation.py \
  --dataset nist_heat_transfer \
  --confidence_level 0.95
```

#### **Performance Benchmarking**
```bash
# Run performance benchmarks
./scripts/run-performance-benchmarks.sh \
  --baseline Development \
  --feature physics/conjugate-heat-transfer \
  --output benchmarks/results/

# Memory profiling
valgrind --tool=memcheck --leak-check=full \
  ./build/openfoam-mcp-server --benchmark-mode
```

### **3. Release Process**

#### **Create Release Branch**
```bash
# From Development when ready for release
git checkout Development
git pull origin Development
git checkout -b release/v1.2.0

# Update version information
echo "1.2.0" > VERSION
sed -i 's/version = ".*"/version = "1.2.0"/' CMakeLists.txt

git commit -am "bump: version to 1.2.0"
git push origin release/v1.2.0
```

#### **Release Testing**
```bash
# Comprehensive testing for release
./scripts/run-full-test-suite.sh release/v1.2.0

# CFD validation across all physics domains
./scripts/validate-all-physics.sh --comprehensive

# Performance regression testing
./scripts/run-performance-regression.sh \
  --baseline v1.1.0 \
  --candidate release/v1.2.0
```

#### **Release Deployment**
```bash
# Merge to main after testing
git checkout main
git merge --no-ff release/v1.2.0
git tag -a v1.2.0 -m "Release version 1.2.0"

# Push to trigger production deployment
git push origin main --tags

# Clean up release branch
git branch -d release/v1.2.0
git push origin --delete release/v1.2.0
```

### **4. Hotfix Process**

#### **Critical Issue Response**
```bash
# Create hotfix from main
git checkout main
git pull origin main
git checkout -b hotfix/critical-cfd-validation-fix

# Fix the issue
# ... make changes ...

# Test the fix
./scripts/run-targeted-tests.sh hotfix/critical-cfd-validation-fix

# Fast-track to main
git checkout main
git merge --no-ff hotfix/critical-cfd-validation-fix
git tag -a v1.1.1 -m "Hotfix version 1.1.1"

# Also merge back to Development
git checkout Development  
git merge hotfix/critical-cfd-validation-fix

git push origin main Development --tags
```

---

## 🔒 **Branch Protection Rules**

### **Main Branch Protection**
```yaml
# GitHub branch protection settings for main
protection_rules:
  required_status_checks:
    strict: true
    contexts:
      - "ci/build-test"
      - "ci/cfd-validation" 
      - "ci/security-scan"
      - "ci/performance-tests"
  enforce_admins: true
  required_pull_request_reviews:
    required_approving_review_count: 2
    dismiss_stale_reviews: true
    require_code_owner_reviews: true
    required_review_from_codeowners: true
  restrictions:
    users: ["webworn"]
    teams: ["core-maintainers", "senior-cfd-experts"]
```

### **Development Branch Protection**
```yaml
# GitHub branch protection settings for Development
protection_rules:
  required_status_checks:
    strict: true
    contexts:
      - "ci/build-test"
      - "ci/basic-validation"
  required_pull_request_reviews:
    required_approving_review_count: 1
    require_code_owner_reviews: true
  allow_force_pushes: false
  allow_deletions: false
```

---

## 🏷️ **Naming Conventions**

### **Branch Naming Standards**

#### **Feature Branches**
```bash
# Format: feature/<domain>-<description>
feature/pipe-flow-laminar-validation
feature/heat-transfer-conjugate-solver
feature/mcp-tool-registration-refactor
feature/external-flow-turbulence-models
feature/documentation-user-guide
```

#### **Physics Branches**
```bash
# Format: physics/<domain>-<physics-area>
physics/incompressible-rans-models
physics/compressible-shock-capturing
physics/multiphase-vof-implementation
physics/heat-transfer-radiation-models
physics/turbulence-les-wall-functions
```

#### **Bug Fix Branches**
```bash
# Format: fix/<severity>-<description>
fix/critical-memory-leak-heat-solver
fix/major-convergence-issue-external-flow
fix/minor-json-schema-validation
```

#### **Release Branches**
```bash
# Format: release/v<major>.<minor>.<patch>[-<prerelease>]
release/v1.0.0
release/v1.1.0-beta
release/v2.0.0-rc1
```

### **Commit Message Standards**

#### **Conventional Commits Format**
```bash
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

#### **Types for CFD Project**
```bash
# Core types
feat: new feature or capability
fix: bug fix
docs: documentation changes
style: formatting, no code change
refactor: code refactoring
test: adding or updating tests
chore: maintenance tasks

# CFD-specific types
physics: changes to physics models or equations
validate: validation against analytical/experimental data
optimize: performance or accuracy optimization
solver: changes to OpenFOAM solver integration
mesh: mesh generation or quality improvements
```

#### **Examples**
```bash
feat(heat-transfer): add conjugate heat transfer solver integration

physics(turbulence): implement k-omega SST model for external flow

validate(pipe-flow): add Poiseuille flow analytical validation

fix(critical): resolve memory leak in case manager cleanup

optimize(performance): reduce memory allocation in mesh generation

docs(physics): add heat transfer theory documentation
```

---

## 🚀 **Implementation Commands**

### **Repository Setup**

#### **Initial Repository Configuration**
```bash
# Clone repository
git clone https://github.com/webworn/openfoam-mcp-server.git
cd openfoam-mcp-server

# Configure git for CFD project
git config user.name "Your Name"
git config user.email "your.email@domain.com"

# Set up branch tracking
git branch --set-upstream-to=origin/Development Development
git branch --set-upstream-to=origin/main main

# Configure commit message template
git config commit.template .gitmessage
```

#### **Development Environment Setup**
```bash
# Install pre-commit hooks
pip install pre-commit
pre-commit install

# Setup OpenFOAM environment
source /opt/openfoam12/etc/bashrc
echo "source /opt/openfoam12/etc/bashrc" >> ~/.bashrc

# Configure build environment
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make -j$(nproc)
```

### **Daily Development Workflow**

#### **Start of Day Routine**
```bash
# Update local repository
git fetch --all --prune

# Switch to Development and update
git checkout Development
git pull origin Development

# Check for conflicts with your feature branches
git branch --merged Development
git branch --no-merged Development
```

#### **Feature Development**
```bash
# Create and switch to feature branch
git checkout -b feature/new-physics-model

# Make changes and test locally
# ... development work ...

# Stage and commit changes
git add .
git commit -m "feat(physics): implement new turbulence model"

# Run local validation
./scripts/validate-changes.sh

# Push and create PR
git push origin feature/new-physics-model
gh pr create --base Development
```

#### **Code Review Process**
```bash
# Address review comments
git add .
git commit -m "fix: address review comments for turbulence model"

# Rebase to clean up history (optional)
git rebase -i HEAD~3

# Force push to update PR (if rebased)
git push --force-with-lease origin feature/new-physics-model
```

### **Release Management**

#### **Prepare Release**
```bash
# Create release branch from Development
git checkout Development
git pull origin Development
git checkout -b release/v1.3.0

# Update version files
./scripts/update-version.sh 1.3.0

# Generate changelog
./scripts/generate-changelog.sh > CHANGELOG.md

# Commit version updates
git commit -am "bump: prepare release v1.3.0"
git push origin release/v1.3.0
```

#### **Deploy Release**
```bash
# After testing, merge to main
git checkout main
git merge --no-ff release/v1.3.0

# Create release tag
git tag -a v1.3.0 -m "Release v1.3.0: Enhanced Heat Transfer Analysis"

# Push to trigger deployment
git push origin main --tags

# Merge back to Development
git checkout Development
git merge main
git push origin Development

# Clean up release branch
git branch -d release/v1.3.0
git push origin --delete release/v1.3.0
```

### **Emergency Hotfix**

#### **Critical Issue Response**
```bash
# Create hotfix branch from main
git checkout main
git pull origin main
git checkout -b hotfix/critical-validation-fix

# Implement fix
# ... fix the issue ...

# Test the fix
./scripts/run-critical-tests.sh

# Fast-track merge to main
git checkout main
git merge --no-ff hotfix/critical-validation-fix
git tag -a v1.2.1 -m "Hotfix v1.2.1: Critical validation fix"

# Deploy immediately
git push origin main --tags

# Merge back to Development
git checkout Development
git merge main
git push origin Development
```

---

## 📊 **Quality Gates**

### **Automated Quality Checks**

#### **Pre-commit Hooks**
```yaml
# .pre-commit-config.yaml
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.4.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-json
      
  - repo: https://github.com/pocc/pre-commit-hooks
    rev: v1.3.5
    hooks:
      - id: clang-format
      - id: clang-tidy
      - id: cppcheck
```

#### **CI/CD Pipeline Gates**
```bash
# Build quality gate
cmake --build build --parallel
if [ $? -ne 0 ]; then
  echo "❌ Build failed"
  exit 1
fi

# Code quality gate
cppcheck --error-exitcode=1 src/
clang-tidy src/*.cpp

# Physics validation gate
python3 tests/validation/run_all_physics_tests.py
if [ $? -ne 0 ]; then
  echo "❌ Physics validation failed"
  exit 1
fi

# Performance regression gate
./scripts/check-performance-regression.sh
```

### **Manual Review Requirements**

#### **Physics Domain Review**
- **CFD Expert Review**: Required for all physics-related changes
- **Validation Specialist**: Required for validation test changes
- **Performance Engineer**: Required for optimization changes

#### **Code Quality Review**
- **Senior Developer**: Required for architectural changes
- **Security Expert**: Required for security-sensitive changes
- **Technical Writer**: Required for documentation changes

---

## 🎯 **CFD-Specific Recommendations**

### **Physics Development Best Practices**

#### **Validation-Driven Development**
```bash
# Always start with validation test
echo "1. Write analytical validation test"
echo "2. Implement physics"
echo "3. Validate against analytical solution"
echo "4. Validate against experimental data"
echo "5. Benchmark performance"
```

#### **Iterative Physics Implementation**
```bash
# Start with simplest case
git checkout -b physics/pipe-flow-laminar

# Implement laminar case first
# Validate against Poiseuille solution
# Then extend to turbulent case
git checkout -b physics/pipe-flow-turbulent

# Finally handle transitional regime
git checkout -b physics/pipe-flow-transitional
```

### **OpenFOAM Integration Guidelines**

#### **Solver Compatibility**
```bash
# Test against multiple OpenFOAM versions
./scripts/test-openfoam-compatibility.sh \
  --versions "10,11,12" \
  --physics "all" \
  --output compatibility-report.html
```

#### **Case Management**
```bash
# Validate case creation and cleanup
./scripts/validate-case-management.sh \
  --create-cases 100 \
  --run-parallel \
  --cleanup-verify
```

### **Performance Considerations**

#### **Memory Management**
```bash
# Profile memory usage for large cases
valgrind --tool=massif \
  ./build/openfoam-mcp-server \
  --case large_heat_transfer_case

# Analyze memory profile
ms_print massif.out.* > memory-analysis.txt
```

#### **Parallel Scaling**
```bash
# Test parallel performance
for cores in 1 2 4 8 16; do
  ./scripts/run-parallel-benchmark.sh \
    --cores $cores \
    --case standard_benchmark \
    --output scaling-results-$cores.json
done
```

---

## 🚨 **Emergency Procedures**

### **Critical Issue Response**

#### **Severity Levels**
- **Critical**: Security vulnerability, data corruption, physics errors
- **High**: Performance regression, CI/CD failure, major feature broken  
- **Medium**: Minor feature issues, documentation errors
- **Low**: Code style, minor optimization opportunities

#### **Response Times**
- **Critical**: Immediate response, hotfix within 2 hours
- **High**: Response within 4 hours, fix within 24 hours
- **Medium**: Response within 24 hours, fix within 1 week
- **Low**: Response within 1 week, fix when convenient

### **Rollback Procedures**

#### **Production Rollback**
```bash
# Immediate rollback to previous version
git checkout main
git log --oneline -10  # Find last stable version
git revert <problematic-commit-hash>
git push origin main

# Or rollback to specific tag
git reset --hard v1.2.0
git push --force origin main  # Use with extreme caution
```

#### **Database/State Rollback**
```bash
# If case database corruption
./scripts/restore-case-database.sh \
  --backup-timestamp "2024-01-15-10:30" \
  --verify-integrity

# Clean up corrupted OpenFOAM cases
./scripts/cleanup-corrupted-cases.sh \
  --scan-all \
  --auto-remove
```

---

**This comprehensive branch strategy ensures high-quality CFD software development while maintaining the flexibility needed for complex physics implementations.** 🌊🔄⚡