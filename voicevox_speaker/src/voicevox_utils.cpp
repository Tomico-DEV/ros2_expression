#include "voicevox_speaker/voicevox_utils.hpp"

namespace voicevox
{

void throw_if_err(VoicevoxResultCode result)
{
    if (result != VOICEVOX_RESULT_OK)
        throw std::runtime_error(voicevox_error_result_to_message(result));
}

}