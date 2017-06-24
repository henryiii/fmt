/*
 Formatting library for C++ - std::ostream support

 Copyright (c) 2012 - 2016, Victor Zverovich
 All rights reserved.

 For the license information refer to format.h.
 */

#ifndef FMT_OSTREAM_H_
#define FMT_OSTREAM_H_

#include "fmt/format.h"
#include <ostream>

namespace fmt {

namespace internal {

template <class Char>
class FormatBuf : public std::basic_streambuf<Char> {
 private:
  typedef typename std::basic_streambuf<Char>::int_type int_type;
  typedef typename std::basic_streambuf<Char>::traits_type traits_type;

  basic_buffer<Char> &buffer_;
  Char *start_;

 public:
  FormatBuf(basic_buffer<Char> &buffer) : buffer_(buffer), start_(&buffer[0]) {
    this->setp(start_, start_ + buffer_.capacity());
  }

  int_type overflow(int_type ch = traits_type::eof()) {
    if (!traits_type::eq_int_type(ch, traits_type::eof())) {
      size_t buf_size = size();
      buffer_.resize(buf_size);
      buffer_.reserve(buf_size * 2);

      start_ = &buffer_[0];
      start_[buf_size] = traits_type::to_char_type(ch);
      this->setp(start_+ buf_size + 1, start_ + buf_size * 2);
    }
    return ch;
  }

  size_t size() const {
    return to_unsigned(this->pptr() - start_);
  }
};

yes &convert(std::ostream &);

struct DummyStream : std::ostream {
  DummyStream();  // Suppress a bogus warning in MSVC.
  // Hide all operator<< overloads from std::ostream.
  void operator<<(null<>);
};

no &operator<<(std::ostream &, int);

template<typename T>
struct convert_to_int_impl<T, true> {
  // Convert to int only if T doesn't have an overloaded operator<<.
  enum {
    value = sizeof(convert(get<DummyStream>() << get<T>())) == sizeof(no)
  };
};

// Write the content of buf to os.
void write(std::ostream &os, buffer &buf);

template <typename Char, typename T>
void format_value(basic_buffer<Char> &buffer, const T &value) {
  internal::FormatBuf<Char> format_buf(buffer);
  std::basic_ostream<Char> output(&format_buf);
  output << value;
  buffer.resize(format_buf.size());
}
}  // namespace internal

// Formats a value.
template <typename T, typename Char>
auto parse_format(basic_context<Char> &ctx)
    -> std::function<void(basic_buffer<Char> &, const T &)> {
  return [&ctx](basic_buffer<Char> &buf, const T &value) {
    basic_memory_buffer<Char> buffer;
    internal::format_value(buffer, value);
    basic_string_view<Char> str(buffer.data(), buffer.size());
    do_format_arg< arg_formatter<Char> >(
          buf, internal::make_arg< basic_context<Char> >(str), ctx);
  };
}

FMT_API void vprint(std::ostream &os, cstring_view format_str, args args);

/**
  \rst
  Prints formatted data to the stream *os*.

  **Example**::

    print(cerr, "Don't {}!", "panic");
  \endrst
 */
template <typename... Args>
inline void print(std::ostream &os, cstring_view format_str,
                  const Args & ... args) {
  vprint(os, format_str, make_args(args...));
}
}  // namespace fmt

#ifdef FMT_HEADER_ONLY
# include "ostream.cc"
#endif

#endif  // FMT_OSTREAM_H_
