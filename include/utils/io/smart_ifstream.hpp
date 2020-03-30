#ifndef UTILS_IO_SMART_IFSTREAM_HPP
#define UTILS_IO_SMART_IFSTREAM_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

namespace utils {

  namespace io {

    class smart_ifstream {
    public:
      smart_ifstream(const std::string& filename)
      : m_ifstreambuf(filename)
      {}

      template <typename T>
      smart_ifstream& operator>>(T& value) {
        if (!(m_sstreambuf >> value)) {
          char ch;
          m_sstreambuf >> ch;
          if (m_sstreambuf.eof()) {
            while (true) {
              std::string s;
              if (!std::getline(m_ifstreambuf, s)) {
                break;
              }
              s = s.substr(0, s.find_first_of('#'));
              if (!s.empty()) {
                m_sstreambuf = std::stringstream(s);
                break;
              }
            }
          }
          if (m_sstreambuf.eof() || !(m_sstreambuf >> value)) {
            is_eof = true;
          }
        }
        return *this;
      }

      explicit operator bool() const {
        return !is_eof;
      }

    private:
      std::ifstream m_ifstreambuf;
      std::stringstream m_sstreambuf;
      bool is_eof = false;
    };
  } // namespace io

} // namespace utils

#endif // UTILS_IO_SMART_IFSTREAM_HPP