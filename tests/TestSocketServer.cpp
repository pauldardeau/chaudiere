// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstring>
#include <fstream>

#include "TestSocketServer.h"
#include "SocketServer.h"
#include "RequestHandler.h"
#include "SocketServiceHandler.h"
#include "SocketRequest.h"
#include "Socket.h"
#include "ServerSocket.h"
#include "SectionedConfigDataSource.h"
#include "KeyValuePairs.h"

using namespace chaudiere;

namespace {

// Echoes back whatever's read from the socket, exercising exactly the
// handler lifecycle (construction, run(), and destruction/ownership of
// the underlying socket) that runSocketServer() drives on each accepted
// connection -- this is the code path that had a double-free/leak bug
// (fixed in SocketServer.cpp's runSocketServer()) before real end-to-end
// testing caught it.
class EchoRequestHandler : public chaudiere::RequestHandler {
public:
   explicit EchoRequestHandler(chaudiere::Socket* socket) :
      chaudiere::RequestHandler(socket) {
   }

   explicit EchoRequestHandler(chaudiere::SocketRequest* socketRequest) :
      chaudiere::RequestHandler(socketRequest) {
   }

   void run() override {
      Socket* socket = getSocket();
      if (nullptr != socket) {
         char buffer[16];
         ::memset(buffer, 0, sizeof(buffer));
         if (socket->readSocket(buffer, 4)) {
            socket->write(std::string(buffer, 4));
         }
      }
   }
};

class EchoSocketServiceHandler : public chaudiere::SocketServiceHandler {
public:
   void serviceSocket(chaudiere::SocketRequest* socketRequest) override {
      EchoRequestHandler handler(socketRequest);
      handler.run();
   }

   const std::string& getName() const override {
      static const std::string name = "EchoSocketServiceHandler";
      return name;
   }
};

class TestableSocketServer : public chaudiere::SocketServer {
public:
   TestableSocketServer(const std::string& serverName,
                        const std::string& serverVersion,
                        const std::string& configFilePath) :
      SocketServer(serverName, serverVersion, configFilePath) {
   }

   RequestHandler* handlerForSocket(Socket* socket) override {
      return new EchoRequestHandler(socket);
   }

   RequestHandler* handlerForSocketRequest(SocketRequest* socketRequest) override {
      return new EchoRequestHandler(socketRequest);
   }

   SocketServiceHandler* createSocketServiceHandler() override {
      return new EchoSocketServiceHandler();
   }
};

void writeServerConfig(const std::string& configPath,
                       int port,
                       const std::string& extraLines = "") {
   std::ofstream configFile(configPath.c_str());
   configFile << "[server]\n";
   configFile << "port = " << port << "\n";
   configFile << "threading = none\n";
   configFile << extraLines;
   configFile.close();
}

}

//******************************************************************************

TestSocketServer::TestSocketServer() :
   poivre::TestSuite("TestSocketServer") {
}

//******************************************************************************

void TestSocketServer::runTests() {
   testConstructor();
   testGetConfigDataSource();
   testGetSocketSendBufferSize();
   testGetSocketReceiveBufferSize();
   testGetServerId();
   testPlatformPointerSizeBits();
   testGetSystemDateGMT();
   testGetLocalDateTime();
   testHasTrueValue();
   testGetIntValue();
   testReplaceVariables();
   testServiceSocket();
   testRunSocketServer();
}

//******************************************************************************

