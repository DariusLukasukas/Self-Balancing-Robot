#pragma once

#include <WebServer.h>
#include "types.h"

// Initialize web server and start listening
void webServerInit();

// Handle incoming HTTP requests
void webServerUpdate();