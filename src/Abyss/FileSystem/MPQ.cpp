#include "MPQ.h"
#include "Abyss/Common/Logging.h"
#include <absl/strings/str_cat.h>
#include <algorithm>
#include <ios>
#include <ranges>

#define STORMLIB_NO_AUTO_LINK 1
#include <StormLib.h>

namespace Abyss::FileSystem {

class MPQStream final : public SizeableStreambuf {
    HANDLE _mpqFile = nullptr;
    std::streamsize _startOfBlock = 0;
    char _buffer[2048] = {};

  public:
    MPQStream(HANDLE mpq, const std::string &fileName);
    ~MPQStream() override { SFileCloseFile(_mpqFile); }
    [[nodiscard]] std::streamsize StartOfBlockForTesting() const;

  protected:
    int underflow() override;
    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override;
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override;
    [[nodiscard]] std::streamsize size() const override;
};

inline std::string fixPath(const std::string_view str) {
    std::string result(str);
    std::ranges::replace(result, '/', '\\');
    if (result.starts_with('\\')) {
        return result.substr(1);
    }

    return result;
}

MPQStream::MPQStream(HANDLE mpq, const std::string &fileName) {
    if (!SFileOpenFileEx(mpq, fileName.c_str(), SFILE_OPEN_FROM_MPQ, &_mpqFile)) {
        throw std::runtime_error(absl::StrCat("Failed to open file '", fileName, "' from MPQ"));
    }
}

std::streamsize MPQStream::StartOfBlockForTesting() const { return _startOfBlock; }

int MPQStream::underflow() {
    if (gptr() == egptr()) {
        _startOfBlock += egptr() - eback();
        DWORD amountRead;
        if (!SFileReadFile(_mpqFile, _buffer, sizeof(_buffer), &amountRead, nullptr)) {
            if (GetLastError() != ERROR_HANDLE_EOF) {
                throw std::runtime_error("Error reading file from MPQ");
            }
        }
        setg(_buffer, _buffer, _buffer + amountRead);
    }

    return gptr() == egptr() ? traits_type::eof() : traits_type::to_int_type(*gptr());
}

std::streambuf::pos_type MPQStream::seekpos(const pos_type pos, const std::ios_base::openmode which) { return seekoff(pos, std::ios_base::beg, which); }

std::streambuf::pos_type MPQStream::seekoff(const off_type off, const std::ios_base::seekdir dir, std::ios_base::openmode which) {
    std::streamsize newPos = 0;
    switch (dir) {
    case std::ios_base::beg:
        newPos = off;
        break;
    case std::ios_base::cur:
        newPos = _startOfBlock + (gptr() - eback()) + off;
        break;
    case std::ios_base::end:
        newPos = size() + off;
        break;
    default:
        break;
    }
    if (newPos >= _startOfBlock && newPos < _startOfBlock + (egptr() - eback())) {
        // The new position is already in the buffer, just repoint the pointer to it
        setg(eback(), eback() + newPos - _startOfBlock, egptr());
    } else {
        // Drop buffer, it will be read in underflow
        SFileSetFilePointer(_mpqFile, static_cast<int>(newPos), nullptr, 0);
        setg(nullptr, nullptr, nullptr);
        _startOfBlock = newPos;
    }
    return _startOfBlock + (gptr() - eback());
}

std::streamsize MPQStream::size() const { return SFileGetFileSize(_mpqFile, nullptr); }

MPQ::MPQ(const std::filesystem::path &mpqPath) : _stormMpq(nullptr) {
    std::string path = std::filesystem::absolute(mpqPath).string();
    Common::Log::debug("Opening MPQ {}", path);
    if (!SFileOpenArchive(path.c_str(), 0, STREAM_PROVIDER_FLAT | BASE_PROVIDER_FILE | STREAM_FLAG_READ_ONLY, &_stormMpq)) {
        throw std::runtime_error(absl::StrCat("Error occurred while opening MPQ ", mpqPath.string()));
    }
}

MPQ::~MPQ() { SFileCloseArchive(_stormMpq); }

bool MPQ::has(const std::string_view fileName) { return SFileHasFile(_stormMpq, fixPath(fileName).c_str()); }

InputStream MPQ::load(const std::string_view fileName) { return InputStream(std::make_unique<MPQStream>(_stormMpq, fixPath(fileName))); }

std::vector<std::string> MPQ::fileList() {
    std::vector<std::string> result;

    if (!has("(listfile)")) {
        Common::Log::error("MPQ does not contain a listfile.");
    }

    auto stream = load("(listfile)");

    std::string line;
    while (true) {
        std::getline(stream, line);
        if (stream.eof()) {
            break;
        }

        // Remove the \r character from the line
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        result.push_back(line);
    }

    return result;
}

} // namespace Abyss::FileSystem
