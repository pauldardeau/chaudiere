// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <string.h>

#include "IniReader.h"
#include "StrUtils.h"
#include "KeyValuePairs.h"
#include "BasicException.h"
#include "Logger.h"
#include "CharBuffer.h"

static const std::string EOL_LF             = "\n";
static const std::string EOL_CR             = "\r";
static const std::string OPEN_BRACKET       = "[";
static const std::string CLOSE_BRACKET      = "]";
static const std::string COMMENT_IDENTIFIER = "#";

using namespace chaudiere;

//******************************************************************************

IniReader::IniReader(const std::string& iniFile) :
   m_iniFile(iniFile) {
   LOG_INSTANCE_CREATE("IniReader")

   if (!readFile()) {
      throw BasicException("unable to read configuration file: " + iniFile);
   }
}

//******************************************************************************

IniReader::~IniReader() {
   LOG_INSTANCE_DESTROY("IniReader")
}

//******************************************************************************

bool IniReader::readSection(const std::string& section,
                            KeyValuePairs& mapSectionValues) const {
   const std::string sectionId = bracketedSection(section);
   std::size_t posSection = m_fileContents.find(sectionId);

   if (posSection == std::string::npos) {
      return false;
   }

   const std::string::size_type posEndSection = posSection + sectionId.length();
   const std::string::size_type startNextSection =
      m_fileContents.find(OPEN_BRACKET, posEndSection);

   std::string sectionContents;

   // do we have another section?
   if (startNextSection != std::string::npos) {
      // yes, we have another section in the file -- read everything
      // up to the next section
      sectionContents = m_fileContents.substr(posEndSection,
                                              startNextSection - posEndSection);
   } else {
      // no, this is the last section -- read everything left in
      // the file
      sectionContents = m_fileContents.substr(posEndSection);
   }

   auto processLine = [&mapSectionValues](std::string line) {
      if (line.empty()) {
         return;
      }

      std::string::size_type posCR = line.find('\r');
      if (posCR != std::string::npos) {
         line = line.substr(0, posCR);
      }

      const std::string::size_type posEqual = line.find('=');

      if ((posEqual != std::string::npos) && (posEqual < line.length())) {
         const std::string key = StrUtils::strip(line.substr(0, posEqual));

         // if the line is not a comment
         if (!StrUtils::startsWith(key, COMMENT_IDENTIFIER)) {
            mapSectionValues.addPair(key,
                                     StrUtils::strip(line.substr(posEqual + 1)));
         }
      }
   };

   std::string::size_type posEol;
   std::string::size_type index = 0;

   // process each newline-terminated line of the section
   while ((posEol = sectionContents.find(EOL_LF, index)) != std::string::npos) {
      processLine(sectionContents.substr(index, posEol - index));
      index = posEol + 1;
   }

   // process a final line that isn't newline-terminated (e.g. the file/
   // section doesn't end with a trailing newline)
   if (index < sectionContents.length()) {
      processLine(sectionContents.substr(index));
   }

   return true;
}

//******************************************************************************

bool IniReader::getSectionKeyValue(const std::string& section,
                                   const std::string& key,
                                   std::string& value) const {
   KeyValuePairs map;

   if (!readSection(section, map)) {
      LOG_WARNING("IniReader readSection returned false")
      return false;
   }

   const std::string strippedKey = StrUtils::strip(key);

   if (!map.hasKey(strippedKey)) {
      if (Logger::isLogging(LogLevel::Debug)) {
         char msg[128];
         ::snprintf(msg, 128, "map does not contain key '%s'", key.c_str());
         LOG_DEBUG(msg)
      }
      return false;
   }

   value = map.getValue(strippedKey);

   return true;
}

//******************************************************************************

bool IniReader::hasSection(const std::string& section) const {
   const std::string sectionId = bracketedSection(section);
   return (std::string::npos != m_fileContents.find(sectionId));
}

//******************************************************************************

bool IniReader::readFile() {
   FILE* f = ::fopen(m_iniFile.c_str(), "rb");
   if (f == nullptr) {
      return false;
   }

   ::fseek(f, 0, SEEK_END);
   const long fileBytes = ::ftell(f);
   ::fseek(f, 0, SEEK_SET);

   if (fileBytes < 0L) {
      LOG_ERROR("unable to determine size of ini file")
      ::fclose(f);
      return false;
   }

   if (fileBytes == 0L) {
      // a legitimately empty ini file is valid -- nothing to parse
      ::fclose(f);
      m_fileContents.clear();
      return true;
   }

   CharBuffer fileContents;
   fileContents.ensureCapacity(fileBytes + 1);
   const size_t numObjectsRead = ::fread(fileContents.data(), fileBytes, 1, f);

   ::fclose(f);

   if (numObjectsRead < 1) {
      LOG_ERROR("reading from ini file failed")
      return false;
   }

   fileContents.nullAt(fileBytes);
   m_fileContents = fileContents.data();

   // strip out any comments -- a '#' only starts a comment when nothing
   // but whitespace precedes it on its line; a '#' embedded within a
   // value (e.g. a URL fragment) is left untouched
   bool strippingComments = true;
   size_t posCommentStart = 0;
   size_t posCR;
   size_t posLF;
   size_t posEOL;

   while (strippingComments) {
      posCommentStart = m_fileContents.find(COMMENT_IDENTIFIER, posCommentStart);
      if (std::string::npos == posCommentStart) {
         // not found
         strippingComments = false;
         continue;
      }

      size_t lineContentStart = 0;
      if (posCommentStart > 0) {
         const size_t posLineBreak = m_fileContents.find_last_of("\r\n", posCommentStart - 1);
         if (posLineBreak != std::string::npos) {
            lineContentStart = posLineBreak + 1;
         }
      }

      const std::string beforeHash =
         m_fileContents.substr(lineContentStart, posCommentStart - lineContentStart);

      if (beforeHash.find_first_not_of(" \t") != std::string::npos) {
         // there's real content before the '#' on this line -- it's
         // embedded within a value, not a comment marker
         ++posCommentStart;
         continue;
      }

      posCR = m_fileContents.find(EOL_CR, posCommentStart + 1);
      posLF = m_fileContents.find(EOL_LF, posCommentStart + 1);
      const bool haveCR = (std::string::npos != posCR);
      const bool haveLF = (std::string::npos != posLF);

      if (!haveCR && !haveLF) {
         // no end-of-line marker remaining
         // erase from start of comment to end of file
         m_fileContents = m_fileContents.substr(0, posCommentStart);
         strippingComments = false;
      } else {
         // at least one end-of-line marker was found

         // were both types found
         if (haveCR && haveLF) {
            posEOL = posCR;

            if (posLF < posEOL) {
               posEOL = posLF;
            }
         } else {
            if (haveCR) {
               // CR found
               posEOL = posCR;
            } else {
               // LF found
               posEOL = posLF;
            }
         }

         const std::string beforeComment = m_fileContents.substr(0, posCommentStart);
         const std::string afterComment = m_fileContents.substr(posEOL, std::string::npos);
         m_fileContents = beforeComment + afterComment;
         posCommentStart = beforeComment.length();
      }
   }

   return true;
}

//******************************************************************************

std::string IniReader::bracketedSection(const std::string& sectionName) const {
   return OPEN_BRACKET + StrUtils::strip(sectionName) + CLOSE_BRACKET;
}

//******************************************************************************

