#pragma once

#include <vector>
#include <string>
#include <sodium.h>
#include "sodium-buffer.hpp"
#include "packaged-sealed-message.hpp"

/**
 * @brief A PublicKey is used to _seal_ messages, in combination with a
 * PrivateKey which can _unseal_ them.
 * The key pair of this PublicKey and the matching PrivateKey are generated
 * from a seed and a set of key-derivation specified options in JSON format
 * @ref key_derivation_options_format.
 * 
 * To derive a public key from a seed, first derive the corresponding
 * PrivateKey and then call PrivateKey::getPublicKey.
 *
 * Sealing a message (_plaintext_) creates a _ciphertext which contains
 * the message but from which observers who do not have the PrivateKey
 * cannot discern the contents of the message.
 * Sealing also provides integrity-protection, which will prevent the
 * message from being unsealed if it is modified.
 * We use the verbs seal and unseal, rather than encrypt and decrypt,
 * because the encrypting alone does not confer that the message includes
 * an integrity (message authentication) code to prove that the ciphertext
 * has not been tampered with.
 * 
 * Note that sealing data does not prevent attackers who capture a sealed message
 * (ciphertext) in transit with another validly-sealed message. A SigningKey
 * can be used to sign messages that another party can verify that the
 * message has not been forged or modified since the signer approved it.
 * 
 * @ingroup DerivedFromSeeds
 */
class PublicKey {
public:
  /**
   * @brief Construct a PublicKey from a JSON string
   * 
   * @param publicKeyAsJson The JSON encoding of a PublicKey
   */
  static PublicKey fromJson(const std::string& publicKeyAsJson);
  
  /**
   * @brief The binary representation of the public key used for sealing
   */
  const std::vector<unsigned char> publicKeyBytes;
  /**
   * @brief A @ref key_derivation_options_format string used to specify how this key is derived.
   */
  const std::string keyDerivationOptionsJson;

  /**
   * @brief Construct a new Public Key object by passing its members.
   */
  PublicKey(
    const std::vector<unsigned char>& publicKeyBytes,
    const std::string& keyDerivationOptionsJson
  );

  /**
   * @brief Serialize this object to a JSON-formatted string
   * 
   * It can be reconstituted by calling the constructor with this string.
   * 
   * @param indent The number of characters to indent the JSON (optional)
   * @param indent_char The character with which to indent the JSON (optional)
   * @return const std::string
   */
  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;
  
  /**
   * @brief *Avoid Using* Seal a message using a raw libsodium public key.
   * 
   * Instead of using this static method, we recommend you use the seal
   * method on an instance of a PublicKey object.
   * This static method is used internally to libsodium's seal operation.
   * We have exposed so that others can replicate the internals of this class
   * if necessary, but recommend that only when there are reasons not to call
   * the non-static seal operation on an instance of this class.
   * 
   * @param message The plaintext message to seal
   * @param publicKey The public key matching the private key used to unseal it.
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * RefPDI.
   * @return const std::vector<unsigned char> The sealed message (ciphertext)
   */
  static const std::vector<unsigned char> sealToCiphertextOnly(
    const SodiumBuffer& message,
    const std::vector<unsigned char>& publicKey,
    const std::string& postDecryptionInstructions = {}
  );

