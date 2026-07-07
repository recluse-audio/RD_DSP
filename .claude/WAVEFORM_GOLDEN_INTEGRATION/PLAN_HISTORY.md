# WAVEFORM_GOLDEN_INTEGRATION — PLAN HISTORY
Append-only. Newest at bottom. Source of truth for /rd_plan --resume.

## 2026-07-07 12:59 CDT (2026-07-07T17:59Z) — focus-set
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- commit: 1fa28bc
- note: Plan created. Direction decided = C++ matches regenerated goldens (RMS normalize + 0.95*tanh peak protect). Decisions abandoned: revert golden tanh; investigate-only.

## 2026-07-07 13:05 CDT (2026-07-07T18:05Z) — note
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- commit: 1fa28bc
- note: User correction (verified in source). The 9 failures split into Bucket A (C++ generation fix) and Bucket B (CSV-loading tests: WaveformFileManager, Synth, Pulsar) that no C++ waveform-gen change can fix. gen_golden_pulsar.py synthesizes a pure analytic sine*window and never loads the wavetable; fix is to make downstream generators derive from the regenerated wavetable. Added the tanh tension (sine peak ~0.743) as open Q4. PLAN_PHASE_3/4 rewritten for golden-chain regeneration in dependency order.

## 2026-07-07 13:12 CDT (2026-07-07T18:12Z) — decision
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- commit: 1fa28bc
- note: Q4 RESOLVED = NO tanh. Verified current goldens carry tanh (measured peaks: sine 0.744, tri 0.808, saw 0.855, square 0.845; RMS 0.56-0.71). Decision: RMS-normalize to 0.7071 only, allow peaks >1.0 (pre-tanh peaks: sine 1.0, tri ~1.19, square ~1.35, saw ~1.40), headroom handled downstream (Synth output gain, non-blocking Q5). Abandoned: tanh soft-clip, peak-normalize-to-1.0, RMS+peak-normalize. Consequence: strip protect_peaks from gen_golden_waveform.py and regenerate ALL waveform goldens without tanh. Bonus: wavetable sine becomes pure unit sine, so Pulsar golden likely matches after a plain regen (PLAN_STEP_4_1 downgraded to verify-first).

## 2026-07-07 13:47 CDT (2026-07-07T18:47Z) — decision
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- commit: 61303ae
- note: MAJOR PIVOT. Single-variant "RMS-only, no tanh" decision ABANDONED. New direction = four scaling variants generated for every waveform AND wavetable: RAW (no scaling), RMS_SCALED (rms to 0.7071), PEAK_SCALED (tanh only), RMS_PEAK_SCALED (rms then tanh). Output to new plural TESTS/WAVEFORM/GOLDEN/WAVEFORMS/<VARIANT>/ and WAVETABLES/<VARIANT>/. Peak op = tanh soft-clip 0.95*tanh(x/0.95) (user-selected, not peak-normalize/clamp). Constants: target_rms 1/sqrt(2), ceiling 0.95. C++ matches each level. nlohmann/json + rapidcsv now dependencies via CMake FetchContent; user elected core lib MAY use them (conflicts with zero-dep/no-file-IO hard rule -> reconciled via RD_DSP_ENABLE_FILE_IO guard, default OFF, open Q1). Plan fully rewritten: PLAN_PHASE_0 generator refactor to emit 4 variants, PLAN_PHASE_1 CMake deps, PLAN_PHASE_2 four C++ scaling stages, PLAN_PHASE_3 per-variant tests, PLAN_PHASE_4 migrate legacy WAVEFORM/ALT_WAVEFORMS/SAW + downstream consumers. Open Qs: Q1 core-dep guard, Q2 which variant downstream wants, Q3 JSON gains vs 0.5 default, Q4 precision/tolerance.

## 2026-07-07 13:47 CDT (2026-07-07T18:47Z) — decision
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- commit: 61303ae
- note: Deps + remaining Qs resolved. nlohmann/json + rapidcsv are PERMANENT project deps usable anywhere incl. core lib; zero-dep/no-file-IO rule retired for them, do not raise again (saved to memory). Q1/Q2 dropped. Q3 resolved: HARMONIC_DATA JSONs are single source of truth for BOTH Python and C++ (this is why nlohmann is needed); C++ seeds harmonic sum from JSON gains, hardcoded 0.5 in _initHarmonicData removed. Q4 resolved: compute in double, narrow to float32 on store so goldens represent the exact C++ float value; tolerance stays 1e-6. No open questions remain.

