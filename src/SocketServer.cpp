// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <string>
#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "SocketServer.h"

// sockets
#include "ServerSocket.h"
#include "Socket.h"

#include "RequestHandler.h"
#include "SocketRequest.h"

// utils
#include "BasicException.h"
#include "IniReader.h"
#include "KeyValuePairs.h"
#include "StrUtils.h"
#include "Logger.h"
#include "SystemInfo.h"

// threading
#include "Mutex.h"
#include "Runnable.h"
#include "Thread.h"
#include "ThreadPoolDispatcher.h"
#include "ThreadingFactory.h"
#include "PthreadsThreadingFactory.h"
//#include "StdThreadingFactory.h"

// kernel events
#include "EpollServer.h"
#include "KqueueServer.h"

#include "AutoPointer.h"


static const std::string CFG_TRUE_SETTING_VALUES = "yes|true|1";

static const std::string EMPTY = "";
static const std::string SPACE = " ";
static const std::string EOL   = "\n";
static const std::string COLON = ":";


// default settings
static const int CFG_DEFAULT_SEND_BUFFER_SIZE     = 8192;
static const int CFG_DEFAULT_RECEIVE_BUFFER_SIZE  = 8192;

static const int CFG_DEFAULT_PORT_NUMBER          = 9000;

static const int CFG_DEFAULT_THREAD_POOL_SIZE     = 4;


// configuration sections
static const std::string CFG_SECTION_SERVER                 = "server";
static const std::string CFG_SECTION_LOGGING                = "logging";

// logging config values
static const std::string CFG_LOGFILE_ACCESS                 = "access_log";
static const std::string CFG_LOGFILE_ERROR                  = "error_log";

// server config values
static const std::string CFG_SERVER_PORT                    = "port";
static const std::string CFG_SERVER_THREADING               = "threading";
static const std::string CFG_SERVER_THREAD_POOL_SIZE        = "thread_pool_size";
static const std::string CFG_SERVER_LOG_LEVEL               = "log_level";
static const std::string CFG_SERVER_SEND_BUFFER_SIZE        = "socket_send_buffer_size";
static const std::string CFG_SERVER_RECEIVE_BUFFER_SIZE     = "socket_receive_buffer_size";
static const std::string CFG_SERVER_ALLOW_BUILTIN_HANDLERS  = "allow_builtin_handlers";
static const std::string CFG_SERVER_STRING                  = "server_string";
static const std::string CFG_SERVER_SOCKETS                 = "sockets";

// socket options
static const std::string CFG_SOCKETS_SOCKET_SERVER          = "socket_server";
static const std::string CFG_SOCKETS_KERNEL_EVENTS          = "kernel_events";


// threading options
static const std::string CFG_THREADING_PTHREADS             = "pthreads";
static const std::string CFG_THREADING_CPP11                = "c++11";
static const std::string CFG_THREADING_GCD_LIBDISPATCH      = "gcd_libdispatch";
static const std::string CFG_THREADING_NONE                 = "none";

// logging level options
static const std::string CFG_LOGGING_CRITICAL               = "critical";
static const std::string CFG_LOGGING_ERROR                  = "error";
static const std::string CFG_LOGGING_WARNING                = "warning";
static const std::string CFG_LOGGING_INFO                   = "info";
static const std::string CFG_LOGGING_DEBUG                  = "debug";
static const std::string CFG_LOGGING_VERBOSE                = "verbose";

// mime types
static const std::string MIME_APPLICATION_JSON  = "application/json";
static const std::string MIME_APPLICATION_XML   = "application/xml";
static const std::string MIME_TEXT_HTML         = "text/html";
static const std::string MIME_TEXT_PLAIN        = "text/plain";


// module config values
static const std::string APP_PREFIX = "app:";

static const std::size_t APP_PREFIX_LEN = APP_PREFIX.length();

