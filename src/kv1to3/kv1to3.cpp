#include <kv3lib/keyvalues3.h>
#include <tier0/icommandline.h>

int main(int argc, char* argv[])
{
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

    KV3ID_t encodingId = KV3_ENCODING_TEXT;
    if (strcmp(szEncodingParam, "binary_compressed") == 0)
    {
        encodingId = KV3_ENCODING_BINARY_BLOCK_COMPRESSED;
    }
    else if (strcmp(szEncodingParam, "binary") == 0)
    {
        encodingId = KV3_ENCODING_BINARY_UNCOMPRESSED;
    }
    else if (strcmp(szEncodingParam, "text") != 0)
    {
        Warning("Unknown encoding '%s' specified. Defaulting to standard text configuration.\n", szEncodingParam);
    }

    KV3ID_t formatId = KV3_FORMAT_GENERIC;

    KeyValues3 kv3;
    CUtlString errorMsg;

    bool bLoadSuccess = LoadKV3FromKV1File(&kv3, &errorMsg, ".", szInFile);
    if (!bLoadSuccess)
    {
        Warning("KV1 Parsing Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Unknown Error Encountered.");
        return 2;
    }

    bool bSaveSuccess = SaveKV3ToFile(encodingId, formatId, &kv3, &errorMsg, szOutFile, ".");
    if (!bSaveSuccess)
    {
        Warning("KV3 Serialization Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Failed to write target bytes to disk.");
        return 3;
    }

    Msg("%s -> %s\n", szInFile, szOutFile);
    return 0;
}
