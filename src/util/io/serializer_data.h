#pragma once

#include "util/data_structures/string_manipulation.h"

#define TEST_NAME_CONVERSION(variable)		AT::util::extract_variable_name(#variable)
#define SERIALIZE_KEY_VALUE(variable)		serialize_key_value(AT::util::extract_variable_name(#variable), variable);

// @brief used with [serializer] to shorten the serializer::yaml::entry call
#define KEY_VALUE(var)						AT::util::extract_variable_name(#var), var

namespace AT::serializer {

	enum class option {

		save_to_file,
		load_from_file,
	};
	
}