## 2026-07-07 14:22 CDT (2026-07-07T19:22Z) — phase-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_3"
- commit: 61303ae (uncommitted working tree)
- note: PLAN_PHASE_0 complete. gen_golden_waveform.py refactored: RMS and tanh decoupled into pure independent functions; four scaling variants (RAW/RMS_SCALED/PEAK_SCALED/RMS_PEAK_SCALED) emitted per JSON into WAVEFORMS/<VARIANT>/; samples narrowed to float32 on store. New driver HELPER_SCRIPTS/regen_goldens.py regenerates 4 shapes x 4 variants + 1 wavetable per variant into WAVETABLES/<VARIANT>/. Verified: every RMS_SCALED variant rms=0.7071; RAW peaks saw 1.759 tri 1.203 sine 1.000 square 0.927; tanh variants compressed as expected. Legacy singular WAVEFORM/, ALT_WAVEFORMS/, SAW/ still present (removal deferred to PLAN_PHASE_4).

## 2026-07-07 14:22 CDT (2026-07-07T19:22Z) — focus-set
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_1 :: PLAN_STEP_1_1"
- commit: 61303ae (uncommitted working tree)
- note: Next: add nlohmann/json + rapidcsv via CMake FetchContent, exposed to RD_DSP and Tests.

## 2026-07-07 14:26 CDT (2026-07-07T19:26Z) — phase-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_1 :: PLAN_STEP_1_1"
- commit: 61303ae
- note: PLAN_PHASE_1 complete. nlohmann/json + rapidcsv are git submodules under SUBMODULES/, already wired via add_subdirectory (not FetchContent) and linked to RD_DSP, Standalone, and Tests (CMakeLists.txt:50-56,82-86,103-108). Submodule self-tests disabled. Tests carry RD_DSP_TESTS_DIR compile def to find goldens at runtime. Verified: build_tests.py configures + builds RD_DSP/Catch2/Tests clean. Guard step (old PLAN_STEP_1_2) not needed (zero-dep rule retired).

## 2026-07-07 14:26 CDT (2026-07-07T19:26Z) — focus-set
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_2 :: PLAN_STEP_2_1"
- commit: 61303ae
- note: Next: implement the four C++ scaling stages (raw/rmsScale/peakScale tanh/rmsPeakScale) in WaveFactory matching the Python constants + order.

## 2026-07-07 14:52 CDT (2026-07-07T19:52Z) — step-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_3 :: PLAN_STEP_3_1"
- commit: 61303ae (uncommitted)
- note: DESIGN CHANGE (user): scaling stays test-callable but lives as REAL WaveFactory methods, not a test helper and not an enum. Added WaveFactory::rmsScale(Waveform&, float targetRms=kDefaultTargetRMS) and peakScale(Waveform&, float ceiling=kDefaultPeakCeiling), both compute in double, narrow to float32 on store; replaced the empty normalizeWaveform stub; added kDefaultTargetRMS/kDefaultPeakCeiling consts to WaveFactory.h. PLAN_STEP_2_1 done. PLAN_STEP_2_2 (double-accumulate fill rewrite) was STARTED then interrupted by user - fillWaveformWithHarmonics left as-is for now. PLAN_STEP_3_1 begun: new TESTS/WAVEFORM/test_WaveformVariants.cpp loads HARMONIC_DATA JSON via nlohmann, sets harmonics, fills RAW, applies the actual WaveFactory scaling fns, compares to WAVEFORMS/<VARIANT>/ via rapidcsv, one SECTION per variant. Sine passes ALL four variants at margin 1e-6 (32776 assertions). Note: fixed TEST_CASE name/tag to use a comma so [tag] filtering works (repo's existing tests concatenate name+tag, a latent bug). Next: extend to saw/square/tri (multi-harmonic) - may need the interrupted double-accumulate fill for 1e-6 to hold.
