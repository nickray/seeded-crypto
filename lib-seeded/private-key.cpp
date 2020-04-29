#include "github-com-nlohmann-json/json.hpp"
#include "private-key.hpp"
#include "crypto_box_seal_salted.h"
#include "derivation-options.hpp"
#include "convert.hpp"
#include "exceptions.hpp"

PrivateKey::PrivateKey(
    const SodiumBuffer _privateKeyBytes,
    const std::vector<unsigned char> _publicKeyBytes,
    const std::string _derivationOptionsJson
  ) :
    privateKeyBytes(_privateKeyBytes),
    publicKeyBytes(_publicKeyBytes),
    derivationOptionsJson(_derivationOptionsJson)
    {
    if (publicKeyBytes.size() != crypto_box_PUBLICKEYBYTES) {
      throw InvalidKeyDerivationOptionValueException("Invalid public key size");
    }
    if (privateKeyBytes.length != crypto_box_SECRETKEYBYTES) {
      throw InvalidKeyDerivationOptionValueException("Invalid private key size for public/private key pair");
    }
  }

PrivateKey::PrivateKey(
  const SodiumBuffer &seedBuffer,
  const std::string& _derivationOptionsJson
) : derivationOptionsJson(_derivationOptionsJson), publicKeyBytes(crypto_box_PUBLICKEYBYTES), privateKeyBytes(crypto_box_SECRETKEYBYTES) {
  if (seedBuffer.length < crypto_box_SEEDBYTES){
    throw std::invalid_argument("Insufficient seed length");
  }
  crypto_box_seed_keypair((unsigned char *) publicKeyBytes.data(), privateKeyBytes.data, seedBuffer.data);
}

  PrivateKey::PrivateKey(
    const std::string& _seedString,
    const std::string& _derivationOptionsJson
  ) : PrivateKey(
      DerivationOptions::deriveMasterSecret(_seedString, _derivationOptionsJson, DerivationOptionsJson::type::Public, crypto_box_SEEDBYTES),
      _derivationOptionsJson
  ) {}

PrivateKey::PrivateKey(
  const PrivateKey &other
):
  publicKeyBytes(other.publicKeyBytes), 
  derivationOptionsJson(other.derivationOptionsJson),
  privateKeyBytes(other.privateKeyBytes)
  {}

const SodiumBuffer PrivateKey::unseal(
  const unsigned char* ciphertext,
  const size_t ciphertextLength,
  const std::string& postDecryptionInstructions
) const {
  if (ciphertextLength <= crypto_box_SEALBYTES) {
    throw CryptographicVerificationFailureException("Public/Private unseal failed: Invalid message length");
  }
  SodiumBuffer plaintext(ciphertextLength -crypto_box_SEALBYTES);

  const int result = crypto_box_salted_seal_open(
    plaintext.data,
    ciphertext,
    ciphertextLength,
    publicKeyBytes.data(),
    privateKeyBytes.data,
    postDecryptionInstructions.c_str(),
    postDecryptionInstructions.length()
  );
  if (result != 0) {
    throw CryptographicVerificationFailureException("Public/Private unseal failed: the private key doesn't match the public key used to seal the message, the post-decryption instructions do not match those used to seal the message, or the ciphertext was modified/corrupted.");
  }
  return plaintext;
}

const SodiumBuffer PrivateKey::unseal(
  const std::vector<unsigned char> &ciphertext,
  const std::string& postDecryptionInstructions
) const {
  return unseal(ciphertext.data(), ciphertext.size(), postDecryptionInstructions
  );
};

const SodiumBuffer PrivateKey::unseal(
  const PackagedSealedMessage &packagedSealedMessage
) const {
  return unseal(packagedSealedMessage.ciphertext, packagedSealedMessage.postDecryptionInstructions);
}

const PublicKey PrivateKey::getPublicKey() const {
  return PublicKey(publicKeyBytes, derivationOptionsJson);
}


/////
//  JSON
////
namespace PrivateKeyJsonField {
  const std::string publicKeyBytes = "publicKeyBytes";
  const std::string privateKeyBytes = "privateKeyBytes";
  const std::string derivationOptionsJson = "derivationOptionsJson";
}

PrivateKey PrivateKey::fromJson(
  const std::string& PrivateKeyAsJson
) {
  try {
    nlohmann::json jsonObject = nlohmann::json::parse(PrivateKeyAsJson);
    return PrivateKey(
      SodiumBuffer::fromHexString(jsonObject.at(PrivateKeyJsonField::privateKeyBytes)),
      hexStrToByteVector(jsonObject.at(PrivateKeyJsonField::publicKeyBytes)),
      jsonObject.value(PrivateKeyJsonField::derivationOptionsJson, ""));
  } catch (nlohmann::json::exception e) {
    throw JsonParsingException(e.what());
  }
}

const std::string PrivateKey::toJson(
  int indent,
  const char indent_char
) const {
  nlohmann::json asJson;
  asJson[PrivateKeyJsonField::privateKeyBytes] = privateKeyBytes.toHexString();
  asJson[PrivateKeyJsonField::publicKeyBytes] = toHexStr(publicKeyBytes);
  asJson[PrivateKeyJsonField::derivationOptionsJson] = derivationOptionsJson;
  return asJson.dump(indent, indent_char);
};


const SodiumBuffer PrivateKey::toSerializedBinaryForm() const {
  SodiumBuffer derivationOptionsJsonBuffer = SodiumBuffer(derivationOptionsJson);
  SodiumBuffer _publicKeyBytes(publicKeyBytes);
  SodiumBuffer _derivationOptionsJson(derivationOptionsJson);
  return SodiumBuffer::combineFixedLengthList({
    &privateKeyBytes,
    &_publicKeyBytes,
    &_derivationOptionsJson
  });
}

PrivateKey PrivateKey::fromSerializedBinaryForm(SodiumBuffer serializedBinaryForm) {
  const auto fields = serializedBinaryForm.splitFixedLengthList(3);
  return PrivateKey(fields[0], fields[1].toVector(), fields[2].toUtf8String());
}
