// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef MOCKSOCKET_H
#define MOCKSOCKET_H

#include <string>

#include "Socket.h"


class MockSocket : public Socket
{
public:
   MockSocket(const std::string& payload);
};

#endif
