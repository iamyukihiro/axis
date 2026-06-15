#include "AxisParameterModel.h"

#include <array>

namespace axis::domain {

namespace {

constexpr std::array<ParameterSpec, parameterCount> specs{{
    {ParameterId::input,
     "input",
     "Input",
     ParameterKind::floatValue,
     {-24.0f, 12.0f, 0.01f},
     0.0f,
     "dB"},
    {ParameterId::center,
     "center",
     "Mid Gain",
     ParameterKind::floatValue,
     {-24.0f, 12.0f, 0.01f},
     0.0f,
     "dB"},
    {ParameterId::sideGain,
     "sideGain",
     "Side Gain",
     ParameterKind::floatValue,
     {-24.0f, 12.0f, 0.01f},
     0.0f,
     "dB"},
    {ParameterId::density,
     "density",
     "Side Density",
     ParameterKind::floatValue,
     {0.0f, 100.0f, 0.01f},
     0.0f,
     "%"},
    {ParameterId::sideSpark,
     "sideSpark",
     "Side Spark",
     ParameterKind::floatValue,
     {0.0f, 150.0f, 0.01f},
     0.0f,
     "%"},
    {ParameterId::sparkSend,
     "sparkSend",
     "Spark Send",
     ParameterKind::floatValue,
     {-24.0f, 24.0f, 0.01f},
     -9.0f,
     "dB"},
    {ParameterId::sparkGain,
     "sparkGain",
     "Spark Gain",
     ParameterKind::floatValue,
     {0.0f, 36.0f, 0.01f},
     6.0f,
     "dB"},
    {ParameterId::sparkDuck,
     "sparkDuck",
     "Spark Duck",
     ParameterKind::floatValue,
     {0.0f, 100.0f, 0.01f},
     15.0f,
     "%"},
    {ParameterId::sparkThreshold,
     "sparkThreshold",
     "Spark Threshold",
     ParameterKind::floatValue,
     {0.0f, 100.0f, 0.01f},
     50.0f,
     "%"},
    {ParameterId::width,
     "width",
     "Spark Width",
     ParameterKind::floatValue,
     {0.0f, 200.0f, 0.01f},
     100.0f,
     "%"},
    {ParameterId::sparkPitch,
     "sparkPitch",
     "Spark Pitch",
     ParameterKind::floatValue,
     {-24.0f, 24.0f, 0.01f},
     0.0f,
     "st"},
    {ParameterId::output,
     "output",
     "Output",
     ParameterKind::floatValue,
     {-24.0f, 12.0f, 0.01f},
     0.0f,
     "dB"},
    {ParameterId::autoGain, "autoGain", "Auto Gain", ParameterKind::boolValue, {}, 1.0f, ""},
    {ParameterId::softClip, "softClip", "Soft Clip", ParameterKind::boolValue, {}, 0.0f, ""},
    {ParameterId::sparkSolo, "sparkSolo", "Spark Solo", ParameterKind::boolValue, {}, 0.0f, ""},
    {ParameterId::bypass, "bypass", "Bypass", ParameterKind::boolValue, {}, 0.0f, ""},
}};

constexpr std::size_t toIndex(ParameterId id) noexcept { return static_cast<std::size_t>(id); }

}

const ParameterSpec &parameterSpec(ParameterId id) noexcept { return specs[toIndex(id)]; }

const std::array<ParameterSpec, parameterCount> &parameterSpecs() noexcept { return specs; }

std::string_view parameterKey(ParameterId id) noexcept { return parameterSpec(id).key; }

}
