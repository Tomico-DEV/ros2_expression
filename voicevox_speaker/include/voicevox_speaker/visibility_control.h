// taken from https://docs.ros.org/en/jazzy/Tutorials/Intermediate/Writing-an-Action-Server-Client/Cpp.html
// dunno if this works since this package mainly targets Ubuntu
// PRs are always open for Windows peeps

#ifndef VOICEVOX_SPEAKER_CPP__VISIBILITY_CONTROL_H_
#define VOICEVOX_SPEAKER_CPP__VISIBILITY_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define VOICEVOX_SPEAKER_CPP_EXPORT __attribute__ ((dllexport))
    #define VOICEVOX_SPEAKER_CPP_IMPORT __attribute__ ((dllimport))
  #else
    #define VOICEVOX_SPEAKER_CPP_EXPORT __declspec(dllexport)
    #define VOICEVOX_SPEAKER_CPP_IMPORT __declspec(dllimport)
  #endif
  #ifdef VOICEVOX_SPEAKER_CPP_BUILDING_DLL
    #define VOICEVOX_SPEAKER_CPP_PUBLIC VOICEVOX_SPEAKER_CPP_EXPORT
  #else
    #define VOICEVOX_SPEAKER_CPP_PUBLIC VOICEVOX_SPEAKER_CPP_IMPORT
  #endif
  #define VOICEVOX_SPEAKER_CPP_PUBLIC_TYPE VOICEVOX_SPEAKER_CPP_PUBLIC
  #define VOICEVOX_SPEAKER_CPP_LOCAL
#else
  #define VOICEVOX_SPEAKER_CPP_EXPORT __attribute__ ((visibility("default")))
  #define VOICEVOX_SPEAKER_CPP_IMPORT
  #if __GNUC__ >= 4
    #define VOICEVOX_SPEAKER_CPP_PUBLIC __attribute__ ((visibility("default")))
    #define VOICEVOX_SPEAKER_CPP_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define VOICEVOX_SPEAKER_CPP_PUBLIC
    #define VOICEVOX_SPEAKER_CPP_LOCAL
  #endif
  #define VOICEVOX_SPEAKER_CPP_PUBLIC_TYPE
#endif

#ifdef __cplusplus
}
#endif

#endif  // VOICEVOX_SPEAKER_CPP__VISIBILITY_CONTROL_H_