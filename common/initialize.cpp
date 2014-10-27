/**
 * @file
 *
 * Startup code that must be called by @c main.
 */

#include <thread>
#include <Eigen/Core>
#include <ImfThreading.h>
#include "initialize.h"

void utsInitialize()
{
    Eigen::initParallel();
    Imf::setGlobalThreadCount(std::thread::hardware_concurrency());
}