  /**
   * @brief *Avoid Using* Seal a message using a raw libsodium public key.
   * 
   * Instead of using this static method, we recommend you use the seal
   * method on an instance of a PublicKey object.
   * This static method is used internally to libsodium's seal operation.
   * We have exposed so that others can replicate the internals of this class
   * if necessary, but recommend that only when there are reasons not to call
   * the non-static seal operation on an instance of this class.
   * 
   * @param message The plaintext message to seal
   * @param messageLength The length of the plaintext message to seal (in bytes)
   * @param publicKey The public key matching the private key used to unseal it.
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * RefPDI.
   * @return const std::vector<unsigned char> The sealed message (ciphertext)
   */
  static const std::vector<unsigned char> sealToCiphertextOnly(
    const unsigned char* message,
    const size_t messageLength,
    const std::vector<unsigned char> &publicKey,
    const std::string& postDecryptionInstructions = {}
  );

  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal 
   * @param messageLength The length of the plaintext message in bytes
   * @param postDecryptionInstructions If this optional string
   * is passed, the same string must be passed to unseal the message.
   * It can be used to pair a secret (sealed) message with public instructions
   * about what should happen after the message is unsealed.
   * @return const std::vector<unsigned char> 
   */
  const std::vector<unsigned char> sealToCiphertextOnly(
    const unsigned char* message,
    const size_t messageLength,
    const std::string& postDecryptionInstructions = {}
  ) const;

  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintext message to seal
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * @return const std::vector<unsigned char> 
   */
  const std::vector<unsigned char> sealToCiphertextOnly(
    const SodiumBuffer &message,
    const std::string& postDecryptionInstructions = {}
  ) const;


  /**
   * @brief Seal a plaintext message and then package the results
   * along with its keyDerivationOptionsJson and postDecryptionInstructions
   * into a SealedPackagedMessage.
   * 
   * @param message The plaintext message to seal
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * @return const PackagedSealedMessage Everything needed to re-derive
   * the PrivateKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */
  const PackagedSealedMessage seal(
    const SodiumBuffer& message,
    const std::string& postDecryptionInstructions
  ) const;

  /**
   * @brief Seal a plaintext message and then package the results
   * along with its keyDerivationOptionsJson and postDecryptionInstructions
   * into a SealedPackagedMessage.
   * 
   * @param message The plaintext message to seal
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * @return const PackagedSealedMessage Everything needed to re-derive
   * the PrivateKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */
  const PackagedSealedMessage seal(
    const std::vector<unsigned char>& message,
    const std::string& postDecryptionInstructions = ""
  ) const;



  /**
   * @brief Seal a plaintext message and then package the results
   * along with its keyDerivationOptionsJson and postDecryptionInstructions
   * into a SealedPackagedMessage.
   * 
   * @param message The plaintext message to seal
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * @return const PackagedSealedMessage Everything needed to re-derive
   * the PrivateKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */
  const PackagedSealedMessage seal(
    const std::string& message,
    const std::string& postDecryptionInstructions = {}
  ) const;


  /**
   * @brief Seal a plaintext message and then package the results
   * along with its keyDerivationOptionsJson and postDecryptionInstructions
   * into a SealedPackagedMessage.
   * 
   * @param message The plaintext message to seal
   * @param messageLength The length of the plaintext to seal
   * @param postDecryptionInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * @return const PackagedSealedMessage Everything needed to re-derive
   * the PrivateKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */
  const PackagedSealedMessage seal(
    const unsigned char* message,
    const size_t messageLength,
    const std::string& postDecryptionInstructions
  ) const;

  /**
   * @brief Get the copy of the raw public key bytes used by lib-sodium
   * 
   * @return const std::vector<unsigned char> 
   */
  const std::vector<unsigned char> getPublicKeyBytes() const;

  /**
   * @brief Get the JSON-formatted key-derivation options string used to generate
   * the public-private key pair.
   * 
   * @return const std::string in  @ref key_derivation_options_format
   */
  const std::string getKeyDerivationOptionsJson() const {
    return keyDerivationOptionsJson; 
  }

  /**
   * @brief Serialize to byte array as a list of:
   *   (publicKeyBytes, keyDerivationOptionsJson)
   * 
   * Stored in SodiumBuffer's fixed-length list format.
   * Strings are stored as UTF8 byte arrays.
   */
  const SodiumBuffer toSerializedBinaryForm() const;

  /**
   * @brief Deserialize from a byte array stored as a list of:
   *   (publicKeyBytes, keyDerivationOptionsJson)
   * 
   * Stored in SodiumBuffer's fixed-length list format.
   * Strings are stored as UTF8 byte arrays.
   */
  static PublicKey fromSerializedBinaryForm(SodiumBuffer serializedBinaryForm);

};

