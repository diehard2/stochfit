// Standalone profiling harness for the full SA fit loop.
// Usage: stochfit_profile [path/to/data.txt] [iterations]
// Defaults: resources/test1refl.txt, 5,000,000 iterations

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#endif

#include "StochFitHarness.h"

#ifndef RESOURCES_DIR
#define RESOURCES_DIR "../../resources"
#endif

static bool LoadReflData(const std::string& path,
                         std::vector<double>& q,
                         std::vector<double>& refl,
                         std::vector<double>& reflErr)
{
    std::ifstream f(path);
    if (!f) {
        std::cerr << "Cannot open: " << path << "\n";
        return false;
    }
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        double qv, rv, ev;
        if (!(ss >> qv >> rv >> ev)) continue;
        q.push_back(qv);
        refl.push_back(rv);
        reflErr.push_back(ev);
    }
    return !q.empty();
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::string dataPath = RESOURCES_DIR "/test1refl.txt";
    int    iterations = 5'000'000;
    int    resolution = 3;
    bool   xrOnly     = true;
    double targetChi  = 0.0;   // 0 = disabled

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg.starts_with("--resolution="))
            resolution = std::stoi(std::string(arg.substr(13)));
        else if (arg.starts_with("--iterations="))
            iterations = std::stoi(std::string(arg.substr(13)));
        else if (arg.starts_with("--target-chi="))
            targetChi = std::stod(std::string(arg.substr(13)));
        else if (arg == "--opaque")
            xrOnly = false;
        else if (arg.starts_with("--"))
            std::fprintf(stderr, "Unknown flag: %s\n", argv[i]);
        else if (i == 1)
            dataPath = argv[i];
        else
            iterations = std::stoi(argv[i]);
    }

    std::vector<double> q, refl, reflErr;
    if (!LoadReflData(dataPath, q, refl, reflErr)) return 1;
    std::cout << "Loaded " << q.size() << " data points from " << dataPath << "\n";

    ReflSettings settings{};
    settings.Q         = q;
    settings.Refl      = refl;
    settings.ReflError = reflErr;
    settings.Directory = ".";

    // SLD parameters (silicon substrate, X-ray at 1.24 Å)
    settings.SubSLD     = 9.38;
    settings.FilmSLD    = 9.38;
    settings.SupSLD     = 0.0;
    settings.SubAbs     = 2e-8;
    settings.FilmAbs    = 1e-14;
    settings.SupAbs     = 0.0;
    settings.UseSurfAbs = false;
    settings.Wavelength = 1.24;

    // Box model
    settings.Boxes      = 40;
    settings.FilmLength = 25.0;
    settings.Resolution = resolution;
    settings.Forcesig   = 0.0;

    // Q trimming
    settings.CritEdgeOffset = 0;
    settings.HighQOffset    = 0;

    // Algorithm — Simulated Annealing with Log(R) objective (matches tuned defaults)
    settings.Objectivefunction       = 0;
    settings.Algorithm               = 0;
    settings.Paramtemp               = 0.03;
    settings.Inittemp                = 10.0;
    settings.Platiter                = 4000;
    settings.Slope                   = 0.95;
    settings.Gamma                   = 0.05;
    settings.Tempiter                = 100;
    settings.STUNdeciter             = 200000;
    settings.Gammadec                = 0.85;
    settings.STUNfunc                = 0;
    settings.Adaptive                = false;
    settings.Sigmasearch             = 10;
    settings.NormalizationSearchPerc = 0;
    settings.AbsorptionSearchPerc    = 0;
    settings.Impnorm                 = false;
    settings.XRonly                  = xrOnly;
    settings.Iterations              = iterations;

    StochFit harness(settings);
    if (auto err = harness.GetInitError(); !err) {
        std::cerr << "Init error: " << err.error() << "\n";
        return 1;
    }

    auto wallStart = std::chrono::steady_clock::now();
    harness.Start(iterations);
    std::printf("SA started: %d iterations, %d boxes, %zu data points, resolution=%d, xrOnly=%s\n\n",
                iterations, settings.Boxes, q.size(), resolution, xrOnly ? "true" : "false");

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        DataSnapshot snap = harness.GetData();
        double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - wallStart).count();
        double pct = 100.0 * snap.iteration / iterations;
        double ips  = snap.iteration / elapsed;
        std::printf("[%5.1f%%] iter=%-9d  χ²=%.4e  GoF=%.4f  rough=%.3f  %.0f iter/s\n",
                    pct, snap.iteration, snap.chiSquare, snap.goodnessOfFit,
                    snap.roughness, ips);
        std::fflush(stdout);
        if (snap.isFinished) break;
        if (targetChi > 0.0 && snap.chiSquare > 0.0 && snap.chiSquare <= targetChi) {
            std::printf("  --> χ² target %.6g reached\n", targetChi);
            break;
        }
    }

    harness.Stop();
    double totalSec = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - wallStart).count();

    DataSnapshot final = harness.GetData();
    std::printf("\n=== Profile Complete ===\n");
    std::printf("Iterations : %d\n", final.iteration);
    std::printf("Wall time  : %.3f s\n", totalSec);
    std::printf("Throughput : %.0f iter/s\n", final.iteration / totalSec);
    std::printf("Final χ²  : %.6g\n", final.chiSquare);
    std::printf("Final GoF  : %.4f\n", final.goodnessOfFit);
    std::printf("Roughness  : %.3f Å\n", final.roughness);
    return 0;
}
