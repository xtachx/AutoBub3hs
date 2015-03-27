/*
  Header file to track all storage / memory
  structure definitions.
 */

#include <string>
#include <sstream>


#ifndef CDATA_HPP_INCLUDED
#define CDATA_HPP_INCLUDED

//to_string
template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

//Time class

class TimePoint{

    public:
        /*Essentials */
        int year;
        int month;
        int day;
        int hour;
        int minute;
        float sec;

        /*Instantiator routines*/
        //TimePoint (){}

        TimePoint(int y,int m,int d,int H,int M,float S) : year(y), month(m), day(d), hour(H), minute(M), sec(S){}

        TimePoint(int y,int m,int d,int H,int M,int S, float cS){
            this->year = y;
            this->month = m;
            this->day = d;
            this->hour = H;
            this->minute = M;
            this->sec = S + cS/100.0;
        }

        /*DebugRoutines*/
        std::string printTimeString(){
            return to_string(this->year)+" : "+to_string(this->month)+" : "+to_string(this->day)+" - "+to_string(this->hour)+" : "+to_string(this->minute)+" : "+to_string(this->sec);
        }

    private:



};




#endif // Storage_HPP_INCLUDED
