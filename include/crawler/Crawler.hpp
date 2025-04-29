#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <string>
#include <curl/curl.h>
#include <sstream>
#include <iostream>

std::string fetch_weather(const std::string& city);

#endif