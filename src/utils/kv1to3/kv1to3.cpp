#include "kv3lib/keyvalues3.h"
#include "tier0/icommandline.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "filesystem.h"
#include "filesystem_init.h"

// Print usage
int PrintArgSummary()
{
    fflush(stderr);
    Msg(
        "Usage: kv1to3 [options]\n\n"
        "Options:\n"
        "  -i, --in-file \tinput file\n"
        "  -o, --out-file \toutput file\n"
        "  -e, --encoding \toutput encoding [default: text]\n"
    );

    return 1;
}

// Get argument
const char* GetArg(const char* psz)
{
    const char pszSimple[] = { psz[1], psz[2], '\0'};
    const char* ret = CommandLine()->ParmValue(psz);

    if (!ret)
    {
        ret = CommandLine()->ParmValue(pszSimple);
    }

    return ret;
}

int main(int argc, char** argv)
{
    // This will create cmdline and setup base paths for filesystem.
    InitCommandLineProgram(argc, argv);

    if (!GetArg("--in_file") ||
        !GetArg("--out_file"))
    {
        return PrintArgSummary();
    }

    const char* szInFile = GetArg("--in_file");
    const char* szOutFile = GetArg("--out_file");
    const char* szEncodingParam = GetArg("--encoding");
    if (!szEncodingParam) // defaults to 'text' if not specified.
    {
        szEncodingParam = "text";
    }

    const KV3ID_t* pEncodingId = &KV3_ENCODING_TEXT;
    if (strcmp(szEncodingParam, "binary_compressed") == 0)
    {
        pEncodingId = &KV3_ENCODING_BINARY_BLOCK_COMPRESSED;
    }
    else if (strcmp(szEncodingParam, "binary") == 0)
    {
        pEncodingId = &KV3_ENCODING_BINARY_UNCOMPRESSED;
    }
    else if (strcmp(szEncodingParam, "text") != 0)
    {
        Warning("Unknown encoding '%s' specified. Defaulting to standard text configuration.\n", szEncodingParam);
    }

    const KV3ID_t *pFormatId = &KV3_FORMAT_GENERIC;

    KeyValues3 kv3;
    CUtlString errorMsg;

    bool bLoadSuccess = LoadKV3FromKV1File( &kv3, &errorMsg, "LOCAL_PATH", szInFile );
    if (!bLoadSuccess)
    {
        Warning("KV1 Parsing Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Unknown Error Encountered.");
        return 2;
    }

    bool bSaveSuccess = SaveKV3ToFile(*pEncodingId, *pFormatId, &kv3, &errorMsg, szOutFile, "LOCAL_PATH");
    if (!bSaveSuccess)
    {
        Warning("KV3 Serialization Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Failed to write target bytes to disk.");
        return 3;
    }

    bool bKV3LoadSuccess = LoadKV3FromFile(&kv3, &errorMsg, szOutFile, "LOCAL_PATH");
    if (!bKV3LoadSuccess)
    {
        Warning("KV3 Check Failure: %s\n", errorMsg.Get() ? errorMsg.Get() : "Unknown Error Encountered.");
        return 4;
    }

    Msg("%s -> %s\n", szInFile, szOutFile);

    ShutdownDefaultFileSystem();

    return 0;
}