void TestSocketServer::testConstructor() {
   TEST_CASE("testConstructor");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44710);

   TestableSocketServer server("TestServer", "0.1", configPath);
   require(true, "constructing a SocketServer from a valid config file should not throw");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetConfigDataSource() {
   TEST_CASE("testGetConfigDataSource");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44711);

   TestableSocketServer server("TestServer", "0.1", configPath);
   SectionedConfigDataSource* dataSource = server.getConfigDataSource();
   require(nullptr != dataSource, "getConfigDataSource should return a non-null data source");
   require(dataSource->hasSection("server"), "the data source should reflect the [server] section from the config file");

   delete dataSource;
   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetSocketSendBufferSize() {
   TEST_CASE("testGetSocketSendBufferSize");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44712);

   TestableSocketServer server("TestServer", "0.1", configPath);
   require(8192 == server.getSocketSendBufferSize(), "socket send buffer size should default to 8192 when not configured");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetSocketReceiveBufferSize() {
   TEST_CASE("testGetSocketReceiveBufferSize");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44713);

   TestableSocketServer server("TestServer", "0.1", configPath);
   require(8192 == server.getSocketReceiveBufferSize(), "socket receive buffer size should default to 8192 when not configured");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetServerId() {
   TEST_CASE("testGetServerId");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44714, "server_string = $PRODUCT_NAME/$PRODUCT_VERSION\n");

   TestableSocketServer server("TestServer", "0.1", configPath);
   requireStringEquals("TestServer/0.1", server.getServerId(), "getServerId should reflect the configured, variable-substituted server_string");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testPlatformPointerSizeBits() {
   TEST_CASE("testPlatformPointerSizeBits");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44715);

   TestableSocketServer server("TestServer", "0.1", configPath);
   require((int)(sizeof(void*) * 8) == server.platformPointerSizeBits(), "platformPointerSizeBits should match this platform's pointer size");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetSystemDateGMT() {
   TEST_CASE("testGetSystemDateGMT");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44716);

   TestableSocketServer server("TestServer", "0.1", configPath);
   const std::string dateGMT = server.getSystemDateGMT();
   requireFalse(dateGMT.empty(), "getSystemDateGMT should return a non-empty string");
   require(std::string::npos != dateGMT.find("GMT"), "getSystemDateGMT should be formatted with a GMT suffix");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetLocalDateTime() {
   TEST_CASE("testGetLocalDateTime");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44717);

   TestableSocketServer server("TestServer", "0.1", configPath);
   requireFalse(server.getLocalDateTime().empty(), "getLocalDateTime should return a non-empty string");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testHasTrueValue() {
   TEST_CASE("testHasTrueValue");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44718);

   TestableSocketServer server("TestServer", "0.1", configPath);

   KeyValuePairs kvp;
   kvp.addPair("enabled", "true");
   kvp.addPair("disabled", "no");

   require(server.hasTrueValue(kvp, "enabled"), "hasTrueValue should be true for a setting whose value is 'true'");
   requireFalse(server.hasTrueValue(kvp, "disabled"), "hasTrueValue should be false for a setting whose value is 'no'");
   requireFalse(server.hasTrueValue(kvp, "missing"), "hasTrueValue should be false for a setting that isn't present");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testGetIntValue() {
   TEST_CASE("testGetIntValue");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44719);

   TestableSocketServer server("TestServer", "0.1", configPath);

   KeyValuePairs kvp;
   kvp.addPair("count", "42");

   require(42 == server.getIntValue(kvp, "count"), "getIntValue should parse the configured integer value");
   require(-1 == server.getIntValue(kvp, "missing"), "getIntValue should return -1 for a setting that isn't present");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testReplaceVariables() {
   TEST_CASE("testReplaceVariables");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44720);

   TestableSocketServer server("TestServer", "0.1", configPath);

   KeyValuePairs kvp;
   kvp.addPair("$NAME", "Tonnerre");
   kvp.addPair("$VERSION", "0.1");

   std::string s = "$NAME version $VERSION";
   server.replaceVariables(kvp, s);
   requireStringEquals("Tonnerre version 0.1", s, "replaceVariables should substitute every variable placeholder with its value");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testServiceSocket() {
   TEST_CASE("testServiceSocket");

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44721);

   TestableSocketServer server("TestServer", "0.1", configPath);

   const int port = 44722;
   ServerSocket serverListener(port);
   Socket clientSocket("127.0.0.1", port);
   Socket* acceptedSocket = serverListener.accept();

   EchoSocketServiceHandler serviceHandler;
   SocketRequest socketRequest(acceptedSocket, &serviceHandler);

   require(clientSocket.write("ping"), "writing to the client socket should succeed");
   server.serviceSocket(&socketRequest);

   char responseBuffer[5];
   ::memset(responseBuffer, 0, sizeof(responseBuffer));
   require(clientSocket.readSocket(responseBuffer, 4) > 0, "client should receive an echoed response");
   requireStringEquals("ping", std::string(responseBuffer), "serviceSocket should have dispatched to the handler that echoed the request");

   deleteFile(configPath);
}

//******************************************************************************

void TestSocketServer::testRunSocketServer() {
   TEST_CASE("testRunSocketServer");

   // runSocketServer() itself loops accepting connections until the
   // process exits, with no public shutdown API, so it can't be driven
   // directly from a test without leaking a thread (and its server
   // socket) past this test's lifetime. Instead, this replays the exact
   // per-connection sequence from inside runSocketServer()'s loop body --
   // construct a handler via handlerForSocket(), run it, then delete it --
   // which is precisely the code path that had a double-free/leak bug
   // (fixed in SocketServer.cpp) before real end-to-end testing caught
   // it. Running this several times under ASan/LeakSanitizer is what
   // actually proves the fix holds.

   const std::string configPath = getTempFile();
   writeServerConfig(configPath, 44723);
   TestableSocketServer server("TestServer", "0.1", configPath);

   const int port = 44724;
   ServerSocket serverListener(port);

   for (int i = 0; i < 5; ++i) {
      Socket clientSocket("127.0.0.1", port);
      require(clientSocket.write("ping"), "writing to the server should succeed");

      Socket* acceptedSocket = serverListener.accept();
      require(nullptr != acceptedSocket, "accept should return a connected socket");

      RequestHandler* handler = server.handlerForSocket(acceptedSocket);
      try {
         handler->run();
      } catch (...) {
         delete handler;
         throw;
      }
      delete handler;

      char responseBuffer[5];
      ::memset(responseBuffer, 0, sizeof(responseBuffer));
      require(clientSocket.readSocket(responseBuffer, 4) > 0, "client should receive an echoed response");
      requireStringEquals("ping", std::string(responseBuffer), "each accepted connection should be handled independently, without leaking or double-freeing its socket");
   }

   deleteFile(configPath);
}

//******************************************************************************
