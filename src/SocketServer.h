// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SOCKETSERVER_H
#define CHAUDIERE_SOCKETSERVER_H

#include <string>

#include "KernelEventServer.h"
#include "KeyValuePairs.h"


namespace chaudiere
{
   class RequestHandler;
   class ServerSocket;
   class Socket;
   class SocketRequest;
   class ThreadPoolDispatcher;
   class SectionedConfigDataSource;
   class ThreadingFactory;


class SocketServer
{
   public:
      /**
       * Constructs a SocketServer with the file name/path for a configuration file
       * @param serverName the name of the server
       * @param serverVersion the version of the server
       * @param configFilePath the file name/path for configuration settings
       */
      SocketServer(const std::string& serverName,
                   const std::string& serverVersion,
                   const std::string& configFilePath);
   
      /**
       * Destructor
       */
      virtual ~SocketServer();

      /**
       *
       * @param socket
       * @return
       */
      virtual RequestHandler* handlerForSocket(Socket* socket) = 0;
      
      /**
       *
       * @param socketRequest
       * @return
       */
      virtual RequestHandler* handlerForSocketRequest(SocketRequest* socketRequest) = 0;
      
      /**
       *
       * @return
       */
      virtual SocketServiceHandler* createSocketServiceHandler() = 0;

      /**
       * Retrieves the current time in Greenwich Mean Time (GMT)
       * @return current time in GMT
       */
      std::string getSystemDateGMT() const;
   
      /**
       * Retrieves the current time for server in local time
       * @return current time as local server time
       */
      std::string getLocalDateTime() const;

      /**
       * Runs the built-in socket server
       * @return exit code for the server process
       */
      int runSocketServer();
   
      /**
       * Runs a kernel event server (e.g., kqueue or epoll)
       * @return exit code for the server process
       */
      int runKernelEventServer();
   
      /**
       * Runs the server using either the built-in socket server or a kernel event server
       * @return exit code for the server process
       */
      int run();

      /**
       * Retrieves the configuration data source of configuration settings
       * @see SectionedConfigDataSource()
       * @return the configuration data source
       */
      SectionedConfigDataSource* getConfigDataSource();
   
      /**
       * Retrieves the size of the socket send buffer
       * @return size of the socket send buffers
       */
      int getSocketSendBufferSize() const;
   
      /**
       * Retrieves the size of the socket receive buffer
       * @return size of the socket receive buffers
       */
      int getSocketReceiveBufferSize() const;
   
      /**
       * Retrieves the identifier for the server
       * @return server identifier
       */
      const std::string& getServerId() const;
   
      /**
       * Retrieves the size in bytes of a generic (void*) pointer
       * @return platform pointer size
       */
      int platformPointerSizeBits() const;

      /**
       * Service a request for a socket when using a kernel event server
       * @param socketRequest the SocketRequest to process
       * @see SocketRequest()
       */
      void serviceSocket(SocketRequest* socketRequest);

      /**
       * Convenience method to retrieve a setting and convert it to a boolean
       * @param kvp the collection of key/value pair settings
       * @param setting the name of the setting whose value is to be retrieved and converted
       * @see KeyValuePairs()
       * @return boolean value (or false if value cannot be retrieved or converted)
       */
      bool hasTrueValue(const KeyValuePairs& kvp,
                        const std::string& setting) const;
   
      /**
       * Convenience method to retrieve a setting and convert it to an integer
       * @param kvp the collection of key/value pair settings
       * @param setting the name of the setting whose value is to be retrieved and converted
       * @see KeyValuePairs()
       * @return integer value (or -1 if value cannot be retrieved or converted)
       */
      int getIntValue(const KeyValuePairs& kvp,
                      const std::string& setting) const;
   
      /**
       * Convenience method to replace all occurrences of keys in collection with their values
       * @param kvp the collection of key/value pairs for replacement
       * @param s the string to search and replace all variables in
       */
      void replaceVariables(const KeyValuePairs& kvp,
                            std::string& s) const;
   
      /**
       * Determines if compression is turned on for the specified mime type
       * @param mimeType the mime type to check whether to compress
       * @return boolean indicating whether the specified mime type is to be compressed
       */
      bool compressResponse(const std::string& mimeType) const;
   
      /**
       * Determines if gzip compression is enabled for the server
       * @return boolean indicating if gzip compression is enabled
       */
      bool compressionEnabled() const;
   
      /**
       * Retrieves the minimum size of the response payload to be compressed
       * @return minimum size of response payload (in bytes) to be compressed
       */
      int minimumCompressionSize() const;

   
   protected:
      /**
       * Initializes the socket server on the specified port by default by reading and
       * applying configuration values read from configuration data source
       * @param port the default port (may be overridden by configuration values)
       * @return boolean indicating whether initialiation was successful
       */
      virtual bool init(int port);
   


   private:
      KernelEventServer* m_kernelEventServer;
      ServerSocket* m_serverSocket;
      ThreadPoolDispatcher* m_threadPool; //weak
      ThreadingFactory* m_threadingFactory;
      KeyValuePairs m_properties;
      std::string m_logLevel;
      std::string m_concurrencyModel;
      std::string m_configFilePath;
      std::string m_startupTime;
      std::string m_serverString;
      std::string m_threading;
      std::string m_sockets;
      std::string m_serverName;
      std::string m_serverVersion;
      bool m_isDone;
      bool m_isThreaded;
      bool m_isUsingKernelEventServer;
      bool m_isFullyInitialized;
      bool m_compressionEnabled;
      int m_threadPoolSize;
      int m_serverPort;
      int m_socketSendBufferSize;
      int m_socketReceiveBufferSize;
      int m_minimumCompressionSize;

      // copies not allowed
      SocketServer(const SocketServer&);
      SocketServer& operator=(const SocketServer&);

};

}

#endif


