// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdlib>
#include <ctime>

#include "DateTime.h"

using namespace chaudiere;

//******************************************************************************

DateTime* DateTime::gmtDateTime()
{
   time_t currentGMT;
   ::time(&currentGMT);
   
   struct tm* timeptr = ::gmtime(&currentGMT);
   if (timeptr != nullptr) {
      DateTime* dt = new DateTime(0);
      dt->m_year = timeptr->tm_year + 1900;
      dt->m_month = timeptr->tm_mon + 1;
      dt->m_day = timeptr->tm_mday;
      dt->m_hour = timeptr->tm_hour;
      dt->m_minute = timeptr->tm_min;
      dt->m_second = timeptr->tm_sec;
      dt->m_weekDay = timeptr->tm_wday;
      dt->m_initialized = true;
      return dt;
   }
   
   return nullptr;
}

//******************************************************************************

DateTime::DateTime() :
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_initialized(false)
{
   time_t t = time(nullptr);
   struct tm* now = localtime(&t);
   m_year = now->tm_year + 1900;
   m_month = now->tm_mon + 1;
   m_day = now->tm_mday;
   m_hour = now->tm_hour;
   m_minute = now->tm_min;
   m_second = now->tm_sec;
   m_weekDay = now->tm_wday;
   m_initialized = true;
}

//******************************************************************************

DateTime::DateTime(int dummy) :
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_initialized(false)
{
}

//******************************************************************************

DateTime::DateTime(const std::string& dateTime) :
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0),
   m_weekDay(-1),
   m_initialized(false)
{
   if (dateTime.length() == 14) {
      const std::string year = dateTime.substr(0, 4);
      const std::string month = dateTime.substr(4, 2);
      const std::string day = dateTime.substr(6, 2);
      const std::string hour = dateTime.substr(8, 2);
      const std::string minute = dateTime.substr(10, 2);
      const std::string second = dateTime.substr(12, 2);
      
      m_year = atoi(year.c_str());
      m_month = atoi(month.c_str());
      m_day = atoi(day.c_str());
      m_hour = atoi(hour.c_str());
      m_minute = atoi(minute.c_str());
      m_second = atoi(second.c_str());
      
      m_initialized = true;
   }
}

//******************************************************************************

DateTime::DateTime(const DateTime& copy) :
   m_year(copy.m_year),
   m_month(copy.m_month),
   m_day(copy.m_day),
   m_hour(copy.m_hour),
   m_minute(copy.m_minute),
   m_second(copy.m_second),
   m_weekDay(copy.m_weekDay),
   m_initialized(copy.m_initialized)
{
}

//******************************************************************************
      
DateTime& DateTime::operator=(const DateTime& copy)
{
   if (this == &copy) {
      return *this;
   }
   
   if (m_initialized || copy.m_initialized) {
      m_year = copy.m_year;
      m_month = copy.m_month;
      m_day = copy.m_day;
      m_hour = copy.m_hour;
      m_minute = copy.m_minute;
      m_second = copy.m_second;
      m_weekDay = copy.m_weekDay;
      m_initialized = copy.m_initialized;
   }
   
   return *this;
}

//******************************************************************************
      
bool DateTime::operator==(const DateTime& compare) const
{
   if (!m_initialized && !compare.m_initialized) {
      return true;
   }
   
   return m_second == compare.m_second &&
          m_minute == compare.m_minute &&
          m_hour == compare.m_hour &&
          m_day == compare.m_day &&
          m_month == compare.m_month &&
          m_year == compare.m_year;
}

//******************************************************************************

bool DateTime::operator<(const DateTime& compare) const
{
   if (!m_initialized && !compare.m_initialized) {
      return false;
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
      
std::string DateTime::formattedString() const
{
   char stringBuffer[20];
   snprintf(stringBuffer, 20, "%04d-%02d-%02d %02d:%02d:%02d",
            m_year, m_month, m_day, m_hour, m_minute, m_second);
   return std::string(stringBuffer);
}

//******************************************************************************

std::string DateTime::unformattedString() const
{
   char stringBuffer[20];
   snprintf(stringBuffer, 20, "%04d%02d%02d%02d%02d%02d",
            m_year, m_month, m_day, m_hour, m_minute, m_second);
   return std::string(stringBuffer);
}

//******************************************************************************

void DateTime::setYear(int year)
{
   m_year = year;
}

//******************************************************************************

int DateTime::getYear() const
{
   return m_year;
}

//******************************************************************************
      
void DateTime::setMonth(int month)
{
   m_month = month;
}

//******************************************************************************

int DateTime::getMonth() const
{
   return m_month;
}

//******************************************************************************

void DateTime::setDay(int day)
{
   m_day = day;
}

//******************************************************************************

int DateTime::getDay() const
{
   return m_day;
}

//******************************************************************************

void DateTime::setHour(int hour)
{
   m_hour = hour;
}

//******************************************************************************

int DateTime::getHour() const
{
   return m_hour;
}

//******************************************************************************

void DateTime::setMinute(int minute)
{
   m_minute = minute;
}

//******************************************************************************

int DateTime::getMinute() const
{
   return m_minute;
}

//******************************************************************************

void DateTime::setSecond(int second)
{
   m_second = second;
}

//******************************************************************************

int DateTime::getSecond() const
{
   return m_second;
}

//******************************************************************************

void DateTime::setWeekDay(int weekDay)
{
   m_weekDay = weekDay;
}

//******************************************************************************

int DateTime::getWeekDay() const
{
   return m_weekDay;
}

//******************************************************************************
