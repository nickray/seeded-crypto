#pragma once

#include <cassert>
#include "hash-functions.hpp"
#include "github-com-nlohmann-json/json.hpp"
// Must come after json.hpp
#include "./externally-generated/derivation-parameters.hpp"

class UnsealingInstructions {
/**
 * This class represents key generation options,
 * provided in JSON format, as an immutable class.
 */

public:
	std::vector<std::string> clientApplicationIdMustHavePrefix;
	std::string userMustAcknowledgeThisMessage;

	/**
	 * Create a UnsealingInstructions class from the JSON representation
	 * of the key generation options.
	 **/
	UnsealingInstructions(
		const std::string& unsealingInstructions
	);

	UnsealingInstructions(
		std::vector<std::string> clientApplicationIdMustHavePrefix,
		std::string userMustAcknowledgeThisMessage = {}
	);

	bool isApplicationIdAllowed(const std::string& applicationId) const;
	void validateApplicationId(const std::string& applicationId) const;

	std::string	toJson(
		int indent = -1,
	  const char indent_char = ' '
	) const;
};