#pragma once
#include <string>
#include "ConfigReader.h"
#include "Logger.h"

// Default Test Information
static const std::uint32_t NUM_OF_TEST_RUNS = 100;

static const std::uint32_t DEFAULT_BUFLEN = 1200;

static uint16_t const DEFAULT_SERVER_PORT_NUMBER = 27015;
static uint16_t const DEFAULT_CLIENT_PORT_NUMBER = 28015;

static char * const CONFIG_FILE_PATH = "config.txt";//"../Content/config.txt";