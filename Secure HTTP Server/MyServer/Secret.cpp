#include"Secret.h"

void Secret::deal_des(byte* input_string, byte*key, int state)
{
	BlockTransformation *t = NULL;
	if (state == 1)
	{
		t = new DES_EDE2_Encryption(key, DES_EDE2::KEYLENGTH);
	}
	else
		t = new DES_EDE2_Decryption(key, DES_EDE2::KEYLENGTH);
	int steps = 1024 / t->BlockSize();
	for (int i = 0; i < steps; i++)
	{
		int offset = i*t->BlockSize();
		t->ProcessBlock(input_string + offset);
	}
}

void Secret::Encode_RES(string &message)
{
	AutoSeededRandomPool rng;
	InvertibleRSAFunction parameters;
	parameters.GenerateRandomWithKeySize(rng, 1024);
	RSA::PublicKey priKey;
	FileSource file1("priKey.txt",true);
	priKey.BERDecode(file1);
	RSAES_OAEP_SHA_Encryptor e(priKey);
	string cipher;
	StringSource(message, true, new PK_EncryptorFilter(rng, e, new StringSink(cipher)));
	message = cipher;
}

void Secret::Decode_RES(string&message)
{
	AutoSeededRandomPool rng;
	RSA::PublicKey priKey;
	RSA::PrivateKey pubKey;
	FileSource pubvSink("pubKey.txt",true);
	pubKey.BERDecode(pubvSink);
	RSAES_OAEP_SHA_Decryptor d(pubKey);
	string recover;
	StringSource(message, true, new PK_DecryptorFilter(rng, d, new StringSink(recover)));
	message = recover;
}

