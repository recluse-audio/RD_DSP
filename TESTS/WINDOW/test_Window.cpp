/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>
#include <string>
#include <vector>

#include "WINDOW/Window.h"
#include "HELPERS/CsvLoader.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

namespace
{
    constexpr double kPi = 3.14159265358979323846;

    void verifyWindowAgainstGolden (rd_dsp::Window::Shape shape,
                                    const std::string& relPath,
                                    int numSamples)
    {
        rd_dsp::Window window;
        window.setSize (numSamples);
        window.setShape (shape);

        const std::string goldenPath = std::string (RD_DSP_TESTS_DIR) + relPath;

        std::vector<std::vector<float>> rows;
        const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/false);
        INFO ("Golden path: " << goldenPath);
        REQUIRE (loaded);
        REQUIRE (rows.size() == 1);
        REQUIRE (static_cast<int> (rows[0].size()) == numSamples);

        const auto& goldenRow = rows[0];
        for (int i = 0; i < numSamples; ++i)
        {
            const float generated = window.getSample (i);
            const float golden    = goldenRow[static_cast<std::size_t> (i)];

            INFO ("sample index " << i);
            REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
        }
    }
}

TEST_CASE ("Window default-constructs with kNone shape (all ones)", "[Window]")
{
    rd_dsp::Window window;
    REQUIRE (window.getShape() == rd_dsp::Window::Shape::kNone);
    REQUIRE (window.getNumSamples() > 0);

    for (int i = 0; i < window.getNumSamples(); ++i)
        CHECK (window.getSample (i) == Catch::Approx (1.0f).margin (1e-6));
}

TEST_CASE ("Window::setSize resizes buffer and refills shape", "[Window]")
{
    rd_dsp::Window window;
    window.setShape (rd_dsp::Window::Shape::kHanning);

    window.setSize (256);
    CHECK (window.getNumSamples() == 256);

    // Hanning still applied: endpoints near zero, midpoint = 1
    CHECK (window.getSample (0)   == Catch::Approx (0.0f).margin (1e-6));
    CHECK (window.getSample (255) == Catch::Approx (0.0f).margin (1e-6));
}

TEST_CASE ("Window::setShape kHanning matches analytic 0.5 * (1 - cos(2*pi*n/(N-1)))", "[Window]")
{
    rd_dsp::Window window;
    window.setSize (1024);
    window.setShape (rd_dsp::Window::Shape::kHanning);

    const int N = window.getNumSamples();
    REQUIRE (N == 1024);

    for (int i = 0; i < N; ++i)
    {
        const double expected = 0.5 * (1.0 - std::cos (2.0 * kPi * static_cast<double>(i)
                                                       / static_cast<double>(N - 1)));
        INFO ("sample index " << i);
        REQUIRE (window.getSample (i) == Catch::Approx (static_cast<float>(expected)).margin (1e-6));
    }
}

TEST_CASE ("Window::setShape kHanning has zeros at endpoints and 1.0 in the middle", "[Window]")
{
    rd_dsp::Window window;
    window.setSize (1025); // odd -> exact center sample
    window.setShape (rd_dsp::Window::Shape::kHanning);

    CHECK (window.getSample (0)    == Catch::Approx (0.0f).margin (1e-6));
    CHECK (window.getSample (1024) == Catch::Approx (0.0f).margin (1e-6));
    CHECK (window.getSample (512)  == Catch::Approx (1.0f).margin (1e-6));
}

TEST_CASE ("Window::setShape kTukey has zeros at endpoints and flat 1.0 in middle", "[Window]")
{
    rd_dsp::Window window;
    window.setSize (1001);
    window.setShape (rd_dsp::Window::Shape::kTukey);

    // alpha=0.8, N-1=1000, taperEnd = 400.
    CHECK (window.getSample (0)    == Catch::Approx (0.0f).margin (1e-6));
    CHECK (window.getSample (1000) == Catch::Approx (0.0f).margin (1e-6));

    // middle flat region (n in [400, 600]) should be 1.0
    CHECK (window.getSample (500) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (window.getSample (400) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (window.getSample (600) == Catch::Approx (1.0f).margin (1e-6));
}

TEST_CASE ("Window::setShape kNone fills with all ones", "[Window]")
{
    rd_dsp::Window window;
    window.setSize (64);
    window.setShape (rd_dsp::Window::Shape::kHanning); // change away from default
    window.setShape (rd_dsp::Window::Shape::kNone);

    for (int i = 0; i < window.getNumSamples(); ++i)
        CHECK (window.getSample (i) == Catch::Approx (1.0f).margin (1e-6));
}

TEST_CASE ("Window::getInterpolatedSampleAtIndex returns exact sample at integer index", "[Window]")
{
    rd_dsp::Window window;
    window.setSize (128);
    window.setShape (rd_dsp::Window::Shape::kHanning);

    for (int i = 0; i < window.getNumSamples(); ++i)
        CHECK (window.getInterpolatedSampleAtIndex (static_cast<float>(i))
               == Catch::Approx (window.getSample (i)).margin (1e-6));
}

TEST_CASE ("Window none fill matches golden CSV exactly at N=8096", "[Window]")
{
    verifyWindowAgainstGolden (rd_dsp::Window::Shape::kNone,
                               "/WINDOW/GOLDEN/NONE/GOLDEN_WINDOW_NONE_8096.csv",
                               8096);
}

TEST_CASE ("Window hanning fill matches golden CSV exactly at N=8096", "[Window]")
{
    verifyWindowAgainstGolden (rd_dsp::Window::Shape::kHanning,
                               "/WINDOW/GOLDEN/HANNING/GOLDEN_WINDOW_HANNING_8096.csv",
                               8096);
}

TEST_CASE ("Window tukey fill matches golden CSV exactly at N=8096", "[Window]")
{
    verifyWindowAgainstGolden (rd_dsp::Window::Shape::kTukey,
                               "/WINDOW/GOLDEN/TUKEY/GOLDEN_WINDOW_TUKEY_8096.csv",
                               8096);
}

TEST_CASE ("Window::getInterpolatedSampleAtIndex linearly interpolates between samples", "[Window]")
{
    rd_dsp::Window window;
    window.setSize (128);
    window.setShape (rd_dsp::Window::Shape::kHanning);

    const float a = window.getSample (10);
    const float b = window.getSample (11);

    CHECK (window.getInterpolatedSampleAtIndex (10.5f)
           == Catch::Approx (0.5f * (a + b)).margin (1e-6));
}
