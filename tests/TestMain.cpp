#include <JuceHeader.h>

#include <iostream>

class ConsoleUnitTestRunner final : public juce::UnitTestRunner {
  public:
    void logMessage(const juce::String &message) override { std::cout << message << std::endl; }
};

int main() {
    ConsoleUnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.setPassesAreLogged(false);
    runner.runAllTests();

    auto totalFailures = 0;
    for (int index = 0; index < runner.getNumResults(); ++index)
        totalFailures += runner.getResult(index)->failures;

    return totalFailures == 0 ? 0 : 1;
}
