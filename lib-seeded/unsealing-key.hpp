#pragma once

#include "sodium-buffer.hpp"
#include "sealing-key.hpp"

/**
 * @brief an UnsealingKey is used to _unseal_ messages sealed with its
 * corresponding SealingKey.
 * The UnsealingKey and SealingKey are generated
 * from a seed and a set of derivation specified options in
 * @ref derivation_options_format.
 * 
 * The UnsealingKey includes a copy of the SealingKey, which can be
 * reconstituted as a SealingKey object via the getSealingKey method.
 */
class UnsealingKey {
public:
  /**
   * @brief The libSodium private key used for unsealing
   */
  const SodiumBuffer unsealingKeyBytes;
  /**
   * @brief The libsodium public key used for sealing
   */
  const std::vector<unsigned char> sealingKeyBytes;
  /**
   * @brief A @ref derivation_options_format string used to specify how this key is derived.
   */
  const std::string derivationOptionsJson;

  /**
   * @brief Construct a new UnsealingKey by passing its members.
   */
  UnsealingKey(
    const SodiumBuffer unsealingKeyBytes,
    const std::vector<unsigned char> sealingKeyBytes,
    const std::string derivationOptionsJson
  );

  /**
   * @brief Construct a new UnsealingKey by deriving a public/private
   * key pair from a seedBuffer and a set of derivation options
   * in @ref derivation_options_format.
   * 
   * @param seedBuffer The seed as sequence of bytes
   * @param derivationOptionsJson The derivation options in @ref derivation_options_format.
   */
  UnsealingKey(
    const SodiumBuffer& seedBuffer,
    const std::string& derivationOptionsJson
  );

  /**
   * @brief Construct a new UnsealingKey by deriving a public/private
   * key pair from a seed string and a set of derivation options
   * in @ref derivation_options_format.
   * 
   * @param seedString The private seed which is used to generate the key pair.
   * Anyone who knows (or can guess) this seed can re-generate the key pair
   * by passing it along with the derivationOptionsJson.
   * @param derivationOptionsJson The derivation options in @ref derivation_options_format.
   */
  UnsealingKey(
    const std::string& seedString,
    const std::string& derivationOptionsJson
  );

  /**
   * @brief Construct a new UnsealingKey by deriving a public/private
   * key pair from a seed string and a set of derivation options
   * in @ref derivation_options_format.
   * 
   * @param seedString The private seed which is used to generate the key pair.
   * Anyone who knows (or can guess) this seed can re-generate the key pair
   * by passing it along with the derivationOptionsJson.
   * @param derivationOptionsJson The derivation options in @ref derivation_options_format.
   */
  static UnsealingKey deriveFromSeed(
    const std::string& seedString,
    const std::string& derivationOptionsJson
  );



  /**
   * @brief Construct (reconstitute) from serialized JSON format
   * 
   * @param unsealingKeyAsJson 
   */
  static UnsealingKey fromJson(
    const std::string& unsealingKeyAsJson
  );


  /**
   * @brief Construct by copying another UnsealingKey
   */
  UnsealingKey(
    const UnsealingKey& other
  );

  /**
   * @brief Get the SealingKey used to seal messages that can be unsealed 
   * with this UnsealingKey
   */
  const SealingKey getSealingKey() const;

  /**
   * @brief Unseal a message 
   * 
   * @param ciphertext The sealed message to be unsealed
   * @param ciphertextLength The length of the sealed message
   * @param unsealingInstructions If this optional value was
   * set during the SealingKey::seal operation, the same value must
   * be provided to unseal the message or the operation will fail.
   * It can be used to pair a secret (sealed) message with public instructions
   * about what should happen after the message is unsealed.
   * @return const SodiumBuffer 
   * 
   * @exception CryptographicVerificationFailureException Thrown if the ciphertext
   * is not valid and cannot be unsealed.
   */
  const SodiumBuffer unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength,
    const std::string& unsealingInstructions
  ) const;

  /**
   * @brief Unseal a message 
   * 
   * @param ciphertext The sealed message to be unsealed
   * @param unsealingInstructions If this optional value was
   * set during the SealingKey::seal operation, the same value must
   * be provided to unseal the message or the operation will fail.
   * @return const SodiumBuffer 
   * 
   * @exception CryptographicVerificationFailureException Thrown if the ciphertext
   * is not valid and cannot be unsealed.
   */
  const SodiumBuffer unseal(
    const std::vector<unsigned char> &ciphertext,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Unseal a message from packaged format, ignoring the
   * derivationOptionsJson since this UnsealingKey has been
   * instantiated. (If it's the wrong key, the unseal will fail.)
   * 
   * @param packagedSealedMessage The message to be unsealed
   * @return const SodiumBuffer The plaintesxt message that had been sealed
   */
  const SodiumBuffer unseal(
    const PackagedSealedMessage& packagedSealedMessage
  ) const;

  /**
   * @brief Unseal a message by re-deriving the UnsealingKey from its seed. 
   * 
   * @param packagedSealedMessage The message to be unsealed
   * @param seedString The seed string used to generate the key pair of the
   * SealingKey used to seal this message and the UnsealingKey needed to unseal it.
   * @return const SodiumBuffer The plaintesxt message that had been sealed
   */
  static const SodiumBuffer unseal(
    const PackagedSealedMessage &packagedSealedMessage,
      const std::string& seedString
  ) {
    return UnsealingKey(seedString, packagedSealedMessage.derivationOptionsJson)
      .unseal(packagedSealedMessage.ciphertext, packagedSealedMessage.unsealingInstructions);
  }

  /**
   * @brief Serialize this object to a JSON-formatted string
   * 
   * It can be reconstituted by calling the constructor with this string.
   * 
   * @param indent The number of characters to indent the JSON (optional)
   * @param indent_char The character with which to indent the JSON (optional)
   * @return const std::string an UnsealingKey serialized to JSON format.
   */
  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;

  /**
   * @brief Serialize to byte array as a list of:
   *   (unsealingKeyBytes, sealingKeyBytes, derivationOptionsJson)
   * 
   * Stored in SodiumBuffer's fixed-length list format.
   * Strings are stored as UTF8 byte arrays.
   */
  const SodiumBuffer toSerializedBinaryForm() const;

  /**
   * @brief Deserialize from a byte array stored as a list of:
   *   (unsealingKeyBytes, sealingKeyBytes, derivationOptionsJson)
   * 
   * Stored in SodiumBuffer's fixed-length list format.
   * Strings are stored as UTF8 byte arrays.
   */
  static UnsealingKey fromSerializedBinaryForm(const SodiumBuffer &serializedBinaryForm);


};
