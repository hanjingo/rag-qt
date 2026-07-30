#ifndef CRASH_H
#define CRASH_H

#include <QDebug>
#include <QMessageBox>

#include <hj/testing/crash.hpp>
#include <hj/net/http/http_client.hpp>

static const char *sentryKey = "94a9e7aba6c44ed1955167682d585bc0";
static void        uploadMinidump(const std::string &dmpPath,
                                  const std::string &sentryKey)
{
    hj::crash_handler::print("uploadMinidump called, dmpPath: ");
    hj::crash_handler::print(dmpPath.c_str());
    std::ifstream file(dmpPath, std::ios::binary);
    if(!file.is_open())
    {
        hj::crash_handler::print("Cannot open dump file: ");
        hj::crash_handler::print(dmpPath.c_str());
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();
    hj::crash_handler::print("File size: ");
    hj::crash_handler::print(std::to_string(fileContent.size()).c_str());
    if(fileContent.empty())
    {
        hj::crash_handler::print("File content is empty!");
        return;
    }

    std::string filename = dmpPath;
    size_t      pos      = filename.find_last_of("\\/");
    if(pos != std::string::npos)
    {
        filename = filename.substr(pos + 1);
    }
    hj::crash_handler::print("filename: ");
    hj::crash_handler::print(filename.c_str());

    std::string boundary = "----WebKitFormBoundary" + std::to_string(rand())
                           + std::to_string(rand());
    std::string body;

    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"upload_file_minidump\"; "
            "filename=\""
            + filename + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += fileContent;
    body += "\r\n";
    body += "--" + boundary + "--\r\n";
    hj::crash_handler::print("Uploading to Sentry...");
    try
    {
        std::string     host = "www.hango.fun";
        int             port = 9000;
        std::string     path = "/api/2/minidump/?sentry_key=" + sentryKey;
        httplib::Client client(host, port);
        client.set_connection_timeout(10);
        client.set_read_timeout(10);

        auto res = client.Post(path,
                               body,
                               "multipart/form-data; boundary=" + boundary);

        if(res && res->status == 200)
        {
            hj::crash_handler::print("Upload successful! Event ID: ");
            hj::crash_handler::print(res->body.c_str());
        } else if(res)
        {
            hj::crash_handler::print("Upload failed. HTTP status: ");
            hj::crash_handler::print(std::to_string(res->status).c_str());
            hj::crash_handler::print("Response: ");
            hj::crash_handler::print(res->body.c_str());
        } else
        {
            hj::crash_handler::print(
                "Upload failed. Cannot connect to server.");
        }
    }
    catch(const std::exception &e)
    {
        hj::crash_handler::print("Upload exception: ");
        hj::crash_handler::print(e.what());
    }
}

#if defined(_WIN32)
static bool crashCallback(const wchar_t      *dump_path,
                          const wchar_t      *minidump_id,
                          void               *context,
                          EXCEPTION_POINTERS *exinfo,
                          MDRawAssertionInfo *assertion,
                          bool                succeeded)
{
    hj::crash_handler::print("crashCallback CALLED");

    std::string dumpPathStr;
    std::string minidumpIdStr;
    if(dump_path)
    {
        int len =
            WideCharToMultiByte(CP_UTF8, 0, dump_path, -1, NULL, 0, NULL, NULL);
        if(len > 0)
        {
            std::vector<char> buffer(len);
            WideCharToMultiByte(CP_UTF8,
                                0,
                                dump_path,
                                -1,
                                buffer.data(),
                                len,
                                NULL,
                                NULL);
            dumpPathStr = buffer.data();
        }
    }
    if(minidump_id)
    {
        int len = WideCharToMultiByte(CP_UTF8,
                                      0,
                                      minidump_id,
                                      -1,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL);
        if(len > 0)
        {
            std::vector<char> buffer(len);
            WideCharToMultiByte(CP_UTF8,
                                0,
                                minidump_id,
                                -1,
                                buffer.data(),
                                len,
                                NULL,
                                NULL);
            minidumpIdStr = buffer.data();
        }
    }

    hj::crash_handler::print("dump_path: ");
    hj::crash_handler::print(dumpPathStr.c_str());
    hj::crash_handler::print("minidump_id: ");
    hj::crash_handler::print(minidumpIdStr.c_str());
    if(!succeeded)
    {
        hj::crash_handler::print("Crash dump failed to write.");
        return false;
    }
    hj::crash_handler::print("succeed");

    std::wstring fullPath =
        std::wstring(dump_path) + L"\\" + std::wstring(minidump_id) + L".dmp";
    std::string dmpPath;
    int         len = WideCharToMultiByte(CP_UTF8,
                                          0,
                                          fullPath.c_str(),
                                          -1,
                                          NULL,
                                          0,
                                          NULL,
                                          NULL);
    if(len > 0)
    {
        std::vector<char> buffer(len);
        WideCharToMultiByte(CP_UTF8,
                            0,
                            fullPath.c_str(),
                            -1,
                            buffer.data(),
                            len,
                            NULL,
                            NULL);
        dmpPath = buffer.data();
    }

    hj::crash_handler::print("Full path: ");
    hj::crash_handler::print(dmpPath.c_str());

    uploadMinidump(dmpPath, sentryKey);
    return true;
};

#elif __APPLE__
static bool crashCallback(const char *dump_path,
                          const char *minidump_id,
                          void       *context,
                          bool        succeeded)
{
    hj::crash_handler::print("crashCallback CALLED");

    std::string dumpPathStr   = dump_path ? dump_path : "";
    std::string minidumpIdStr = minidump_id ? minidump_id : "";

    hj::crash_handler::print("dump_path: ");
    hj::crash_handler::print(dumpPathStr.c_str());
    hj::crash_handler::print("minidump_id: ");
    hj::crash_handler::print(minidumpIdStr.c_str());
    if(!succeeded)
    {
        hj::crash_handler::print("Crash dump failed to write.");
        return false;
    }
    hj::crash_handler::print("succeed");

    std::string dmpPath = dumpPathStr + "/" + minidumpIdStr + ".dmp";

    hj::crash_handler::print("Full path: ");
    hj::crash_handler::print(dmpPath.c_str());

    uploadMinidump(dmpPath, sentryKey);
    return true;
};
#else
static bool crashCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                          void                                      *context,
                          bool                                       succeeded)
{
    hj::crash_handler::print("crashCallback CALLED");
    if(!succeeded)
    {
        hj::crash_handler::print("Crash dump failed to write.");
        return false;
    }
    std::string dmpPath = descriptor.path();
    hj::crash_handler::print("Full path: ");
    hj::crash_handler::print(dmpPath.c_str());

    uploadMinidump(dmpPath, sentryKey);
    return true;
}
#endif

#endif // CRASH_H