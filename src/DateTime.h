// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_DATETIME_H
#define CHAUDIERE_DATETIME_H

#include <string>

namespace chaudiere
{
   /**
    * DateTime is a utility class for retrieving and storing date and time. Local system
    * date and time can be retrieved through the default constructor. GMT date and time
    * can be retrieved using the static gmtDateTime method.
    */
   class DateTime
   {
   private:
      int m_year;
      int m_month;
      int m_day;
      int m_hour;
      int m_minute;
      int m_second;
      int m_weekDay;
      bool m_initialized;
      
   public:
      /**
       * Retrieves current date and time as GMT
       * @return DateTime instance with GMT date time
       */
      static DateTime* gmtDateTime();
      
      /**
       * Default constructor. Retrieves local (system) date and time.
       */
      DateTime();
      
      /**
       * Constructs a DateTime instance without retrieving date/time
       * from system. This is useful when the date/time values are
       * already available from another source.
       * @param dummy ignored
       */
      DateTime(int dummy);
      
      /**
       * Constructs a DateTime instance by parsing a string that
       * has the unformatted date/time (yyyymmddhhmmss).
       * @param dateTime the unformatted date/time as a string
       */
      DateTime(const std::string& dateTime);
      
      /**
       * Copy constructor
       * @param copy the source of the copy
       */
      DateTime(const DateTime& copy);
      
      /**
       * Destructor
       */
      ~DateTime() {}
      
      /**
       * Copy operator
       * @param copy the source of the copy
       * @return reference to the destination of the copy
       */
      DateTime& operator=(const DateTime& copy);
      
      /**
       * Equality operator
       * @param compare the DateTime instance to test for equality
       * @return boolean indicating whether the 2 objects are equal
       */
      bool operator==(const DateTime& compare) const;
      
      /**
       * Less-than operator
       * @param compare the DateTime instance to determine if current object is smaller than
       * @return boolean indicating if current object is smaller than argument object
       */
      bool operator<(const DateTime& compare) const;
      
      /**
       * Retrieves the date/time as a formatted string (yyyy-mm-dd hh:mm:ss)
       * @return the date/time as a formatted string
       */
      std::string formattedString() const;
      
      /**
       * Retrieves the date/time as an unformatted string (yyyymmddhhmmss)
       * @return the date/time as an unformatted string
       */
      std::string unformattedString() const;
      
      /**
       * Sets the year value
       * @param year the new year value
       */
      void setYear(int year);
      
      /**
       * Retrieves the year value
       * @return year value
       */
      int getYear() const;

     /**
      * Sets the month value
      * @param month the new month value
      */
      void setMonth(int month);
      
     /**
      * Retrieves the month value
      * @return month value
      */
      int getMonth() const;

     /**
      * Sets the day value
      * @param day the new day value
      */
      void setDay(int day);
      
     /**
      * Retrieves the day value
      * @return day value
      */
      int getDay() const;

     /**
      * Sets the hour value
      * @param hour the new hour value
      */
      void setHour(int hour);
      
     /**
      * Retrieves the hour value
      * @return hour value
      */
      int getHour() const;

     /**
      * Sets the minute value
      * @param minute the new minute value
      */
      void setMinute(int minute);
      
     /**
      * Retrieves the minute value
      * @return minute value
      */
      int getMinute() const;

     /**
      * Sets the second value
      * @param second the new second value
      */
      void setSecond(int second);
      
     /**
      * Retrieves the second value
      * @return second value
      */
      int getSecond() const;

     /**
      * Sets the weekday value
      * @param weekDay the new weekday value
      */
      void setWeekDay(int weekDay);
      
     /**
      * Retrieves the weekday value
      * @return weekday value
      */
      int getWeekDay() const;
   };
}

#endif
