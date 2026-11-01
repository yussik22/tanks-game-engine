#pragma once

#include <optional>
#include <string>
#include "tanks/Types.hpp"

namespace tanks {

class KeyboardInput
{
public:
    std::optional<Command> parse(const std::string& key) const;
};

} // namespace tanks
