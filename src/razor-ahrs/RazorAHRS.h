/******************************************************************************************
* Mac OSX / Unix / Linux C++ Interface for Razor AHRS v1.4.1
* 9 Degree of Measurement Attitude and Heading Reference System
* for Sparkfun "9DOF Razor IMU" and "9DOF Sensor Stick"
*
* Released under GNU GPL (General Public License) v3.0
* Copyright (C) 2013 Peter Bartz
* Copyright (C) 2011-2012 Quality & Usability Lab, Deutsche Telekom Laboratories, TU Berlin
* Written by Peter Bartz (peter-bartz@gmx.de)
*
* Infos, updates, bug reports and feedback:
*     https://github.com/ptrbrtz/razor-9dof-ahrs
******************************************************************************************/

#ifndef RAZORAHRS_H
#define RAZORAHRS_H

#include <thread>
#include <string>
#include <memory>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <unistd.h>  // for write(), close(), ...
#include <termios.h> // for speed_t, cfsetispeed(), ...
#include <fcntl.h>   // for open(), ...
#include <errno.h>
#include <sys/time.h>


// Razor AHRS tracker
class RazorAHRS
{
  public:
    enum Mode {
      YAW_PITCH_ROLL,
      ACC_MAG_GYR_RAW,
      ACC_MAG_GYR_CALIBRATED
    };

    typedef std::function<void(const float[])> DataCallbackFunc;
    typedef std::function<void(const std::string&)> ErrorCallbackFunc;

    RazorAHRS(const std::string &port, DataCallbackFunc data_func, ErrorCallbackFunc error_func,
        Mode mode, int connect_timeout_ms = 5000, speed_t speed = B57600);
    ~RazorAHRS();

  private:
    Mode _mode;

    // serial port helpers
    bool _open_serial_port(const char *port);
    bool _set_blocking_io();
    bool _set_nonblocking_io();
    bool _is_io_blocking();

    bool _read_token(const std::string &token, char c);
    bool _init_razor();

    // timing
    inline long elapsed_ms(struct timeval start, struct timeval end)
    {
      return static_cast<long> ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
    }

    // input buffer
    union
    {
      float ypr[3]; // yaw, pitch, roll
      float amg[9]; // 3 axes of accelerometer, magnetometer and gyroscope
    }
    _input_buf;
    size_t _input_pos;

    int _connect_timeout_ms;
    int _serial_port;

    // callbacks
    DataCallbackFunc data;
    ErrorCallbackFunc error;

    inline std::string to_str(int i)
    {
      std::stringstream ss;
      ss << i;
      return ss.str();
    }

    inline bool _big_endian()
    {
        const int num = 1;
        return (*(reinterpret_cast<const char*> (&num))) != 1;
    }

    // swap endianess of int
    inline void _swap_endianess(int &i)
    {
      i = (i >> 24) | ((i << 8) & 0x00FF0000) | ((i >> 8) & 0x0000FF00) | (i << 24);
    }

    // swap endianess of float
    inline void _swap_endianess(float &f)
    {
      float swapped;
      char *f_as_char = reinterpret_cast<char*> (&f);
      char *swapped_as_char = reinterpret_cast<char*> (&swapped);

      // swap the bytes into a temporary buffer
      swapped_as_char[0] = f_as_char[3];
      swapped_as_char[1] = f_as_char[2];
      swapped_as_char[2] = f_as_char[1];
      swapped_as_char[3] = f_as_char[0];

      f = swapped;
    }

    // swap endianess of int array
    inline void _swap_endianess(int arr[], int arr_length)
    {
      for (int i = 0; i < arr_length; i++)
        _swap_endianess(arr[i]);
    }

    // swap endianess of float array
    inline void _swap_endianess(float arr[], int arr_length)
    {
      for (int i = 0; i < arr_length; i++)
        _swap_endianess(arr[i]);
    }

    // thread related stuff
    std::thread _tracker_thread;
    std::thread::id _thread_id;

    volatile bool _stop_thread; // thread stop flag
    void _start(); ///< start the tracking thread
    void _stop();  ///< stop the tracking thread

    static void* _thread_starter(void*);
    void* _thread(void*);  // thread main function
};

#endif // RAZORAHRS_H
