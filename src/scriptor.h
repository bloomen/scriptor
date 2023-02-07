#pragma once

#include <atomic>

namespace scriptor
{

void
stop(int signal);

int
run(int argc, char** argv);

} // namespace scriptor
