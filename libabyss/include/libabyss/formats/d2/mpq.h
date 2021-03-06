#ifndef LIBABYSS_MPQ_H
#define LIBABYSS_MPQ_H

#include <filesystem>
#include <fstream>
#include <istream>
#include "libabyss/streams/inputstream.h"
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace LibAbyss {

class MPQ {
  public:
    /// Proxy constructor that creates an MPQ based on the specified filename.
    /// \param mpqPath Path to the MPQ file to load.
    explicit MPQ(const std::filesystem::path &mpqPath);

    ~MPQ();

    bool HasFile(std::string_view fileName);
    InputStream Load(std::string_view fileName);
    std::vector<std::string> FileList();

  private:
    void *_stormMpq;

    static std::string FixPath(std::string_view str);
};

} // namespace LibAbyss

#endif // LIBABYSS_MPQ_H
