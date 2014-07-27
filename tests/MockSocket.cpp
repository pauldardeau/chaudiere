// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "MockSocket.h"

MockSocket::MockSocket(const std::string& payload) :
   Socket(-1)
{
   setLineInputBuffer(payload);
}

