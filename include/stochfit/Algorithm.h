#pragma once

enum class SaAlgorithm : int {
    Greedy    = 0,
    Simulated = 1,
    Stun      = 2,
};

inline constexpr SaAlgorithm AlgorithmFromInt(int v) {
    switch (v) {
    case 0: return SaAlgorithm::Greedy;
    case 1: return SaAlgorithm::Simulated;
    case 2: return SaAlgorithm::Stun;
    default: return SaAlgorithm::Greedy;
    }
}

inline constexpr int AlgorithmToInt(SaAlgorithm a) {
    return static_cast<int>(a);
}
