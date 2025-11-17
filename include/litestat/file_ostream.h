#pragma once
#include <cstdio>
#include <ostream>
#include <streambuf>
#include <vector>

namespace litestat
{
class FileStreamBuf : public std::streambuf
{
public:
    static constexpr size_t kDefaultBufferSize = 4096;

    explicit FileStreamBuf(FILE *fp, size_t buf_size = kDefaultBufferSize)
        : file_(fp), buffer_(buf_size)
    {
        // 设置 streambuf 的 put 区域
        setp(buffer_.data(), buffer_.data() + buffer_.size());
    }

    ~FileStreamBuf() override
    {
        sync();  // flush buffer
    }

protected:
    // buffer满时 flush
    int overflow(int ch) override
    {
        if (sync() == -1)
            return EOF;
        if (ch != EOF)
        {
            *pptr() = static_cast<char>(ch);
            pbump(1);
        }
        return ch;
    }

    // flush buffer
    int sync() override
    {
        size_t n = pptr() - pbase();
        if (n > 0)
        {
            if (std::fwrite(pbase(), 1, n, file_) != n)
            {
                return -1;  // write error
            }
            pbump(-static_cast<int>(n));  // reset put pointer
        }
        return 0;
    }

private:
    FILE *file_;
    std::vector<char> buffer_;
};

class FileOStream : public std::ostream
{
public:
    explicit FileOStream(FILE *fp,
                         size_t buf_size = FileStreamBuf::kDefaultBufferSize)
        : std::ostream(nullptr), buf_(fp, buf_size)
    {
        rdbuf(&buf_);
    }

private:
    FileStreamBuf buf_;
};
}  // namespace litestat