static const char* LOG_WEEKDAY_NAME[7] = {
   "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char* LOG_MONTH_NAME[12] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

using namespace chaudiere;

//******************************************************************************
//******************************************************************************

SocketServer::SocketServer(const std::string& serverName,
                           const std::string& serverVersion,
                           const std::string& configFilePath) :
   m_kernelEventServer(NULL),
   m_serverSocket(NULL),
   m_threadPool(NULL),
   m_threadingFactory(NULL),
   m_configFilePath(configFilePath),
   m_serverName(serverName),
   m_serverVersion(serverVersion),
   m_isDone(false),
   m_isThreaded(true),
   m_isUsingKernelEventServer(false),
   m_isFullyInitialized(false),
   m_threadPoolSize(CFG_DEFAULT_THREAD_POOL_SIZE),
   m_serverPort(CFG_DEFAULT_PORT_NUMBER) {
   LOG_INSTANCE_CREATE("SocketServer")
   init(CFG_DEFAULT_PORT_NUMBER);
}

//******************************************************************************

SectionedConfigDataSource* SocketServer::getConfigDataSource() {
   return new IniReader(m_configFilePath);
}

//******************************************************************************

int SocketServer::getSocketSendBufferSize() const {
   return m_socketSendBufferSize;
}

//******************************************************************************

int SocketServer::getSocketReceiveBufferSize() const {
   return m_socketReceiveBufferSize;
}

//******************************************************************************

const std::string& SocketServer::getServerId() const {
   return m_serverString;
}

//******************************************************************************

bool SocketServer::hasTrueValue(const KeyValuePairs& kvp,
                                const std::string& setting) const {
   bool hasTrueValue = false;
   
   if (kvp.hasKey(setting)) {
      const std::string& settingValue = kvp.getValue(setting);
      
      if (StrUtils::containsString(CFG_TRUE_SETTING_VALUES, settingValue)) {
         hasTrueValue = true;
      }
   }
   
   return hasTrueValue;
}

//******************************************************************************

int SocketServer::getIntValue(const KeyValuePairs& kvp,
                              const std::string& setting) const {
   int value = -1;
   
   if (kvp.hasKey(setting)) {
      const std::string& valueAsString = kvp.getValue(setting);
      const int intValue = StrUtils::parseInt(valueAsString);
      
      if (intValue > 0) {
         value = intValue;
      }
   }
   
   return value;
}

//******************************************************************************

void SocketServer::replaceVariables(const KeyValuePairs& kvp,
                                    std::string& s) const {
   if (!s.empty()) {
      std::vector<std::string> keys;
      kvp.getKeys(keys);
      const std::vector<std::string>::const_iterator itEnd = keys.end();
      std::vector<std::string>::const_iterator it = keys.begin();
      
      for (; it != itEnd; it++) {
         const std::string& key = *it;
         if (StrUtils::containsString(s, key)) {
            StrUtils::replaceAll(s, key, kvp.getValue(key));
         }
      }
   }
}

//******************************************************************************

bool SocketServer::init(int port)
{
   const bool isLoggingDebug = Logger::isLogging(Debug);
   
   m_serverPort = port;

   AutoPointer<SectionedConfigDataSource*> configDataSource(NULL);
   
   try {
      configDataSource.assign(getConfigDataSource());
   } catch (const BasicException& be) {
      LOG_ERROR("exception retrieving config data: " + be.whatString())
      return false;
   } catch (const std::exception& e) {
      LOG_ERROR("exception retrieving config data: " + std::string(e.what()))
      return false;
   } catch (...) {
      LOG_ERROR("exception retrieving config data")
      return false;
   }
   
   if (!configDataSource.haveObject()) {
      LOG_ERROR("unable to retrieve config data")
      return false;
   }

   // start out with our default settings
   m_socketSendBufferSize = CFG_DEFAULT_SEND_BUFFER_SIZE;
   m_socketReceiveBufferSize = CFG_DEFAULT_RECEIVE_BUFFER_SIZE;

   try {
      KeyValuePairs kvpServerSettings;

      // read and process "server" section
      if (configDataSource->hasSection(CFG_SECTION_SERVER) &&
          configDataSource->readSection(CFG_SECTION_SERVER, kvpServerSettings)) {
         
         if (kvpServerSettings.hasKey(CFG_SERVER_PORT)) {
            const int portNumber =
               getIntValue(kvpServerSettings, CFG_SERVER_PORT);

            if (portNumber > 0) {
               port = portNumber;
               m_serverPort = portNumber;
               
               if (isLoggingDebug) {
                  char msg[128];
                  ::snprintf(msg, 128, "port number=%d", port);
                  LOG_DEBUG(msg)
               }
            }
         }

         // defaults
         m_isThreaded = true;
         m_threading = CFG_THREADING_PTHREADS;
         m_threadPoolSize = 4;
         
         if (kvpServerSettings.hasKey(CFG_SERVER_THREADING)) {
            const std::string& threading =
               kvpServerSettings.getValue(CFG_SERVER_THREADING);
            if (!threading.empty()) {
               if ((threading == CFG_THREADING_PTHREADS) ||
                   (threading == CFG_THREADING_CPP11) ||
                   (threading == CFG_THREADING_GCD_LIBDISPATCH)) {
                  m_threading = threading;
                  m_isThreaded = true;
               } else if (threading == CFG_THREADING_NONE) {
                  m_isThreaded = false;
               }
            }
         }
         
         if (kvpServerSettings.hasKey(CFG_SERVER_THREAD_POOL_SIZE)) {
            const int poolSize =
               getIntValue(kvpServerSettings, CFG_SERVER_THREAD_POOL_SIZE);

            if (poolSize > 0) {
               m_threadPoolSize = poolSize;
            }
         }
         
         // defaults
         m_sockets = CFG_SOCKETS_SOCKET_SERVER;
         
         if (kvpServerSettings.hasKey(CFG_SERVER_SOCKETS)) {
            const std::string& sockets = kvpServerSettings.getValue(CFG_SERVER_SOCKETS);
            if (sockets == CFG_SOCKETS_KERNEL_EVENTS) {
               m_isUsingKernelEventServer = true;
               m_sockets = CFG_SOCKETS_KERNEL_EVENTS;
            }
         }

         if (kvpServerSettings.hasKey(CFG_SERVER_LOG_LEVEL)) {
            m_logLevel =
               kvpServerSettings.getValue(CFG_SERVER_LOG_LEVEL);
            if (!m_logLevel.empty()) {
               StrUtils::toLowerCase(m_logLevel);
               LOG_INFO(std::string("log level: ") + m_logLevel)
               Logger* logger = Logger::getLogger();
               
               if (logger != NULL) {
                  if (m_logLevel == CFG_LOGGING_CRITICAL) {
                     logger->setLogLevel(Critical);
                  } else if (m_logLevel == CFG_LOGGING_ERROR) {
                     logger->setLogLevel(Error);
                  } else if (m_logLevel == CFG_LOGGING_WARNING) {
                     logger->setLogLevel(Warning);
                  } else if (m_logLevel == CFG_LOGGING_INFO) {
                     logger->setLogLevel(Info);
                  } else if (m_logLevel == CFG_LOGGING_DEBUG) {
                     logger->setLogLevel(Debug);
                  } else if (m_logLevel == CFG_LOGGING_VERBOSE) {
                     logger->setLogLevel(Verbose);
                  } else {
                     LOG_WARNING("unrecognized log level: '" + m_logLevel)
                  }
               }
            }
         }

         if (kvpServerSettings.hasKey(CFG_SERVER_SEND_BUFFER_SIZE)) {
            const int buffSize =
               getIntValue(kvpServerSettings, CFG_SERVER_SEND_BUFFER_SIZE);

            if (buffSize > 0) {
               m_socketSendBufferSize = buffSize;
            }
         }

         if (kvpServerSettings.hasKey(CFG_SERVER_RECEIVE_BUFFER_SIZE)) {
            const int buffSize =
               getIntValue(kvpServerSettings, CFG_SERVER_RECEIVE_BUFFER_SIZE);

            if (buffSize > 0) {
               m_socketReceiveBufferSize = buffSize;
            }
         }
         
         if (kvpServerSettings.hasKey(CFG_SERVER_STRING)) {
            const std::string& serverString =
               kvpServerSettings.getValue(CFG_SERVER_STRING);
            if (!serverString.empty()) {
               m_serverString = serverString;

               const std::size_t posDollar = serverString.find("$");
               if (posDollar != std::string::npos) {
                  KeyValuePairs kvpVars;
                  kvpVars.addPair("$PRODUCT_NAME", m_serverName);
                  kvpVars.addPair("$PRODUCT_VERSION", m_serverVersion);
                  kvpVars.addPair("$CFG_SOCKETS", m_sockets);
                  kvpVars.addPair("$CFG_THREADING", m_threading);
                  
                  const std::size_t posDollarOS = serverString.find("$OS_");
                  
                  if (posDollarOS != std::string::npos) {
                     SystemInfo systemInfo;
                     if (systemInfo.retrievedSystemInfo()) {
                        kvpVars.addPair("$OS_SYSNAME", systemInfo.sysName());
                        kvpVars.addPair("$OS_NODENAME", systemInfo.nodeName());
                        kvpVars.addPair("$OS_RELEASE", systemInfo.release());
                        kvpVars.addPair("$OS_VERSION", systemInfo.version());
                        kvpVars.addPair("$OS_MACHINE", systemInfo.machine());
                     } else {
                        LOG_WARNING("unable to retrieve system information to populate server string")
                     }
                  }
                  
                  replaceVariables(kvpVars, m_serverString);
               }
               
               LOG_INFO("setting server string: '" + m_serverString + "'")
            }
         }
      }
      
      m_startupTime = getLocalDateTime();
   } catch (const BasicException& be) {
      LOG_CRITICAL("exception initializing server: " + be.whatString())
      return false;
   } catch (const std::exception& e) {
      LOG_CRITICAL("exception initializing server: " +
                   std::string(e.what()))
      return false;
   } catch (...) {
      LOG_CRITICAL("unknown exception initializing server")
      return false;
   }

   if (!m_isUsingKernelEventServer) {
      try {
         if (isLoggingDebug) {
            char msg[128];
            ::snprintf(msg, 128, "creating server socket on port=%d", port);
            LOG_DEBUG(msg)
         }
      
         if (m_serverSocket != NULL) {
            delete m_serverSocket;
         }
         
         m_serverSocket = new ServerSocket(port);
      } catch (...) {
         std::string exception = "unable to open server socket port '";
         exception += StrUtils::toString(port);
         exception += "'";
         LOG_CRITICAL(exception)
         return false;
      }
   }

   std::string concurrencyModel = EMPTY;

   if (m_isThreaded) {
      bool isUsingLibDispatch = false;
      
      if (m_threading == CFG_THREADING_PTHREADS) {
         m_threadingFactory = new PthreadsThreadingFactory();
      //} else if (m_threading == CFG_THREADING_CPP11) {
      //   m_threadingFactory = new StdThreadingFactory();
      //} else if (m_threading == CFG_THREADING_GCD_LIBDISPATCH) {
      //   isUsingLibDispatch = true;
      //   m_threadingFactory = new PthreadsThreadingFactory();
      } else {
         m_threadingFactory = new PthreadsThreadingFactory();
      }
      
      ThreadingFactory::setThreadingFactory(m_threadingFactory);
      
      if (m_threadPool) {
         delete m_threadPool;
      }
      
      m_threadPool =
         m_threadingFactory->createThreadPoolDispatcher(m_threadPoolSize, "threadpool");
      m_threadPool->start();

      concurrencyModel = "multithreaded - ";
      concurrencyModel += m_threading;
     
      if (isUsingLibDispatch) {
         concurrencyModel += " [libdispatch]";
      } else {
         char numberThreads[128];
         ::snprintf(numberThreads, 128, " [%d threads]",
                       m_threadPoolSize);
         concurrencyModel += numberThreads;
      }
   } else {
      concurrencyModel = "serial";
      m_threadPoolSize = 1;   // not a pool, we have 1 processing thread
   }

   m_concurrencyModel = concurrencyModel;

   std::string portAsString = StrUtils::toString(port);

   std::string startupMsg = m_serverName;
   startupMsg += " ";
   startupMsg += m_serverVersion;
   startupMsg += " listening on port ";
   startupMsg += portAsString;
   startupMsg += " (request concurrency: ";
   startupMsg += concurrencyModel;
   startupMsg += ")";
   startupMsg += " (sockets: ";
   startupMsg += m_sockets;
   startupMsg += ")";

   ::printf("%s\n", startupMsg.c_str());

   m_isFullyInitialized = true;
   
   return true;
}

//******************************************************************************

SocketServer::~SocketServer() {
   LOG_INSTANCE_DESTROY("SocketServer")

   if (m_serverSocket) {
      m_serverSocket->close();
      delete m_serverSocket;
   }
   
   if (m_kernelEventServer) {
      delete m_kernelEventServer;
   }

   if (m_threadPool) {
      m_threadPool->stop();
      delete m_threadPool;
   }
   
   if (m_threadingFactory) {
      delete m_threadingFactory;
   }
}

//******************************************************************************

std::string SocketServer::getSystemDateGMT() const {
   time_t currentGMT;
   ::time(&currentGMT);
   
   struct tm* timeptr = ::gmtime(&currentGMT);
   char dateBuffer[128];
   
   ::snprintf(dateBuffer, 128,
                 "%.3s, %02d %.3s %d %.2d:%.2d:%.2d GMT",
                 LOG_WEEKDAY_NAME[timeptr->tm_wday],
                 timeptr->tm_mday,
                 LOG_MONTH_NAME[timeptr->tm_mon],
                 1900 + timeptr->tm_year,
                 timeptr->tm_hour,
                 timeptr->tm_min,
                 timeptr->tm_sec);
   
   return std::string(dateBuffer);
}

//******************************************************************************

std::string SocketServer::getLocalDateTime() const {
   time_t currentTime;
   ::time(&currentTime);
   
   struct tm* timeptr = ::localtime(&currentTime);
   char dateBuffer[128];
   
   ::snprintf(dateBuffer, 128,
                 "%d-%02d-%02d %.2d:%.2d:%.2d",
                 1900 + timeptr->tm_year,
                 timeptr->tm_mon + 1,
                 timeptr->tm_mday,
                 timeptr->tm_hour,
                 timeptr->tm_min,
                 timeptr->tm_sec);
   
   return std::string(dateBuffer);
}

//******************************************************************************

int SocketServer::platformPointerSizeBits() const {
   return sizeof(void*) * 8;
}

//******************************************************************************

void SocketServer::serviceSocket(SocketRequest* socketRequest) {
   if (NULL != m_threadPool) {
      // Hand off the request to the thread pool for asynchronous processing
      RequestHandler* requestHandler = handlerForSocketRequest(socketRequest);
      requestHandler->setThreadPooling(true);
      requestHandler->setSocketOwned(socketRequest->isSocketOwned());
      m_threadPool->addRequest(requestHandler);
   } else {
      // no thread pool available -- process it synchronously
      RequestHandler* requestHandler = handlerForSocketRequest(socketRequest);
      requestHandler->run();
   }
}

//******************************************************************************

int SocketServer::runSocketServer() {
   if (!m_serverSocket) {
      LOG_CRITICAL("runSocketServer called with null serverSocket")
      return 1;
   }
   
   while (!m_isDone) {
      
      Socket* socket = m_serverSocket->accept();

      if (NULL == socket) {
         continue;
      }

      //if (Logger::isLogging(LogLevel::Debug)) {
      //   LOG_DEBUG("*****************************************")
      //   LOG_DEBUG("client connected")
      //}

      try {
         RequestHandler* handler = handlerForSocket(socket);
         if (m_isThreaded && (NULL != m_threadPool)) {
            handler->setThreadPooling(true);

            // give it to the thread pool
            m_threadPool->addRequest(handler);
         } else {
            handler->run();
         }
      } catch (const BasicException& be) {
         LOG_ERROR("SocketServer runServer exception caught: " +
                   be.whatString())
      } catch (const std::exception& e) {
         LOG_ERROR(std::string("SocketServer runServer exception caught: ") +
                   std::string(e.what()))
      } catch (...) {
         LOG_ERROR("SocketServer runServer unknown exception caught")
      }
      
      delete socket;
   }
   
   return 0;
}

//******************************************************************************

int SocketServer::runKernelEventServer() {
   const int MAX_CON = 1200;
   
   int rc = 0;
   
   if (m_threadingFactory != NULL) {
      Mutex* mutexFD = m_threadingFactory->createMutex("fdMutex");
      Mutex* mutexHWMConnections =
         m_threadingFactory->createMutex("hwmConnectionsMutex");
      
      if (m_kernelEventServer) {
         delete m_kernelEventServer;
         m_kernelEventServer = NULL;
      }
      
      if (KqueueServer::isSupportedPlatform()) {
         m_kernelEventServer =
            new KqueueServer(*mutexFD, *mutexHWMConnections);
      } else if (EpollServer::isSupportedPlatform()) {
         m_kernelEventServer =
            new EpollServer(*mutexFD, *mutexHWMConnections);
      } else {
         LOG_CRITICAL("no kernel event server available for platform")
         rc = 1;
      }
      
      if (m_kernelEventServer != NULL) {
         try {
            SocketServiceHandler* serviceHandler = createSocketServiceHandler();

            if (m_kernelEventServer->init(serviceHandler, m_serverPort, MAX_CON)) {
               m_kernelEventServer->run();
            } else {
               rc = 1;
            }
         } catch (const BasicException& be) {
            LOG_CRITICAL("exception running kernel event server: " +
                         be.whatString())
         } catch (const std::exception& e) {
            LOG_CRITICAL("exception running kernel event server: " +
                         std::string(e.what()))
         } catch (...) {
            LOG_CRITICAL("unidentified exception running kernel event server")
         }
      }
   } else {
      LOG_CRITICAL("no threading factory configured")
      rc = 1;
   }
   
   return rc;
}

//******************************************************************************

int SocketServer::run() {
   if (!m_isFullyInitialized) {
      LOG_CRITICAL("server not initialized")
      return 1;
   } else {
      if (m_isUsingKernelEventServer) {
         return runKernelEventServer();
      } else {
         return runSocketServer();
      }
   }
}

//******************************************************************************


