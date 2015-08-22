// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdlib>
#include <ctime>
#include <string.h>

#include "DateTime.h"

using namespace chaudiere;

//******************************************************************************

static int numberLeadingZeros(const char* value, int length) {
   int leadingZeros = 0;
   for (int i=0; i < length; ++i) {
      if (value[i] != '0') {
         break;
      } else {
         ++leadingZeros;
      }
   }
   
   return leadingZeros;
}

//******************************************************************************

void DateTime::dateFromString(DateTime* date, const char* dateValue) {
   const size_t valueLength = ::strlen(dateValue);
   if ((valueLength > 18) && (date != nullptr)) {
      const char* firstDash = ::strchr(dateValue, '-');
      if (firstDash > dateValue) {
         const char* secondDash = ::strchr(firstDash+1, '-');
         if (secondDash) {
            const char* space = ::strchr(secondDash+1, ' ');
            if (space) {
               const char* firstColon = ::strchr(space+1, ':');
               if (firstColon) {
                  const char* secondColon = ::strchr(firstColon+1, ':');
                  if (secondColon) {
                     const long yearLen = firstDash - dateValue;
                     const long monthLen = secondDash - firstDash - 1;
                     const long dayLen = space - secondDash - 1;
                     const long hourLen = firstColon - space - 1;
                     const long minuteLen = secondColon - firstColon - 1;
                     const size_t secondLen = ::strlen(secondColon+1);
                     
                     if ((yearLen == 4L) &&
                         (monthLen == 2L) &&
                         (dayLen == 2L) &&
                         (hourLen == 2L) &&
                         (minuteLen == 2L) &&
                         (secondLen == 2)) {
                           
                        char yearAsString[5];
                        char monthAsString[3];
                        char dayAsString[3];
                        char hourAsString[3];
                        char minuteAsString[3];
                        char secondAsString[3];
                        
                        ::memset(yearAsString, 0, 5);
                        ::memset(monthAsString, 0, 3);
                        ::memset(dayAsString, 0, 3);
                        ::memset(hourAsString, 0, 3);
                        ::memset(minuteAsString, 0, 3);
                        ::memset(secondAsString, 0, 3);
                        
                        ::strncpy(yearAsString, dateValue, 4);
                        ::strncpy(monthAsString, firstDash+1, 2);
                        ::strncpy(dayAsString, secondDash+1, 2);
                        ::strncpy(hourAsString, space+1, 2);
                        ::strncpy(minuteAsString, firstColon+1, 2);
                        ::strncpy(secondAsString, secondColon+1, 2);
                        
                        const int leadingZeros1 = numberLeadingZeros(yearAsString, 4);
                        const int leadingZeros2 = numberLeadingZeros(monthAsString, 2);
                        const int leadingZeros3 = numberLeadingZeros(dayAsString, 2);
                        const int leadingZeros4 = numberLeadingZeros(hourAsString, 2);
                        const int leadingZeros5 = numberLeadingZeros(minuteAsString, 2);
                        const int leadingZeros6 = numberLeadingZeros(secondAsString, 2);
                        
                        if ((leadingZeros1 < 4) &&
                           (leadingZeros2 < 2) &&
                           (leadingZeros3 < 2)) {
                           date->m_year = ::atoi(yearAsString+leadingZeros1);
                           date->m_month = ::atoi(monthAsString+leadingZeros2);
                           date->m_day = ::atoi(dayAsString+leadingZeros3);
                           date->m_hour = ::atoi(hourAsString+leadingZeros4);
                           date->m_minute = ::atoi(minuteAsString+leadingZeros5);
                           date->m_second = ::atoi(secondAsString+leadingZeros6);
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

//******************************************************************************

double DateTime::unixTimeValue(const DateTime& date) {
   if (date.m_haveUnixTimeValue) {
      return date.m_timeIntervalSince1970;
   } else {
      if (date.m_year >= 1970) {
         // elements of struct tm (from man page)
         /*
         int tm_sec;     // seconds (0 - 60)
         int tm_min;     // minutes (0 - 59)
         int tm_hour;    // hours (0 - 23)
         int tm_mday;    // day of month (1 - 31)
         int tm_mon;     // month of year (0 - 11)
         int tm_year;    // year - 1900
         int tm_wday;    // day of week (Sunday = 0)
         int tm_yday;    // day of year (0 - 365)
         int tm_isdst;   // is summer time in effect?
         char *tm_zone;  // abbreviation of timezone name
         long tm_gmtoff; // offset from UTC in seconds
          */
         tm time;
         ::memset(&time, 0, sizeof(tm));
         time.tm_year = date.m_year - 1900;
         time.tm_mon = date.m_month - 1;
         time.tm_mday = date.m_day;
         time.tm_hour = date.m_hour;
         time.tm_min = date.m_minute;
         time.tm_sec = date.m_second;
         time.tm_isdst = -1;
         return ::mktime(&time);
      } else {
         return 0.0;
      }
   }
}

//******************************************************************************

bool DateTime::populateFromUnixTime(DateTime& date, double unixTime) {
   //DQ: use local time or gm time?
   const time_t epochTime = unixTime;
   struct tm timeComponents;
   ::memset(&timeComponents, 0, sizeof(timeComponents));
      
   if (::localtime_r(&epochTime, &timeComponents)) {
      date.m_year = timeComponents.tm_year + 1900;
      date.m_month = timeComponents.tm_mon + 1;
      date.m_day = timeComponents.tm_mday;
      date.m_hour = timeComponents.tm_hour;
      date.m_minute = timeComponents.tm_min;
      date.m_second = timeComponents.tm_sec;
      date.m_timeIntervalSince1970 = unixTime;
      date.m_haveUnixTimeValue = true;
         
      return true;
   } else {
      return false;
   }
}

//******************************************************************************

DateTime* DateTime::gmtDateTime() {
   time_t currentGMT;
   ::time(&currentGMT);
   
   struct tm* timeptr = ::gmtime(&currentGMT);
   if (timeptr != nullptr) {
      // caller responsible for deleting
      DateTime* dt = new DateTime(0);
      dt->m_year = timeptr->tm_year + 1900;
      dt->m_month = timeptr->tm_mon + 1;
      dt->m_day = timeptr->tm_mday;
      dt->m_hour = timeptr->tm_hour;
      dt->m_minute = timeptr->tm_min;
      dt->m_second = timeptr->tm_sec;
      dt->m_weekDay = timeptr->tm_wday;
      dt->m_timeIntervalSince1970 = 0.0;
      dt->m_haveUnixTimeValue = false;
      return dt;
   }
   
   return nullptr;
}

//******************************************************************************

DateTime::DateTime() :
   m_timeIntervalSince1970(0.0),
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_haveUnixTimeValue(false) {
   time_t t = ::time(nullptr);
   struct tm* now = ::localtime(&t);
   if (now != nullptr) {
      m_year = now->tm_year + 1900;
      m_month = now->tm_mon + 1;
      m_day = now->tm_mday;
      m_hour = now->tm_hour;
      m_minute = now->tm_min;
      m_second = now->tm_sec;
      m_weekDay = now->tm_wday;
   }
}

//******************************************************************************

DateTime::DateTime(int dummy) :
   m_timeIntervalSince1970(0.0),
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_haveUnixTimeValue(false) {
}

//******************************************************************************

DateTime::DateTime(const std::string& dateTime) :
   m_timeIntervalSince1970(0.0),
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_haveUnixTimeValue(false) {
   if (dateTime.length() == 14) {
      const std::string year = dateTime.substr(0, 4);
      const std::string month = dateTime.substr(4, 2);
      const std::string day = dateTime.substr(6, 2);
      const std::string hour = dateTime.substr(8, 2);
      const std::string minute = dateTime.substr(10, 2);
      const std::string second = dateTime.substr(12, 2);
      
      m_year = ::atoi(year.c_str());
      m_month = ::atoi(month.c_str());
      m_day = ::atoi(day.c_str());
      m_hour = ::atoi(hour.c_str());
      m_minute = ::atoi(minute.c_str());
      m_second = ::atoi(second.c_str());
   }
}

//******************************************************************************

DateTime::DateTime(int year,
           int month,
           int day,
           int hour,
           int minute,
           int second) :
   m_timeIntervalSince1970(0.0),
   m_year(year),
   m_month(month),
   m_day(day),
   m_hour(hour),
   m_minute(minute),
   m_second(second),
   m_weekDay(-1),
   m_haveUnixTimeValue(false) {
}

//******************************************************************************

DateTime::DateTime(double timeIntervalSince1970) :
   m_timeIntervalSince1970(timeIntervalSince1970),
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_haveUnixTimeValue(true) {
   populateFromUnixTime(*this, timeIntervalSince1970);
}

//*****************************************************************************

DateTime::DateTime(const DateTime& copy) :
   m_timeIntervalSince1970(copy.m_timeIntervalSince1970),
   m_year(copy.m_year),
   m_month(copy.m_month),
   m_day(copy.m_day),
   m_hour(copy.m_hour),
   m_minute(copy.m_minute),
   m_second(copy.m_second),
   m_weekDay(copy.m_weekDay),
   m_haveUnixTimeValue(copy.m_haveUnixTimeValue) {
}

//******************************************************************************
      
DateTime& DateTime::operator=(const DateTime& copy) {
   if (this == &copy) {
      return *this;
   }
   
   m_timeIntervalSince1970 = copy.m_timeIntervalSince1970;
   m_year = copy.m_year;
   m_month = copy.m_month;
   m_day = copy.m_day;
   m_hour = copy.m_hour;
   m_minute = copy.m_minute;
   m_second = copy.m_second;
   m_weekDay = copy.m_weekDay;
   m_haveUnixTimeValue = copy.m_haveUnixTimeValue;
   
   return *this;
}

//******************************************************************************
      
bool DateTime::operator==(const DateTime& compare) const {
   if (m_haveUnixTimeValue && compare.m_haveUnixTimeValue) {
      return (m_timeIntervalSince1970 == compare.m_timeIntervalSince1970);
   }

   return m_second == compare.m_second &&
          m_minute == compare.m_minute &&
          m_hour == compare.m_hour &&
          m_day == compare.m_day &&
          m_month == compare.m_month &&
          m_year == compare.m_year;
}

//******************************************************************************

bool DateTime::operator<(const DateTime& compare) const {
   if (m_haveUnixTimeValue && compare.m_haveUnixTimeValue) {
      return (m_timeIntervalSince1970 < compare.m_timeIntervalSince1970);
   }

   if (m_year < compare.m_year) {
      return true;
   } else if (m_year > compare.m_year) {
      return false;
   } else {
      // matching year value
      if (m_month < compare.m_month) {
         return true;
      } else if (m_month > compare.m_month) {
         return false;
      } else {
         // matching month value
         if (m_day < compare.m_day) {
            return true;
         } else if (m_day > compare.m_day) {
            return false;
         } else {
            // matching day value
            if (m_hour < compare.m_hour) {
               return true;
            } else if (m_hour > compare.m_hour) {
               return false;
            } else {
               // matching hour value
               if (m_minute < compare.m_minute) {
                  return true;
               } else if (m_minute > compare.m_minute) {
                  return false;
               } else {
                  // matching minute value
                  if (m_second < compare.m_second) {
                     return true;
                  } else if (m_second > compare.m_second) {
                     return false;
                  } else {
                     // matching second value
                     return false;
                  }
               }
            }
         }
      }
   }
}

//******************************************************************************
      
std::string DateTime::formattedString() const {
   //TODO: what if we only have unix time populated?
   char stringBuffer[20];
   snprintf(stringBuffer, 20, "%04d-%02d-%02d %02d:%02d:%02d",
            m_year, m_month, m_day, m_hour, m_minute, m_second);
   return std::string(stringBuffer);
}

//******************************************************************************

std::string DateTime::unformattedString() const {
   //TODO: what if we only have unix time populated?
   char stringBuffer[20];
   snprintf(stringBuffer, 20, "%04d%02d%02d%02d%02d%02d",
            m_year, m_month, m_day, m_hour, m_minute, m_second);
   return std::string(stringBuffer);
}

//******************************************************************************

double DateTime::timeIntervalSinceDate(const DateTime& compare) const {
   if (m_haveUnixTimeValue && compare.m_haveUnixTimeValue) {
      return (m_timeIntervalSince1970 - compare.m_timeIntervalSince1970);
   }
   
   double thisUnixTime;
   double compareUnixTime;
   
   if (!m_haveUnixTimeValue ) {
      thisUnixTime = unixTimeValue(*this);
   } else {
      thisUnixTime = m_timeIntervalSince1970;
   }
   
   if (!compare.m_haveUnixTimeValue ) {
      compareUnixTime = unixTimeValue(compare);
   }
   else {
      compareUnixTime = compare.m_timeIntervalSince1970;
   }
   
   return (thisUnixTime - compareUnixTime);
}

//******************************************************************************

double DateTime::timeIntervalSince1970() const {
   if (!m_haveUnixTimeValue) {
      m_timeIntervalSince1970 = unixTimeValue(*this);
      m_haveUnixTimeValue = true;
   }
   
   return m_timeIntervalSince1970;
}

//******************************************************************************

void DateTime::setYear(int year) {
   m_year = year;
}

//******************************************************************************

int DateTime::getYear() const {
   return m_year;
}

//******************************************************************************
      
void DateTime::setMonth(int month) {
   m_month = month;
}

//******************************************************************************

int DateTime::getMonth() const {
   return m_month;
}

//******************************************************************************

void DateTime::setDay(int day) {
   m_day = day;
}

//******************************************************************************

int DateTime::getDay() const {
   return m_day;
}

//******************************************************************************

void DateTime::setHour(int hour) {
   m_hour = hour;
}

//******************************************************************************

int DateTime::getHour() const {
   return m_hour;
}

//******************************************************************************

void DateTime::setMinute(int minute) {
   m_minute = minute;
}

//******************************************************************************

int DateTime::getMinute() const {
   return m_minute;
}

//******************************************************************************

void DateTime::setSecond(int second) {
   m_second = second;
}

//******************************************************************************

int DateTime::getSecond() const {
   return m_second;
}

//******************************************************************************

void DateTime::setWeekDay(int weekDay) {
   m_weekDay = weekDay;
}

//******************************************************************************

int DateTime::getWeekDay() const {
   return m_weekDay;
}

//******************************************************************************
