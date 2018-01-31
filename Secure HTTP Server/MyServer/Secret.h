#include<stdio.h>
#include<iostream>
#include<cryptlib.h>
#include<cstdlib>
#include<des.h>
#include<string>
#include<randpool.h>
#include<rsa.h>
#include<hex.h>
#include<sstream>
#include<files.h>
#include<osrng.h>
using namespace std;
using namespace CryptoPP;

class Secret
{
public:
	static void deal_des(byte* input_string, byte*key, int state);
	static void Encode_RES(string &message);
	static void Decode_RES(string&message);
};