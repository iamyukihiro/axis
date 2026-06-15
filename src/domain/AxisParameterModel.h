#pragma once

#include <array>
#include <string_view>

namespace axis::domain {

enum class ParameterId {
    input,
    center,
    sideGain,
    density,
    sideSpark,
    sparkSend,
    sparkGain,
    sparkDuck,
    sparkThreshold,
    width,
    sparkPitch,
    output,
    autoGain,
    softClip,
    sparkSolo,
    bypass
};

enum class ParameterKind { floatValue, boolValue };

struct ParameterRange {
    float min = 0.0f;
    float max = 1.0f;
    float step = 0.01f;
};

struct ParameterSpec {
    ParameterId id;
    std::string_view key;
    std::string_view name;
    ParameterKind kind;
    ParameterRange range;
    float defaultValue;
    std::string_view unitLabel;
};

constexpr std::size_t parameterCount = 16;

const ParameterSpec &parameterSpec(ParameterId id) noexcept;
const std::array<ParameterSpec, parameterCount> &parameterSpecs() noexcept;
std::string_view parameterKey(ParameterId id) noexcept;

}
