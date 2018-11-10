#include "config.h"

Config::Config()
{

}
Config::~Config()
{

}

const int Config::DEFAULT_PORT=80;
const int Config::BUF_LENGTH=1024;
const int Config::MIN_BUF=128;
const int Config::USER_ERROR=-1;
const string Config::SERVER_STR="Server: csr_http1.1\r\n";
const u_long Config::BLOCK_MODE=1;
const int Config::MAXCONNECTION=100;
