# Executed operations and verification commands

Source changes were made with file patches and heredoc writes; complete changes are in the branch diff. The following is the chronological record of build, test, inspection and packaging operations, with reruns consolidated where they used the same command. No remote push, merge, release tag or PR was performed.

## Audit and branch
```bash
git clone https://github.com/Parinagpal11/aether-5g.git aether-sprint2
cd aether-sprint2
git switch -c feat/sprint2-ul-dl-harq
git log -1 --oneline
rg --files -g AGENTS.md -g CMakeLists.txt -g '*scheduler*' -g '*main*' -g '*test*'
```
Inspected CMakeLists.txt, both scheduler headers/sources, simulator/main.cpp, all existing tests, README, architecture/scope/status docs and .github/workflows/ci.yml. Starting commit: a27e313. No AGENTS.md was present in the repository. git status initially clean.

## Baseline
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```
Passed 19/19. Repeated with output saved to baseline.txt before source edits.

## Verified slices
```bash
cmake --build build -j 4
ctest --test-dir build -R '^Harq' --output-on-failure
ctest --test-dir build -R '^(Harq|Controller|Bsr)' --output-on-failure
ctest --test-dir build --output-on-failure
```
First HARQ slice: 8/8. Controller/BSR/HARQ slice: 19/19 selected cases. Initial PHY integration: 46/46 full suite (debug-initial.txt). Final independent-ledger and CLI integration: 50/50 (debug-tests.txt). No functional test failure occurred in these runs.

## Release
```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$PWD/build/_deps/googletest-src"
cmake --build build-release -j 4
ctest --test-dir build-release --output-on-failure
```
Passed 50/50, repeated after C++ formatting and sanitizer configuration changes. Configure/build/test outputs saved in release-*.txt.

## Sanitizers: failed default and successful constrained run
```bash
cmake -S . -B build-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DAETHER_SANITIZER=address,undefined \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$PWD/build/_deps/googletest-src"
cmake --build build-sanitize -j 4
```
Default build failed in GoogleTest discovery, with LeakSanitizer fatal error under ptrace. It did not reach CTest. Full failure evidence: sanitize-build-default.txt.

```bash
ASAN_OPTIONS=detect_leaks=0 cmake --build build-sanitize -j 4
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir build-sanitize --output-on-failure
```
Passed 50/50. Address/UB instrumentation enabled; leak detection disabled. Outputs: sanitize-configure.txt, sanitize-build.txt, sanitize-tests.txt.

## Scenario and consistency
```bash
python3 tools/run_bidirectional.py scenarios/bidirectional.json \
  ./build-release/aether_bidirectional > docs/sprint2/scenario-result.json
./build/aether_bidirectional rr 100 200 42 250 30 > docs/sprint2/scenario-debug.json
cmp docs/sprint2/scenario-result.json docs/sprint2/scenario-debug.json
```
JSON summaries match byte-for-byte across Debug/Release. Tests separately compare full traces within each build.

## Formatting and compatibility
Installed clang-format using `python -m pip install clang-format --quiet`. Formatted only new C++ files with the repository's new .clang-format. Checked those same files using `clang-format --dry-run --Werror`.

```bash
git diff --check
git diff --exit-code -- include/aether/mac/scheduler.hpp src/mac/scheduler.cpp \
  tests/mac_test.cpp tests/scheduler_test.cpp tests/scenario_test.py simulator/main.cpp
```
The initial staged whitespace check found four trailing-space lines in the captured sanitizer failure log. Those trailing spaces were normalized, then the staged check passed. No diagnostic content was removed. Legacy files are unchanged. Local Markdown targets checked with a Python path validator. Source and patch archives checked using Python zipfile; generated patch verified against a clean base using git apply --check.

## Not executed
Remote CI, Clang compilation, macOS compilation, Docker, leak detection to completion, TSan, coverage, long stress, or Sprint 2 performance benchmarks. Tool configuration is not evidence of execution.

## Local delivery
```bash
git add .
git diff --cached --check
git diff --cached --binary > ../AETHER-5G-Sprint2.patch
git worktree add --detach ../aether-sprint2-applycheck a27e313
git -C ../aether-sprint2-applycheck apply --check ../AETHER-5G-Sprint2.patch
git -c user.name=Codex -c user.email=codex@openai.com commit -m "Implement Sprint 2 bidirectional MAC and bounded HARQ"
git archive --format=zip --prefix=aether-5g/ HEAD -o ../AETHER-5G-Sprint2.zip
```
Patch is checked against the audited base before delivery. Source archive excludes build directories and contains the committed branch content. No GitHub push is performed.
