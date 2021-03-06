#ifndef LIBABYSS_CASCSTREAM_H
#define LIBABYSS_CASCSTREAM_H

#include "inputstream.h"
#include "libabyss/formats/d2/casc.h"
#include <ios>
#include <streambuf>
#include <vector>

namespace LibAbyss {

class CASCStream : public SizeableStreambuf {
  public:
    CASCStream(void *casc, std::string fileName);

    ~CASCStream() override;

  protected:
    int underflow() override;
    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override;
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override;
    std::streamsize size() const override;

  private:
    void *_file = 0;
    std::streamsize _startOfBlock = 0;
    char _buffer[2048] = {};
};

} // namespace LibAbyss

#endif // LIBABYSS_CASCSTREAM_H
