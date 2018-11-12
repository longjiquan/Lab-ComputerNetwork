#include "config.h"

Config::Config()
{

}
Config::~Config()
{

}

const int Config::DEFAULT_SERVER_PORT=23456;
const int Config::DEFAULT_CLIENT_PORT=8080;
const string Config::DEFAULT_SERVER_ADDR_STR="0.0.0.0";
const int Config::MAX_CONNECTION_NUM=100;
const int Config::BUF_LENGTH=1024;
const string Config::OK_STATUS_STR="200 OK";
const string Config::ERROR_METHOD_STR="501 Not Implemented";
const string Config::DENY_STR="403 Forbidden";
const string Config::NOT_FOUND_STR="404 Not Found";
const string Config::DEFAULT_HTTP_VERSION_STR="HTTP/1.1 ";
const string Config::SERVER_STR="Server: csr_http1.1";
