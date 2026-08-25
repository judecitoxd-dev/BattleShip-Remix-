#include "remix_extra_source.h"

#include <libultraship/libultraship.h>

#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>
#if !defined(_WIN32)
#include <sys/types.h>
#endif

namespace {

constexpr const char *kRelativePath = "remix_extra/SmashRemixExtra.z64";
constexpr uint64_t kExpectedSize = 80312584ULL;

std::once_flag gPathOnce;
std::string gSourcePath;
std::once_flag gFileOnce;
FILE *gSourceFile = nullptr;
std::mutex gSourceFileMutex;

void resolveSourcePath() {
    try {
        gSourcePath = Ship::Context::GetPathRelativeToAppDirectory(kRelativePath);
    } catch (...) {
        gSourcePath.clear();
    }
}

const std::string &sourcePath() {
    std::call_once(gPathOnce, resolveSourcePath);
    return gSourcePath;
}

void openSourceFile() {
    const std::string &path = sourcePath();
    if (path.empty()) return;

    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size != kExpectedSize) return;

    gSourceFile = std::fopen(path.c_str(), "rb");
}

FILE *sourceFile() {
    std::call_once(gFileOnce, openSourceFile);
    return gSourceFile;
}

bool seekSource(FILE *f, uint64_t offset) {
#if defined(_WIN32)
    return _fseeki64(f, static_cast<__int64>(offset), SEEK_SET) == 0;
#else
    return fseeko(f, static_cast<off_t>(offset), SEEK_SET) == 0;
#endif
}

} // namespace

extern "C" const char *remix_extra_source_get_path(void) {
    return sourcePath().c_str();
}

extern "C" int remix_extra_source_exists(void) {
    return sourceFile() != nullptr ? 1 : 0;
}

extern "C" size_t remix_extra_source_read(uint64_t offset, void *dst, size_t size) {
    if (dst == nullptr || size == 0) return 0;
    if (offset > kExpectedSize || size > kExpectedSize - offset) return 0;

    FILE *f = sourceFile();
    if (f == nullptr) return 0;

    // All reloc reads share one immutable ROM handle. Serializing seek+read is
    // much cheaper than reopening a ~77 MiB file several times per asset and
    // is safe if resource loading later becomes multi-threaded.
    std::lock_guard<std::mutex> lock(gSourceFileMutex);
    if (!seekSource(f, offset)) return 0;
    return std::fread(dst, 1, size, f);
}

extern "C" int remix_extra_source_read_be32(uint64_t offset, uint32_t *value_out) {
    if (value_out == nullptr) return 0;
    uint8_t b[4];
    if (remix_extra_source_read(offset, b, sizeof(b)) != sizeof(b)) return 0;
    *value_out = (static_cast<uint32_t>(b[0]) << 24) |
                 (static_cast<uint32_t>(b[1]) << 16) |
                 (static_cast<uint32_t>(b[2]) << 8) |
                  static_cast<uint32_t>(b[3]);
    return 1;
}
