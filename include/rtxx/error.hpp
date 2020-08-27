#pragma once

#include <rtxx/config.hpp>
#include <system_error>

namespace rtxx
{

using error_code = std::error_code;
using system_error = std::system_error;
using std::generic_category;
using std::system_category;
using errc = std::errc;
using std::make_error_code;

} // namespace rtxx
