#include "TestHarness.hpp"
#include "Parsers.hpp"

int main() {
    std::vector<std::unique_ptr<lab::Parser>> parsers;
    parsers.push_back(std::make_unique<lab::SelectParser>());
    parsers.push_back(std::make_unique<lab::InsertParser>());
    parsers.push_back(std::make_unique<lab::CreateParser>());
    parsers.push_back(std::make_unique<lab::DropParser>());

    lab::TestHarness harness(std::move(parsers));
    return harness.runAll() ? 0 : 1;
}
