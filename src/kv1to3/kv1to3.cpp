#include <kv3lib/keyvalues3.h>
#include <tier0/icommandline.h>
#include <tier1/interface.h>
#include <tier1/utlstring.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>
#include <filesystem.h>

CreateInterfaceFn fsFactory = nullptr;

void* Factory(const char* pName, int* pReturnCode)
{
    void* ret = nullptr;

    if ((ret = fsFactory(pName, pReturnCode)) != nullptr)
    {
        return ret;
    }

    return nullptr;
}

CreateInterfaceFn RealFactory = Factory;

int main(int argc, char* argv[])
{
    CSysModule* fs = Sys_LoadModule("filesystem_stdio");
    if (fs)
    {
        fsFactory = Sys_GetFactory(fs);
    }

    ConnectTier1Libraries(&RealFactory, 1);
    ConnectTier2Libraries(&RealFactory, 1);
    ConnectTier3Libraries(&RealFactory, 1);

    g_pFullFileSystem->Connect(RealFactory);
    g_pFullFileSystem->Init();
    g_pFullFileSystem->AddSearchPath(".", "GAME");

    CommandLine()->CreateCmdLine(argc, argv);

    if (!CommandLine()->FindParm("--in_file") ||
        !CommandLine()->FindParm("--out_file") ||
        !CommandLine()->FindParm("--encoding"))
    {
        Msg("Usage: %s --in_file <file> --out_file <file> --encoding <enc>\n", argv[0]);
        return 1;
    }

    const char* szInFile = CommandLine()->ParmValue("--in_file", "");
    const char* szOutFile = CommandLine()->ParmValue("--out_file", "");
    const char* szEncodingParam = CommandLine()->ParmValue("--encoding", "text");

    const KV3ID_t* encodingId = &KV3_ENCODING_TEXT;
    if (strcmp(szEncodingParam, "binary_compressed") == 0)
    {
        encodingId = &KV3_ENCODING_BINARY_BLOCK_COMPRESSED;
    }
    else if (strcmp(szEncodingParam, "binary") == 0)
    {
        encodingId = &KV3_ENCODING_BINARY_UNCOMPRESSED;
    }
    else if (strcmp(szEncodingParam, "text") != 0)
    {
        Warning("Unknown encoding '%s' specified. Defaulting to standard text configuration.\n", szEncodingParam);
    }

    const KV3ID_t *formatId = &KV3_FORMAT_GENERIC;

    KeyValues3 kv3;
    CUtlString errorMsg;

    bool bLoadSuccess = LoadKV3FromKV1File(&kv3, &errorMsg, "GAME", szInFile);
    if (!bLoadSuccess)
    {
        Warning("KV1 Parsing Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Unknown Error Encountered.");
        return 2;
    }

    bool bSaveSuccess = SaveKV3ToFile(*encodingId, *formatId, &kv3, &errorMsg, szOutFile, "GAME");
    if (!bSaveSuccess)
    {
        Warning("KV3 Serialization Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Failed to write target bytes to disk.");
        return 3;
    }

    bool bKV3LoadSuccess = LoadKV3FromFile(&kv3, &errorMsg, szOutFile, "GAME");
    if (!bKV3LoadSuccess)
    {
        Warning("KV3 Check Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Unknown Error Encountered.");
        return 4;
    }

    Msg("%s -> %s\n", szInFile, szOutFile);
    return 0;
}
