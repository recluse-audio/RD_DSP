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

## 2026-07-07 18:54 CDT (2026-07-07T23:54Z) — step-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_3 :: PLAN_STEP_3_2"
- commit: 61303ae (uncommitted)
- note: PLAN_STEP_3_1 done. Extended test_WaveformVariants.cpp to all four shapes (sine/saw/square/triangle) x four variants, data-driven. First run: saw+square RAW failed 1e-6 at 2 samples (float accumulation). Root cause was the FLOAT phase argument in _writeHarmonicToWaveform: ratio*sampleIndex grows large (up to ~65k) and float32 loses precision inside std::sin. Fix (minimal, user-directed style: keep _writeHarmonicToWaveform, keep names, store cast in local calculatedSample): compute phasePos/harmonicSample/prevSampleValue in double via a local kTwoPiDouble, narrow to float once. Now ALL 16 shape/variant combos pass at 1e-6 (131104 assertions in the tag). Full suite: 164 passed / 9 failed. The 9 are PRE-EXISTING legacy failures (not regressions): test_Waveform.cpp:52 x4 (Waveform analytic _fillWith* vs old goldens, untouched), test_WaveFactory.cpp:95/229 (RAW sine vs old singular WAVEFORM/ tanh golden), test_WaveformFileManager:122, test_Synth:521, test_PulsarTrain:710 (Bucket B, old goldens). All are PLAN_PHASE_4 migration work.

## 2026-07-07 19:01 CDT (2026-07-08T00:01Z) — phase-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_3 :: PLAN_STEP_3_2"
- commit: 61303ae (uncommitted)
- note: PLAN_STEP_3_2 done, PLAN_PHASE_3 complete for the new-tree tests. Refactored test_WaveformVariants.cpp (no duplication): extracted buildScaledWaveform + compareWaveformToRow + loadGoldenRows; added a wavetable TEST_CASE that loads WAVETABLES/<VARIANT>/GOLDEN_Wavetable_BasicShapes_8192.csv and compares each row (order saw/sine/square/triangle per gen_golden_wavetable.py sorted glob) to the C++-generated shape+scaling. All 5 cases (4 shapes + wavetable) x 4 variants pass at 1e-6 (262212 assertions). PLAN_STEP_3_3 (full suite green) still blocked by the 9 pre-existing legacy failures = PLAN_PHASE_4. Next: PLAN_STEP_4_1 migrate legacy tests/consumers onto WAVEFORMS/<VARIANT>/ and retire old singular WAVEFORM/, ALT_WAVEFORMS/, SAW/, old BASIC_TABLE. NOTE: old goldens + legacy tests were authored by the user - confirm before deleting.

## 2026-07-07 19:31 CDT (2026-07-08T00:31Z) — step-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_4 :: PLAN_STEP_4_1"
- commit: 61303ae (uncommitted)
- note: PLAN_PHASE_4 groups 1+2 done, 9 -> 3 failing. Group 1: repointed the two test_WaveFactory.cpp sine cases at WAVEFORMS/RMS_PEAK_SCALED/ and applied applyScaleRMS+applyPeakNormalization (kept CsvLoader, 1e-5 margin) - pass. Group 2 (user: retire): removed the 4 test_Waveform.cpp shape-fill golden compares (sine/tri/square/saw) plus the now-unused verifyWaveAgainstGolden helper; those used Waveform::_fillWith* analytic path (31/32 harmonics, different from JSON method) - coverage now lives in test_WaveformVariants.cpp. Remaining group 3 (downstream, harder): test_WaveformFileManager:122, test_Synth:521, test_PulsarTrain:710. KEY BLOCKER found: gen_golden_wavetable.py assembles rows in ALPHABETICAL filename order (saw,sine,square,triangle) but consumers map wavePosition 0.0->sine,0.25->tri,0.5->square,0.75->saw, so row 0 is saw not sine. Synth/Pulsar also need their OUTPUT goldens regenerated from the new wavetable. Needs user direction before touching.

## 2026-07-07 19:37 CDT (2026-07-08T00:37Z) — step-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_4 :: PLAN_STEP_4_1"
- commit: 61303ae (uncommitted)
- note: Group 3 progress, 3 -> 2 failing. WAVETABLE ROW ORDER fixed (user: order by wavePosition): added --order to gen_golden_wavetable.py (explicit basename order, falls back to sorted glob); MOVED the regen driver from HELPER_SCRIPTS/ to TESTS/WAVEFORM/GOLDEN/SCRIPTS/regen_goldens.py (user rule: golden scripts live under TESTS/<TOPIC>/GOLDEN/SCRIPTS) with WAVETABLE_ROW_ORDER = sine,tri,square,saw; regenerated all WAVETABLES/<VARIANT>/ (sine now row 0). Updated test_WaveformVariants.cpp wavetable orderedStems to match; [WaveformVariants] still green (262212 assertions). Repointed ALL of test_WaveformFileManager.cpp off old singular WAVEFORM/ and BASIC_TABLE onto WAVEFORMS/RMS_PEAK_SCALED/ and WAVETABLES/RMS_PEAK_SCALED/ (two replace_all) - passes. Remaining 2: test_Synth:521 (synth.process vs SYNTH/GOLDEN/OUTPUT) and test_PulsarTrain:710 (pulsar.process vs PULSAR/GOLDEN/OUTPUT; still loads old non-variant WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv at line 686). These need their OUTPUT goldens regenerated from current wavetable/DSP - a separate sub-problem (synth-output generator may not exist yet). Nothing committed all session.

## 2026-07-07 19:49 CDT (2026-07-08T00:49Z) — step-done
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_4 :: PLAN_STEP_4_2"
- commit: 61303ae (uncommitted)
- note: FULL SUITE GREEN, 170/170 (406873 assertions). Group 3 finished. Pulsar (test_PulsarTrain:710): repointed the failing test's wavetable load to WAVETABLES/RAW/ (row 0 = unit sine peak 1.0), matching gen_golden_pulsar.py's pure sin*window; golden unchanged; passes (confirms the plan's prediction). Synth (test_Synth:521): repointed generate_golden_synth_sine.py to read WAVEFORMS/RAW/ (was non-existent old SINE/..._8096.csv paths), FIXED its OUTPUT_DIR bug (SCRIPT_DIR.parent/OUTPUT -> SCRIPT_DIR/OUTPUT so it writes to SYNTH/GOLDEN/OUTPUT not SYNTH/OUTPUT), repointed the failing test's wavetable load to WAVETABLES/RAW/, regenerated the 4 M69 shape goldens; removed the stray SYNTH/OUTPUT dir from the first buggy run. DECISION: synth + pulsar consume the RAW variant (unit-level shapes) so their analytic generators match. Remaining OPTIONAL PLAN_STEP_4_2: retire legacy dirs (singular WAVEFORM/, ALT_WAVEFORMS/, SAW/, SINE/TRIANGLE/SQUARE/, WAVETABLES/BASIC_TABLE/, old non-variant WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv still referenced by other passing pulsar/synth tests) - needs repointing those refs first. Also NOTE: synth/pulsar generators still sit directly under GOLDEN/ not GOLDEN/SCRIPTS (user rule) - follow-up. Nothing committed since user's checkpoint.
