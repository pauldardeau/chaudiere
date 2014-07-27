// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_MOCKSOCKET_H
#define CHAUDIERE_MOCKSOCKET_H

#include <string>

#include "Socket.h"

namespace chaudiere
{

class MockSocket : public Socket
{
public:
   MockSocket(const std::string& payload);
};

}

#endif
